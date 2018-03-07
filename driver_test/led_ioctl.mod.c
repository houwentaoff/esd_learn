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
	{ 0x885b74e3, "module_layout" },
	{ 0x54ff95f, "misc_deregister" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0xc5ace6f8, "misc_register" },
	{ 0xfaef0ed, "__tasklet_schedule" },
	{ 0x27e1a049, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "A60EEB78E47C5168FEC8FD6");
