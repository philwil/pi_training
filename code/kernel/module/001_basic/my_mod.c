
/*  
 *  my_mod.c - Demonstes the module_init() and module_exit() macros.
 *             Used to identify the entry and exit points of ouor module
 * Note that we also seem to need the module license statement
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

static int __init my_mod_init(void)
{
	printk(KERN_INFO "Hello, my_mod\n");
	return 0;
}

static void __exit my_mod_exit(void)
{
	printk(KERN_INFO "Goodbye, my_mod\n");
}

module_init(my_mod_init);
module_exit(my_mod_exit);

MODULE_LICENSE("GPL");
