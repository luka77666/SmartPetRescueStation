package com.example.app.components

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/**
 * 修复版自适应 DataCard
 * 抛弃死板比例，采用上下分层防挤压布局
 */
@Composable
fun DataCard(
    modifier: Modifier = Modifier,
    title: String,
    value: String,
    icon: ImageVector,
    valueColor: Color = Color(0xFF2B2D42),
    iconTint: Color = Color(0xFF4361EE),
) {
    Card(
        modifier = modifier.fillMaxWidth(), // 自适应高度，绝对不再重叠
        shape = RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White),
        // 恢复极轻微的阴影，去掉容易显得廉价的边框
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(14.dp), // 缩小内边距留出更多呼吸空间
            horizontalAlignment = Alignment.Start
        ) {
            // 顶部：图标与标题横向排列，节省垂直空间
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth()
            ) {
                Box(
                    modifier = Modifier
                        .size(32.dp) // 缩小底托
                        .clip(CircleShape)
                        .background(iconTint.copy(alpha = 0.12f)),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = icon,
                        contentDescription = title,
                        modifier = Modifier.size(18.dp),
                        tint = iconTint
                    )
                }
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = title,
                    fontSize = 13.sp,
                    color = Color(0xFF78909C), // 更柔和的灰色
                    fontWeight = FontWeight.Medium
                )
            }
            
            Spacer(modifier = Modifier.height(10.dp))
            
            // 底部：数值展示
            Text(
                text = value,
                fontSize = 24.sp, // 稍微克制字号，防止不同分辨率下换行越界
                fontWeight = FontWeight.Bold,
                color = valueColor
            )
        }
    }
}
