
/*#define DEBUG			1*/
/*#define VERBOSE_DEBUG		1*/
#include <config.h>
#include <common.h>
#include <linux/err.h>
#include <linux/list.h>
#include <regulator.h>
#include <ingenic_soft_i2c.h>
#include <power/cw2015.h>
#include <power/axp173.h>

#define CW2015_I2C_ADDR    0x62

static struct i2c cw2015_i2c;
static struct i2c *i2c;

int cw2015_write_reg(u8 reg, u8 *val)
{
	unsigned int  ret;
	ret = i2c_write(i2c, CW2015_I2C_ADDR, reg, 1, val, 1);
	if(ret) {
		debug("cw2015 write register error\n");
		return -EIO;
	}
	return 0;
}

int cw2015_read_reg(u8 reg, u8 *val, u32 len)
{
	int ret;
	ret = i2c_read(i2c, CW2015_I2C_ADDR, reg, 1, val, len);
	if(ret) {
		debug("cw2015 read register error\n");
		return -EIO;
	}
	return 0;
}

int cw2015_power_off(void)
{
	int ret;
	uint8_t reg_val;

	ret = cw2015_read_reg(AXP173_POWER_OFF, &reg_val,1);
	if (ret < 0)
		debug_x("Error in reading the POWEROFF_Reg\n");
	reg_val |= (1 << 7);
	ret = cw2015_write_reg(AXP173_POWER_OFF, &reg_val);
	if (ret < 0)
		debug_x("Error in writing the POWEROFF_Reg\n");
	return 0;
}

int cw2015_regulator_init(void)
{
	int ret;
	cw2015_i2c.scl = CONFIG_CW2015_I2C_SCL;
	cw2015_i2c.sda = CONFIG_CW2015_I2C_SDA;
	i2c = &cw2015_i2c;
	i2c_init(i2c);

	ret = i2c_probe(i2c, CW2015_I2C_ADDR);
	if(ret) {
		debug_x("probe cw2015_i2c error, i2c addr 0x%x \n\n\n", CW2015_I2C_ADDR);
		return -EIO;
	} else {
		debug_x("successful \n");
	}

	return 0;
}

