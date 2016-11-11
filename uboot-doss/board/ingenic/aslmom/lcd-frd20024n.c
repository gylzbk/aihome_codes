/*
 * Ingenic dorado lcd code
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
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

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v13.h>
#include <jz_lcd/frd240a3602b.h>

unsigned long frd240a3602b_cmd_buf[]= {
    0x22222222,
};

struct smart_lcd_data_table frd240a3602b_data_table[] = {
	{SMART_CONFIG_CMD,	0x0001},{SMART_CONFIG_DATA,	0x01},{SMART_CONFIG_DATA,	0x1c},
	{SMART_CONFIG_CMD,	0x0002},{SMART_CONFIG_DATA,	0x01},{SMART_CONFIG_DATA,	0x00},
	//{SMART_CONFIG_CMD,	0x0003},{SMART_CONFIG_DATA,	0x12},{SMART_CONFIG_DATA,	0x30},
	{SMART_CONFIG_CMD,	0x0003},{SMART_CONFIG_DATA,	0x10},{SMART_CONFIG_DATA,	0x30},
	{SMART_CONFIG_CMD,	0x0008},{SMART_CONFIG_DATA,	0x08},{SMART_CONFIG_DATA,	0x08},
	{SMART_CONFIG_CMD,	0x000c},{SMART_CONFIG_DATA,		0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x000f},{SMART_CONFIG_DATA,		0x0e},{SMART_CONFIG_DATA,	0x01},
	//{SMART_CONFIG_CMD,	0x000f},{SMART_CONFIG_DATA,		0x00},{SMART_CONFIG_DATA,	0x01},
	{SMART_CONFIG_CMD,	0x0020},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0021},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},

	{SMART_CONFIG_UDELAY,10},

	{SMART_CONFIG_CMD,	0x0010},{SMART_CONFIG_DATA,	0x0a},{SMART_CONFIG_DATA,	0x00},
	//{SMART_CONFIG_CMD,	0x0010},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0011},{SMART_CONFIG_DATA,	0x10},{SMART_CONFIG_DATA,	0x38},
	//{SMART_CONFIG_CMD,	0x0011},{SMART_CONFIG_DATA,	0x10},{SMART_CONFIG_DATA,	0x00},

	{SMART_CONFIG_UDELAY,10},

	{SMART_CONFIG_CMD,	0x0030},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0031},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0xdb},
	{SMART_CONFIG_CMD,	0x0032},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0033},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0034},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0xdb},
	{SMART_CONFIG_CMD,	0x0035},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0036},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0xaf},
	{SMART_CONFIG_CMD,	0x0037},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0038},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0xdb},
	{SMART_CONFIG_CMD,	0x0039},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x00},
	
	//{SMART_CONFIG_UDELAY,10},
	{SMART_CONFIG_UDELAY,100},

	{SMART_CONFIG_CMD,	0x00ff},{SMART_CONFIG_DATA,		0x00},{SMART_CONFIG_DATA,	0x03},
	{SMART_CONFIG_CMD,	0x00b0},{SMART_CONFIG_DATA,	0x16},{SMART_CONFIG_DATA,	0x01},
	{SMART_CONFIG_CMD,	0x00b1},{SMART_CONFIG_DATA,	0x06},{SMART_CONFIG_DATA,	0x06},
	{SMART_CONFIG_CMD,	0x0050},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x02},
	{SMART_CONFIG_CMD,	0x0051},{SMART_CONFIG_DATA,	0x08},{SMART_CONFIG_DATA,	0x07},
	{SMART_CONFIG_CMD,	0x0052},{SMART_CONFIG_DATA,	0x01},{SMART_CONFIG_DATA,	0x07},
	{SMART_CONFIG_CMD,	0x0053},{SMART_CONFIG_DATA,	0x18},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0054},{SMART_CONFIG_DATA,	0x06},{SMART_CONFIG_DATA,	0x01},
	{SMART_CONFIG_CMD,	0x0055},{SMART_CONFIG_DATA,	0x00},{SMART_CONFIG_DATA,	0x02},
	{SMART_CONFIG_CMD,	0x0056},{SMART_CONFIG_DATA,	0x07},{SMART_CONFIG_DATA,	0x07},
	{SMART_CONFIG_CMD,	0x0057},{SMART_CONFIG_DATA,	0x01},{SMART_CONFIG_DATA,	0x06},
	{SMART_CONFIG_CMD,	0x0058},{SMART_CONFIG_DATA,	0x18},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0059},{SMART_CONFIG_DATA,	0x05},{SMART_CONFIG_DATA,	0x01},
	{SMART_CONFIG_CMD,	0x00ff},{SMART_CONFIG_DATA,		0x00},{SMART_CONFIG_DATA,	0x00},
	{SMART_CONFIG_CMD,	0x0007},{SMART_CONFIG_DATA,	0x10},{SMART_CONFIG_DATA,	0x17},
	{SMART_CONFIG_CMD,	0x0022},
};

struct jzfb_config_info jzfb1_init_data = {
	.num_modes = 1,
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp    = 18,
	.pinmd  = 0,

	.smart_config.rsply_cmd_high       = 0,
	.smart_config.csply_active_high    = 0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.newcfg_fmt_conv =  1,
	.smart_config.write_gram_cmd = frd240a3602b_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(frd240a3602b_cmd_buf),
	.smart_config.bus_width = 8,
	.smart_config.length_data_table =  ARRAY_SIZE(frd240a3602b_data_table),
	.smart_config.data_table = frd240a3602b_data_table,
	.dither_enable = 0,
};

struct frd240a3602b_data frd240a3602b_pdata = {
	.gpio_lcd_rd  = GPIO_PB(16),
	.gpio_lcd_rst = GPIO_PB(19),
	.gpio_lcd_cs  = GPIO_PB(18),
	.gpio_lcd_bl  = GPIO_PC(24),
};
