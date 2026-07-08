package com.example.app.screens

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Restaurant
import androidx.compose.material.icons.filled.Thermostat
import androidx.compose.material.icons.filled.WaterDrop
import androidx.compose.material.icons.filled.Opacity
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.app.DeviceData
import com.example.app.R
import com.example.app.components.DataCard

private fun extractNumber(s: String): Int? {
    return s.replace(Regex("[^0-9\\-]"), "").toIntOrNull()
}

// 全新高级主题色
private val GeekBlue = Color(0xFF4361EE) 
private val MintGreen = Color(0xFF26A69A)
private val CoralRed = Color(0xFFEF5350)
private val AlarmRed = Color(0xFFFF5252)
private val TextDark = Color(0xFF2B2D42)
private val TextMuted = Color(0xFF8D99AE)

@Composable
fun HomeScreen(
    deviceData: DeviceData,
    onConnect: () -> Unit,
    onDisconnect: () -> Unit,
    onSendCommand: (String) -> Unit,
) {
    var showDialog by remember { mutableStateOf(false) }
    var dialogMessage by remember { mutableStateOf("") }
    val listState = rememberLazyListState()

    val waterValue = extractNumber(deviceData.waterLevel)
    val weightValue = extractNumber(deviceData.weight)
    val waterColor = if (waterValue != null && waterValue < deviceData.waterThreshold) AlarmRed else TextDark
    val weightColor = if (weightValue != null && weightValue < deviceData.weightThreshold) AlarmRed else TextDark

    LaunchedEffect(deviceData.logs.size) {
        if (deviceData.logs.isNotEmpty()) listState.animateScrollToItem(deviceData.logs.size - 1)
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp),
        horizontalAlignment = Alignment.Start // 整体左对齐
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 24.dp, bottom = 20.dp),
            horizontalArrangement = Arrangement.Start,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Image(
                painter = painterResource(id = R.drawable.school_logo),
                contentDescription = "校徽",
                modifier = Modifier.size(72.dp).clip(CircleShape),
                contentScale = ContentScale.Crop
            )
            Spacer(Modifier.width(16.dp))
            Column {
                Text("智能宠物救助站", fontSize = 28.sp, fontWeight = FontWeight.ExtraBold, color = TextDark)
                Text("校园物联网控制系统", fontSize = 15.sp, color = TextMuted)
            }
        }

        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White),
            elevation = CardDefaults.cardElevation(defaultElevation = 0.dp),
            border = BorderStroke(1.dp, Color(0xFFE2E8F0))
        ) {
            Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 12.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    val connColor = when {
                        deviceData.isConnected -> MintGreen
                        deviceData.isReconnecting -> Color(0xFFFF9800)
                        else -> CoralRed
                    }
                    val connText = when {
                        deviceData.isConnected -> "设备已在线"
                        deviceData.isReconnecting -> "正在重连..."
                        else -> "设备已离线"
                    }
                    Box(Modifier.size(8.dp).clip(CircleShape).background(connColor))
                    Spacer(Modifier.width(8.dp))
                    Text(connText, fontSize = 14.sp, fontWeight = FontWeight.Bold, color = TextDark)
                }
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    if (!deviceData.isConnected) {
                        Button(
                            onClick = onConnect,
                            colors = ButtonDefaults.buttonColors(containerColor = GeekBlue),
                            shape = RoundedCornerShape(50),
                            contentPadding = PaddingValues(horizontal = 16.dp, vertical = 0.dp),
                            modifier = Modifier.height(32.dp)
                        ) { Text("连接", fontSize = 13.sp) }
                    } else {
                        OutlinedButton(
                            onClick = { onDisconnect(); dialogMessage = "已断开连接"; showDialog = true },
                            colors = ButtonDefaults.outlinedButtonColors(contentColor = CoralRed),
                            border = BorderStroke(1.dp, CoralRed),
                            shape = RoundedCornerShape(50),
                            contentPadding = PaddingValues(horizontal = 16.dp, vertical = 0.dp),
                            modifier = Modifier.height(32.dp)
                        ) { Text("断开", fontSize = 13.sp) }
                    }
                }
            }
        }

        Spacer(Modifier.height(20.dp))

        // 增大间距到 16dp
        Column(
            Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                DataCard(
                    Modifier.weight(1f), "环境温度", deviceData.temperature,
                    icon = Icons.Filled.Thermostat, iconTint = CoralRed, valueColor = TextDark
                )
                DataCard(
                    Modifier.weight(1f), "环境湿度", deviceData.humidity,
                    icon = Icons.Filled.WaterDrop, iconTint = GeekBlue, valueColor = TextDark
                )
            }
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                DataCard(
                    Modifier.weight(1f), "当前水位", deviceData.waterLevel,
                    icon = Icons.Filled.Opacity, iconTint = MintGreen, valueColor = waterColor
                )
                DataCard(
                    Modifier.weight(1f), "余粮重量", deviceData.weight,
                    icon = Icons.Filled.Restaurant, iconTint = Color(0xFFFF9800), valueColor = weightColor
                )
            }
        }

        Spacer(Modifier.height(20.dp))

        // 日志面板优化
        Card(
            modifier = Modifier.fillMaxSize().weight(1f, fill = true),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White),
            elevation = CardDefaults.cardElevation(defaultElevation = 0.dp),
            border = BorderStroke(1.dp, Color(0xFFE2E8F0))
        ) {
            Column(Modifier.fillMaxSize().padding(16.dp)) {
                Text("终端日志", fontSize = 14.sp, color = TextDark, fontWeight = FontWeight.Bold)
                Spacer(Modifier.height(8.dp))
                // 日志框内灰底，提升层次感
                Box(modifier = Modifier.fillMaxSize().clip(RoundedCornerShape(8.dp)).background(Color(0xFFF8F9FA)).padding(8.dp)) {
                    LazyColumn(state = listState, modifier = Modifier.fillMaxSize()) {
                        items(deviceData.logs) { log ->
                            Text(log, fontSize = 11.sp, color = Color(0xFF546E7A), maxLines = 2, overflow = TextOverflow.Ellipsis, modifier = Modifier.padding(vertical = 2.dp))
                        }
                    }
                }
            }
        }

        if (showDialog) {
            AlertDialog(
                onDismissRequest = { showDialog = false },
                title = { Text("系统提示") },
                text = { Text(dialogMessage) },
                confirmButton = {
                    Button(onClick = { showDialog = false }, colors = ButtonDefaults.buttonColors(containerColor = GeekBlue)) { Text("确定") }
                }
            )
        }
    }
}
