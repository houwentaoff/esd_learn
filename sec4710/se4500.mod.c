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
	{ 0x84fbace2, "module_layout" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0xe2e8065e, "memdup_user" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0x9c0ba064, "malloc_sizes" },
	{ 0x7bd9314f, "dev_printk" },
	{ 0x6f449893, "misc_register" },
	{ 0x333f1194, "v4l2_int_device_register" },
	{ 0x94dea09a, "kmem_cache_alloc_trace" },
	{ 0xfe990052, "gpio_free" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xec4b13dd, "i2c_register_driver" },
	{ 0x4588bf0f, "__dynamic_pr_debug" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0x9d669763, "memcpy" },
	{ 0x328a05f1, "strncpy" },
	{ 0x37a0cba, "kfree" },
	{ 0xf7dd6f0a, "misc_deregister" },
	{ 0xf144b5c7, "dev_get_drvdata" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0xf9a482f9, "msleep" },
	{ 0x4cdb5b0b, "dev_err" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xc70ea74d, "i2c_transfer" },
	{ 0x27e1a049, "printk" },
	{ 0x16f75f93, "i2c_del_driver" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:se4710");
