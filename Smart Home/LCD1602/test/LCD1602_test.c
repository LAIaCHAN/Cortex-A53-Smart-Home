//点阵LCD模块

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "lcd.h"
int main(void)
{
	int fd;
	int i,j,k;
	i=8;
	unsigned char uctech[] = {"UP-TECH"};
	unsigned char net[] = {"Happy every day!"};
	fd=open("/dev/LCD1602",O_RDWR);				//打开模块设备节点，O_RDWR为读写打开
	if(fd < 0){
		printf("####spi  device open fail####\n");
		return (-1);
	}
	while(1)
	{
	BSP_LCD1602_Init(fd);					//初始化LCD
	
	DisplayListChar(fd,0, 0, uctech);
	DisplayListChar(fd,0, 5, net);
	sleep(2);
	}
	close(fd);//关闭文件
	return 0;
}
