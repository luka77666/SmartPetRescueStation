package com.example.app.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.app.DeviceData

@Composable
fun WifiScreen(
    deviceData: DeviceData,
    onSendCommand: (String) -> Unit,
    onSwitchHost: (String) -> Unit,
) {
    var ssidInput by remember { mutableStateOf("") }
    var passwordInput by remember { mutableStateOf("") }
    var configStatus by remember { mutableStateOf("") }
    var isConfiguring by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Transparent)
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        // 标题
        Text(
            "WiFi 配网",
            fontSize = 22.sp,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )

        // 使用说明
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.95f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(modifier = Modifier.padding(14.dp)) {
                Text(
                    "配网步骤",
                    fontSize = 16.sp,
                    fontWeight = FontWeight.Bold,
                    color = Color(0xFF333333)
                )
                Spacer(Modifier.height(6.dp))
                Text(
                    "1. 确保手机和设备在同一WiFi网络下\n" +
                    "2. 在首页输入设备的IP地址并点击\"应用\"\n" +
                    "3. 点击\"连接\"连接到设备\n" +
                    "4. 如需更换WiFi，填写目标WiFi信息后点击\"开始配网\"\n" +
                    "5. 配网成功后设备将自动连接新WiFi并回传IP",
                    fontSize = 13.sp,
                    color = Color(0xFF666666),
                    lineHeight = 20.sp
                )
            }
        }

        // 连接状态
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.95f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth().padding(14.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                val connColor = when {
                    deviceData.isConnected -> Color(0xFF4CAF50)
                    deviceData.isReconnecting -> Color(0xFFFF9800)
                    else -> Color(0xFFE53935)
                }
                val connText = when {
                    deviceData.isConnected -> "设备已连接 - 可以配网"
                    deviceData.isReconnecting -> "正在重连..."
                    else -> "设备未连接 - 请先在首页输入IP并连接"
                }
                Box(Modifier.size(12.dp).background(connColor, RoundedCornerShape(6.dp)))
                Spacer(Modifier.width(8.dp))
                Text(text = connText, fontSize = 14.sp, fontWeight = FontWeight.Medium)
            }
        }

        // WiFi 名称输入
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.95f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(modifier = Modifier.padding(14.dp)) {
                Text("目标 WiFi 名称", fontSize = 14.sp, fontWeight = FontWeight.Medium, color = Color(0xFF333333))
                Spacer(Modifier.height(6.dp))
                OutlinedTextField(
                    value = ssidInput,
                    onValueChange = { ssidInput = it },
                    placeholder = { Text("如: 龙的Xiaomi14", fontSize = 13.sp) },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth(),
                    shape = RoundedCornerShape(8.dp),
                    textStyle = androidx.compose.ui.text.TextStyle(fontSize = 15.sp)
                )
            }
        }

        // WiFi 密码输入
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White.copy(alpha = 0.95f)),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(modifier = Modifier.padding(14.dp)) {
                Text("WiFi 密码", fontSize = 14.sp, fontWeight = FontWeight.Medium, color = Color(0xFF333333))
                Spacer(Modifier.height(6.dp))
                OutlinedTextField(
                    value = passwordInput,
                    onValueChange = { passwordInput = it },
                    placeholder = { Text("输入密码", fontSize = 13.sp) },
                    singleLine = true,
                    visualTransformation = PasswordVisualTransformation(),
                    modifier = Modifier.fillMaxWidth(),
                    shape = RoundedCornerShape(8.dp),
                    textStyle = androidx.compose.ui.text.TextStyle(fontSize = 15.sp)
                )
            }
        }

        // 开始配网按钮
        Button(
            onClick = {
                if (ssidInput.isNotEmpty()) {
                    isConfiguring = true
                    configStatus = "正在配网，请等待..."
                    val cmd = "WIFI:$ssidInput,$passwordInput"
                    onSendCommand(cmd)
                } else {
                    configStatus = "请输入WiFi名称"
                }
            },
            modifier = Modifier.fillMaxWidth().height(50.dp),
            enabled = deviceData.isConnected && ssidInput.isNotEmpty() && !isConfiguring,
            shape = RoundedCornerShape(12.dp),
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF4CAF50))
        ) {
            if (isConfiguring) {
                CircularProgressIndicator(
                    modifier = Modifier.size(20.dp),
                    color = Color.White,
                    strokeWidth = 2.dp
                )
                Spacer(Modifier.width(8.dp))
                Text("配网中...", fontSize = 16.sp)
            } else {
                Text("开始配网", fontSize = 16.sp, fontWeight = FontWeight.Bold)
            }
        }

        // 配网状态显示
        if (configStatus.isNotEmpty()) {
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp),
                colors = CardDefaults.cardColors(
                    containerColor = if (configStatus.startsWith("成功")) 
                        Color(0xFFE8F5E9) 
                    else if (configStatus.startsWith("失败"))
                        Color(0xFFFFEBEE)
                    else 
                        Color.White.copy(alpha = 0.95f)
                ),
                elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
            ) {
                Text(
                    configStatus,
                    modifier = Modifier.padding(14.dp),
                    fontSize = 14.sp,
                    color = if (configStatus.startsWith("成功")) Color(0xFF2E7D32) 
                           else if (configStatus.startsWith("失败")) Color(0xFFC62828) 
                           else Color(0xFF666666)
                )
            }
        }
    }

    // 监听设备数据变化，检测配网结果
    LaunchedEffect(deviceData.logs.lastOrNull()) {
        val lastLog = deviceData.logs.lastOrNull() ?: return@LaunchedEffect
        if (lastLog.contains("WIFI_OK:")) {
            // 提取IP
            val ipStart = lastLog.indexOf("WIFI_OK:") + 8
            val ip = if (ipStart < lastLog.length) {
                lastLog.substring(ipStart).trim()
            } else ""
            configStatus = "成功! 设备IP: $ip\n请切换IP地址后重新连接"
            isConfiguring = false
            if (ip.isNotEmpty()) {
                onSwitchHost(ip)
            }
        } else if (lastLog.contains("WIFI_FAIL")) {
            configStatus = "失败! WiFi名称或密码错误，请重试"
            isConfiguring = false
        }
    }
}
