#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include "linein.h"
#include "app.h"
#include "key_function.h"

extern snd_source_t snd_source;
#if (SUPPORT_VR != VR_NULL)
static int wakeup_mode_mark = VOICE_KEY_MIX_WAKEUP;
#endif

#if (SUPPORT_DMR == 1)
void geak_callback(char *ActionName, struct Upnp_Action_Request *ca_event)
{
    printf("Not support AVTransportAction: %s\n", ActionName);
    return;
}
#endif

int start(app_t app)
{
	switch(app){
#if (SUPPORT_DMR == 1)
	case DMR:
		{
			/*
			 * this is our default frendlyname rule in librender.so,
			 * and you can create your own frendlyname rule.
			 */
			char deviceName[64] = {};
			char macaddr[] = "255.255.255.255";
			memset(macaddr, 0, sizeof (macaddr));

			//FIXME: replace "wlan0" with other way. such as config file.
			get_mac_addr("wlan0", macaddr, "");
			sprintf(deviceName, "SmartAudio-%s", macaddr + 4);

                        mozart_render_AVTaction_callback(geak_callback);
			mozart_render_start(deviceName);
		}
		break;
#endif
#if (SUPPORT_DMS == 1)
	case DMS:
                if (start_dms())
                        printf("ushare service start failed in %s:%d \
                               You Should Check It As Soon As Possible!!!!!\n\n\n", __func__, __LINE__);
                break;
#endif
	case DMC:
		break;
#if (SUPPORT_AIRPLAY == 1)
	case AIRPLAY:
                mozart_airplay_init();
		mozart_airplay_start_service();
		break;
#endif
#if (SUPPORT_LOCALPLAYER == 1)
	case LOCALPLAYER:
		mozart_localplayer_startup();
		break;
#endif
#if (SUPPORT_BT != BT_NULL)
	case BT:
		start_bt();
		break;
#endif

#if (SUPPORT_VR != VR_NULL)
	case VR:
#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_VOICE)
		wakeup_mode_mark = VOICE_WAKEUP;
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_SHORTPRESS)
		wakeup_mode_mark = KEY_SHORTPRESS_WAKEUP;
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
		wakeup_mode_mark = KEY_LONGPRESS_WAKEUP;
#endif
#if (SUPPORT_VR == VR_BAIDU)
		mozart_vr_startup();
#elif (SUPPORT_VR == VR_SPEECH)
		mozart_vr_speech_startup(wakeup_mode_mark, mozart_vr_speech_interface_callback);
#elif (SUPPORT_VR == VR_IFLYTEK)
		mozart_vr_iflytek_startup(wakeup_mode_mark, mozart_vr_iflytek_interface_callback);
#elif (SUPPORT_VR == VR_UNISOUND)
		printf("TODO: vr unisound start\n");
#elif (SUPPORT_VR == VR_ATALK)
		mozart_vr_atalk_startup(wakeup_mode_mark, mozart_vr_atalk_interface_callback);
#endif
		break;
#endif

#if (SUPPORT_LAPSULE == 1)
	case LAPSULE:
                if (start_lapsule_app()) {
                        printf("start lapsule services failed in %s: %s.\n", __func__, strerror(errno));
                }
		break;
#endif
#if (SUPPORT_ATALK == 1)
	case ATALK:
		if (start_atalk_app())
			printf("start atalk services failed in %s: %s.\n", __func__, strerror(errno));
		break;
#endif
	default:
		printf("WARNING: Not support this module(id = %d)\n", app);
		break;
	}

	return 0;
}

int stop(app_t app)
{
	switch(app){
#if (SUPPORT_DMR == 1)
        case DMR:
                mozart_render_terminate();
		break;
#endif
#if (SUPPORT_DMS == 1)
	case DMS:
                stop_dms();
		break;
#endif
	case DMC:
		break;
#if (SUPPORT_AIRPLAY == 1)
	case AIRPLAY:
		mozart_airplay_shutdown();
		break;
#endif
#if (SUPPORT_LOCALPLAYER == 1)
	case LOCALPLAYER:
		mozart_localplayer_shutdown();
		break;
#endif
#if (SUPPORT_BT != BT_NULL)
	case BT:
		stop_bt();
		break;
#endif
#if (SUPPORT_VR != VR_NULL)
	case VR:
#if (SUPPORT_VR == VR_BAIDU)
		mozart_vr_shutdown();
#elif (SUPPORT_VR == VR_SPEECH)
		mozart_vr_speech_shutdown();
#elif (SUPPORT_VR == VR_IFLYTEK)
		mozart_vr_iflytek_shutdown();
#elif (SUPPORT_VR == VR_UNISOUND)
		printf("TODO: vr unisound stop\n");
#elif (SUPPORT_VR == VR_ATALK)
		mozart_vr_atalk_shutdown();
#endif
		break;
#endif
#if (SUPPORT_LAPSULE == 1)
	case LAPSULE:
		stop_lapsule_app();
		break;
#endif
#if (SUPPORT_ATALK == 1)
	case ATALK:
		stop_atalk_app();
		break;
#endif
	default:
		printf("WARNING: Not support this module(id = %d)\n", app);
		break;
	}

	return 0;
}


int startall(int depend_network)
{
	if (depend_network == -1 || depend_network == 1) {
#if (SUPPORT_DMR == 1)
		start(DMR);
#endif
#if (SUPPORT_DMS == 1)
		start(DMS);
#endif
		start(DMC);
#if (SUPPORT_AIRPLAY == 1)
		start(AIRPLAY);
#endif
#if (SUPPORT_VR != VR_NULL)
		start(VR);
#endif
#if (SUPPORT_LAPSULE == 1)
		start(LAPSULE);
#endif
	}

	if (depend_network == -1 || depend_network == 0) {
#if (SUPPORT_ATALK == 1)
		start(ATALK);
#endif
#if (SUPPORT_BT != BT_NULL)
		start(BT);
#endif
#if (SUPPORT_LOCALPLAYER == 1)
		start(LOCALPLAYER);
#endif
	}

	return 0;
}

int stopall(int depend_network)
{
	if (depend_network == -1 || depend_network == 1) {
#if (SUPPORT_LAPSULE == 1)
		stop(LAPSULE);
#endif
#if (SUPPORT_DMR == 1)
		stop(DMR);
#endif
#if (SUPPORT_DMS == 1)
		stop(DMS);
#endif
#if (SUPPORT_AIRPLAY == 1)
		stop(AIRPLAY);
#endif
#if (SUPPORT_VR != VR_NULL)
		stop(VR);
#endif
	}

	if (depend_network == -1 || depend_network == 0) {
#if (SUPPORT_ATALK == 1)
		stop(ATALK);
#endif
#if (SUPPORT_BT != BT_NULL)
		stop(BT);
#endif
#if (SUPPORT_LOCALPLAYER == 1)
		stop(LOCALPLAYER);
#endif
	}

	return 0;
}

int restartall(int depend_network)
{
	stopall(depend_network);
	startall(depend_network);

	return 0;
}

extern int mozart_stop_handler(bool mode_stop);
int mozart_stop(void)
{
	return mozart_stop_handler(true);
}
