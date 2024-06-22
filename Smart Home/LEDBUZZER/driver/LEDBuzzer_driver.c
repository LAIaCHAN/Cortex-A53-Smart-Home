//LED蜂鸣器模块
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/sched.h>  
#include <linux/wait.h>  
#include <linux/irqreturn.h>  
#include <linux/interrupt.h>  
#include <asm/uaccess.h>  
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/unistd.h>

#define DEVICE_NAME     "LEDBuzzer"		//设备节点名字
#define DEV_MAJOR        0			//主设备号    0:动态获取设备节点号



#define GPIO_NAME0    "led"
#define GPIO_NAME1    "buzzer"
/*ioctl操作命令*/
#define IOCTL_LED_ON		0	//灯开
#define IOCTL_LED_OFF		1	//灯灭
#define IOCTL_BUZZER_OFF	3	//蜂鸣器关
#define IOCTL_BUZZER_ON		4	//蜂鸣器开
static unsigned int gpio_table [2] = {};
dev_t devno;
static struct cdev mycdev ;
static struct class *myclass = NULL;

static long gpio_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
    //if(arg>3){
      //  return -EINVAL;
    //}
    switch(cmd) {
    case IOCTL_LED_ON:			//led亮
        gpio_set_value(gpio_table[1], 1);
        break;
    case IOCTL_LED_OFF:			//led灭
        gpio_set_value(gpio_table[1], 0);
        break;	
    case IOCTL_BUZZER_ON:		//蜂鸣器响
        gpio_set_value(gpio_table[0], 1);
        break;
    case IOCTL_BUZZER_OFF:		//蜂鸣器关闭
        gpio_set_value(gpio_table[0], 0);
        break;
    default:
        return -EINVAL;
    }
	return 0;
}
/*struct file_operations赋值，重载ioctl函数*/
static struct file_operations gpio_fops = {
    .owner        = THIS_MODULE,
    .unlocked_ioctl = gpio_ioctl,
};

static int creat_dev_node(void)
{
    int ret;    
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
    cdev_init(&mycdev, &gpio_fops);
    ret = cdev_add(&mycdev, devno, 1);
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

static int gpio_probe_func(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    struct device_node *of_node = dev->of_node;
    if(!of_node) {
        return -ENODEV;
    }
/*获得GPIO引脚*/
    gpio_table[0] = of_get_named_gpio(of_node, "p8_gpio0", 0);
    gpio_table[1] = of_get_named_gpio(of_node, "p8_gpio1", 0);

/*设置引脚为输出引脚*/
    gpio_request(gpio_table[0], GPIO_NAME0);
    gpio_direction_output(gpio_table[0],0);   

    gpio_request(gpio_table[1], GPIO_NAME1);
    gpio_direction_output(gpio_table[1],0);   

/*创建设备节点*/
    ret = creat_dev_node(); 
    if(ret)
        return ret;

    return 0;
}

static int gpio_remove_func (struct platform_device *pdev)
{
	gpio_set_value(gpio_table[0], 0);
	gpio_set_value(gpio_table[1], 0);
	/*释放gpio*/
	gpio_free(gpio_table[0]);
	gpio_free(gpio_table[1]);
	/*删除设备节点*/
	cdev_del(&mycdev);
	device_destroy(myclass,devno);
	class_destroy(myclass);
	/*释放设备号*/
	unregister_chrdev_region(devno, 1);
	return 0;
}
/*P8端口的设备信息描述*/
static struct of_device_id port_p8_of_match[] = {
        { .compatible = "fsl,p8-ports"},
        { },
};
/*struct platform_driver结构体赋值*/
static struct platform_driver gpio_device_driver = {
    .probe        = gpio_probe_func,			//重载probe函数
    .remove       = gpio_remove_func,			//重载remove函数
    .driver       = {
        .name    = DEVICE_NAME,				//设备驱动程序的名称
        .owner   = THIS_MODULE, 			//设备驱动程序所有者
        .of_match_table = of_match_ptr(port_p8_of_match),//驱动程序匹配的设备信息
    }
};
module_platform_driver(gpio_device_driver);		//驱动程序入口与出口

MODULE_LICENSE("GPL");                              

