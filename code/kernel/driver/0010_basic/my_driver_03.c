
/*  
 *  my_driver_03.c - Basic driver training
 * add ioctl
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

#define GET_DEV_CMD  001
#define SET_DEV_CMD  002

struct driver_dev {

   unsigned long size;       /* amount of data stored here */
   char *data;               /* data pointer */
   unsigned long read_ix;   /* read index */
   unsigned long write_ix;  /* write index */
   struct semaphore sem;     /* mutual exclusion semaphore     */
   struct cdev cdev;	  /* Char device structure		*/

};

struct driver_dev *my_devices;
struct class *driver_class = NULL;
 
int driver_major   = 0;
int driver_minor   = 0;
int driver_no_devs = 4;
int data_size      = 1024;

module_param(driver_major, int, S_IRUGO);
module_param(driver_minor, int, S_IRUGO);
module_param(driver_no_devs, int, S_IRUGO);
module_param(data_size, int, S_IRUGO);

/*
 * Open the device; in fact, there's nothing to do here.
 */
static int driver_open (struct inode *inode, struct file *filp)
{
    struct driver_dev *dev; /* device information */
  
    dev = container_of(inode->i_cdev, struct driver_dev, cdev);
    filp->private_data = dev; /* for other methods */
    
    /* now trim to 0 the length of the device if open was write-only */
    if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) 
      {
	if (down_interruptible(&dev->sem))
	  return -ERESTARTSYS;
	//scull_trim(dev); /* ignore errors */
	up(&dev->sem);
      }

    return 0;          /* success */
}

/*
 * Closing is just as simpler.
 */
 static int driver_release(struct inode *inode, struct file *filp)
 {
     return 0;
 }

/*
 * read data from device if there is any
 * for now just read data from any where
 */
 
static ssize_t driver_read(struct file *file, char __user *buf
			   , size_t count, loff_t *f_pos)
{
    ssize_t retval = 0;
    struct driver_dev *dev = file->private_data;
    
    printk(KERN_INFO "Driver: read()\n");
    
    if (down_interruptible(&dev->sem))
      return -ERESTARTSYS;
    if (*f_pos >= dev->size)
      goto out;
    if (*f_pos + count > dev->size)
      count = dev->size - *f_pos;
    
    if (copy_to_user(buf, &dev->data[*f_pos], count)) 
      {
	retval = -EFAULT;
	goto out;
      }
    *f_pos += count;
    retval = count;   
 out:
    up(&dev->sem);
    return retval;
}

/*
 * write data to device 
 * for now just write data to any where
 */
static ssize_t driver_write(struct file *file, const char __user *buf,
			    size_t len, loff_t *f_pos)
{
    ssize_t retval = len;
    struct driver_dev *dev = file->private_data;

    printk(KERN_INFO "Driver: write()\n");
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + len > dev->size)
        retval = dev->size - *f_pos;
    
    if (copy_from_user((void *)&dev->data[*f_pos], buf, retval)) 
      {
        retval = -EFAULT;
	goto out;
      }
    *f_pos += retval;
 out:
    up(&dev->sem);
    return retval;
}
/*
 * The "extended" operations -- only seek
 */

loff_t driver_llseek(struct file *filp, loff_t off, int whence)
{
    struct driver_dev *dev = filp->private_data;
    loff_t newpos;
    
    switch(whence) {
    case 0: /* SEEK_SET */
      newpos = off;
      break;
      
    case 1: /* SEEK_CUR */
      newpos = filp->f_pos + off;
      break;
      
    case 2: /* SEEK_END */
      newpos = dev->size + off;
      break;
      
    default: /* can't happen */
      return -EINVAL;
    }
    if (newpos < 0) return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}


static long driver_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct driver_dev *dev = filp->private_data;
    struct driver_dev my_dev;

    switch (cmd)
    {
        case GET_DEV_CMD:
            if (copy_to_user((void *)arg, dev, sizeof(*dev)))
            {
                return -EACCES;
            }
            break;

        case SET_DEV_CMD:
	  if (copy_from_user((void *)&my_dev, (const void *)arg, sizeof(my_dev)))
            {
                return -EACCES;
            }
	    dev->read_ix = my_dev.read_ix;

            break;
        default:
            return -EINVAL;
    }
     return 0;
}

struct file_operations my_fops = {
    .owner =    THIS_MODULE,
    .llseek =   driver_llseek,
    .read =     driver_read,
    .write =    driver_write,
    .unlocked_ioctl = driver_ioctl,
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
	    cdev_del(&my_devices[i].cdev);
	    if(driver_class)
	      {
		device_destroy(driver_class
			       , MKDEV(driver_major, driver_minor+i));
	      }
	    kfree(my_devices[i].data);
	  }
	kfree(my_devices);
      }
    
    if(driver_class)
      {
	class_destroy(driver_class);
	driver_class =  NULL;
      }
    
    /* cleanup_module is never called if registering failed */
    unregister_chrdev_region(devno, driver_no_devs);
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
      {
	printk(KERN_NOTICE "Error %d adding driver %d\n", err, index);
      }
    else
      {
	printk(KERN_NOTICE "NOTE added driver %d\n", index);
      }
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
    if (!my_devices) 
      {
	result = -ENOMEM;
	goto fail;  /* Make this more graceful */
      }
    
    memset(my_devices, 0, driver_no_devs * sizeof(struct driver_dev));
    
    /* Initialize each device. */
    for (i = 0; i < driver_no_devs; i++) 
      {
	my_devices[i].data = kmalloc(data_size, GFP_KERNEL);
	my_devices[i].size = data_size;
	sema_init(&my_devices[i].sem, 1);
	driver_setup_cdev(&my_devices[i], driver_class, i);
      }
    
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
