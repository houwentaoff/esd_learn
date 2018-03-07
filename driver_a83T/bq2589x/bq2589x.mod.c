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
	{ 0x73e20c1c, "strlcpy" },
	{ 0x7802b26a, "power_supply_changed" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x132a7a5b, "init_timer_key" },
	{ 0xfa8bd03d, "power_supply_register" },
	{ 0x60eff1c6, "devm_request_threaded_irq" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0x9451f857, "dev_set_drvdata" },
	{ 0xb96c57e9, "devm_kzalloc" },
	{ 0x27e1a049, "printk" },
	{ 0xbc601ad6, "script_get_item" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0xba2786bd, "power_supply_unregister" },
	{ 0x37a0cba, "kfree" },
	{ 0xfe990052, "gpio_free" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x978de421, "i2c_smbus_write_byte_data" },
	{ 0x88902302, "i2c_smbus_read_byte_data" },
	{ 0xe1d61c3a, "cancel_delayed_work_sync" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x31bd442e, "schedule_delayed_work" },
	{ 0x5aef5ea3, "dev_get_drvdata" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "8BF93A6E0ACDBB1F494F8FF");
