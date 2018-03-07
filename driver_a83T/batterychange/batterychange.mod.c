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
	{ 0xfd6293c2, "boot_tvec_bases" },
	{ 0xb7b05423, "platform_device_unregister" },
	{ 0x873857f, "platform_driver_unregister" },
	{ 0x74b123e5, "platform_driver_register" },
	{ 0x40745e68, "platform_device_register" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0xbc601ad6, "script_get_item" },
	{ 0x177e765, "kobject_uevent_env" },
	{ 0x27e1a049, "printk" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0xfcec0987, "enable_irq" },
	{ 0x31bd442e, "schedule_delayed_work" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0xe1d61c3a, "cancel_delayed_work_sync" },
	{ 0xfe990052, "gpio_free" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "66C0E02E7A2402C607D2E88");
