CC = mipsel-linux-gcc
CXX= mipsel-linux-g++
STRIP = mipsel-linux-strip

CFLAGS = -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include
LDFLAGS = -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib

CFLAGS += -g -Wall -O2 -I$(MOZART_DIR)/usr/include
LDFLAGS += -lpthread -L$(MOZART_DIR)/usr/lib -lcgi -lm -lwifi -ljson-c -lutils -lini -lalarm -lpower -lota -ltips -lplayer -llinklist -lalarm_interface

DESTDIR ?=
PREFIX = /usr
SBIN_DIR = $(PREFIX)/sbin
CGI_BIN = /var/www/cgi-bin
INITDIR = /etc/init.d/

INSTALL = install

TARGETS = config_sta download_mp3 wifi_mode_switch app_service ap_config_page ap_config_confirm_page sta_config_page sta_config_confirm_page app_get_music_list app_get_deviceinfo app_start_update app_start_recovery check_mp3file app_time_get_clock app_time_set_clock app_add_alarm app_get_alarm_list app_delete_alarm app_update_alarm app_check_sdcard app_get_downloadlist app_get_timer_timestamp app_set_timer 

all:$(TARGETS)

config_sta:config_sta.o
	$(CC) $(LDFLAGS) $^ -o $@

sta_config_page:sta_config_page.o
	$(CC) $(LDFLAGS) -llinklist $^ -o $@

sta_config_confirm_page:sta_config_confirm_page.o
	$(CC) $(LDFLAGS) -llinklist $^ -o $@

ap_config_page:ap_config_page.o
	$(CC) $(LDFLAGS) $^ -o $@

ap_config_confirm_page:ap_config_confirm_page.o
	$(CC) $(LDFLAGS) -llinklist $^ -o $@

download_mp3:download_mp3.o
	$(CC) $(LDFLAGS) $^ -o $@

wifi_mode_switch:wifi_mode_switch.o
	$(CC) $(LDFLAGS) $^ -o $@

app_service:app_service.o
	$(CC) $(LDFLAGS) $^ -o $@

app_get_music_list:cJSON.o app_get_music_list.o
	$(CC) $(LDFLAGS) $^ -o $@

app_get_deviceinfo:cJSON.o app_get_deviceinfo.o
	$(CC) $(LDFLAGS) $^ -o $@

app_start_update:app_start_update.o
	$(CC) $(LDFLAGS) $^ -o $@

app_start_recovery:app_start_recovery.o
	$(CC) $(LDFLAGS) $^ -o $@

check_mp3file:check_mp3file.o
	$(CC) $(LDFLAGS) $^ -o $@

app_time_get_clock:app_time_get_clock.o
	$(CC) $(LDFLAGS) $^ -o $@

app_time_set_clock:app_time_set_clock.o
	$(CC) $(LDFLAGS) $^ -o $@

app_add_alarm:cJSON.o app_add_alarm.o
	$(CC) $(LDFLAGS) $^ -o $@

app_get_alarm_list: app_get_alarm_list.o
	$(CC) $(LDFLAGS) $^ -o $@

app_delete_alarm: app_delete_alarm.o
	$(CC) $(LDFLAGS) $^ -o $@

app_update_alarm: cJSON.o app_update_alarm.o
	$(CC) $(LDFLAGS) $^ -o $@

app_check_sdcard:app_check_sdcard.o
	$(CC) $(LDFLAGS) $^ -o $@

app_get_downloadlist:cJSON.o app_get_downloadlist.o
	$(CC) $(LDFLAGS) $^ -o $@

app_get_timer_timestamp:cJSON.o app_get_timer_timestamp.o
	$(CC) $(LDFLAGS) $^ -o $@

app_set_timer:app_set_timer.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	$(INSTALL) -d $(DESTDIR)$(CGI_BIN)
	$(INSTALL) -d $(DESTDIR)$(SBIN_DIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)

	$(INSTALL) -m 755 app_service $(DESTDIR)$(SBIN_DIR)
	$(INSTALL) -m 755 config_sta $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 download_mp3 $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 wifi_mode_switch $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 ap_config_page $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 ap_config_confirm_page $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 sta_config_page $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 sta_config_confirm_page $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_get_deviceinfo $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_get_music_list $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_start_update $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_start_recovery $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 check_mp3file $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_time_get_clock $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_time_set_clock $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_add_alarm $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_get_alarm_list $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_delete_alarm $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_update_alarm $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_check_sdcard $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_get_downloadlist $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_get_timer_timestamp $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_set_timer $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 S88app.sh $(DESTDIR)$(INITDIR)
	$(INSTALL) -m 755 S99webserver.sh $(DESTDIR)$(INITDIR)

clean:
	-rm -rf *.o $(TARGETS)

uninstall:
	-rm -f $(DESTDIR)$(SBIN_DIR)/app_service
	-rm -f $(DESTDIR)$(CGI_BIN)/config_sta
	-rm -f $(DESTDIR)$(CGI_BIN)/download_mp3
	-rm -f $(DESTDIR)$(CGI_BIN)/wifi_mode_switch
	-rm -f $(DESTDIR)$(CGI_BIN)/ap_config_page
	-rm -f $(DESTDIR)$(CGI_BIN)/sta_config_page
	-rm -f $(DESTDIR)$(CGI_BIN)/app_get_music_list
	-rm -f $(DESTDIR)$(CGI_BIN)/app_get_deviceinfo
	-rm -f $(DESTDIR)$(CGI_BIN)/app_start_update
	-rm -f $(DESTDIR)$(CGI_BIN)/app_start_recovery
	-rm -f $(DESTDIR)$(CGI_BIN)/sta_config_confirm_page
	-rm -f $(DESTDIR)$(CGI_BIN)/check_mp3file
	-rm -f $(DESTDIR)$(CGI_BIN)/app_time_get_clock
	-rm -f $(DESTDIR)$(CGI_BIN)/app_time_set_clock
	-rm -f $(DESTDIR)$(CGI_BIN)/app_time_set_clock
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_add_alarm
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_get_alarm_list
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_delete_alarm
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_update_alarm
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_check_sdcard
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_get_downloadlist
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_get_timer_timestamp
	-rm -rf $(DESTDIR)$(CGI_BIN)/app_set_timer
	-rm -rf $(DESTDIR)$(CGI_BIN)/
	-rm -f $(DESTDIR)$(INITDIR)/S88app.sh

.PHONY:all clean install uninstall $(TARGETS)
