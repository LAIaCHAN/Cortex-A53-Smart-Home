#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "lcd.h"

void BSP_LCD1602_Init(int fd)
{
	BSP_LCD1602_WriteCommand(fd,0x38,1); //显示模式设置,开始要求每次检测忙信号
	BSP_LCD1602_WriteCommand(fd,0x08,1); //关闭显示
	BSP_LCD1602_WriteCommand(fd,0x01,1); //显示清屏
	BSP_LCD1602_WriteCommand(fd,0x06,1); // 显示光标移动设置
	BSP_LCD1602_WriteCommand(fd,0x0C,1); // 显示开及光标设置

}
void BSP_LCD1602_WriteBit(int fd,uint8_t *buf, uint8_t len)
{
	
	write(fd,(void*)buf,len);
}
//读状态
void BSP_LCD1602_ReadStatus(int fd)
{
	uint32_t count = 8000;
	unsigned char buf0[2]={0x14,0xFF};
	unsigned char buf1[2]={0x15,0xFF};
	BSP_LCD1602_WriteBit(fd,buf0,2);
	while(count--);
	BSP_LCD1602_WriteBit(fd,buf1,2);
}
//写指令
void BSP_LCD1602_WriteCommand(int fd,unsigned char WCLCD, unsigned char BuysC) //BuysC为0时忽略忙检测
{
	uint32_t count = 800;
	
	unsigned char buf0[2]={0x10,0xFF};
	unsigned char buf1[2]={0x11,0xFF};
	if (BuysC) 
		BSP_LCD1602_ReadStatus(fd); //根据需要检测忙
	buf0[1] = WCLCD;  buf1[1] = WCLCD;
	BSP_LCD1602_WriteBit(fd,buf1, 2);
	//while(count--);
	BSP_LCD1602_WriteBit(fd,buf0, 2);
}

//写数据
void BSP_LCD1602_WriteData(int fd,unsigned char WDLCD)
{
	uint32_t count = 800;
	unsigned char buf0[2]={0x18,0xFF};
	unsigned char buf1[2]={0x19,0xFF};

	BSP_LCD1602_ReadStatus(fd); //检测忙
	buf0[1] = WDLCD;  buf1[1] = WDLCD;

	BSP_LCD1602_WriteBit(fd,buf1, 2);
	//while(count--);
	BSP_LCD1602_WriteBit(fd,buf0, 2);
	//while(count--);
}

//按指定位置显示一个字符
void DisplayOneChar(int fd,unsigned char X, unsigned char Y, unsigned char DData)
{
	Y &= 0x1;
	X &= 0xF; //限制X不能大于15，Y不能大于1
	if (Y) 
		X |= 0x40; //当要显示第二行时地址码+0x40;
	X |= 0x80; // 算出指令码
	BSP_LCD1602_WriteCommand(fd,X, 0); //这里不检测忙信号，发送地址码
	BSP_LCD1602_WriteData(fd,DData);
}

//按指定位置显示一串字符
void DisplayListChar(int fd,unsigned char X, unsigned char Y, unsigned char *DData)
{
	unsigned char ListLength;

	ListLength = 0;
	Y &= 0x1;
	X &= 0xF; //限制X不能大于15，Y不能大于1
	while (DData[ListLength]>=0x20) //若到达字串尾则退出
	{
		if (X <= 0xF) //X坐标应小于0xF
		{
			DisplayOneChar(fd,X, Y, DData[ListLength]); //显示单个字符
			ListLength++;
			X++;
			sleep(1);
		}
	}
}
