#ifndef __STEPMOTOR_H
#define	__STEPMOTOR_H
#include "stm32f10x.h"
#include "sys.h"

// 步进电机 GPIO宏定义
#define	MOTOR_CLK				RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA // 步进电机使用的GPIO时钟（GPIOB和GPIOA）

#define MOTOR_A 				GPIO_Pin_8  // 电机A相引脚
#define MOTOR_B 				GPIO_Pin_9  // 电机B相引脚
#define MOTOR_C 				GPIO_Pin_12 // 电机C相引脚
#define MOTOR_D 				GPIO_Pin_15 // 电机D相引脚
#define MOTOR_AB_PORT 			GPIOB       // A相和B相所在端口
#define MOTOR_CD_PORT 			GPIOA       // C相和D相所在端口

#define MOTOR_A_HIGH GPIO_SetBits(MOTOR_AB_PORT,MOTOR_A)    // A相置高电平
#define MOTOR_A_LOW GPIO_ResetBits(MOTOR_AB_PORT,MOTOR_A)   // A相置低电平

#define MOTOR_B_HIGH GPIO_SetBits(MOTOR_AB_PORT,MOTOR_B)    // B相置高电平
#define MOTOR_B_LOW GPIO_ResetBits(MOTOR_AB_PORT,MOTOR_B)   // B相置低电平

#define MOTOR_C_HIGH GPIO_SetBits(MOTOR_CD_PORT,MOTOR_C)    // C相置高电平
#define MOTOR_C_LOW GPIO_ResetBits(MOTOR_CD_PORT,MOTOR_C)   // C相置低电平

#define MOTOR_D_HIGH GPIO_SetBits(MOTOR_CD_PORT,MOTOR_D)    // D相置高电平
#define MOTOR_D_LOW GPIO_ResetBits(MOTOR_CD_PORT,MOTOR_D)   // D相置低电平

// 电机状态机（供中断驱动）
extern volatile uint8_t  motor_running;   // 0=停止, 1=运行中
extern volatile int16_t  motor_steps;      // 剩余步数（>0=正转, <0=反转）
extern volatile uint8_t  motor_step_idx;   // 当前节拍索引(0-7)
extern volatile uint8_t  motor_dir;        // 0=正转(开), 1=反转(关)
extern volatile uint8_t  motor_continuous; // 1=持续转动模式(不限步数), 0=定角度模式

void MOTOR_Init(void);                                    // 步进电机GPIO初始化
void MOTOR_START(uint8_t dir, uint16_t angle);            // 非阻塞启动，按角度转动
void MOTOR_START_CONTINUOUS(uint8_t dir);                 // 非阻塞启动，持续转动(直到MOTOR_ABORT)
void MOTOR_ABORT(void);                                   // 立即停止
void MOTOR_StepTick(void);                                // 单步推进（中断中调用）
void MOTOR_STOP(void);                                    // 引脚全部拉低

#endif
