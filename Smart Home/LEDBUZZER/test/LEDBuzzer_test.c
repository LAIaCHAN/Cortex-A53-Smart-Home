//led蜂鸣器模块
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/*ioctl操作命令*/
#define IOCTL_LED_ON		0	//灯开
#define IOCTL_LED_OFF		1	//灯灭
#define IOCTL_BUZZER_OFF	3	//蜂鸣器关
#define IOCTL_BUZZER_ON		4	//蜂鸣器开

void print_menu()
{
	printf("----------------------------\n");
	printf("|   1.turn on the led      |\n");
	printf("|   2.turn off the led     |\n");
	printf("|   3.turn on the buzzer   |\n");
	printf("|   4.turn off the buzzer  |\n");
	printf("|   5.EXIT                 |\n");
	printf("----------------------------\n");
}
int main(void)
{
    unsigned int led;	//定义led
    unsigned int puzzer;//定义蜂鸣器
    int fd = -1,menu_num;
        
    fd = open("/dev/LEDBuzzer", 0);		//打开模块设备文件，0为O_RDONLY
    if (fd < 0) {				//打开失败
        printf("Can't open /dev/LEDBuzzer\n");
        return -1;
    }
    
    while(1)
    {
	print_menu();
	scanf("%d", &menu_num);	
	/*跳出循环，退出程序*/			
	if(menu_num == 5)				
		break;
	if(menu_num == 1)				
		ioctl(fd,IOCTL_LED_ON);			//调用ioctl命令让灯开	
	if(menu_num == 2)				
		ioctl(fd,IOCTL_LED_OFF);		//调用ioctl命令让灯关
	if(menu_num == 3)				
		ioctl(fd,IOCTL_BUZZER_ON);		//打开蜂鸣器
	if(menu_num == 4)				
		ioctl(fd,IOCTL_BUZZER_OFF);		//关闭蜂鸣器					
    }
	ioctl(fd,IOCTL_LED_OFF);
	ioctl(fd,IOCTL_BUZZER_OFF);
    	close(fd);					//关闭文件
	return 0;
}

