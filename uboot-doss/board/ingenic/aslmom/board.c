/*
 * Ingenic mensa setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/arch/rtc.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

struct cgu_clk_src cgu_clk_src[] = {
	{OTG, EXCLK},
	{LCD, MPLL},
	{MSC, MPLL},
	{SFC, MPLL},
	{CIM, MPLL},
	{PCM, MPLL},
	{I2S, EXCLK},
	{SRC_EOF,SRC_EOF}
};
extern int jz_net_initialize(bd_t *bis);
#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

#ifdef CONFIG_PMU_AXP173
extern int axp173_regulator_init(void);
#endif

#ifdef CONFIG_PMU_CW2015
extern int cw2015_regulator_init(void);
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
static void battery_init_gpio(void)
{
}
#endif

#ifdef CONFIG_REGULATOR
int regulator_init(void)
{
	int ret;
#ifdef CONFIG_PMU_AXP173
	ret = axp173_regulator_init();
#endif

#ifdef CONFIG_PMU_CW2015
       ret = cw2015_regulator_init();
#endif

	return ret;
}
#endif /* CONFIG_REGULATOR */

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_REGULATOR
	regulator_init();
#endif

	return 0;
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	printf("USB_udc_probe\n");
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

int misc_init_r(void)
{
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
	battery_init_gpio();
#endif
	return 0;
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
#ifdef CONFIG_NET_GMAC
	rv = jz_net_initialize(bis);
#endif
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
}

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: Aslmom (Ingenic XBurst X1000 SoC)\n");
	return 0;
}

extern int low_power_detect(void);

#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
#ifdef CONFIG_PMU_AXP173
	axp173_regulator_init();
#endif
#ifdef CONFIG_PMU_CW2015
       cw2015_regulator_init();
#endif

	/* close boost */
	gpio_request(32 * 1 + 5, NULL);
	gpio_direction_output(32 * 1 + 5, 0);
	/* close codec mute */
	gpio_request(32 * 3 + 2, NULL);
	gpio_direction_output(32 * 3 + 2, 0);
}

extern int mmc_block_read(u32 start, u32 blkcnt, u32 *dst);
extern void sfc_nand_load(long offs,long size,void *dst);
extern void sfc_nor_load(unsigned int src_addr, unsigned int count,unsigned int dst_addr);

#ifdef CONFIG_GET_WIFI_MAC
static char *spl_board_process_wifimac_arg(char *arg)
{
	char *wifi_mac_str = NULL;
	unsigned int mac_addr[512] = {};

#if defined(CONFIG_SPL_SFC_NOR)
	sfc_nor_load(WIFI_MAC_READ_ADDR, WIFI_MAC_READ_COUNT, mac_addr);
#elif defined(CONFIG_SPL_SFC_NAND)
	sfc_nand_load(WIFI_MAC_READ_ADDR, WIFI_MAC_READ_COUNT, mac_addr)
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
	mmc_block_read(WIFI_MAC_READ_ADDR / 0x200, 1, mac_addr);
#endif

	wifi_mac_str = strstr(arg, "wifi_mac");
	if (wifi_mac_str != NULL)
		memcpy(wifi_mac_str + 9, mac_addr, WIFI_MAC_READ_COUNT);

	return arg;
}
#endif

#ifdef CONFIG_GET_BAT_PARAM
static char *spl_board_process_bat_arg(char *arg)
{
	char *bat_param_str = NULL;
	char *bat_str = "4400";
	char buf[512] = {};

#if defined(CONFIG_SPL_SFC_NOR)
	sfc_nor_load(BAT_PARAM_READ_ADDR, BAT_PARAM_READ_COUNT, (unsigned int)buf);
#elif defined(CONFIG_SPL_SFC_NAND)
	sfc_nand_load(BAT_PARAM_READ_ADDR, BAT_PARAM_READ_COUNT, (unsigned int)buf)
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
	mmc_block_read(BAT_PARAM_READ_ADDR / 0x200, 1, buf);
#endif

	bat_param_str = strstr(arg, "bat");
	/* [0x69, 0xaa, 0x55] new battery's flag in nv */
	if((bat_param_str != NULL) && (buf[0] == 0x69) && (buf[1] == 0xaa)
			&& (buf[2] ==0x55))
		memcpy(bat_param_str + 4, bat_str, 4);

	return arg;
}
#endif

char *spl_board_process_bootargs(char *arg)
{
#ifdef CONFIG_GET_WIFI_MAC
	arg = spl_board_process_wifimac_arg(arg);
#endif

#ifdef CONFIG_GET_BAT_PARAM
	arg = spl_board_process_bat_arg(arg);
#endif

	return arg;
}

/**
 * @brief get boot type
 *
 * @return return 0 on default, return 1 on boot uboot, return 2 on boot kernel
 */
int spl_get_boot_type(void)
{
	int usb_insert;
	int low_power;
	int rsr = cpm_inl(CPM_RSR);
	int hspr = readl(RTC_BASE + RTC_HSPR);

	//set PB(8),USB_DETE PIN as input
	gpio_port_direction_input(1, 8);

	usb_insert = !gpio_get_value(40);
	low_power = low_power_detect();

	if (rsr & CPM_RSR_WR) {
		/* power_off */
		if (hspr == 0x50574f46) {
			return 1;
		/* low power */
		} else if (low_power) {
			simple_puts("\n2\n");
			return 1;
		} else {
			simple_puts("\n2222\n");
			return 0;
		}
	} else if (usb_insert) {
		/* usb insert */
		simple_puts("\n3\n");
		return 1;
	} else if (low_power) {
		/* low power */
		simple_puts("\n4\n");
		return 1;
	} else {
		return 0;
	}

	return 0;
}

#endif /* CONFIG_SPL_BUILD */
