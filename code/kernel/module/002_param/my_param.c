
/*  
 *  my_mod.c - Demonstes the module_init() and module_exit() macros.
 *             Used to identify the entry and exit points of ouor module
 * Note that we also seem to need the module license statement
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

static int foo = 20;

module_param(foo, int, 0664);

static int __init my_param_init(void)
{
        printk(KERN_INFO "Hello, my_param foo started as %d\n", foo);
	return 0;
}

static void __exit my_param_exit(void)
{
        printk(KERN_INFO "Goodbye, my_param foo finished as %d\n", foo);
	return ;
}

module_init(my_param_init);
module_exit(my_param_exit);

MODULE_LICENSE("GPL");
