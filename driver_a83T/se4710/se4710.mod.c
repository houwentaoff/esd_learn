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
	{ 0x96d3a101, "cci_dev_exit_helper" },
	{ 0x5151b2ec, "cci_dev_init_helper" },
	{ 0x6e771bc4, "misc_deregister" },
	{ 0xc5e0e362, "cci_dev_remove_helper" },
	{ 0x5aef5ea3, "dev_get_drvdata" },
	{ 0xc10d2f, "malloc_sizes" },
	{ 0xc6625567, "misc_register" },
	{ 0x2b69e516, "cci_dev_probe_helper" },
	{ 0xee148918, "kmem_cache_alloc_trace" },
	{ 0x2eed114e, "v4l2_chip_ident_i2c_client" },
	{ 0x791ab099, "v4l2_ctrl_query_fill" },
	{ 0xf9a482f9, "msleep" },
	{ 0x37a0cba, "kfree" },
	{ 0x98082893, "__copy_to_user" },
	{ 0x4ecc6c3b, "i2c_transfer" },
	{ 0xe2e8065e, "memdup_user" },
	{ 0x17a142df, "__copy_from_user" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x27e1a049, "printk" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=cci";


MODULE_INFO(srcversion, "061ECF21A4DEBE04EA13E84");
