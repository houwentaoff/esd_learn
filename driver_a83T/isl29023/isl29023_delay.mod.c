#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
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
	{ 0x619a0a55, "module_layout" },
	{ 0x6dd5c15f, "i2c_del_driver" },
	{ 0x4d4adb3, "i2c_register_driver" },
	{ 0xbbfb2630, "input_event" },
	{ 0xf9a482f9, "msleep" },
	{ 0xc10d2f, "malloc_sizes" },
	{ 0x31bd442e, "schedule_delayed_work" },
	{ 0x132a7a5b, "init_timer_key" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0xbc477a2, "irq_set_irq_type" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0x58fea5fa, "input_register_device" },
	{ 0x98381fdf, "input_set_abs_params" },
	{ 0x5ca76bb6, "input_allocate_device" },
	{ 0xbb9b48ce, "sysfs_create_group" },
	{ 0x9e21f167, "__mutex_init" },
	{ 0x9451f857, "dev_set_drvdata" },
	{ 0xb81960ca, "snprintf" },
	{ 0xee148918, "kmem_cache_alloc_trace" },
	{ 0x88902302, "i2c_smbus_read_byte_data" },
	{ 0x37a0cba, "kfree" },
	{ 0x4ce4bb3, "sysfs_remove_group" },
	{ 0xb5e29212, "input_free_device" },
	{ 0x6567a2a6, "input_unregister_device" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0x4205ad24, "cancel_work_sync" },
	{ 0xda198c53, "mutex_unlock" },
	{ 0x978de421, "i2c_smbus_write_byte_data" },
	{ 0xac06626d, "mutex_lock" },
	{ 0x54a9db5f, "_kstrtoul" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0x2196324, "__aeabi_idiv" },
	{ 0x91715312, "sprintf" },
	{ 0x5aef5ea3, "dev_get_drvdata" },
	{ 0x27e1a049, "printk" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "97D20E89D6FCD33C79C1583");
