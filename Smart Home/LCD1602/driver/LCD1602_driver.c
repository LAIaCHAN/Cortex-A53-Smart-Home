//点阵LCD模块

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/io.h>

#define DEVICE_NAME		"LCD1602"	//设备名
#define DEV_MAJOR		0    // 0:动态获取设备节点号
/*使用内核宏，找到使用的GPIO引脚*/
#define IMX_GPIO_NR(bank, nr)	(((bank) - 1) * 32 + (nr))
#define SPI_SHCP		IMX_GPIO_NR(5, 10)
#define SPI_MISO		IMX_GPIO_NR(5, 12)
#define SPI_MOSI		IMX_GPIO_NR(5, 11)
#define SPI_STCP		IMX_GPIO_NR(3, 1)	
/*驱动信息：作者、描述*/
#define DRIVER_AUTHOR		"qcj"
#define DRIVER_DESCRIPTION	 "UP-MAGIC dot-matrix LCD Driver"
/*创建字符设备驱动，需要的结构体及变量*/
dev_t devno;
struct cdev *mycdev;
struct class *myclass;

static void SPISend ( unsigned char val)
{
	int i;
	for(i=0; i<8; i++)
	{
		gpio_set_value(SPI_SHCP,0);
		if( (val<<i) & 0x80 ){
			gpio_set_value(SPI_MOSI,1);
		}
		else{
			gpio_set_value(SPI_MOSI,0);
		}
		gpio_set_value(SPI_SHCP,1);
		udelay(5);
	}
}

/*设备驱动写入操作*/
static ssize_t lcd_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int i;
	unsigned char ptr[count];
	if (copy_from_user(ptr, buffer, count))	//将用户空间数据拷贝到内核
		return -EFAULT;
	gpio_set_value(SPI_STCP,0);
	for(i=0;i<count;i++)
	{
		SPISend(ptr[i]);
	}
	udelay(100);
	gpio_set_value(SPI_STCP,1);
	return count;
}
/*驱动层file_operations接口函数初始化*/
static struct file_operations uptech_LCD_fops=
{
	.owner			= THIS_MODULE,
	.write			= lcd_write,	
};
/*驱动程序入口函数，进行设备节点的创建等工作*/
static int __init uptech_LCD_init(void)
{
	int ret;   
	/*向系统申请GPIO引脚的使用*/
	gpio_request(SPI_SHCP,"SPI_SHCP");		
	gpio_request(SPI_MOSI,"SPI_MOSI");		
	gpio_request(SPI_STCP,"SPI_STCP");
	/*设置引脚为输出引脚，并置为低电平*/
	gpio_direction_output(SPI_MOSI,0);
	gpio_direction_output(SPI_STCP,0);
	gpio_direction_output(SPI_SHCP,0);
    
	/*判断DEV_MAJOR值，如果为0则自动获取主设备号*/
	if(DEV_MAJOR)
	{
		devno = MKDEV(DEV_MAJOR, 0);
		ret = register_chrdev_region(devno, 1, DEVICE_NAME);
	}
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);        
	}
	if(0 < ret){
		return ret;
	}
	/*创建字符设备*/
	mycdev = cdev_alloc();
	cdev_init(mycdev, &uptech_LCD_fops);
	ret = cdev_add(mycdev, devno, 1);
	if(ret){
		printk("cdev_add(); failed! ret_num=%d\n",ret);
	}
	myclass = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(myclass)){
		printk("Err: failed in creating class.\n");
		return -1;
	}
	device_create(myclass,NULL, devno, NULL, DEVICE_NAME);
	return 0;
}
/*驱动卸载函数*/
static void __exit uptech_LCD_exit(void)
{
	/*释放GPIO引脚*/
	gpio_free(SPI_STCP);
	gpio_free(SPI_SHCP);
	gpio_free(SPI_MOSI);
	/*删除设备节点、释放主设备号*/
	cdev_del(mycdev);
	device_destroy(myclass,devno);
	class_destroy(myclass);
}
module_init(uptech_LCD_init);//驱动程序入口
module_exit(uptech_LCD_exit);//驱动程序出口
/*驱动属性：遵循协议、作者、驱动描述*/ 
MODULE_LICENSE("GPL");                              
MODULE_AUTHOR(DRIVER_AUTHOR);             
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);  
