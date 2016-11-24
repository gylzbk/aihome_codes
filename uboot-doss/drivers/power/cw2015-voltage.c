#include <power/axp173.h>
#include <power/cw2015.h>
#include <config.h>
#include <common.h>

#define CHARGING_ON 	1
#define INTER_RESIST    300
#define INTER_RESIST1   280
#define SAMPLE_COUNT	10

extern int cw2015_read_reg(u8 reg, u8 *val, u32 len);
extern int cw2015_write_reg(u8 reg, u8 *val);
extern int get_battery_flag(void);

static int which_battery = -1;

#define SIZE_BATINFO    64

static unsigned char config_info[SIZE_BATINFO] = {
	0x15,0x86,0x76,0x67,0x53,0x44,0x4D,0x54,
	0x54,0x4B,0x4D,0x53,0x62,0x66,0x60,0x55,
	0x4B,0x41,0x39,0x36,0x3D,0x46,0x4D,0x5A,
	0x5C,0x47,0x0A,0x3E,0x19,0x32,0x59,0x76,
	0x81,0x86,0x8A,0x8B,0x3F,0x19,0x96,0x2C,
	0x00,0x1F,0x52,0x87,0x8F,0x91,0x94,0x52,
	0x82,0x8C,0x92,0x96,0x8C,0x5E,0x64,0xCB,
	0x2F,0x7D,0x72,0xA5,0xB5,0xC1,0x5C,0x89
};

struct ocv2soc ocv2soc[] = {
        {4321, 100},
        {4152,  94},
        {4096,  88},
        {4030,  84},
        {3974,  73},
        {3920,  65},
        {3868,  56},
        {3804,  45},
        {3764,  33},
        {3721,  20},
        {3679,  13},
        {3653,   9},
        {3628,   0}
};

struct ocv2soc ocv2soc1[] = {
	{4227, 100},
	{4154,  95},
	{4117,  88},
	{4042,  82},
	{3986,  76},
	{3954,  69},
	{3901,  61},
	{3844,  52},
	{3782,  43},
	{3714,  34},
	{3671,  25},
	{3652,  18},
	{3635,  10},
	{3608,   0},
};

enum adc_type {
        ACIN_VOL = 0,
        ACIN_CUR,
        VBUS_VOL,
        VBUS_CUR,
        BAT_VOL,
        BAT_CUR,
};

static int cw_update_config_info(void)
{
	int ret;
	u8 reg_val;
	int i;
	u8 reset_val;

	/* make sure no in sleep mode */
	ret = cw2015_read_reg(REG_MODE, &reg_val, 1);
	if (ret < 0)
		return ret;

	reset_val = reg_val;
	if((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) {
		debug_x( "Error, device in sleep mode, cannot update battery info\n");
		return -1;
	}

	/* update new battery info */
	for (i = 0; i < SIZE_BATINFO; i++) {
		debug_x("cw_bat->plat_data->cw_bat_config_info[%d] = 0x%x\n", i, config_info[i]);
		ret = cw2015_write_reg(REG_BATINFO + i, &config_info[i]);

		if (ret < 0)
			return ret;
	}

	/* readback & check */
	for (i = 0; i < SIZE_BATINFO; i++) {
		ret = cw2015_read_reg(REG_BATINFO + i, &reg_val, 1);
		if (reg_val != config_info[i])
			return -1;
	}

	/* set cw2015/cw2013 to use new battery info */
	ret = cw2015_read_reg(REG_CONFIG, &reg_val, 1);
	if (ret < 0)
		return ret;

	reg_val |= CONFIG_UPDATE_FLG;   /* set UPDATE_FLAG */
	reg_val &= 0x07;                /* clear ATHD */
	reg_val |= ATHD;                /* set ATHD */
	ret = cw2015_write_reg(REG_CONFIG, &reg_val);
	if (ret < 0)
		return ret;

	/* check 2015/cw2013 for ATHD & update_flag */
	ret = cw2015_read_reg(REG_CONFIG, &reg_val, 1);
	if (ret < 0)
		return ret;

	if (!(reg_val & CONFIG_UPDATE_FLG))
		debug_x("update flag for new battery info have not set..\n");

	if ((reg_val & 0xf8) != ATHD)
		debug_x( "the new ATHD have not set..\n");

	/* reset */
	reset_val &= ~(MODE_RESTART);
	reg_val = reset_val | MODE_RESTART;
	ret = cw2015_write_reg(REG_MODE, &reg_val);
	if (ret < 0)
		return ret;

	mdelay(10);
	ret = cw2015_write_reg(REG_MODE, &reset_val);
	if (ret < 0)
		return ret;
	mdelay(10);

	return 0;
}

static int cw_init(void)
{
	int ret;
	int i;
	u8 reg_val = MODE_SLEEP;

	if ((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) {
		reg_val = MODE_NORMAL;
		ret = cw2015_write_reg(REG_MODE, &reg_val);
		if (ret < 0) {
			debug_x("[%s %s %d] error\n",
				__FILE__, __func__, __LINE__);
			return ret;
		}
	}

	ret = cw2015_read_reg(REG_CONFIG, &reg_val, 1);
	if (ret < 0) {
		debug_x("[%s %s %d] error\n",
			__FILE__, __func__, __LINE__);
		return ret;
	}
	if ((reg_val & 0xf8) != ATHD) {
		debug_x("the new ATHD have not set\n");
		reg_val &= 0x07;    /* clear ATHD */
		reg_val |= ATHD;    /* set ATHD */
		ret = cw2015_write_reg(REG_CONFIG, &reg_val);
		if (ret < 0) {
			debug_x("[%s %s %d] error\n",
				__FILE__, __func__, __LINE__);
			return ret;
		}
	}

	ret = cw2015_read_reg(REG_CONFIG, &reg_val, 1);
	if (ret < 0) {
		debug_x("[%s %s %d] error\n",
				__FILE__, __func__, __LINE__);
		return ret;
	}

	if (!(reg_val & CONFIG_UPDATE_FLG)) {
		debug_x("update flag for new battery info have not set\n");
		ret = cw_update_config_info();
		if (ret < 0) {
			debug_x("[%s %s %d] error\n",
				__FILE__, __func__, __LINE__);
			return ret;
		}
	} else {
		for(i = 0; i < SIZE_BATINFO; i++) {
			ret = cw2015_read_reg((REG_BATINFO + i), &reg_val, 1);
			if (ret < 0) {
				debug_x("[%s %s %d] error\n",
					__FILE__, __func__, __LINE__);
				return ret;
			}

			if (config_info[i] != reg_val)
				break;
		}

		if (i != SIZE_BATINFO) {
			debug_x("update flag for new battery info have not set\n");
			ret = cw_update_config_info();
			if (ret < 0) {
				debug_x("[%s %s %d] error\n",
					__FILE__, __func__, __LINE__);
				return ret;
			}
		}
	}

	for (i = 0; i < 30; i++) {
		ret = cw2015_read_reg(REG_SOC, &reg_val, 1);
		if (ret < 0) {
			debug_x("[%s %s %d] error\n",
				__FILE__, __func__, __LINE__);
			return ret;
		}
		else if (reg_val <= 0x64)
			break;

		mdelay(100);
		if (i > 25)
			debug_x("cw2015/cw2013 input unvalid power error\n");

	}
	if (i >=30) {
		reg_val = MODE_SLEEP;
		ret = cw2015_write_reg(REG_MODE, &reg_val);
		debug_x("cw2015/cw2013 input unvalid power error_2\n");
		return -1;
	}

	debug_x("[%s %s %d] cw_init success\n",
		__FILE__, __func__, __LINE__);
	return 0;
}

static unsigned int axp173_get_single_adc_data(enum adc_type type)
{
	unsigned char tmp[2] = {0,};
	unsigned int val = 0;

#define GET_ADC_VALUE(reg1, reg2)		\
	cw2015_read_reg(reg1, tmp, 1);		\
	cw2015_read_reg(reg2, tmp+1, 1)

	switch (type) {
	case ACIN_VOL:
		GET_ADC_VALUE(POWER_ACIN_VOL_H8, POWER_ACIN_VOL_L4);
		val = ((tmp[0] << 4) + (tmp[1] & 0x0f)) * 17 / 10;
		break;
	case ACIN_CUR:
		GET_ADC_VALUE(POWER_ACIN_CUR_H8, POWER_ACIN_CUR_L4);
		val = ((tmp[0] << 4) + (tmp[1] & 0x0f)) * 5 / 8;
		break;
	case VBUS_VOL:
		GET_ADC_VALUE(POWER_VBUS_VOL_H8, POWER_VBUS_VOL_L4);
		val = ((tmp[0] << 4) + (tmp[1] & 0x0f)) * 17 / 10;
		break;
	case VBUS_CUR:
		GET_ADC_VALUE(POWER_VBUS_CUR_H8, POWER_VBUS_CUR_L4);
		val = ((tmp[0] << 4) + (tmp[1] & 0x0f)) * 375 / 1000;
		break;
	case BAT_VOL:
		GET_ADC_VALUE(POWER_BAT_AVERVOL_H8, POWER_BAT_AVERVOL_L4);
		val = ((tmp[0] << 4) + ((tmp[1] & 0x0f))) * 11 / 10;
		break;
	case BAT_CUR:
		GET_ADC_VALUE(POWER_BAT_AVERCHGCUR_H8, POWER_BAT_AVERCHGCUR_L5);
			val = ((tmp[0] << 5) + (tmp[1] & 0x1f)) / 2;
		GET_ADC_VALUE(POWER_BAT_AVERDISCHGCUR_H8,
			POWER_BAT_AVERDISCHGCUR_L5);
		val += ((tmp[0] << 5) + (tmp[1] & 0x1f)) / 2;
		break;
	default:
		break;
	}
#undef GET_ADC_VALUE

	return val;
}

static unsigned int axp173_get_adjust_adc_data(enum adc_type type)
{
	int i = 0;
	unsigned int max_val = 0;
	unsigned int min_val = 0;
	unsigned int count = 0;
	unsigned int value = 0;
	int tmp = 0;
	int times = 0;

	for (; i < SAMPLE_COUNT; ++i) {
		tmp = axp173_get_single_adc_data(type);
		if (tmp > 0) {
			if (i == 0)
				max_val = min_val = tmp;
			count += tmp;
			times++;
			if (tmp > max_val)
				max_val = tmp;
			else if (tmp < min_val)
				min_val = tmp;
		}
	}

	switch (times) {
	case 0:
		value = 0;
		break;
	case 1:
	case 2:
		value = count / times;
		break;
	default:
		value = (count-max_val-min_val) / (times-2);
		break;
	}

	return value;
}

static unsigned int axp173_get_adc_data(enum adc_type type, int sw_adjust)
{
	return sw_adjust + axp173_get_adjust_adc_data(type);
}

static int get_pmu_current(void)
{
	return axp173_get_adc_data(BAT_CUR, 0);
}

static int get_pmu_voltage(void)
{
	return axp173_get_adc_data(BAT_VOL, 0);
}

static unsigned int jz_current_battery_voltage()
{
	unsigned int voltage = 0;
	int pmu_charging = 0;
	int pmu_current = 0;
	int value;

	gpio_direction_input(USB_DETE);
	value = gpio_get_value(USB_DETE);
	if(value == 0)
		pmu_charging = 1;
	if(value == 1)
		pmu_charging = 0;

	voltage = get_pmu_voltage();
	pmu_current = get_pmu_current();
	if (!which_battery) {
		voltage = pmu_charging == CHARGING_ON ? voltage - (pmu_current * INTER_RESIST
				/ 1000) : voltage + (pmu_current * INTER_RESIST / 1000);
	}
	else
		voltage = pmu_charging == CHARGING_ON ? voltage - (pmu_current * INTER_RESIST1
				/ 1000) : voltage + (pmu_current * INTER_RESIST1 / 1000);
//	printf("========++>pmu_charging = %d, pmu_current = %d, voltage = %d\n",
//		pmu_charging, pmu_current, voltage);

	return voltage;
}

static int jz_current_battery_current_cpt(unsigned int voltage)
{
	int i = 0;
	int cpt = 0;

	if (!which_battery) {
		for (; i < ARRAY_SIZE(ocv2soc); ++i)
			if (voltage >= ocv2soc[i].vol)
				break;
		if (i == 0)
			cpt = ocv2soc[i].cpt;
		else if (i == ARRAY_SIZE(ocv2soc))
			cpt = ocv2soc[i-1].cpt;
		else {
			int cpt_step = (ocv2soc[i-1].vol - ocv2soc[i].vol) /
					(ocv2soc[i-1].cpt - ocv2soc[i].cpt);
			int vol = ocv2soc[i-1].vol - voltage;

			cpt = ocv2soc[i-1].cpt - vol / cpt_step;
		}
	}
	else {
		for (; i < ARRAY_SIZE(ocv2soc1); ++i)
			if (voltage >= ocv2soc1[i].vol)
				break;
		if (i == 0)
			cpt = ocv2soc1[i].cpt;
		else if (i == ARRAY_SIZE(ocv2soc1))
			cpt = ocv2soc1[i-1].cpt;
		else {
			int cpt_step = (ocv2soc1[i-1].vol - ocv2soc1[i].vol) /
					(ocv2soc1[i-1].cpt - ocv2soc1[i].cpt);
			int vol = ocv2soc1[i-1].vol - voltage;

			cpt = ocv2soc1[i-1].cpt - vol / cpt_step;
		}
	}

        if(cpt < 0)
                cpt = 0;

	return cpt;
}

static void get_battery_nv_flag(void)
{
	if (which_battery == -1)
		which_battery = get_battery_flag();
}

int get_battery_status(void)
{
	int ret = 0;
	unsigned char power_status = 0;
	unsigned char power_mode_chgstatus = 0;

	cw2015_read_reg(POWER_STATUS, &power_status, 1);
        cw2015_read_reg(POWER_MODE_CHGSTATUS, &power_mode_chgstatus, 1);
	if ((!(power_mode_chgstatus & CHARGED_STATUS)) &&
		(power_status & (AC_AVAILABLE | USB_AVAILABLE)))
		ret = 100;

	return ret;
}

int get_battery_current_cpt(void)
{
	int cpt = 0;

	cw_init();

	cw2015_read_reg(0x04, &cpt, 1);

	return cpt;	
}


int low_power_detect(void)
{
	return 0;
	unsigned int voltage = 0;
	int i = 100;

	for(;i > 0;i--) {
		voltage = get_pmu_voltage();
		if (voltage == 0)
			continue;
		else if (voltage < 3500)
			return 1;
	}

	unsigned char tmp;
	tmp = 0;
	cw2015_write_reg(0x81, &tmp);

	return 0;
}

void disable_ldo4(void)
{
	int ret;
	unsigned char tmp;
	unsigned char reg = POWER_ON_OFF_REG;

	cw2015_read_reg(reg, &tmp, 1);

	tmp &= (~(1 << 1));
	ret = cw2015_write_reg(reg, &tmp);
	if (ret < 0)
		debug_x("Error in writing the POWER_ON_OFF_REG_Reg\n");
}

int axp173_disable_charge(void)
{
	int ret;
	unsigned char temp;
	unsigned char reg = POWER_CHARGE1;

	cw2015_read_reg(reg, &temp, 1);
	temp &= 0x7f;
	ret = cw2015_write_reg(reg, &temp);
	if (ret < 0) {
		debug_x("Error in writing the POWER_ON_OFF_REG_Reg\n");
		return -1;
	}

	return 0;
}


