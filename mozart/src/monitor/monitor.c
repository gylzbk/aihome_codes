#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>


char *global_app_name = NULL;
char *save_path = NULL;

const static char *signal_str[] = {
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

	printf("\n\n[%s: %d] mozart crashed by signal %s.\n", __func__, __LINE__, signal_str[signo]);

	if (signo == SIGSEGV || signo == SIGBUS ||
	    signo == SIGTRAP || signo == SIGABRT) {
		sprintf(cmd, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmd);
	}

	free(save_path);
	free(global_app_name);
	exit(-1);
}

int main(int argc, char **argv){
	if (argc < 3){
		printf("please input as: \n");
		printf(" monitor -b /mnt/sdcard/ \n");
		return -1;
	}

	 /* signal hander */
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	save_path = strdup(argv[2]);
	char cmd [128] = {0}; 

	while (1) {
		sleep(5);
		system("date  >> /mnt/sdcard/mem.log");
		sprintf(cmd,"ps wwl | grep mozart | grep -v grep | tr -s ' ' | cut -d' ' -f6  >> %s/mem.log", save_path);
		system(cmd);
	}
	return 0;
}
