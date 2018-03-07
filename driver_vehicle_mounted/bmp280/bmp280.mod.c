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
	{ 0xc10d2f, "malloc_sizes" },
	{ 0xc6625567, "misc_register" },
	{ 0x9451f857, "dev_set_drvdata" },
	{ 0xee148918, "kmem_cache_alloc_trace" },
	{ 0x98082893, "__copy_to_user" },
	{ 0x27e1a049, "printk" },
	{ 0x64ae49ac, "i2c_smbus_read_i2c_block_data" },
	{ 0x5f754e5a, "memset" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0x6e771bc4, "misc_deregister" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x48a1ea5f, "i2c_smbus_write_byte_data" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "63CB9EDEAFFFCF5093A34EB");
