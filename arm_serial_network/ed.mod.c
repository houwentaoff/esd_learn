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
	{ 0xf997ecc4, "module_layout" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x718565d5, "remove_wait_queue" },
	{ 0x1000e51, "schedule" },
	{ 0x3fdacc6f, "add_wait_queue" },
	{ 0x74c97f9c, "_raw_spin_unlock_irqrestore" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xbd7083bc, "_raw_spin_lock_irqsave" },
	{ 0x7d11c268, "jiffies" },
	{ 0x9f613474, "consume_skb" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0x311b7963, "_raw_spin_unlock" },
	{ 0xc2161e33, "_raw_spin_lock" },
	{ 0x9bbb7d37, "netif_rx" },
	{ 0x5d9aaeab, "eth_type_trans" },
	{ 0x6a20607e, "skb_put" },
	{ 0xebf82e88, "dev_alloc_skb" },
	{ 0x72542c85, "__wake_up" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xd83247d, "device_create" },
	{ 0xf78ff5b6, "__class_create" },
	{ 0x44b01892, "__register_chrdev" },
	{ 0x9c0ba064, "malloc_sizes" },
	{ 0x41e92619, "__init_waitqueue_head" },
	{ 0x94dea09a, "kmem_cache_alloc_trace" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x6d2e5251, "class_destroy" },
	{ 0x7c8bd201, "device_destroy" },
	{ 0x37a0cba, "kfree" },
	{ 0x49ebacbd, "_clear_bit" },
	{ 0x9d669763, "memcpy" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x676bbc0f, "_set_bit" },
	{ 0xf439f380, "__netif_schedule" },
	{ 0x2a3aa678, "_test_and_clear_bit" },
	{ 0xff5810a0, "register_netdev" },
	{ 0x7609e84b, "alloc_netdev_mqs" },
	{ 0x5434d539, "free_netdev" },
	{ 0x65981fcf, "unregister_netdev" },
	{ 0x27e1a049, "printk" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

