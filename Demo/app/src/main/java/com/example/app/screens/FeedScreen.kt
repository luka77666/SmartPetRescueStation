package com.example.app.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.app.DeviceData
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

// ===== 统一主题色 =====
private val PrimaryBlue = Color(0xFF5C6BC0)
private val MintGreen = Color(0xFF26A69A)
private val CoralRed = Color(0xFFEF5350)
private val DisabledBg = Color(0xFFE0E0E0)
private val DisabledText = Color(0xFF9E9E9E)
private val TextDark = Color(0xFF37474F)
private val TextMuted = Color(0xFF90A4AE)
private val CapsuleShape = RoundedCornerShape(50)

/**
 * 带防抖的按钮状态管理。
 * 点击后立即禁用按钮 [cooldownMs] 毫秒，倒计时结束后恢复。
 */
@Composable
fun rememberDebouncedAction(cooldownMs: Long = 1000L): (action: () -> Unit) -> Unit {
    var cooldown by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    return remember {
        { action: () -> Unit ->
            if (cooldown) return@remember
            cooldown = true
            action()
            scope.launch {
                delay(cooldownMs)
                cooldown = false
            }
        }
    }
}

@Composable
fun FeedScreen(
    deviceData: DeviceData,
    onSendCommand: (String) -> Unit,
    onUpdateSettings: (Boolean, Int, Int) -> Unit,
    onSetFeeding: (Boolean) -> Unit,
    onSetWaterPump: (Boolean) -> Unit,
    onApplyWaterThreshold: (Int) -> Unit,
    onApplyWeightThreshold: (Int) -> Unit,
) {
    var showTip by remember { mutableStateOf("") }
    val scope = rememberCoroutineScope()

    val debounce = rememberDebouncedAction(1000L)

    var localWaterThresh by remember { mutableStateOf(deviceData.waterThreshold) }
    var localWeightThresh by remember { mutableStateOf(deviceData.weightThreshold) }

    LaunchedEffect(deviceData.waterThreshold) { localWaterThresh = deviceData.waterThreshold }
    LaunchedEffect(deviceData.weightThreshold) { localWeightThresh = deviceData.weightThreshold }

    fun ensureConnected(): Boolean {
        if (!deviceData.isConnected) { showTip = "请先连接设备"; return false }
        return true
    }

    fun sendDirect(cmd: String) {
        onSendCommand(cmd)
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp)
            .padding(bottom = 80.dp)
            .imePadding(),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text("救助控制", fontSize = 22.sp, fontWeight = FontWeight.Bold, color = TextDark)
        Spacer(Modifier.height(20.dp))

        // 模式切换
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(20.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.85f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
        ) {
            Column(Modifier.padding(16.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column {
                        Text("模式切换", fontSize = 16.sp, fontWeight = FontWeight.Bold, color = TextDark)
                        Spacer(Modifier.height(4.dp))
                        Surface(
                            shape = RoundedCornerShape(8.dp),
                            color = if (deviceData.isAutoMode) Color(0xFFE0F2F1) else Color(0xFFFFEBEE)
                        ) {
                            Text(
                                "当前: ${if (deviceData.isAutoMode) "自动模式" else "手动模式"}",
                                fontSize = 12.sp,
                                color = if (deviceData.isAutoMode) MintGreen else CoralRed,
                                modifier = Modifier.padding(horizontal = 10.dp, vertical = 3.dp)
                            )
                        }
                    }
                    Button(
                        onClick = {
                            debounce {
                                if (ensureConnected()) {
                                    onSendCommand("KA")
                                    showTip = "已发送模式切换命令"
                                }
                            }
                        },
                        enabled = deviceData.isConnected,
                        shape = CapsuleShape,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = PrimaryBlue,
                            disabledContainerColor = DisabledBg
                        ),
                        contentPadding = PaddingValues(horizontal = 20.dp, vertical = 8.dp)
                    ) {
                        Text("切换模式", fontSize = 14.sp)
                    }
                }
            }
        }

        Spacer(Modifier.height(14.dp))

        // 水泵控制
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(20.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.85f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
        ) {
            Column(Modifier.padding(16.dp)) {
                Text("水泵控制", fontSize = 16.sp, fontWeight = FontWeight.Bold, color = TextDark)
                Spacer(Modifier.height(10.dp))
                Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                    // 开启水泵 - 薄荷绿
                    Button(
                        onClick = {
                            debounce {
                                if (ensureConnected()) {
                                    sendDirect("MW0")
                                    showTip = "开启水泵 (MW0)"
                                }
                            }
                        },
                        modifier = Modifier.weight(1f).height(46.dp),
                        shape = CapsuleShape,
                        enabled = !deviceData.isWaterPumpOn && deviceData.isConnected,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MintGreen,
                            disabledContainerColor = DisabledBg
                        )
                    ) {
                        Text("开启水泵", fontSize = 14.sp)
                    }
                    // 关闭水泵 - 珊瑚红
                    Button(
                        onClick = {
                            debounce {
                                if (ensureConnected()) {
                                    sendDirect("MW1")
                                    showTip = "关闭水泵 (MW1)"
                                }
                            }
                        },
                        modifier = Modifier.weight(1f).height(46.dp),
                        shape = CapsuleShape,
                        enabled = deviceData.isWaterPumpOn && deviceData.isConnected,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = CoralRed,
                            disabledContainerColor = DisabledBg
                        )
                    ) {
                        Text("关闭水泵", fontSize = 14.sp)
                    }
                }
            }
        }

        Spacer(Modifier.height(14.dp))

        // 喂食控制
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(20.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.85f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
        ) {
            Column(Modifier.padding(16.dp)) {
                Text("救助控制", fontSize = 16.sp, fontWeight = FontWeight.Bold, color = TextDark)
                Spacer(Modifier.height(10.dp))
                Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                    // 开始喂食 - 薄荷绿
                    Button(
                        onClick = {
                            debounce {
                                if (ensureConnected()) {
                                    sendDirect("MF0")
                                    showTip = "开始喂食 (MF0)"
                                }
                            }
                        },
                        modifier = Modifier.weight(1f).height(46.dp),
                        shape = CapsuleShape,
                        enabled = !deviceData.isFeeding && deviceData.isConnected,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MintGreen,
                            disabledContainerColor = DisabledBg
                        )
                    ) {
                        Text("开始喂食", fontSize = 14.sp)
                    }
                    // 停止喂食 - 珊瑚红
                    Button(
                        onClick = {
                            debounce {
                                if (ensureConnected()) {
                                    sendDirect("MF1")
                                    showTip = "停止喂食 (MF1)"
                                }
                            }
                        },
                        modifier = Modifier.weight(1f).height(46.dp),
                        shape = CapsuleShape,
                        enabled = deviceData.isFeeding && deviceData.isConnected,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = CoralRed,
                            disabledContainerColor = DisabledBg
                        )
                    ) {
                        Text("停止喂食", fontSize = 14.sp)
                    }
                }
            }
        }

        Spacer(Modifier.height(14.dp))

        // 阈值设置
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(20.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.85f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
        ) {
            Column(Modifier.padding(16.dp)) {
                Text("阈值设置", fontSize = 16.sp, fontWeight = FontWeight.Bold, color = TextDark)
                Spacer(Modifier.height(14.dp))

                // 水位阈值
                Text("水位阈值: ${localWaterThresh}%", fontSize = 14.sp, color = TextDark)
                Spacer(Modifier.height(6.dp))
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Slider(
                        value = localWaterThresh.toFloat(),
                        onValueChange = { localWaterThresh = it.toInt() },
                        valueRange = 10f..100f,
                        steps = 17,
                        modifier = Modifier.weight(1f),
                        colors = SliderDefaults.colors(
                            thumbColor = PrimaryBlue,
                            activeTrackColor = PrimaryBlue
                        )
                    )
                    Spacer(Modifier.width(10.dp))
                    Button(
                        onClick = {
                            if (ensureConnected()) {
                                onApplyWaterThreshold(localWaterThresh)
                                showTip = "水位阈值已设置为${localWaterThresh}%"
                            }
                        },
                        enabled = deviceData.isConnected,
                        shape = CapsuleShape,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = PrimaryBlue,
                            disabledContainerColor = DisabledBg
                        ),
                        contentPadding = PaddingValues(horizontal = 14.dp, vertical = 4.dp)
                    ) {
                        Text("应用", fontSize = 12.sp)
                    }
                }

                Spacer(Modifier.height(16.dp))

                // 称重阈值
                Text("称重阈值: ${localWeightThresh}g", fontSize = 14.sp, color = TextDark)
                Spacer(Modifier.height(6.dp))
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Slider(
                        value = weightToSlider(localWeightThresh),
                        onValueChange = { localWeightThresh = sliderToWeight(it) },
                        valueRange = 0f..1f,
                        modifier = Modifier.weight(1f),
                        colors = SliderDefaults.colors(
                            thumbColor = PrimaryBlue,
                            activeTrackColor = PrimaryBlue
                        )
                    )
                    Spacer(Modifier.width(10.dp))
                    Button(
                        onClick = {
                            if (ensureConnected()) {
                                onApplyWeightThreshold(localWeightThresh)
                                showTip = "称重阈值已设置为${localWeightThresh}g"
                            }
                        },
                        enabled = deviceData.isConnected,
                        shape = CapsuleShape,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = PrimaryBlue,
                            disabledContainerColor = DisabledBg
                        ),
                        contentPadding = PaddingValues(horizontal = 14.dp, vertical = 4.dp)
                    ) {
                        Text("应用", fontSize = 12.sp)
                    }
                }
            }
        }

        Spacer(Modifier.height(14.dp))

        // 进入设置 - 治愈蓝胶囊
        Button(
            onClick = {
                debounce {
                    if (ensureConnected()) { onSendCommand("KE"); showTip = "已发送KE - 进入设置模式" }
                }
            },
            enabled = deviceData.isConnected,
            modifier = Modifier.fillMaxWidth().height(46.dp),
            shape = CapsuleShape,
            colors = ButtonDefaults.buttonColors(
                containerColor = PrimaryBlue,
                disabledContainerColor = DisabledBg
            )
        ) {
            Text("进入设置模式 (KE)", fontSize = 14.sp)
        }

        Spacer(Modifier.weight(1f))

        if (showTip.isNotEmpty()) {
            Surface(
                shape = RoundedCornerShape(12.dp),
                color = Color(0xFFE8EAF6)  // 淡紫蓝色提示
            ) {
                Text(
                    showTip,
                    fontSize = 13.sp,
                    color = PrimaryBlue,
                    fontWeight = FontWeight.Medium,
                    modifier = Modifier.padding(horizontal = 16.dp, vertical = 10.dp)
                )
            }
        }
    }
}

/**
 * 非线性称重阈值映射：低重量精细，高重量粗略
 */
private fun sliderToWeight(s: Float): Int {
    val p = s.coerceIn(0f, 1f) * 1000
    return when {
        p <= 300  -> (p * 100 / 300).toInt()
        p <= 550  -> (100 + (p - 300) * 400 / 250).toInt()
        p <= 800  -> (500 + (p - 550) * 1500 / 250).toInt()
        else      -> (2000 + (p - 800) * 7999 / 200).toInt()
    }.coerceIn(0, 9999)
}

private fun weightToSlider(w: Int): Float {
    val ww = w.coerceIn(0, 9999)
    val p = when {
        ww <= 100  -> ww * 300 / 100
        ww <= 500  -> 300 + (ww - 100) * 250 / 400
        ww <= 2000 -> 550 + (ww - 500) * 250 / 1500
        else       -> 800 + (ww - 2000) * 200 / 7999
    }
    return (p / 1000f).coerceIn(0f, 1f)
}
