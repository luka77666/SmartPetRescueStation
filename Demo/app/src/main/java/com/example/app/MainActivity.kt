package com.example.app

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Pets
import androidx.compose.material.icons.filled.Schedule
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import androidx.compose.runtime.collectAsState
import androidx.compose.foundation.BorderStroke
import androidx.lifecycle.lifecycleScope
import androidx.navigation.NavDestination.Companion.hierarchy
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.example.app.ui.theme.AppTheme
import com.example.app.screens.HomeScreen
import com.example.app.screens.FeedScreen
import com.example.app.screens.ScheduleScreen
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.PrintWriter
import java.net.InetSocketAddress
import java.net.Socket
import java.net.SocketTimeoutException
import java.text.SimpleDateFormat
import java.util.*
import java.util.regex.Pattern

// ===================== 数据类 =====================

data class ScheduleItem(
    val hour: Int = 7,
    val minute: Int = 0,
    val feedWeight: Int = 50,
    val enabled: Boolean = true
)

data class DeviceData(
    val temperature: String = "--\u00B0C",
    val humidity: String = "--%",
    val weight: String = "-- g",
    val waterLevel: String = "--%",
    val isConnected: Boolean = false,
    val isReconnecting: Boolean = false,
    val logs: List<String> = listOf(createTimestamp() + " 系统初始化"),
    val isAutoMode: Boolean = false,
    val isFeeding: Boolean = false,
    val isWaterPumpOn: Boolean = false,
    val feedAmount: Int = 200,
    val waterThreshold: Int = 50,
    val weightThreshold: Int = 200,
    val schedule1: ScheduleItem = ScheduleItem(7, 0, 50, true),
    val schedule2: ScheduleItem = ScheduleItem(12, 30, 100, true),
    val schedule3: ScheduleItem = ScheduleItem(19, 0, 150, true),
)

// ===================== TCP 客户端 =====================

private fun parseFeedWeight(raw: Int?, default: Int): Int {
    if (raw == null || raw == 0 || raw > 9999 || raw == 65535 || raw == 0xFFFF) return default
    return raw
}

class TcpClient(private val scope: kotlinx.coroutines.CoroutineScope) {
    private var socket: Socket? = null
    private var reader: BufferedReader? = null
    private var writer: PrintWriter? = null
    private var receiveJob: Job? = null
    private var reconnectJob: Job? = null

    private var lastValidTemp: Int = 0
    private var lastValidHumi: Int = 0
    private var lastValidWeight: Int = 0
    private var lastValidWater: Int = 0

    private val _deviceData = MutableStateFlow(DeviceData())
    val deviceData: StateFlow<DeviceData> = _deviceData.asStateFlow()

    // AP模式默认IP (ESP8266 AP模式固定IP)
    private var host = "192.168.4.1"
    private val port = 5000

    fun setHost(newHost: String) {
        host = newHost
    }

    /**
     * 正则匹配完整数据帧，安全提取各字段。
     * 格式: T25H60W0200L080M1F0P0WT050FT0200T10700F10050E1T21230F20100E1T31900F30150E1
     */
    private val dataPattern = Pattern.compile(
        "^T(\\d+)H(\\d+)W(\\d+)L(\\d+)M(\\d)F(\\d)P(\\d)WT(\\d+)FT(\\d+)" +
        "T1(\\d{2})(\\d{2})F1(\\d+)E(\\d)" +
        "T2(\\d{2})(\\d{2})F2(\\d+)E(\\d)" +
        "T3(\\d{2})(\\d{2})F3(\\d+)E(\\d).*"
    )

    suspend fun connect(): Boolean = withContext(kotlinx.coroutines.Dispatchers.IO) {
        try {
            _deviceData.update { it.copy(isReconnecting = false) }
            addLog("正在连接 $host:$port ...")
            socket = Socket()
            socket!!.connect(InetSocketAddress(host, port), 5000)
            socket!!.soTimeout = 10000 // 10秒读取超时，防止TCP假死
            reader = BufferedReader(InputStreamReader(socket!!.getInputStream()))
            writer = PrintWriter(socket!!.getOutputStream(), true)
            _deviceData.update { it.copy(isConnected = true) }
            addLog("连接成功")
            startReceiving()
            // 连接成功后同步时间到单片机
            syncTimeToDevice()
            true
        } catch (e: Exception) {
            addLog("连接失败: ${e.message}")
            Log.e("TcpClient", "connect error", e)
            _deviceData.update { it.copy(isConnected = false) }
            startAutoReconnect()
            false
        }
    }

    suspend fun disconnect() = withContext(kotlinx.coroutines.Dispatchers.IO) {
        reconnectJob?.cancel()
        reconnectJob = null
        try {
            receiveJob?.cancel()
            writer?.close()
            reader?.close()
            socket?.close()
            addLog("已断开连接")
        } catch (e: Exception) {
            addLog("断开异常: ${e.message}")
        } finally {
            socket = null
            reader = null
            writer = null
            _deviceData.value = _deviceData.value.copy(
                isConnected = false,
                isReconnecting = false,
                temperature = "--\u00B0C",
                humidity = "--%",
                weight = "-- g",
                waterLevel = "--%",
                isFeeding = false,
                isWaterPumpOn = false
            )
        }
    }

    private fun startAutoReconnect() {
        reconnectJob?.cancel()
        reconnectJob = scope.launch(kotlinx.coroutines.Dispatchers.IO) {
            _deviceData.update { it.copy(isReconnecting = true) }
            var delayMs = 1000L
            while (isActive) {
                addLog("将在 ${delayMs / 1000}s 后重连...")
                delay(delayMs)
                try {
                    socket = Socket()
                    socket!!.connect(InetSocketAddress(host, port), 5000)
                    socket!!.soTimeout = 10000
                    reader = BufferedReader(InputStreamReader(socket!!.getInputStream()))
                    writer = PrintWriter(socket!!.getOutputStream(), true)
                    _deviceData.update { it.copy(isConnected = true, isReconnecting = false) }
                    addLog("重连成功")
                    startReceiving()
                    syncTimeToDevice()
                    return@launch
                } catch (e: Exception) {
                    addLog("重连失败: ${e.message}")
                    delayMs = (delayMs * 2).coerceAtMost(30000L)
                }
            }
        }
    }

    suspend fun sendCommand(command: String) = withContext(kotlinx.coroutines.Dispatchers.IO) {
        try {
            if (socket?.isConnected == true) {
                writer?.println(command)
                addLog("发送: $command")
            } else {
                addLog("发送失败: 未连接")
            }
        } catch (e: Exception) {
            addLog("发送异常: ${e.message}")
        }
    }

    fun updateFeedSettings(autoMode: Boolean, feedAmount: Int, waterThreshold: Int) {
        _deviceData.value = _deviceData.value.copy(
            isAutoMode = autoMode,
            feedAmount = feedAmount,
            waterThreshold = waterThreshold
        )
    }

    fun applyWaterThreshold(value: Int) {
        val cmd = String.format("SW%03d", value)
        scope.launch(kotlinx.coroutines.Dispatchers.IO) { sendCommand(cmd) }
    }

    fun applyWeightThreshold(value: Int) {
        val cmd = String.format("SF%04d", value)
        scope.launch(kotlinx.coroutines.Dispatchers.IO) { sendCommand(cmd) }
    }

    fun applySchedule(index: Int, hour: Int, minute: Int) {
        val cmd = String.format("ST%d%02d%02d", index, hour, minute)
        scope.launch(kotlinx.coroutines.Dispatchers.IO) {
            sendCommand(cmd)
            delay(100)
        }
    }

    fun applyScheduleFeedWeight(index: Int, weight: Int) {
        val cmd = String.format("SD%d%04d", index, weight)
        scope.launch(kotlinx.coroutines.Dispatchers.IO) {
            sendCommand(cmd)
            delay(100)
        }
    }

    fun applyScheduleEnabled(index: Int, enabled: Boolean) {
        val cmd = String.format("SE%d%d", index, if (enabled) 1 else 0)
        scope.launch(kotlinx.coroutines.Dispatchers.IO) {
            sendCommand(cmd)
            delay(100)
        }
    }

    /**
     * 同步手机时间到单片机：格式 TIMEHHMMSS 如 TIME213045 表示21:30:45
     */
    fun syncTimeToDevice() {
        val cal = Calendar.getInstance()
        val h = cal.get(Calendar.HOUR_OF_DAY)
        val m = cal.get(Calendar.MINUTE)
        val s = cal.get(Calendar.SECOND)
        val cmd = String.format("TIME%02d%02d%02d", h, m, s)
        scope.launch(kotlinx.coroutines.Dispatchers.IO) {
            delay(200)
            sendCommand(cmd)
        }
    }

    fun setFeedingState(feeding: Boolean) {
        _deviceData.value = _deviceData.value.copy(isFeeding = feeding)
    }

    fun setWaterPumpState(on: Boolean) {
        _deviceData.value = _deviceData.value.copy(isWaterPumpOn = on)
    }

    private fun startReceiving() {
        receiveJob?.cancel()
        receiveJob = scope.launch(kotlinx.coroutines.Dispatchers.IO) {
            try {
                while (isActive && socket?.isConnected == true) {
                    try {
                        val line = reader?.readLine()
                        if (line == null) {
                            addLog("对端关闭")
                            break
                        }
                        val data = line.trim()
                        if (data.startsWith("T")) {
                            parseData(data)
                        }
                    } catch (e: SocketTimeoutException) {
                        // 读取超时是正常的，不中断接收循环，继续等待下一条数据
                        Log.d("TcpClient", "read timeout, continue waiting...")
                    }
                }
            } catch (e: Exception) {
                if (isActive) {
                    val msg = if (e is SocketTimeoutException) "读取超时" else "接收异常: ${e.message}"
                    addLog(msg)
                }
            } finally {
                if (socket?.isConnected != true && isActive) {
                    _deviceData.update { it.copy(isConnected = false) }
                    startAutoReconnect()
                }
            }
        }
    }

    /**
     * 使用正则表达式安全解析数据帧，彻底消除 substring 索引越界风险。
     * 对于正则不匹配的残缺数据帧，保留上一次的有效值（fallback）。
     */
    private fun parseData(data: String) {
        try {
            val matcher = dataPattern.matcher(data)
            if (!matcher.matches()) {
                // 数据帧不完整或格式异常，跳过不崩溃
                Log.w("TcpClient", "Frame incomplete, skipping: $data")
                return
            }

            val cur = _deviceData.value

            // 传感器基本数据
            val rawTemp = matcher.group(1)?.toIntOrNull() ?: lastValidTemp
            val rawHumi = matcher.group(2)?.toIntOrNull() ?: lastValidHumi
            val rawWeight = matcher.group(3)?.toIntOrNull() ?: lastValidWeight
            val rawWater = matcher.group(4)?.toIntOrNull() ?: lastValidWater

            val temp = if (rawTemp in -40..80) { lastValidTemp = rawTemp; rawTemp } else lastValidTemp
            val humi = if (rawHumi in 0..100) { lastValidHumi = rawHumi; rawHumi } else lastValidHumi
            val weight = if (rawWeight in 0..5000) { lastValidWeight = rawWeight; rawWeight } else lastValidWeight
            val water = if (rawWater in 0..100) { lastValidWater = rawWater; rawWater } else lastValidWater

            // 状态标志
            val isAuto = matcher.group(5)?.toIntOrNull() == 1
            val isFeeding = matcher.group(6)?.toIntOrNull() == 1
            val isPump = matcher.group(7)?.toIntOrNull() == 1

            // 阈值
            val waterThresh = matcher.group(8)?.toIntOrNull() ?: cur.waterThreshold
            val weightThresh = matcher.group(9)?.toIntOrNull() ?: cur.weightThreshold

            // 定时1
            val s1h = matcher.group(10)?.toIntOrNull() ?: cur.schedule1.hour
            val s1m = matcher.group(11)?.toIntOrNull() ?: cur.schedule1.minute
            val s1f = parseFeedWeight(matcher.group(12)?.toIntOrNull(), cur.schedule1.feedWeight)
            val s1e = matcher.group(13)?.toIntOrNull() == 1

            // 定时2
            val s2h = matcher.group(14)?.toIntOrNull() ?: cur.schedule2.hour
            val s2m = matcher.group(15)?.toIntOrNull() ?: cur.schedule2.minute
            val s2f = parseFeedWeight(matcher.group(16)?.toIntOrNull(), cur.schedule2.feedWeight)
            val s2e = matcher.group(17)?.toIntOrNull() == 1

            // 定时3
            val s3h = matcher.group(18)?.toIntOrNull() ?: cur.schedule3.hour
            val s3m = matcher.group(19)?.toIntOrNull() ?: cur.schedule3.minute
            val s3f = parseFeedWeight(matcher.group(20)?.toIntOrNull(), cur.schedule3.feedWeight)
            val s3e = matcher.group(21)?.toIntOrNull() == 1

            _deviceData.value = cur.copy(
                temperature = "$temp\u00B0C",
                humidity = "$humi%",
                weight = "$weight g",
                waterLevel = "$water%",
                isAutoMode = isAuto,
                isFeeding = isFeeding,
                isWaterPumpOn = isPump,
                waterThreshold = waterThresh,
                weightThreshold = weightThresh,
                schedule1 = ScheduleItem(s1h, s1m, s1f, s1e),
                schedule2 = ScheduleItem(s2h, s2m, s2f, s2e),
                schedule3 = ScheduleItem(s3h, s3m, s3f, s3e)
            )
        } catch (e: Exception) {
            Log.w("TcpClient", "parseData unexpected error", e)
        }
    }

    private fun addLog(msg: String) {
        val ts = createTimestamp()
        val cur = _deviceData.value
        val newLogs = cur.logs.toMutableList()
        newLogs.add("$ts $msg")
        if (newLogs.size > 100) newLogs.removeAt(0)
        _deviceData.value = cur.copy(logs = newLogs)
    }
}

// ===================== Activity =====================

class MainActivity : ComponentActivity() {
    private lateinit var tcpClient: TcpClient

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        tcpClient = TcpClient(lifecycleScope)
        setContent {
            AppTheme {
                val deviceData by tcpClient.deviceData.collectAsState()
                AppScaffold(
                    deviceData = deviceData,
                    onConnect = { lifecycleScope.launch { tcpClient.connect() } },
                    onDisconnect = { lifecycleScope.launch { tcpClient.disconnect() } },
                    onSendCommand = { cmd -> lifecycleScope.launch { tcpClient.sendCommand(cmd) } },
                    onUpdateSettings = { auto, amount, water -> tcpClient.updateFeedSettings(auto, amount, water) },
                    onSetFeeding = { feeding -> tcpClient.setFeedingState(feeding) },
                    onSetWaterPump = { on -> tcpClient.setWaterPumpState(on) },
                    onApplyWaterThreshold = { value -> tcpClient.applyWaterThreshold(value) },
                    onApplyWeightThreshold = { value -> tcpClient.applyWeightThreshold(value) },
                    onApplySchedule = { index, hour, minute -> tcpClient.applySchedule(index, hour, minute) },
                    onApplyFeedWeight = { index, weight -> tcpClient.applyScheduleFeedWeight(index, weight) },
                    onApplyEnabled = { index, enabled -> tcpClient.applyScheduleEnabled(index, enabled) }
                )
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        lifecycleScope.launch { tcpClient.disconnect() }
    }
}

// ===================== 导航 =====================

sealed class Screen(val route: String, val title: String, val icon: ImageVector) {
    object Home : Screen("home", "\u9996\u9875", Icons.Default.Home)
    object Feed : Screen("feed", "\u6551\u52A9", Icons.Default.Pets)
    object Schedule : Screen("schedule", "\u5B9A\u65F6", Icons.Default.Schedule)
}

val screens = listOf(Screen.Home, Screen.Feed, Screen.Schedule)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppScaffold(
    deviceData: DeviceData,
    onConnect: () -> Unit,
    onDisconnect: () -> Unit,
    onSendCommand: (String) -> Unit,
    onUpdateSettings: (Boolean, Int, Int) -> Unit,
    onSetFeeding: (Boolean) -> Unit,
    onSetWaterPump: (Boolean) -> Unit,
    onApplyWaterThreshold: (Int) -> Unit,
    onApplyWeightThreshold: (Int) -> Unit,
    onApplySchedule: (Int, Int, Int) -> Unit,
    onApplyFeedWeight: (Int, Int) -> Unit,
    onApplyEnabled: (Int, Boolean) -> Unit,
) {
    val navController = rememberNavController()
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    val currentDestination = navBackStackEntry?.destination

    // 废弃原有的渐变背景，改用高级蓝灰色实底
    val backgroundColor = Color(0xFFF4F7FB)
    val primaryColor = Color(0xFF4361EE) // GeekBlue

    Scaffold(
        bottomBar = {
            NavigationBar(
                containerColor = Color.White,
                tonalElevation = 8.dp
            ) {
                screens.forEach { screen ->
                    val selected = currentDestination?.hierarchy?.any { it.route == screen.route } == true
                    NavigationBarItem(
                        icon = {
                            Icon(
                                screen.icon,
                                contentDescription = screen.title,
                                tint = if (selected) primaryColor else Color(0xFF9E9E9E)
                            )
                        },
                        label = {
                            Text(
                                screen.title,
                                fontSize = 12.sp,
                                color = if (selected) primaryColor else Color(0xFF9E9E9E),
                                fontWeight = if (selected) FontWeight.Bold else FontWeight.Normal
                            )
                        },
                        selected = selected,
                        onClick = {
                            navController.navigate(screen.route) {
                                popUpTo(navController.graph.findStartDestination().id) { saveState = true }
                                launchSingleTop = true
                                restoreState = true
                            }
                        },
                        colors = NavigationBarItemDefaults.colors(
                            indicatorColor = primaryColor.copy(alpha = 0.1f) // 选中底色变柔和
                        )
                    )
                }
            }
        },
        containerColor = backgroundColor // 应用全新背景色
    ) { paddingValues ->
        Box(
            Modifier
                .fillMaxSize()
                .padding(paddingValues)
        ) {
            NavHost(navController = navController, startDestination = "home") {
                composable("home") {
                    HomeScreen(deviceData, onConnect, onDisconnect, onSendCommand)
                }
                composable("feed") {
                    FeedScreen(deviceData, onSendCommand, onUpdateSettings, onSetFeeding, onSetWaterPump, onApplyWaterThreshold, onApplyWeightThreshold)
                }
                composable("schedule") {
                    ScheduleScreen(deviceData, onSendCommand, onApplySchedule, onApplyFeedWeight, onApplyEnabled)
                }
            }
        }
    }
}

fun createTimestamp(): String =
    SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).format(Date())
