package com.example.app.screens

import androidx.compose.animation.animateContentSize
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Restaurant
import androidx.compose.material.icons.rounded.Add
import androidx.compose.material.icons.rounded.Remove
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.app.DeviceData
import com.example.app.ScheduleItem
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.util.Locale

// ===== 极简现代大厂主题色 =====
private val GeekBlue = Color(0xFF4361EE)
private val TextDark = Color(0xFF2B2D42)
private val TextMuted = Color(0xFF8D99AE)
private val BgGray = Color(0xFFF4F7FB)
private val CardBorder = Color(0xFFE2E8F0)

// ==================== 苹果风无界滚轮选择器 ====================

private const val ITEM_HEIGHT = 44 // 增大触摸面积，提升高级感

@Composable
private fun WheelColumn(
    items: List<Int>,
    currentIndex: Int,
    onSelect: (Int) -> Unit,
    enabled: Boolean,
) {
    val topPad = 1
    val totalItems = items.size + topPad * 2
    val listState = rememberLazyListState(initialFirstVisibleItemIndex = currentIndex.coerceAtLeast(0))

    val selectedValue = remember {
        derivedStateOf {
            val first = listState.firstVisibleItemIndex
            val offset = listState.firstVisibleItemScrollOffset
            val center = if (offset > ITEM_HEIGHT / 2) first + 1 else first
            val idx = center.coerceIn(0, items.size - 1)
            items.getOrNull(idx) ?: items.firstOrNull() ?: 0
        }
    }

    LaunchedEffect(listState.isScrollInProgress) {
        if (!listState.isScrollInProgress) {
            val first = listState.firstVisibleItemIndex
            val offset = listState.firstVisibleItemScrollOffset
            val target = if (offset > ITEM_HEIGHT / 2) first + 1 else first
            val safeTarget = target.coerceIn(0, items.size - 1)
            if (first != safeTarget || offset > ITEM_HEIGHT / 2) {
                listState.animateScrollToItem(safeTarget)
            }
            onSelect(items[safeTarget])
        }
    }

    Box(
        contentAlignment = Alignment.Center,
        modifier = Modifier.height((ITEM_HEIGHT * 3).dp).width(72.dp)
    ) {
        // 中央高亮选中条 (极其柔和的蓝色透明底)
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(ITEM_HEIGHT.dp)
                .background(GeekBlue.copy(alpha = 0.08f), RoundedCornerShape(12.dp))
        )
        
        LazyColumn(
            state = listState,
            modifier = Modifier.fillMaxSize(),
            userScrollEnabled = enabled
        ) {
            items(totalItems) { index ->
                val itemIdx = index - topPad
                val isSelected = items.getOrNull(itemIdx) == selectedValue.value
                Box(
                    modifier = Modifier.height(ITEM_HEIGHT.dp).fillMaxWidth(),
                    contentAlignment = Alignment.Center
                ) {
                    if (itemIdx in items.indices) {
                        Text(
                            text = String.format(Locale.US, "%02d", items[itemIdx]),
                            fontSize = if (isSelected) 24.sp else 18.sp, // 选中项文字放大
                            fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Medium,
                            color = if (!enabled) Color(0xFFCFD8DC) else if (isSelected) GeekBlue else Color(0xFFB0BEC5)
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun TimeWheelPicker(
    hour: Int,
    minute: Int,
    enabled: Boolean,
    onHourChange: (Int) -> Unit,
    onMinuteChange: (Int) -> Unit,
) {
    val hours = remember { (0..23).toList() }
    val minutes = remember { (0..59).toList() }

    Row(
        modifier = Modifier.fillMaxWidth().padding(vertical = 12.dp),
        horizontalArrangement = Arrangement.Center,
        verticalAlignment = Alignment.CenterVertically
    ) {
        WheelColumn(items = hours, currentIndex = hour, onSelect = onHourChange, enabled = enabled)
        Text(
            ":",
            fontSize = 32.sp,
            fontWeight = FontWeight.Light,
            color = if (enabled) TextDark else Color(0xFFCFD8DC),
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 0.dp).offset(y = (-4).dp)
        )
        WheelColumn(items = minutes, currentIndex = minute, onSelect = onMinuteChange, enabled = enabled)
    }
}

// ==================== 页面主体 ====================

@Composable
fun ScheduleScreen(
    deviceData: DeviceData,
    onSendCommand: (String) -> Unit,
    onApplySchedule: (Int, Int, Int) -> Unit,
    onApplyFeedWeight: (Int, Int) -> Unit,
    onApplyEnabled: (Int, Boolean) -> Unit,
) {
    var s1 by remember { mutableStateOf(deviceData.schedule1) }
    var s2 by remember { mutableStateOf(deviceData.schedule2) }
    var s3 by remember { mutableStateOf(deviceData.schedule3) }
    var showTip by remember { mutableStateOf("") }
    val scope = rememberCoroutineScope()

    LaunchedEffect(deviceData.schedule1) { s1 = deviceData.schedule1 }
    LaunchedEffect(deviceData.schedule2) { s2 = deviceData.schedule2 }
    LaunchedEffect(deviceData.schedule3) { s3 = deviceData.schedule3 }

    fun ensureConnected(): Boolean {
        if (!deviceData.isConnected) { showTip = "设备未连接，无法同步"; return false }
        return true
    }

    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(scrollState)
            .padding(16.dp)
            .padding(bottom = 90.dp),
        horizontalAlignment = Alignment.Start
    ) {
        // 头部标题区域
        Text("自动化计划", fontSize = 28.sp, fontWeight = FontWeight.ExtraBold, color = TextDark, modifier = Modifier.padding(top = 16.dp))
        Text("设置设备每天的自动运行时间", fontSize = 14.sp, color = TextMuted, modifier = Modifier.padding(top = 4.dp, bottom = 24.dp))

        // 折叠式定时卡片
        ExpandableAlarmCard("计划一", s1, { s1 = it }, deviceData.isConnected, 
            onSave = { if (ensureConnected()) { onApplySchedule(1, s1.hour, s1.minute); onApplyFeedWeight(1, s1.feedWeight); showTip = "计划一 已保存" } },
            onToggle = { if (ensureConnected()) { s1 = s1.copy(enabled = it); onApplyEnabled(1, it) } }
        )
        Spacer(Modifier.height(16.dp))

        ExpandableAlarmCard("计划二", s2, { s2 = it }, deviceData.isConnected,
            onSave = { if (ensureConnected()) { onApplySchedule(2, s2.hour, s2.minute); onApplyFeedWeight(2, s2.feedWeight); showTip = "计划二 已保存" } },
            onToggle = { if (ensureConnected()) { s2 = s2.copy(enabled = it); onApplyEnabled(2, it) } }
        )
        Spacer(Modifier.height(16.dp))

        ExpandableAlarmCard("计划三", s3, { s3 = it }, deviceData.isConnected,
            onSave = { if (ensureConnected()) { onApplySchedule(3, s3.hour, s3.minute); onApplyFeedWeight(3, s3.feedWeight); showTip = "计划三 已保存" } },
            onToggle = { if (ensureConnected()) { s3 = s3.copy(enabled = it); onApplyEnabled(3, it) } }
        )

        Spacer(Modifier.height(32.dp))

        // 底部全局同步按钮
        Button(
            onClick = {
                if (ensureConnected()) {
                    scope.launch {
                        showTip = "正在全量同步至设备..."
                        onApplySchedule(1, s1.hour, s1.minute); delay(150)
                        onApplyFeedWeight(1, s1.feedWeight); delay(150)
                        onApplyEnabled(1, s1.enabled); delay(150)
                        onApplySchedule(2, s2.hour, s2.minute); delay(150)
                        onApplyFeedWeight(2, s2.feedWeight); delay(150)
                        onApplyEnabled(2, s2.enabled); delay(150)
                        onApplySchedule(3, s3.hour, s3.minute); delay(150)
                        onApplyFeedWeight(3, s3.feedWeight); delay(150)
                        onApplyEnabled(3, s3.enabled); delay(150)
                        showTip = "所有计划已同步成功"
                    }
                }
            },
            enabled = deviceData.isConnected,
            shape = RoundedCornerShape(16.dp),
            colors = ButtonDefaults.buttonColors(containerColor = GeekBlue),
            contentPadding = PaddingValues(vertical = 16.dp),
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("同步所有设置", fontSize = 16.sp, fontWeight = FontWeight.Bold)
        }

        if (showTip.isNotEmpty()) {
            Spacer(Modifier.height(16.dp))
            Text(showTip, fontSize = 13.sp, color = GeekBlue, textAlign = TextAlign.Center, modifier = Modifier.fillMaxWidth())
        }
    }
}

// ==================== 折叠式闹钟卡片组件 ====================

@Composable
private fun ExpandableAlarmCard(
    title: String,
    item: ScheduleItem,
    onItemChange: (ScheduleItem) -> Unit,
    enabled: Boolean,
    onSave: () -> Unit,
    onToggle: (Boolean) -> Unit,
) {
    var isExpanded by remember { mutableStateOf(false) }
    val interactionSource = remember { MutableInteractionSource() }

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .animateContentSize() // 折叠/展开平滑动画核心
            .clickable(interactionSource = interactionSource, indication = null) { 
                isExpanded = !isExpanded 
            },
        shape = RoundedCornerShape(24.dp), // 极其圆润的苹果风圆角
        colors = CardDefaults.cardColors(containerColor = Color.White),
        elevation = CardDefaults.cardElevation(defaultElevation = 0.dp),
        border = BorderStroke(1.dp, CardBorder)
    ) {
        Column(Modifier.padding(20.dp)) {
            // 默认可见区域：超大时间 + 喂食量 + 开关
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column {
                    Text(
                        text = String.format(Locale.US, "%02d:%02d", item.hour, item.minute),
                        fontSize = 46.sp, // 超大无衬线感字体
                        fontWeight = FontWeight.Light,
                        color = if (item.enabled) TextDark else Color(0xFFB0BEC5),
                        letterSpacing = (-1.5).sp,
                        modifier = Modifier.offset(x = (-2).dp) // 视觉对齐微调
                    )
                    Spacer(Modifier.height(4.dp))
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Icon(Icons.Filled.Restaurant, contentDescription = null, modifier = Modifier.size(13.dp), tint = TextMuted)
                        Spacer(Modifier.width(4.dp))
                        Text("喂食量: ${item.feedWeight}g", fontSize = 13.sp, color = TextMuted, fontWeight = FontWeight.Medium)
                        Spacer(Modifier.width(8.dp))
                        Text("· $title", fontSize = 13.sp, color = Color(0xFFCFD8DC))
                    }
                }
                
                Switch(
                    checked = item.enabled,
                    onCheckedChange = { onToggle(it) },
                    enabled = enabled,
                    colors = SwitchDefaults.colors(
                        checkedThumbColor = Color.White,
                        checkedTrackColor = GeekBlue,
                        uncheckedThumbColor = Color.White,
                        uncheckedTrackColor = Color(0xFFE2E8F0),
                        uncheckedBorderColor = Color.Transparent
                    )
                )
            }

            // 展开后的编辑区域
            if (isExpanded) {
                Spacer(Modifier.height(24.dp))
                HorizontalDivider(color = BgGray)
                Spacer(Modifier.height(16.dp))

                // 时间调节
                Text("设定执行时间", fontSize = 13.sp, color = TextMuted, fontWeight = FontWeight.Medium)
                TimeWheelPicker(
                    hour = item.hour,
                    minute = item.minute,
                    enabled = enabled && item.enabled,
                    onHourChange = { onItemChange(item.copy(hour = it)) },
                    onMinuteChange = { onItemChange(item.copy(minute = it)) }
                )

                Spacer(Modifier.height(16.dp))

                // 喂食量调节 (步进器)
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween,
                    modifier = Modifier.fillMaxWidth().background(BgGray, RoundedCornerShape(16.dp)).padding(12.dp)
                ) {
                    Text("设定单次喂食量", fontSize = 14.sp, color = TextDark, fontWeight = FontWeight.Medium)
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        // 减号按钮
                        Box(
                            modifier = Modifier.size(32.dp).clip(CircleShape).background(Color.White).clickable(enabled = enabled && item.enabled) { 
                                val newVal = (item.feedWeight - 10).coerceAtLeast(10)
                                onItemChange(item.copy(feedWeight = newVal)) 
                            },
                            contentAlignment = Alignment.Center
                        ) { Icon(Icons.Rounded.Remove, contentDescription = null, tint = GeekBlue, modifier = Modifier.size(18.dp)) }
                        
                        Text(
                            "${item.feedWeight} g",
                            modifier = Modifier.width(60.dp),
                            textAlign = TextAlign.Center,
                            fontWeight = FontWeight.Bold,
                            fontSize = 15.sp,
                            color = TextDark
                        )
                        
                        // 加号按钮
                        Box(
                            modifier = Modifier.size(32.dp).clip(CircleShape).background(Color.White).clickable(enabled = enabled && item.enabled) { 
                                val newVal = (item.feedWeight + 10).coerceAtMost(9990)
                                onItemChange(item.copy(feedWeight = newVal)) 
                            },
                            contentAlignment = Alignment.Center
                        ) { Icon(Icons.Rounded.Add, contentDescription = null, tint = GeekBlue, modifier = Modifier.size(18.dp)) }
                    }
                }

                Spacer(Modifier.height(24.dp))

                // 保存并折叠按钮
                Button(
                    onClick = { onSave(); isExpanded = false },
                    enabled = enabled,
                    modifier = Modifier.fillMaxWidth().height(48.dp),
                    colors = ButtonDefaults.buttonColors(containerColor = TextDark), // 高级黑保存按钮
                    shape = RoundedCornerShape(14.dp)
                ) {
                    Text("保存此计划", fontSize = 14.sp, fontWeight = FontWeight.Bold)
                }
            }
        }
    }
}
