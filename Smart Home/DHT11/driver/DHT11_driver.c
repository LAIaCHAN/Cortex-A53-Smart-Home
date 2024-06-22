//温湿度传感器模块

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>

#define DEVICE_NAME     "DHT11"		//设备节点名字
#define DEV_MAJOR        0			//主设备号    0:动态获取设备节点号
#define GPIO_NAME		"dht11"

static unsigned int gpio_table = 0;
static unsigned char check_flag;
static unsigned char dht11_data_buf[6];

static dev_t devno;
static struct cdev dht11_cdev;
static struct class *dht11_class = NULL;

/*读取一个byte*/
static unsigned char read_byte(void)
{
	int i = 0;
	int num ;
	unsigned char flag = 0;
	unsigned char data = 0;
	for(num = 0; num < 8; num++)
	{
		i = 0;
		while(!gpio_get_value(gpio_table))
		{
			udelay(10);
			i++;
			if(i > 10)
				break;
		}
		flag = 0x00;
		udelay(28);
		if(gpio_get_value(gpio_table))
		{
			flag = 0x01;
		}
		i = 0;
		while(gpio_get_value(gpio_table))
		{
			udelay(10);
			i++;
			if(i > 12)
				break;
		}
		data <<= 1;
		data |= flag;
	}
	return data;
}
/*读取数据*/
static void read_data(void)
{
	int i = 0;
	gpio_direction_output(gpio_table,0);//拉低20ms
	mdelay(20);
	gpio_set_value(gpio_table,1);//主机释放需13us
	gpio_direction_input(gpio_table);
	udelay(13);
	if(gpio_get_value(gpio_table) == 0)
	{
		while(!gpio_get_value(gpio_table))//等待为高电平
		{
			udelay(5);
			i++;
			if(i > 20)
			{
				printk("read_data %d err!\n",__LINE__);
				break;
			}
		}
		i = 0;
		while(gpio_get_value(gpio_table))//等待变为低电平
		{
			udelay(5);
			i++;
			if(i > 20)
			{
				printk("read_data %d err!\n",__LINE__);
				break;
			}
		}
		for(i = 0;i < 5;i++)//读取温湿度数据
			dht11_data_buf[i] = read_byte();

		/* 对读取到的数据进行校验 */
		dht11_data_buf[5] = dht11_data_buf[0] + dht11_data_buf[1] + dht11_data_buf[2] + dht11_data_buf[3];
		/*判断读到的校验值和计算的校验值是否相同*/ 
		if(dht11_data_buf[4] == dht11_data_buf[5])//相同则把标志值设为0xff
		{
			check_flag = 0xff;
		}
		else if(dht11_data_buf[4] != dht11_data_buf[5])//不同则把标志设为0
		{
			check_flag = 0x00;
			printk("dht11 check fail\n");
		}
	}
	

}
/**********************************************
 * 函数名: dht11_read
 * 描述:   从内核空间读取数据到用户空间
 * 参数:   1.文件指针 2.缓冲区 3.读取字节数大小 4.读位置
 * 返回值: 无
 * **********************************************/
static ssize_t dht11_read(struct file *file,char __user *buffer,size_t size ,loff_t *off)
{
	int ret;
	unsigned long flags;
	/*因为DHT11的时序要求很高，所以在读温湿度的时候要让代码进入临界区，防止内核调度和抢占*/
	local_irq_save(flags);
	read_data();
	local_irq_restore(flags);
	if(check_flag == 0xff)
	{
		/*将读取的温湿度数据拷贝到用户空间*/
		ret = copy_to_user(buffer,dht11_data_buf,sizeof(dht11_data_buf));
		if(ret < 0)
		{
			printk("copy to user err\n");
			return -EAGAIN;
		}
		else 
			return 0;
	}
	else 
		return -EAGAIN;
}
static struct file_operations dht11_fops = {
	    .owner        = THIS_MODULE,
		.read 		  = dht11_read,
};

/*创建设备节点*/
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
	cdev_init(&dht11_cdev, &dht11_fops);
	ret = cdev_add(&dht11_cdev, devno, 1);
	if(ret){
		printk("cdev_add(); failed! ret_num=%d\n",ret);
	}
	dht11_class = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(dht11_class)){
		printk("Err: failed in creating class.\n");
		return -1;
	}
	device_create(dht11_class,NULL, devno, NULL, DEVICE_NAME);
	return 0;
}

/**********************************************
 * 函数名：gpio_probe_func
 * 描述	 ：驱动加载函数，负责insmod后的加载工作
 * ***********************************************/
static int gpio_probe_func(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *of_node = dev->of_node;
	if(!of_node) {
		return -ENODEV;
	}
	/*获得GPIO引脚*/
	gpio_table = of_get_named_gpio(of_node, "p8_gpio0", 0);

	/*申请GPIO*/
	gpio_request(gpio_table, GPIO_NAME);


	/*创建设备节点*/
	ret = creat_dev_node(); 
	if(ret)
		return ret;

	return 0;
}
/**********************************************
 * 函数名：gpio_remove_func
 * 描述	 ：驱动卸载函数，负责rmmod后的处理工作
 * ***********************************************/
static int gpio_remove_func (struct platform_device *pdev)
{
	gpio_set_value(gpio_table, 0);
	/*释放gpio*/
	gpio_free(gpio_table);
	/*删除设备节点*/
	cdev_del(&dht11_cdev);
	device_destroy(dht11_class,devno);
	class_destroy(dht11_class);
	/*释放设备号*/
	unregister_chrdev_region(devno, 1);
	return 0;
}

/*P8端口的设备信息描述*/
static struct of_device_id port_p8_of_match[] = {
	{ .compatible = "fsl,p8-ports"},
	{ },
};

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
/*驱动属性：遵循协议、作者、驱动描述*/
MODULE_LICENSE("GPL");                              
MODULE_AUTHOR("KE");             
MODULE_DESCRIPTION("dht11");  
