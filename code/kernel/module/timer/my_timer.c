
/*  
 *  my_timer.c - Demonstes a simple kernel timer
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <linux/timer.h>	/* Needed for the timer api */

static struct timer_list my_timer;

static int timer = 200;
module_param(timer, int, 0664);
static int count = 0;
module_param(count, int, 0664);

void my_timer_callback(unsigned long data)
{
    printk( "my_timer_callback called (%ld) count (%d).\n"
	    , jiffies 
	    , count);
    count++;
    mod_timer( &my_timer, jiffies + msecs_to_jiffies(timer) );

}


static int __init my_timer_init(void)
{
        int ret;
        printk(KERN_INFO "Hello, my_timer timer started as %d\n", timer);

	// my_timer.function, my_timer.data
	setup_timer( &my_timer, my_timer_callback, 0 );

	printk( "Starting timer to fire in %d ms (%ld)\n", timer, jiffies );
	ret = mod_timer( &my_timer, jiffies + msecs_to_jiffies(timer) );
	if (ret) printk("Error in mod_timer\n");
	return 0;
}

static void __exit my_timer_exit(void)
{
        int ret;
        printk(KERN_INFO "Goodbye, my_timer count finished as %d\n", count);
	ret = del_timer( &my_timer );
	if (ret) printk("The timer is still in use...\n");
	return ;
}

module_init(my_timer_init);
module_exit(my_timer_exit);

MODULE_LICENSE("GPL");
