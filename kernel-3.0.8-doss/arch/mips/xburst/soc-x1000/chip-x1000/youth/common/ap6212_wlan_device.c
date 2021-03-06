#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/wlan_plat.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>

#define GPIO_WIFI_WAKE GPIO_PC(16)

static char *wifi_mac_str = NULL;
unsigned char wifi_mac_hex[7] = {0};
static struct wifi_platform_data bcmdhd_wlan_pdata;

static int get_wifi_mac_addr(unsigned char* buf)
{
	int i = 0;

	for (i = 0; i < 6; i++)
		buf[i] = wifi_mac_hex[i];

	printk("buf = %02x:%02x:%02x:%02x:%02x:%02x \n",
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	return 0;
}

static struct resource wlan_resources[] = {
	[0] = {
		.start = GPIO_WIFI_WAKE,
		.end = GPIO_WIFI_WAKE,
		.name = "bcmdhd_wlan_irq",
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE,
	},
};
static struct platform_device wlan_device = {
	.name   = "bcmdhd_wlan",
	.id     = 1,
	.dev    = {
		.platform_data = &bcmdhd_wlan_pdata,
	},
	.resource	= wlan_resources,
	.num_resources	= ARRAY_SIZE(wlan_resources),
};

static int __init get_wifi_mac_addr_from_cmdline(char *str)
{
	wifi_mac_str = str;
	return 1;
}

__setup("wifi_mac=", get_wifi_mac_addr_from_cmdline);

static int __init wlan_device_init(void)
{
	int ret;
	int i = 0;
	int num = 0;
	char *tmp_addr = NULL;

	printk("wifi_mac_hex_ori = %02x:%02x:%02x:%02x:%02x:%02x \n",
			wifi_mac_hex[0], wifi_mac_hex[1],wifi_mac_hex[2],wifi_mac_hex[3],wifi_mac_hex[4],wifi_mac_hex[5]);
	if (wifi_mac_str != NULL) {
		tmp_addr = kmalloc(3, GFP_KERNEL);
		for (num = 0; num <= 10; num = num + 2) {
			memset(tmp_addr, 0, sizeof(tmp_addr));
			memcpy(tmp_addr, wifi_mac_str + num, 2);
			wifi_mac_hex[i] = (unsigned char)simple_strtol(tmp_addr, NULL, 16);
			i++;
		}
	}

	printk("wifi_mac_hex_new = %02x:%02x:%02x:%02x:%02x:%02x \n",
			wifi_mac_hex[0], wifi_mac_hex[1],wifi_mac_hex[2],wifi_mac_hex[3],wifi_mac_hex[4],wifi_mac_hex[5]);
	memset(&bcmdhd_wlan_pdata, 0, sizeof(bcmdhd_wlan_pdata));
	if ((wifi_mac_hex[0] == 0) && (wifi_mac_hex[1] == 0) &&
			(wifi_mac_hex[2] == 0) && (wifi_mac_hex[3] == 0) &&
			(wifi_mac_hex[4] == 0) && (wifi_mac_hex[5] == 0)) {
		wlan_device.dev.platform_data = NULL;
	} else {
		bcmdhd_wlan_pdata.get_mac_addr = get_wifi_mac_addr;
	}

	ret = platform_device_register(&wlan_device);

	return ret;
}

late_initcall(wlan_device_init);
MODULE_DESCRIPTION("Broadcomm wlan driver");
MODULE_LICENSE("GPL");
