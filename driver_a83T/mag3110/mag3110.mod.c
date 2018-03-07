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
	{ 0x27e1a049, "printk" },
	{ 0xf9a482f9, "msleep" },
	{ 0x98f5e860, "i2c_smbus_read_i2c_block_data" },
	{ 0xc10d2f, "malloc_sizes" },
	{ 0xb9752d4b, "_dev_info" },
	{ 0xbb9b48ce, "sysfs_create_group" },
	{ 0x137979f3, "input_register_polled_device" },
	{ 0x98381fdf, "input_set_abs_params" },
	{ 0xf9af158c, "input_allocate_polled_device" },
	{ 0x56007fc7, "hwmon_device_register" },
	{ 0x4467122a, "__init_waitqueue_head" },
	{ 0x9451f857, "dev_set_drvdata" },
	{ 0xee148918, "kmem_cache_alloc_trace" },
	{ 0x37a0cba, "kfree" },
	{ 0x4ce4bb3, "sysfs_remove_group" },
	{ 0xcb1560aa, "hwmon_device_unregister" },
	{ 0x5b57f16b, "input_free_polled_device" },
	{ 0x4491ba06, "input_unregister_polled_device" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x54a9db5f, "_kstrtoul" },
	{ 0x5aef5ea3, "dev_get_drvdata" },
	{ 0xa01d0f4f, "dev_err" },
	{ 0x978de421, "i2c_smbus_write_byte_data" },
	{ 0x88902302, "i2c_smbus_read_byte_data" },
	{ 0x91715312, "sprintf" },
	{ 0xda198c53, "mutex_unlock" },
	{ 0xac06626d, "mutex_lock" },
	{ 0x20000329, "simple_strtoul" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:mag3110");

MODULE_INFO(srcversion, "3EEC054CD94F58F8330BBB1");
