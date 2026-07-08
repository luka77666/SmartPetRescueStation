#include "flash.h" // Flash读写驱动头文件


////FLASH写入数据
void FLASH_W(u32 add, u16 *dat, u8 count)
{
	 u8 i; // 循环计数变量
	 FLASH_Unlock();  //解锁FLASH编程擦除控制器
     FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
     FLASH_ErasePage(add);     //擦除指定地址页
	 for (i = 0; i < count; i++) // 循环写入count个半字数据
	 {
		 FLASH_ProgramHalfWord(add + i * 2, dat[i]); // 在指定地址写入一个半字（16位）
	 }
     FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
     FLASH_Lock();    //锁定FLASH编程擦除控制器
}

//FLASH读出数据
u16 FLASH_R(u32 add)	 //参数1：32位读出FLASH地址。返回值：16位数据
{
	u16 a; // 存放读取到的16位数据
    a = *(u16*)(add);//从指定页的addr地址开始读
	return a; // 返回读取到的数据
}
