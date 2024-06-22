CC  =   aarch64-poky-linux-gcc --sysroot=/opt/fsl-imx-wayland/4.14-sumo/sysroots/aarch64-poky-linux
TARGET = LCD1602_test
all:
	$(CC) -o $(TARGET) $(TARGET).c lcd.c 
	
clean:
	rm $(TARGET) 
