//火焰传感器驱动程序

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
/***********************************************************/
/*宏定义设备名设备号*/
#define DEVICE_NAME		"YS17"		//设备文件名

/***********************************************************/
static dev_t YS17_devno;			//设备号
static struct cdev YS17_cdev;		//字符设备对象
static struct class *YS17_class;	//字符设备类

static unsigned long YS17_GPIO;		//定义GPIO口
static int sensor_state = 0;		//传感器状态
static int *buffer = NULL;

/**********************************************************
 * 函数名: YS17_interrupt
 * 描述: 中断处理函数
 * 参数: 1.中断号  2.指针变量(接收注册中断传递过来的参数)
 * 返回值：IRQ_HANDLED(正常)；IRQ_NONE(异常)
 **********************************************************/
static irqreturn_t YS17_interrupt(int irq,void *dev_id)
{
	sensor_state = gpio_get_value(YS17_GPIO);
	buffer = &sensor_state;
	return IRQ_RETVAL(IRQ_HANDLED);
}
/**********************************************************
 * 函数名: YS17_read
 * 描述: 从内核空间读取数据到用户空间
 * 参数: 1.文件指针 2.缓冲区 3.读取字节数大小 4.读位置
 * 返回值：字节数 or -EFAULT
 **********************************************************/
static ssize_t YS17_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long err;
	if(NULL == buffer)
		return 0;
	err = copy_to_user(buf,(const void *)buffer,sizeof(sensor_state));
	return err ? -EFAULT : sizeof(sensor_state);
}

/*定义初始化硬件操作接口对象*/
static struct file_operations YS17_fops = {
	.owner   = THIS_MODULE,
	.read    = YS17_read,
};
/**********************************************
 *	函数名：creat_dev_node
 *	描述  ：注册字符设备
 *	返回值：成功：0	出错:非0	
 ***********************************************/
static int creat_dev_node(void)
{
	int res;

	alloc_chrdev_region(&YS17_devno, 0, 1, DEVICE_NAME);//动态分配设备号   
	
	cdev_init(&YS17_cdev,&YS17_fops);//初始化字符设备对象

	res = cdev_add(&YS17_cdev,YS17_devno,1);//安装字符设备驱动到内核
	if(res){
		printk("The YS17 device register failed!\n");
	}
	YS17_class = class_create(THIS_MODULE,"YS17");//创建设备类

	device_create(YS17_class,NULL,YS17_devno,NULL,DEVICE_NAME);//创建设备文件
	printk(DEVICE_NAME " initialized\n");
	return 0;
}
/**********************************************
 *	函数名：YS17_probe_func
 *	描述  ：驱动加载函数，负责insmod后的加载工作
 ***********************************************/
static int YS17_probe_func(struct platform_device *pdev)
{
	int ret = 0;
	/* 获得GPIO引脚 */
	struct device *dev = &pdev->dev;
	struct device_node *of_node = dev->of_node;
	if(!of_node){
		return -ENODEV;
	}
	YS17_GPIO = of_get_named_gpio(of_node,"p4_gpio0",0);
	
	if(!gpio_is_valid(YS17_GPIO))		//判断是否有效
		return -ENODEV;

	/*设置引脚为输入引脚*/
	gpio_request(YS17_GPIO,"YS17");
	gpio_direction_input(YS17_GPIO);
	ret = request_irq(gpio_to_irq(YS17_GPIO),YS17_interrupt,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,"YS17",NULL);
	if(ret)
		printk("IRQ not free!\n");

	/*创建设备节点操作*/
	ret = creat_dev_node();
	if(ret)
		return ret;
	return 0;
}
/**********************************************
 *	函数名：YS17_remove_func
 *	描述  ：驱动卸载函数，负责rmmod后的处理工作
 ***********************************************/
static int YS17_remove_func(struct platform_device *pdev)
{
	/*释放中断和GPIO*/
	free_irq(gpio_to_irq(YS17_GPIO), NULL);
	gpio_free(YS17_GPIO);
	/*卸载字符设备对象*/
	cdev_del(&YS17_cdev);
	/*删除设备文件*/
	device_destroy(YS17_class,YS17_devno);
	/*删除设备类*/
	class_destroy(YS17_class);
   /*释放设备号*/
    unregister_chrdev_region(YS17_devno, 1);
	return 0;
}

/*p4端口的设备信息描述*/
static struct of_device_id p4_of_match[] = {
	{ .compatible = "fsl,p4-ports"},
	{},
};
/*struct platform_driver结构体赋值*/
static struct platform_driver p4_device_driver = {
	.probe	= YS17_probe_func,						//驱动加载函数
	.remove = YS17_remove_func,						//驱动卸载函数
	/*struct device_driver结构体赋值*/
	.driver	= {
		.name = DEVICE_NAME,						//设备驱动程序的名称
		.owner = THIS_MODULE,						//设备驱动程序所有者	
		.of_match_table = of_match_ptr(p4_of_match),//驱动程序匹配的设备信息
	},
};
module_platform_driver(p4_device_driver);//驱动程序的入口出口

MODULE_LICENSE("GPL"); 


