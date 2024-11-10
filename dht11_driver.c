#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TUAN");
MODULE_DESCRIPTION("Gpio driver");

static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;
#define DRIVER_NAME "dht11_driver"
#define DRIVER_CLASS "MyModuleClass"
#define GPIO_PIN 516


static int dht11_read_data(unsigned char *data){
        int i,j;
	int timeout;
	if(gpio_direction_output(GPIO_PIN,1)){
		printk("Cannot set GPIO 4 to output!\n");
		return -1;
	}	
	gpio_set_value(GPIO_PIN,0);
	mdelay(20);
	gpio_set_value(GPIO_PIN,1);
	udelay(50);
        if(gpio_direction_input(GPIO_PIN)){
		printk("Cannot set gpio4 to input!\n");
	        return -1;
	}
	udelay(40);

	timeout = 1000;
	while(gpio_get_value(GPIO_PIN)==0&&timeout--){
		udelay(1);
	}
	if(timeout==0){
		printk("gpio_value not change 0\n");
		return -1;
	}
	timeout=1000;
	while(gpio_get_value(GPIO_PIN)==1&&timeout--){
		udelay(1);
	}
	if(timeout==0){
		printk("gpio_value not change 1\n");
		return -1;
	}

	printk("dht11 already setn data\n");
         for(i=0;i<5;i++){
	          data[i] = 0;
		  for(j=0;j<8;j++){
			 timeout = 1000;
                 	 while(gpio_get_value(GPIO_PIN)==0&&timeout--){
				       udelay(1);
			 }
	                  udelay(40);
		          if(gpio_get_value(GPIO_PIN)==1){
				    data[i] |= (1<<(7-j));
			  }
			  timeout=1000;
			  while(gpio_get_value(GPIO_PIN)==1&&timeout--){
				    udelay(1);
			   }
		  }
	  }
	
	return 0;
}

static ssize_t driver_read(struct file *File,char *user_buffer,size_t count,loff_t *offs){
	int to_copy,not_copied,delta;
	unsigned char data[5];

	if(dht11_read_data(data)){
		return -EIO;
	}
	
	int hum = data[0];
	int tem = data[2];

	printk("hum:%d%%, tem: %d\n",hum,tem);

	to_copy = umin(count,sizeof(data));

	
	not_copied = copy_to_user(user_buffer,data,to_copy);

	delta = to_copy - not_copied;

	return delta;
}





static int driver_open(struct inode *device_file,struct file *instance){
	printk("dev_nr - open was called!\n");
	return 0;
}

static int driver_close(struct inode *device_file,struct file *instance){
	printk("dev_nr - close was called\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read
};





static  int __init ModuleInit(void){
	printk("hello, kernel!\n");
	if(alloc_chrdev_region(&my_device_nr,0,1,DRIVER_NAME)<0){
		printk("Device Nr. could not be allocated!\n");
		return -1;
	}
	printk("read_write - Device Nr. Major:%d, Minor: %d was registered!\n",my_device_nr>>20,my_device_nr && 0xfffff);

	if((my_class = class_create(DRIVER_CLASS))== NULL){
		printk("Device class can not e created!\n");
		goto ClassError;
	}
	if(device_create(my_class,NULL,my_device_nr,NULL,DRIVER_NAME) == NULL){
		printk("Can not create device file!\n");
		goto FileError;
	}

	cdev_init(&my_device,&fops);

	if(cdev_add(&my_device,my_device_nr,1) == -1){
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}


	if(gpio_request(GPIO_PIN,"rpi-gpio-516")){
		printk("Can not allocate GPIO 4\n");
		goto AddError;
	}     

	return 0;

AddError:
	device_destroy(my_class,my_device_nr);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev(my_device_nr,DRIVER_NAME);
	return -1;
}

static void __exit ModuleExit(void){
      gpio_free(GPIO_PIN);
      cdev_del(&my_device);	
      device_destroy(my_class,my_device_nr);
      class_destroy(my_class);
      unregister_chrdev(my_device_nr,DRIVER_NAME);
      printk("goodbye, kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);
