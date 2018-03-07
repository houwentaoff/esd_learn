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
	{ 0xf98e17f5, "driver_unregister" },
	{ 0xc7198afd, "spi_register_driver" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xc6625567, "misc_register" },
	{ 0x60eff1c6, "devm_request_threaded_irq" },
	{ 0x12a38747, "usleep_range" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0x4467122a, "__init_waitqueue_head" },
	{ 0x9e21f167, "__mutex_init" },
	{ 0x9451f857, "dev_set_drvdata" },
	{ 0xb96c57e9, "devm_kzalloc" },
	{ 0x98082893, "__copy_to_user" },
	{ 0x4ca50abb, "skb_dequeue" },
	{ 0x78ee3628, "skb_queue_purge" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x1bbfc8ab, "skb_queue_tail" },
	{ 0x9d669763, "memcpy" },
	{ 0x8949858b, "schedule_work" },
	{ 0x4f7f2d1b, "complete" },
	{ 0x6e771bc4, "misc_deregister" },
	{ 0xfe990052, "gpio_free" },
	{ 0x37215cfe, "devm_free_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0x92b57248, "flush_work" },
	{ 0x5aef5ea3, "dev_get_drvdata" },
	{ 0xaf1f4714, "consume_skb" },
	{ 0x1cc4f099, "wait_for_completion_interruptible_timeout" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0x74c97f9c, "_raw_spin_unlock_irqrestore" },
	{ 0xbd7083bc, "_raw_spin_lock_irqsave" },
	{ 0x17a142df, "__copy_from_user" },
	{ 0xb9b838b0, "skb_put" },
	{ 0x85b7a1e9, "__alloc_skb" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0x27e1a049, "printk" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xda198c53, "mutex_unlock" },
	{ 0x4fb731d1, "spi_sync" },
	{ 0xac06626d, "mutex_lock" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x5f754e5a, "memset" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C321228C21A9EACE9CD58DF");
