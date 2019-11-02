#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>	   //for file_operations struct
#include <linux/random.h>  //for get_random_bytes
#include <linux/slab.h>	   //for kmalloc/kfree functions
#include <linux/uaccess.h> //for copy_to_user


//Define 
#define DRIVER_AUTHOR "Huynh Van Tu - Nguyen Xuan Vy"
#define DRIVER_DESC "Module generator random number"

#define DEVICE_NAME "my_rand"
#define CLASS_NAME "my_class"

//The prototype functions 
static int my_rand_open(struct inode *, struct file *);
static int my_rand_release(struct inode *, struct file *);
static ssize_t my_rand_read(struct file *, char *, size_t, loff_t *);

//Value
static int major_number;
static int device_open = 0;
static struct class* my_rand_class =NULL;
static struct device* my_rand_device = NULL;

//Struct 
static struct file_operations fops =
{
	.open = my_rand_open,
	.release = my_rand_release,
	.read = my_rand_read,
};

static int __init my_rand_init(void)
{
	printk(KERN_INFO "Initializing...\n");
	
	// Try to allocate major number for device
	major_number = register_chrdev(0,DEVICE_NAME, &fops);
	if(major_number < 0 )
	{
		printk(KERN_ALERT "Faild to register a major number\n");
		return major_number;
	}

	printk(KERN_INFO  "Registered a major number %d \n", major_number);
	
	// Register the device class
	my_rand_class = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(my_rand_class))
	{
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT " Faild to register device class\n");
		return PTR_ERR(my_rand_class);
	}
	printk(KERN_INFO "Device class registered\n");

	//  Register device
	my_rand_device = device_create(my_rand_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if(IS_ERR(my_rand_device))
	{
		class_destroy(my_rand_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Faild to create the device\n");
		return PTR_ERR(my_rand_device);
	}
	printk(KERN_INFO "Device created\n");
	return 0;
	
}

static void __exit my_rand_exit(void)
{
	device_destroy(my_rand_class, MKDEV(major_number, 0));
	class_unregister(my_rand_class);
	class_destroy(my_rand_class);
	unregister_chrdev(major_number, DEVICE_NAME);

	printk(KERN_INFO "Goodbye!\n");
}


static int my_rand_open(struct inode *inodep, struct file *filep)
{
	device_open++;
	printk(KERN_INFO "Device has been opened %d time(s) \n", device_open);
	return 0;
}	


static int my_rand_release(struct inode *inodep, struct file *filep)
{
	device_open--;
	printk(KERN_INFO "Device is released\n");
	return 0;
}

static ssize_t my_rand_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int status;
	char *arr = kmalloc(len, GFP_KERNEL);
	
	get_random_bytes(arr, len);
	
	status = copy_to_user(buffer, arr, len);
	kfree(arr);

	if(status == 0)
	{
		printk(KERN_INFO "Sent random number to user\n");
		return 0;
	}
	else
	{
		printk(KERN_INFO "Failed to send random number to user\n");
		return -EFAULT;
	}
}





module_init(my_rand_init);
module_exit(my_rand_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
