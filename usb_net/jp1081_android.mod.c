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
	{ 0xed5f4d2, "usbnet_resume" },
	{ 0x6650bc1a, "usbnet_suspend" },
	{ 0x478c7885, "usbnet_disconnect" },
	{ 0xd1c8539d, "usbnet_probe" },
	{ 0xc2844206, "usbnet_nway_reset" },
	{ 0x3b2ef179, "usbnet_set_msglevel" },
	{ 0xa71e07f0, "usbnet_get_msglevel" },
	{ 0xcaae5c3d, "usbnet_set_settings" },
	{ 0xd30f71e3, "usbnet_get_settings" },
	{ 0x68e6e307, "usbnet_tx_timeout" },
	{ 0x2d86c1a2, "usbnet_change_mtu" },
	{ 0x99cd6d93, "eth_validate_addr" },
	{ 0x516b322b, "eth_mac_addr" },
	{ 0xe2eb49c9, "usbnet_start_xmit" },
	{ 0x3d41bb4c, "usbnet_stop" },
	{ 0x53c9d558, "usbnet_open" },
	{ 0x2d8afbbc, "mii_link_ok" },
	{ 0x82b7ec99, "usb_register_driver" },
	{ 0x2bb18e3a, "usb_deregister" },
	{ 0x55efc437, "mii_nway_restart" },
	{ 0xe540d339, "usbnet_get_endpoints" },
	{ 0xfaf98462, "bitrev32" },
	{ 0xa34f1ef5, "crc32_le" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xa1891339, "usb_submit_urb" },
	{ 0x5aa3e125, "usb_alloc_urb" },
	{ 0xf9efef3b, "usb_free_urb" },
	{ 0x9c0ba064, "malloc_sizes" },
	{ 0xecbf78f3, "mutex_unlock" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x94dea09a, "kmem_cache_alloc_trace" },
	{ 0xe15044f, "mutex_lock" },
	{ 0x9d669763, "memcpy" },
	{ 0x37a0cba, "kfree" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x5fea5fd4, "usb_control_msg" },
	{ 0x1fd5d1ae, "generic_mii_ioctl" },
	{ 0xd6c5e952, "usbnet_get_drvinfo" },
	{ 0x10ddbfea, "mii_ethtool_gset" },
	{ 0x129ad39f, "mii_check_media" },
	{ 0xe46eee59, "usbnet_defer_kevent" },
	{ 0x1feee434, "netif_carrier_on" },
	{ 0x27e1a049, "printk" },
	{ 0xda4cedff, "netif_carrier_off" },
	{ 0x462e5ffa, "netdev_err" },
	{ 0x6d53ae3c, "skb_clone" },
	{ 0x7ca212c5, "skb_pull" },
	{ 0x91e7ebc2, "usbnet_skb_return" },
	{ 0x3bc961db, "dev_kfree_skb_any" },
	{ 0x89143fcd, "skb_copy_expand" },
	{ 0xf7802486, "__aeabi_uidivmod" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v0FE6p9700d*dc*dsc*dp*ic*isc*ip*");
