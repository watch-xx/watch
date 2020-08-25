/*
 * ads1282_kernel.c *
 * Created on: Mar 24, 2018
 * Author: root
 * 2017-08-12
 * revise double buff (** case 1: if (irq_cnt<=con_num)***)lost one point data situation
 * 2017-08-14 add get time
 * 2017-08-18 ASYNC I/O
 * 2017-08-25 文件头格式更改为：IP：采样长度：采样率：direction X X X X X X:...........(80字节在用户态生成)
 * 开始时间：结束时间：..................(48字节在核态生成时间戳)
 * 修改08-14添加的时间戳获取代码位于ads1282_kernel.ko内核驱动代码copy_to_user代码段（此方法获取的时间为应用层读取内核时获取的时间，
 * 并不是A/D采样的时间，逻辑存在问题）
 * 2017-08-27 mmap no completed
 * 2018-01-26 修改为单通道采集
 * 2018-03-17 add ioctl
 ==============================================================================*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include "ads1282.h"
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <mach/iomux-mx6q.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spi/spidev.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <asm/mach/arch.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <mach/iomux-v3.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/mm.h>
#include <mach/memory.h>
#include <asm-generic/gpio.h>
#include <linux/ioctl.h>
#include <linux/gfp.h>

#define DEVICE_NAME "i.mx_ads1282"
#define DRIVER_DESC "ads1282_driver_desc"
#define DRIVER_LICENSE "GPL"

#define SPIDEV_ADS1282_MAJOR    0//主设备号
#define N_SPI_MINORS    255 //设备数最大为256

//ads1282 初始化引脚
#define ad_reset  IMX_GPIO_NR(3,22)
#define	ad_pwnd   IMX_GPIO_NR(3,23)
#define	ad_sync   IMX_GPIO_NR(1,30)
#define	ad_sw_en  IMX_GPIO_NR(3,29)
#define	ad_sw_A0  IMX_GPIO_NR(7,7)
#define	ad_sw_A1  IMX_GPIO_NR(5,2)
#define adx_drdy  IMX_GPIO_NR(7,6)

#define ADX_DRDY_IRQ  gpio_to_irq(adx_drdy)

//定义幻数
#define ADS1282_MAGIC	'x'//小写字母
//初始化内存参数配置
#define IOCINIT			_IOW(ADS1282_MAGIC,0,int)
//用户态————>>>核态写参数进来
#define IOCWREG			_IOW(ADS1282_MAGIC,1,int)

#define IOC_MAXNR		2

struct msg{

	u8				sampling_rate;
    u8 				gain;
	unsigned int 	data;
};
static volatile int length;
static int current_buf;
static volatile int read_able=0;//作为读进程唤醒条件
static int irq_cnt=0;//记录中断进入的次数
static int dev_major=SPIDEV_ADS1282_MAJOR;
u8 *ad_data_1,*ad_data_2;//全局指针
char Time_Str[48]={0};//save timestamp

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

struct ADC_DEV {
            wait_queue_head_t wait;//定义等待队列头，用来同步中断写数据
            struct spi_device *spi;
            struct cdev cdev;
            struct mutex buf_lock;//
            struct mutex time_lock;
            struct fasync_struct *async_queue;//
            dev_t devt;//设备号
            spinlock_t lock;//自旋锁//暂时没有用到
            u8 ADS1282_GAIN;//ad增益
            u8 ADS1282_SAMPLING_RATE;//ad采样率
          };
struct ADC_DEV IMX_ADS1282;
struct ADC_DEV *adc;
struct timex txc;
struct timex t_1;
struct timex t_2;
struct rtc_time tm;
struct class *spi_ads1282_class;
struct cdev *ads1282;
struct work_struct  ads1282_wq;//定义一个中断工作队列
void ads1282_wq_func(struct work_struct *work);//定义一个中断底半部处理函数

static void AD_CH_X(void)
{
	gpio_direction_output(ad_sw_A0,0);
	gpio_direction_output(ad_sw_A1,0);
}

static void AD_CH_Y(void)
{
	gpio_direction_output(ad_sw_A0,0);
	gpio_direction_output(ad_sw_A1,1);

}

static void AD_CH_Z(void)
{
	gpio_direction_output(ad_sw_A0,1);
	gpio_direction_output(ad_sw_A1,0);

}
static void SPI_ADS1282_WriteBytes(int cs_num,u8 TxData)
{
	switch(cs_num)
	{
	case 1:AD_CH_X();
	break;
	case 2:AD_CH_Y();
	break;

	case 3:AD_CH_Z();
	break;

	default : AD_CH_X();
	break;
	}

	spi_write(adc->spi,&TxData,1);
}

static void SPI_ADS1282_ReadWrite4Bytes(int cs_num,u8 *RxData)
{
	switch(cs_num)
		{
		case 1:AD_CH_X();
		break;
		case 2:AD_CH_Y();
		break;

		case 3:AD_CH_Z();
		break;

		default : AD_CH_X();
		break;
		}
	spi_read(adc->spi,RxData,4);
}


static void Write_ADC_REG(unsigned int start_addr,unsigned int len,unsigned char* wtptr)
{

    unsigned char command1;
    unsigned char command2;
    command1=0x40+(0x1F & start_addr);
    command2=0x00+(0x1F & (len-1));
    SPI_ADS1282_WriteBytes(1,command1);
    SPI_ADS1282_WriteBytes(2,command1);
    SPI_ADS1282_WriteBytes(3,command1);
//================================
	udelay(10);	//睡眠微秒函数		/* 24 f_CLK cycles required between each byte transaction */
	/* send command2 */

	SPI_ADS1282_WriteBytes(1,command2);
	SPI_ADS1282_WriteBytes(2,command2);
	SPI_ADS1282_WriteBytes(3,command2);
	while(len)//循环写入寄存器中	/* stay in loop until length = 0 */
	{
		udelay(10);
		SPI_ADS1282_WriteBytes(1,*wtptr);
		SPI_ADS1282_WriteBytes(2,*wtptr);
		SPI_ADS1282_WriteBytes(3,*wtptr++);
//===========================================
		len--;					/* reduce string length count by 1*/
	}

}

static int  imx_ads1282_init(void)
{

    u8  ADC_CFG[10];
    current_buf=1;

    gpio_direction_output(ad_sw_en,1);
    udelay(1000);

    gpio_direction_output(ad_pwnd,1);
    udelay(1000);

    gpio_direction_output(ad_reset,1);
    udelay(1000);
    gpio_set_value(ad_reset,0);
    udelay(1000);
    gpio_set_value(ad_reset,1);
    udelay(1000);


   gpio_direction_output(ad_sync,1);
   udelay(500);
   gpio_set_value(ad_sync,0);
   udelay(500);
   gpio_set_value(ad_sync,1);
   udelay(500);
   gpio_set_value(ad_sync,0);
   udelay(1000);
   gpio_set_value(ad_sync,1);
   udelay(1000);

   ADC_CFG[0]=ADC_SYNC_P & ADC_MODE_H & IMX_ADS1282.ADS1282_SAMPLING_RATE & ADC_FIR_LIN & ADC_FILTER_SINC_LPF;
   ADC_CFG[1]=ADC_MUX_1 & ADC_CHOP_EN & IMX_ADS1282.ADS1282_GAIN;

   SPI_ADS1282_WriteBytes(1,ADC_SDATAC);
   SPI_ADS1282_WriteBytes(2,ADC_SDATAC);
   SPI_ADS1282_WriteBytes(3,ADC_SDATAC);

   msleep(20);

   Write_ADC_REG(ADC_ADDR_CFG0,2,ADC_CFG);

   msleep(20);

   SPI_ADS1282_WriteBytes(1,ADC_RDATAC);
   SPI_ADS1282_WriteBytes(2,ADC_RDATAC);
   SPI_ADS1282_WriteBytes(3,ADC_RDATAC);

   msleep(5);
   gpio_set_value(ad_sync,0);
   udelay(500);
   gpio_set_value(ad_sync,1);
   udelay(500);
   gpio_set_value(ad_sync,0);
   udelay(500);
   gpio_set_value(ad_sync,1);

   return 0;
}
/*
static long int imx_adc_convert(signed long int data)//对原始数进行初步转换
{
	signed long int u=0x00000000;
	if((data&0x40000000)==0x00000000)//正数
	{
	  u=data&0x7fffffff;
		return u;
	}
	else if((data&0x40000000)==0x40000000)//负数
  {
    u=data|0x80000000;
		u=data^0xffffffff;
		u=u+0x00000001;
		u&=0x7fffffff;
		return(-1*u);
	}
	else
		return 0;
}
*/
static int ads1282_fasync(int fd,struct file *filp,int mode)
{
	return fasync_helper(fd,filp,mode,&adc->async_queue);
}
//=================================================================================================================
//在此函数中调用SPI 读写 对ADC进行配置
static long imx_ads1282_ioctl(struct file *filp,unsigned int cmd ,unsigned long arg)
{

	int ret=0;
	struct msg my_msg;
	printk("[%s]\n",__func__);
//检查设备类型
	if(_IOC_TYPE(cmd) !=ADS1282_MAGIC) {
		pr_err("[%s] command type [%c] error!\n",__func__,_IOC_TYPE(cmd));
		return -ENOTTY;
	}
//检查序数
	if(_IOC_NR(cmd)> IOC_MAXNR) {
		pr_err("[%s] command number [%d] exceeded!\n",__func__,_IOC_TYPE(cmd));
		return -ENOTTY;
	}
//检查访问类型
	if(_IOC_DIR(cmd) & _IOC_READ)
		ret=!access_ok(VERIFY_WRITE,(void __user *)arg,_IOC_SIZE(cmd));

	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		ret=!access_ok(VERIFY_READ,(void __user *)arg,_IOC_SIZE(cmd));
	if(ret)
		return -EFAULT;

	switch(cmd)	{
	case IOCINIT :
		printk("\r\n ioctl init\r\n");
		ret=copy_from_user(&my_msg,(struct msg __user *)arg,sizeof(my_msg));
		if(ret)
			return -EFAULT;
		length=my_msg.data;
		printk("kernel length=%d\r\n",length);
		ad_data_1=kzalloc(length*4*3+48,GFP_KERNEL);
		ad_data_2=kzalloc(length*4*3+48,GFP_KERNEL);
		 break;
	case IOCWREG :
		printk("\r\n ioctl write\r\n");
		ret=copy_from_user(&my_msg,(struct msg __user *)arg,sizeof(my_msg));
		if(ret)
			return -EFAULT;
		 IMX_ADS1282.ADS1282_GAIN =my_msg.gain;
		 IMX_ADS1282.ADS1282_SAMPLING_RATE =my_msg.sampling_rate;
		 printk("ADS1282_GAIN=%x,ADS1282_SAMPLING_RATE=%x \r\n",my_msg.gain,my_msg.sampling_rate);
		 ret=imx_ads1282_init();
		 if(ret==0)
		{
			printk("ads1282 init success!!!!\n");
		}
		else
			return -EFAULT;
		 enable_irq(ADX_DRDY_IRQ);//开启中断
		break;

	default :
		 printk("error cmd!!!\n");
		 return -EINVAL;

	}
    return 0;
}
//====================================================================================
//loop length times then exchange buf
//====================================================================================
 void ads1282_wq_func(struct work_struct *work)//中断底半部处理函数
{
	 mutex_lock(&adc->buf_lock);
	 switch(current_buf)
	 {
		 case 1:
				 if (irq_cnt<=length)
				 {
					SPI_ADS1282_ReadWrite4Bytes(1,&ad_data_2[48+(irq_cnt-1)*4]);
					SPI_ADS1282_ReadWrite4Bytes(2,&ad_data_2[48+(irq_cnt-1)*4+length*4]);
					SPI_ADS1282_ReadWrite4Bytes(3,&ad_data_2[48+(irq_cnt-1)*4+length*4*2]);
				 }
				 else
				 {
					current_buf=2;
					irq_cnt=0;
					read_able=1;
					do_gettimeofday(&(t_2.time));
					sprintf(Time_Str,"%d.%d:%d.%d:",(int)t_1.time.tv_sec,(int)t_1.time.tv_usec,(int)t_2.time.tv_sec,(int)t_2.time.tv_usec);
					memcpy(ad_data_2,&(Time_Str[0]),sizeof(Time_Str));
					wake_up_interruptible(&IMX_ADS1282.wait);
					kill_fasync(&adc->async_queue,SIGIO,POLL_IN);
				 }
		 break;
		case 2:
				if (irq_cnt<=length)
				{
					SPI_ADS1282_ReadWrite4Bytes(1,&ad_data_1[48+(irq_cnt-1)*4]);
					SPI_ADS1282_ReadWrite4Bytes(2,&ad_data_1[48+(irq_cnt-1)*4+length*4]);
					SPI_ADS1282_ReadWrite4Bytes(3,&ad_data_1[48+(irq_cnt-1)*4+length*4*2]);
				}
				else
				{
					current_buf=1;
					irq_cnt=0;
					read_able=1;
					do_gettimeofday(&(t_1.time));
					sprintf(Time_Str,"%d.%d:%d.%d:",(int)t_2.time.tv_sec,(int)t_2.time.tv_usec,(int)t_1.time.tv_sec,(int)t_1.time.tv_usec);
					memcpy(ad_data_1,&(Time_Str[0]),sizeof(Time_Str));
					wake_up_interruptible(&IMX_ADS1282.wait);
					kill_fasync(&adc->async_queue,SIGIO,POLL_IN);
				}
		break;
	}
	 mutex_unlock(&adc->buf_lock);
	 enable_irq(ADX_DRDY_IRQ);
}
////中断函数中利用双缓存对AD连续转换的数据进行存储
static  irqreturn_t ads_1282_drdy_handler(int irq,void *dev_id)
{
	disable_irq_nosync(ADX_DRDY_IRQ);//关中断
    irq_cnt++;
  	schedule_work(&ads1282_wq);
  	return IRQ_HANDLED;
 }

static  ssize_t imx_ads1282_read(struct file *filp,char __user *buf,size_t size,loff_t *ppos )
{
	int ret=0;
	unsigned long flag;
	wait_event_interruptible(IMX_ADS1282.wait,read_able);
	local_irq_save(flag);//off all irq
	mutex_lock(&adc->buf_lock);
	switch(current_buf)
	{
		case 1:
			if(copy_to_user(buf,ad_data_1,length*4*3+48))
			{
				ret= -EFAULT;
			}
		break;
		case 2:
			if(copy_to_user(buf,ad_data_2,length*4*3+48))
			{
				ret= -EFAULT;
			}
		break;
	}
	mutex_unlock(&adc->buf_lock);
	local_irq_restore(flag);
	read_able=0;
	return ret;
}
//================================================================
static int imx_ads1282_release(struct  inode *inode,struct file *filp)
{
	ads1282_fasync(-1,filp,0);
	disable_irq_nosync(ADX_DRDY_IRQ);//关中断
    printk("ads1282 closed\n");
    return 0;
}
static int imx_ads1282_open(struct inode *inode, struct file *filp)
{
    int ret=0;
	printk("\r\n open ads1282 device successful ! \r\n");
	init_waitqueue_head(&(IMX_ADS1282.wait));//初始化等待队列
	return ret;
}
static const struct file_operations imx_ads1282_fops={
    .owner = THIS_MODULE,
    .unlocked_ioctl = imx_ads1282_ioctl,
    .open =imx_ads1282_open,
    .read =imx_ads1282_read,
    .release= imx_ads1282_release,
//  .poll= imx_ads1282_poll,
    .fasync=ads1282_fasync,
//  .mmap=ads1282_mmap,
   };
//====================================================================
static const struct spi_device_id ads1282_ids[]={
    {"ads1282",1},
    {},
};
MODULE_DEVICE_TABLE(spi,ads1282_ids);

static int  ads1282_probe(struct spi_device *spi)
{
	int ret;
	mxc_iomux_v3_setup_pad(MX6Q_PAD_SD3_DAT2__GPIO_7_6);
	gpio_request(adx_drdy,"adx_drdy");
	gpio_direction_input(adx_drdy);
	gpio_get_value_cansleep(adx_drdy);
	gpio_free(adx_drdy);
	ret=gpio_request(ad_sw_en,"ad_sw_en");
	if (ret<0)
	{
		printk("ERROR :can't request ad_sw_en gpio %d!!\n",ret);
		return 0;
	}

	ret=gpio_request(ad_pwnd,"ad_pwnd");
	if (ret<0)
	{
		printk("ERROR :can't request ad_pwnd gpio %d!!\n",ret);
		return 0;
	}

	ret=gpio_request(ad_reset,"ad_reset");
	if (ret<0)
	{
		printk("ERROR :can't request reset gpio %d!!\n",ret);
		return 0;
	}

	ret=gpio_request(ad_sync,"ad_sync");
	if (ret<0)
	{
		printk("ERROR :can't request ad_sync gpio %d!!\n",ret);
		return 0;
	}

	ret=gpio_request(ad_sw_A0,"ad_sw_A0");
	if (ret<0)
	{
		printk("ERROR :can't request ad_sync gpio %d!!\n",ret);
		return 0;
	}

	ret=gpio_request(ad_sw_A1,"ad_sw_A1");
	if (ret<0)
	{
		printk("ERROR :can't request ad_sync gpio %d!!\n",ret);
		return 0;
	}

	adc =kzalloc(sizeof(struct ADC_DEV),GFP_KERNEL);//为设备描述结构分配内存
	if(!adc)
		return -ENOMEM;
	ret=request_irq(ADX_DRDY_IRQ,ads_1282_drdy_handler,IRQF_TRIGGER_FALLING,DEVICE_NAME,adc);
	if(ret)
	{
		printk("ERROR :can't request irq \r\n");
	}
//  cpu_core->bits[0]=0x0008;
//  cpumask_copy(cpu_core->bits[0],cpu_online_mask);
//	irq_set_affinity(ADX_DRDY_IRQ,cpumask_of(8));//ads1282_kernel: Unknown symbol irq_set_affinity (err 0)
//												      insmod: can't insert 'ads1282_kernel.ko': unknown symbol in module or invalid parameter
//
	INIT_WORK(&ads1282_wq,ads1282_wq_func);//初始化工作队列
	printk("irq= %u\n",ADX_DRDY_IRQ);
	disable_irq_nosync(ADX_DRDY_IRQ);//关中断
//================test time =========================================
	do_gettimeofday(&(txc.time));
	rtc_time_to_tm(txc.time.tv_sec,&tm);
	printk("UTC Time:%d-%d-%d %d:%d:%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	printk("UTC Time_Tamp:%d.%d\n",(int)txc.time.tv_sec,(int)txc.time.tv_usec);
//===================================================================
	adc->spi=spi;
//====================================================================
	cdev_init(&adc->cdev,&imx_ads1282_fops);//初始化adc->cdev结构
	adc->cdev.owner=THIS_MODULE;
//	adc->cdev.ops=&imx_ads1282_fops;
	adc->devt=MKDEV(dev_major,0);
	if (dev_major)//静态申请设备号
		ret=register_chrdev_region(adc->devt,1,"ads1282");
	else
	{//动态分配设备号
		ret=alloc_chrdev_region(&adc->devt,0,1,"ads1282");
		dev_major=MAJOR(adc->devt);
	}
	adc->devt=MKDEV(dev_major,0);
	cdev_add(&adc->cdev,adc->devt,1);//注册字符设备
	printk("i.mx ads1282 add\n");
//=================================================================
	spi_ads1282_class=class_create(THIS_MODULE,"ads1282");//在自动创建设备节点之前，先创建sysfs下对应的类
	if(IS_ERR(spi_ads1282_class))
	{
		unregister_chrdev_region(adc->devt,1);//卸载设备号
	}
//================================================================
	device_create(spi_ads1282_class,NULL,adc->devt,NULL,"ads1282");//在sysfs中注册该设备，创建相应的设备节点
//=================================================================
	mutex_init(&adc->buf_lock);
	spin_lock_init(&adc->lock);
	return 0;
}

static int ads1282_remove(struct spi_device *spi)
{

	 cdev_del(&adc->cdev);
	 free_irq(ADX_DRDY_IRQ,adc);
	 kfree(adc);
	 unregister_chrdev_region(adc->devt,1);//卸载设备号
	 device_destroy(spi_ads1282_class, adc->devt);
	 class_destroy(spi_ads1282_class);
	 kfree(ad_data_1);
	 kfree(ad_data_2);
	 ad_data_1=NULL;
	 ad_data_2=NULL;
	 adc=NULL;
	 gpio_free(ad_sw_en);
	 gpio_free(ad_pwnd);
	 gpio_free(ad_reset);
	 gpio_free(ad_sync);
	 gpio_free(ad_sw_A0);
	 gpio_free(ad_sw_A1);
	 printk("i.mx ads1282 successfuly remove\n");
	 return 0;
}
static struct spi_driver ads1282_driver={
        .driver={
            .name="ads1282",
            .bus=&spi_bus_type,
            .owner=THIS_MODULE,
        },
        .id_table=ads1282_ids,
        .probe= ads1282_probe,
        .remove=__devexit_p(ads1282_remove),
};
//===========================================================================
static int __init ads1282_module_init(void)
{
    int ret;
    ret=spi_register_driver(&ads1282_driver);
    printk(DEVICE_NAME"\tinitalized\n");
    return ret;
}
static void __exit ads1282_module_exit(void)
{
    spi_unregister_driver(&ads1282_driver);
    printk("i.mx ads1282 successfuly exit\n");
}
module_init(ads1282_module_init);
module_exit(ads1282_module_exit);


