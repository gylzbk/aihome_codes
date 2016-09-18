#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "power_interface.h"
#include "volume_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"

#ifndef MOZART_RELEASE
#define MOZART_BATTERY_DEBUG
#endif

#ifdef MOZART_BATTERY_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[BATTERY] %s: "fmt, __func__, ##args)
#else  /* MOZART_BATTERY_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_BATTERY_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[BATTERY] [Error] %s: "fmt, __func__, ##args)

static bool battery_exit;
static pthread_t battery_pthread;
static pthread_mutex_t battery_lock = PTHREAD_MUTEX_INITIALIZER;

static int lowpower_prompted;

#define BATTERY_LOWPOWER_THRESHOLD	20
int mozart_battery_update(void)
{
	int capacity, online;
	enum battery_status battery_st;

	battery_st = mozart_get_battery_status();
	if (battery_st == POWER_SUPPLY_STATUS_UNKNOWN) {
		pr_debug("unknown status ???\n");
		return -1;
	}

	capacity = mozart_get_battery_capacity();

#if SUPPORT_BOARD==BOARD_DS1825
	online = mozart_power_supply_online(POWER_SUPPLY_TYPE_MAINS);
#endif

#if SUPPORT_BOARD==BOARD_WB38
	online = mozart_power_supply_online(POWER_SUPPLY_TYPE_BATTERY);
#endif

	pr_debug("online: %d, status: %d, cpt: %d\n", online, battery_st, capacity);

	mozart_smartui_battery_update(capacity, online);

	if (capacity == 15)
		mozart_module_power_off("电量过低");

	pthread_mutex_lock(&battery_lock);

	if (!online && !lowpower_prompted && capacity < BATTERY_LOWPOWER_THRESHOLD) {
		lowpower_prompted = true;
		mozart_prompt_tone_key_sync("low_power", true);
	} else if (capacity >= BATTERY_LOWPOWER_THRESHOLD) {
		lowpower_prompted = false;
	}

	pthread_mutex_unlock(&battery_lock);

	return 0;
}
#undef BATTERY_LOWPOWER_THRESHOLD

static void *battery_func(void *args)
{

	pthread_detach(pthread_self());
	while (!battery_exit) {
		mozart_battery_update();
		sleep(60);
	}

	return NULL;
}

int mozart_battery_startup(void)
{
	battery_exit = false;

	if (pthread_create(&battery_pthread, NULL, battery_func, NULL) != 0) {
		pr_err("Create pthread fail!\n");
		return -1;
	}
//	pthread_detach(battery_pthread);

	return 0;
}

int mozart_battery_shutdown(void)
{
	battery_exit = true;

	return 0;
}
