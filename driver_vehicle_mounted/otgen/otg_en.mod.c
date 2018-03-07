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
	{ 0xb7b05423, "platform_device_unregister" },
	{ 0x873857f, "platform_driver_unregister" },
	{ 0x74b123e5, "platform_driver_register" },
	{ 0x40745e68, "platform_device_register" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x27e1a049, "printk" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0xbc601ad6, "script_get_item" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xfe990052, "gpio_free" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "11E27C17C52BBFFAE0590B5");
