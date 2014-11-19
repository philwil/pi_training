#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xfd5f6819, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xb6b46a7c, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0xed720d5f, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xccf508a3, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xcc47ae, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x28b3b2f3, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xd8e484f0, __VMLINUX_SYMBOL_STR(register_chrdev_region) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0xcdfdd5c2, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xae6ea92, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xbd96fb78, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xc4554217, __VMLINUX_SYMBOL_STR(up) },
	{ 0x4792c572, __VMLINUX_SYMBOL_STR(down_interruptible) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DE3EAF1528F38AF45EDE6F2");
