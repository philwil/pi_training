
/*  
 *  my_driver.c - Demonstes a basic device driver
 */

#include <linux/module.h>	/* Needed by all modules */
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/device.h>  /* driver_class */

#include <asm/uaccess.h>	/* copy_*_user */

#define DRIVER_NAME "my_driver"

struct driver_dev {
  //struct scull_qset *data;  /* Pointer to first quantum set */
  int qsize;              /* the current quantum size */
  int asize;                 /* the current array size */
  unsigned long size;       /* amount of data stored here */
  unsigned int access_key;  /* used by uid and priv */
  struct semaphore sem;     /* mutual exclusion semaphore     */
  struct cdev cdev;	  /* Char device structure		*/
};

struct driver_dev *my_devices;
struct class *driver_class = NULL;
 
int driver_major=0;
int driver_minor=0;
int driver_no_devs=4;


module_param(driver_major, int, S_IRUGO);
module_param(driver_minor, int, S_IRUGO);
module_param(driver_no_devs, int, S_IRUGO);

/*
 * Open the device; in fact, there's nothing to do here.
 */
static int driver_open (struct inode *inode, struct file *filp)
{
         return 0;
}

/*
 * Closing is just as simpler.
 */
 static int driver_release(struct inode *inode, struct file *filp)
 {
        return 0;
 }

struct file_operations my_fops = {
	.owner =    THIS_MODULE,
	//.llseek =   driver_llseek,
	//.read =     driver_read,
	//.write =    driver_write,
	//.unlocked_ioctl = driver_ioctl,
	.open =     driver_open,
	.release =  driver_release,
};


void driver_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(driver_major, driver_minor);

	/* Get rid of our char dev entries */
	if (my_devices) 
	{
	    for (i = 0; i < driver_no_devs; i++) 
	    {
		//scull_trim(scull_devices + i);
		cdev_del(&my_devices[i].cdev);
		if(driver_class)
		{
		    device_destroy(driver_class
				   , MKDEV(driver_major, driver_minor+i));
		}
	    }
	    kfree(my_devices);
	}

        if(driver_class)
	{
	    class_destroy(driver_class);
	    driver_class =  NULL;
	}
	//scull_remove_proc();
 

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, driver_no_devs);

	/* and call the cleanup functions for friend devices */
	//scull_p_cleanup();
	//scull_access_cleanup();

}


/*
 * Set up the char_dev structure for this device.
 */
static int driver_setup_cdev(struct driver_dev *dev, struct class * class, int index)
{
	int err, devno = MKDEV(driver_major, driver_minor + index);
        struct device *device = NULL;

	cdev_init(&dev->cdev, &my_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &my_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding driver %d\n", err, index);
	else
		printk(KERN_NOTICE "NOTE added driver %d\n", index);

        if (!err)
	{
	    device=device_create(class, NULL, devno, NULL, DRIVER_NAME "%d", driver_minor+index);
	    if (IS_ERR(device))
	    {
		err = PTR_ERR(device);
		printk(KERN_WARNING "Error %d while trying to create %s%d\n",
		       err, DRIVER_NAME, driver_minor+index);
		cdev_del(&dev->cdev);
		return err;
	    }
	}
	return 0;
}


static int __init my_driver_init(void)
{
        int result=0;
        int i; 
	int err;
        dev_t dev;

	printk(KERN_INFO "Hello, my_driver\n");
        if(driver_major)
	{
            dev = MKDEV(driver_major, driver_minor);
            result = register_chrdev_region(dev, driver_no_devs, DRIVER_NAME);
	}  
	else
	{
            driver_minor = 0;
            result = alloc_chrdev_region(&dev, driver_minor, driver_no_devs, DRIVER_NAME);
	    driver_major = MAJOR(dev);
	}

	if (result < 0) 
	{
	    printk(KERN_NOTICE "driver: can't get major %d\n", driver_major);
	    return result;
	}

	printk(KERN_INFO "driver: got major %d\n", driver_major);

	driver_class = class_create(THIS_MODULE, DRIVER_NAME);
        if(IS_ERR(driver_class))
	  {
	    err = PTR_ERR(driver_class);
	    goto fail;
	  }
	my_devices = kmalloc(driver_no_devs * sizeof(struct driver_dev), GFP_KERNEL);
	if (!my_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}

	memset(my_devices, 0, driver_no_devs * sizeof(struct driver_dev));

     /* Initialize each device. */
	for (i = 0; i < driver_no_devs; i++) {
	  //my_devices[i].quantum = scull_quantum;
	  //	scull_devices[i].qset = scull_qset;
		sema_init(&my_devices[i].sem, 1);
		driver_setup_cdev(&my_devices[i], driver_class, i);
	}

        /* At this point call the init function for any friend device */
	dev = MKDEV(driver_major, driver_minor + driver_no_devs);

 fail:
	if(result != 0)
	{
	    driver_cleanup_module();
	}
	return result;
}

static void __exit my_driver_exit(void)
{
	printk(KERN_INFO "Goodbye, my_driver\n");
	driver_cleanup_module();
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
