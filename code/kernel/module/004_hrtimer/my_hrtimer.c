
/*  
 *  my_timer.c - Demonstes a simple kernel timer
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <linux/hrtimer.h>	/* Needed for the timer api */
#include <linux/ktime.h>	/* Needed for the timer api */

#define MS_TO_NS(x)	(x * 1E6L)

static struct hrtimer my_hrtimer;


static long timer = 200;
module_param(timer, long, 0664);
static int count = 0;
module_param(count, int, 0664);

enum hrtimer_restart my_hrtimer_callback(struct hrtimer * hrtimer)
{
    printk( "my_hrtimer callback called (%ld) count (%d).\n"
	    , jiffies 
	    , count);
    count++;
    hrtimer_forward(hrtimer, ktime_get(), ktime_set( 0, MS_TO_NS(timer) ));
    return HRTIMER_RESTART;
    //return HRTIMER_NORESTART;  // use this for a one shot
}


static int __init my_hrtimer_init(void)
{
        int ret=0;
	ktime_t ktime;
	//unsigned long delay_in_ms = 200L;

	printk("HR Timer module installing\n");

	ktime = ktime_set( 0, MS_TO_NS(timer) );

	hrtimer_init( &my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
  
	my_hrtimer.function = &my_hrtimer_callback;

	printk( "Starting timer to fire in %ldms (%ld)\n", 
		timer, jiffies );

	hrtimer_start( &my_hrtimer, ktime, HRTIMER_MODE_REL );
	
	return ret;
}

static void __exit my_hrtimer_exit(void)
{
        int ret;
        printk(KERN_INFO "Goodbye, my_hrtimer count finished as %d\n", count);
	ret = hrtimer_cancel( &my_hrtimer );
	if (ret) printk("The timer was still in use...\n");

	return ;
}

module_init(my_hrtimer_init);
module_exit(my_hrtimer_exit);

MODULE_LICENSE("GPL");
