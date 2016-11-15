#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <execinfo.h>
#include <errno.h>
#include <semaphore.h>

#include "utils_interface.h"
#include "localplayer_interface.h"
#include "tips_interface.h"
#include "event_interface.h"
#include "sharememory_interface.h"
#include "volume_interface.h"

#include "mozart_app.h"
#include "mozart_smartui.h"
#include "mozart_update.h"
#include "mozart_event.h"
#include "mozart_linein.h"
#include "mozart_net.h"
#include "mozart_battery.h"
#include "mozart_prompt_tone.h"
#include "baselib.h"
extern music_obj *global_music;
extern struct op *global_op;

#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#endif

char *global_app_name;
static event_handler *e_handler;
static event_handler *e_key_handler;
static event_handler *e_misc_handler;

static bool is_system_error = false;

static char *signal_str[] = {
	[1] = "SIGHUP",       [2] = "SIGINT",       [3] = "SIGQUIT",      [4] = "SIGILL",      [5] = "SIGTRAP",
	[6] = "SIGABRT",      [7] = "SIGBUS",       [8] = "SIGFPE",       [9] = "SIGKILL",     [10] = "SIGUSR1",
	[11] = "SIGSEGV",     [12] = "SIGUSR2",     [13] = "SIGPIPE",     [14] = "SIGALRM",    [15] = "SIGTERM",
	[16] = "SIGSTKFLT",   [17] = "SIGCHLD",     [18] = "SIGCONT",     [19] = "SIGSTOP",    [20] = "SIGTSTP",
	[21] = "SIGTTIN",     [22] = "SIGTTOU",     [23] = "SIGURG",      [24] = "SIGXCPU",    [25] = "SIGXFSZ",
	[26] = "SIGVTALRM",   [27] = "SIGPROF",     [28] = "SIGWINCH",    [29] = "SIGIO",      [30] = "SIGPWR",
	[31] = "SIGSYS",      [34] = "SIGRTMIN",    [35] = "SIGRTMIN+1",  [36] = "SIGRTMIN+2", [37] = "SIGRTMIN+3",
	[38] = "SIGRTMIN+4",  [39] = "SIGRTMIN+5",  [40] = "SIGRTMIN+6",  [41] = "SIGRTMIN+7", [42] = "SIGRTMIN+8",
	[43] = "SIGRTMIN+9",  [44] = "SIGRTMIN+10", [45] = "SIGRTMIN+11", [46] = "SIGRTMIN+12", [47] = "SIGRTMIN+13",
	[48] = "SIGRTMIN+14", [49] = "SIGRTMIN+15", [50] = "SIGRTMAX-14", [51] = "SIGRTMAX-13", [52] = "SIGRTMAX-12",
	[53] = "SIGRTMAX-11", [54] = "SIGRTMAX-10", [55] = "SIGRTMAX-9",  [56] = "SIGRTMAX-8", [57] = "SIGRTMAX-7",
	[58] = "SIGRTMAX-6",  [59] = "SIGRTMAX-5",  [60] = "SIGRTMAX-4",  [61] = "SIGRTMAX-3", [62] = "SIGRTMAX-2",
	[63] = "SIGRTMAX-1",  [64] = "SIGRTMAX",
};

static void sig_handler(int signo)
{
	char cmd[64] = {};
	void *array[10];
	int size = 0;
	char **strings = NULL;
	int i = 0;

	printf("\n\n[%s: %d] mozart crashed by signal %s.\n", __func__, __LINE__, signal_str[signo]);
	print("machine close\n");
	int fd = op_arg_get(global_op);
	if (fd <= 0) {
		print("\nerror\n");
		exit(0);
	}
	ftruncate(fd, 0);
    	lseek(fd, 0, SEEK_SET);

	machine_close(global_op, global_music);

	/*memory recycle*/
	op_delete(&global_op);
	music_list_destroy(&global_music);
	close(fd);


	printf("Call Trace:\n");
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	if (strings) {
		for (i = 0; i < size; i++)
			printf("  %s\n", strings[i]);
		free(strings);
	} else {
		printf("Not Found\n\n");
	}

	if (signo == SIGSEGV || signo == SIGBUS ||
	    signo == SIGTRAP || signo == SIGABRT) {
		sprintf(cmd, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmd);
	}

	printf("stop all services\n");
	stopall(APP_DEPEND_ALL);
	is_system_error = true;

	printf("unregister event manager\n");
	mozart_event_handler_put(e_handler);
	mozart_event_handler_put(e_key_handler);
	mozart_event_handler_put(e_misc_handler);

	share_mem_clear();
	share_mem_destory();

	ai_aiengine_delete();
	free(global_app_name);
	global_app_name = NULL;

//	ai_aitalk_sem_destory();

	ai_aitalk_send_destroy();
	exit(-1);
}

static void usage(const char *app_name)
{
	printf("%s [-bsh]\n"
	       " -h     help (show this usage text)\n"
	       " -s/-S  TODO\n"
	       " -b/-B  run a daemon in the background\n", app_name);

	return;
}

static inline void dump_compile_info(void)
{
	time_t timep;
	struct passwd *pwd;
	char hostname[16] = "Unknown";

	time(&timep);
	pwd = getpwuid(getuid());
	gethostname(hostname, 16);
	printf("mozart compiled at %s on %s@%s\n", asctime(gmtime(&timep)), pwd->pw_name, hostname);
}

static inline int initall(void)
{
	/* share memory init. */
	if (0 != share_mem_init()) {
		printf("share_mem_init failure.\n");
		exit(-1);
	}

	if (0 != share_mem_clear()) {
		printf("share_mem_clear failure.\n");
		exit(-1);
	}

	if (mozart_path_is_mount("/mnt/sdcard"))
		mozart_localplayer_scan();

	mozart_volume_set(60, BT_CALL_VOLUME);
	mozart_volume_set(40, BT_MUSIC_VOLUME);
	mozart_volume_set(60, MUSIC_VOLUME);

	ai_aitalk_sem_init();

	system("echo 0 > /proc/jz/rtc/alarm_flag");

	return 0;
}

int main(int argc, char **argv)
{
	int c, daemonize = 0;
	is_system_error = false;
	global_app_name = strdup(argv[0]);
	/* Get command line parameters */
	while (1) {
		c = getopt(argc, argv, "bBsSh");
		if (c < 0)
			break;
		switch (c) {
		case 'b':
		case 'B':
			daemonize = 1;
			break;
		case 's':
		case 'S':
			break;
		case 'h':
			usage(global_app_name);
			return 0;
		default:
			usage(global_app_name);
			return 1;
		}
	}

	/* run in the background */
	if (daemonize) {
		if (daemon(0, 1)) {
			perror("daemon");
			return -1;
		}
	}
	dump_compile_info();

	 /* signal hander */
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	initall();

	 /* start modules do not depend network. */
	startall(APP_DEPEND_NO_NET);

	 /* register key event */
	e_handler = mozart_event_handler_get(mozart_event_callback, global_app_name);
	mozart_battery_update();

	e_key_handler = mozart_event_handler_get(mozart_event_key_callback, global_app_name);
	e_misc_handler = mozart_event_handler_get(mozart_event_misc_callback, global_app_name);

	while (1) {
		sleep(20);
	#if (SUPPORT_MEMORY == MEMORY_32M)
		system("echo 3 > /proc/sys/vm/drop_caches");
	#endif
		if (!is_system_error){
			system("echo 3 > /dev/watchdog");
		}
	}

	return 0;
}

