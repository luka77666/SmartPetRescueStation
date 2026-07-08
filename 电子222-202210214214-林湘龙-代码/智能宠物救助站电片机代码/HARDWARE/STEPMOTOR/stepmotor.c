#include "stepmotor.h"         // 步进电机驱动头文件

// ====== 电机状态机变量（volatile因为TIM2中断中也会访问）======
volatile uint8_t  motor_running = 0;    // 电机运行标志：0=停止, 1=运行
volatile int16_t  motor_steps = 0;      // 剩余步数（定角度模式用，每步减1）
volatile uint8_t  motor_step_idx = 0;   // 当前节拍索引（0-7循环）
volatile uint8_t  motor_dir = 0;        // 电机方向：0=正转, 1=反转
volatile uint8_t  motor_continuous = 0; // 持续转动模式：0=定角度, 1=一直转

// 八拍序列（半步模式，精度更高、力矩更均匀）
// 28BYJ-48步进电机：输出轴360度=4096步(64:1减速比×64步/圈)
static const uint8_t motor_seq_fwd[8] = {0x01,0x03,0x02,0x06,0x04,0x0C,0x08,0x09}; // 正转序列
static const uint8_t motor_seq_rev[8] = {0x09,0x08,0x0C,0x04,0x06,0x02,0x03,0x01}; // 反转序列

/**
  * @brief  步进电机GPIO初始化（PB8~PB11，ULN2003驱动板）
  * @param  无
  * @retval 无
  */
void MOTOR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(MOTOR_CLK, ENABLE);  // 使能GPIO时钟

	GPIO_InitStructure.GPIO_Pin = MOTOR_A|MOTOR_B;  // 电机A/B相引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
	GPIO_Init(MOTOR_AB_PORT,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MOTOR_C|MOTOR_D;  // 电机C/D相引脚
	GPIO_Init(MOTOR_CD_PORT,&GPIO_InitStructure);

	GPIO_ResetBits(MOTOR_AB_PORT, MOTOR_A|MOTOR_B);  // 初始化：所有相拉低（断电）
	GPIO_ResetBits(MOTOR_CD_PORT, MOTOR_C|MOTOR_D);
}

/**
  * @brief  非阻塞启动电机（定角度模式）
  * @param  dir: 0=正转(推料), 1=反转(防卡粮)
  * @param  angle: 旋转角度（如1800=5圈）
  * @retval 无
  * 28BYJ-48 八拍模式：输出轴360度=4096步(64减速比×64步)
  */
void MOTOR_START(uint8_t dir, uint16_t angle)
{
	motor_steps = (int16_t)((uint32_t)4096 * angle / 360);  // 角度转换为步数
	motor_dir = dir;               // 设置方向
	motor_step_idx = 0;            // 重置节拍索引
	motor_continuous = 0;          // 定角度模式（非持续）
	motor_running = 1;             // 启动电机
}

/**
  * @brief  持续转动模式启动（电机一直转，直到调用MOTOR_ABORT停止）
  * @param  dir: 0=正转, 1=反转
  * @retval 无
  */
void MOTOR_START_CONTINUOUS(uint8_t dir)
{
	motor_dir = dir;               // 设置方向
	motor_step_idx = 0;            // 重置节拍
	motor_continuous = 1;          // 持续模式
	motor_running = 1;             // 启动
}

/**
  * @brief  立即停止电机（断电）
  * @param  无
  * @retval 无
  */
void MOTOR_ABORT(void)
{
	motor_running = 0;             // 清除运行标志
	motor_steps = 0;               // 清除剩余步数
	MOTOR_STOP();                  // 断电停止
}

/**
  * @brief  单步推进（在TIM2中断中每1ms调用一次）
  * @param  无
  * @retval 无
  * 根据当前方向和节拍索引，输出对应的ABCD相电平
  * 定角度模式：步数用完自动停止；持续模式：一直转直到MOTOR_ABORT
  */
void MOTOR_StepTick(void)
{
	if (!motor_running) return;    // 没在运行则直接返回

	// 选择正转或反转的八拍序列
	const uint8_t *seq = motor_dir ? motor_seq_rev : motor_seq_fwd;
	uint8_t phase = seq[motor_step_idx];  // 获取当前节拍相位

	// 根据相位设置ABCD四相引脚电平
	if (phase & 0x01) MOTOR_A_HIGH; else MOTOR_A_LOW;  // A相
	if (phase & 0x02) MOTOR_B_HIGH; else MOTOR_B_LOW;  // B相
	if (phase & 0x04) MOTOR_C_HIGH; else MOTOR_C_LOW;  // C相
	if (phase & 0x08) MOTOR_D_HIGH; else MOTOR_D_LOW;  // D相

	// 推进步数（定角度模式才递减）
	if (!motor_continuous)
	{
		motor_steps--;             // 步数-1
	}
	motor_step_idx = (motor_step_idx + 1) & 7;  // 节拍索引循环(0-7)

	// 定角度模式：步数用完停止；持续模式：一直转直到MOTOR_ABORT
	if (!motor_continuous && motor_steps <= 0)
	{
		motor_running = 0;         // 标记停止
		MOTOR_STOP();              // 断电
	}
}

/**
  * @brief  电机断电（所有相拉低）
  * @param  无
  * @retval 无
  */
void MOTOR_STOP(void)
{
	GPIO_ResetBits(MOTOR_AB_PORT, MOTOR_A|MOTOR_B);  // AB相断电
	GPIO_ResetBits(MOTOR_CD_PORT, MOTOR_C|MOTOR_D);  // CD相断电
}
