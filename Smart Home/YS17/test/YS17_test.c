//火焰模块的测试程序

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int i;
    int ret;
    int fd;
    int YS17_state;
    
    fd = open("/dev/YS17", 0);//打开设备文件 
    if (fd < 0) {
		printf("Can't open /dev/YS17\n");
		return -1;
	}

	while (1) {
		ret = read(fd,&YS17_state, sizeof(YS17_state));//读取数据
		if (ret > 0) {
			if (!YS17_state)
				printf("Fire!\n");
			else
				printf("Security!\n");
		} 
		sleep(1);
	}
	close(fd);//关闭文件
	return 0;
}

