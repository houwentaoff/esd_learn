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
	{ 0x7b4c39cb, "i2c_del_driver" },
	{ 0x124d5527, "i2c_register_driver" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0xbbfb2630, "input_event" },
	{ 0x64ae49ac, "i2c_smbus_read_i2c_block_data" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xc10d2f, "malloc_sizes" },
	{ 0x56007fc7, "hwmon_device_register" },
	{ 0x9451f857, "dev_set_drvdata" },
	{ 0xbb9b48ce, "sysfs_create_group" },
	{ 0x137979f3, "input_register_polled_device" },
	{ 0x98381fdf, "input_set_abs_params" },
	{ 0xf9af158c, "input_allocate_polled_device" },
	{ 0x37a0cba, "kfree" },
	{ 0xee148918, "kmem_cache_alloc_trace" },
	{ 0xda198c53, "mutex_unlock" },
	{ 0xac06626d, "mutex_lock" },
	{ 0x27e1a049, "printk" },
	{ 0x54a9db5f, "_kstrtoul" },
	{ 0x91715312, "sprintf" },
	{ 0x20000329, "simple_strtoul" },
	{ 0xcb1560aa, "hwmon_device_unregister" },
	{ 0x5b57f16b, "input_free_polled_device" },
	{ 0x5aef5ea3, "dev_get_drvdata" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x48a1ea5f, "i2c_smbus_write_byte_data" },
	{ 0x9d527621, "i2c_smbus_read_byte_data" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "9FF487886D7D27A1C376FA1");
