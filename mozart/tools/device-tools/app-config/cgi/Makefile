CC = mipsel-linux-gcc
CXX= mipsel-linux-g++
STRIP = mipsel-linux-strip

CFLAGS = -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include
LDFLAGS = -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib

CFLAGS += -g -Wall -O2 -I$(MOZART_DIR)/usr/include
LDFLAGS += -lpthread -L$(MOZART_DIR)/usr/lib -lcgi -lm -lwifi -ljson-c -lutils -lini -lalarm -lpower -lota -ltips -lplayer -llinklist -lalarm_interface -lurl

DESTDIR ?=
PREFIX = /usr
SBIN_DIR = $(PREFIX)/sbin
CGI_BIN = /var/www/cgi-bin
INITDIR = /etc/init.d/

INSTALL = install

TARGETS = app_service  app_get_deviceinfo app_elife_register 

all:$(TARGETS)

app_service:app_service.o
	$(CC) $(LDFLAGS) $^ -o $@

app_get_deviceinfo:cJSON.o app_get_deviceinfo.o
	$(CC) $(LDFLAGS) $^ -o $@

app_elife_register:cJSON.o app_elife_register.o
	$(CC) $(LDFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	$(INSTALL) -d $(DESTDIR)$(CGI_BIN)
	$(INSTALL) -d $(DESTDIR)$(SBIN_DIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)

	$(INSTALL) -m 755 app_service $(DESTDIR)$(SBIN_DIR)
	$(INSTALL) -m 755 app_get_deviceinfo $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 app_elife_register $(DESTDIR)$(CGI_BIN)/
	$(INSTALL) -m 755 S88app.sh $(DESTDIR)$(INITDIR)
	$(INSTALL) -m 755 S99webserver.sh $(DESTDIR)$(INITDIR)

clean:
	-rm -rf *.o $(TARGETS)

uninstall:
	-rm -f $(DESTDIR)$(SBIN_DIR)/app_service
	-rm -f $(DESTDIR)$(CGI_BIN)/app_get_deviceinfo
	-rm -f $(DESTDIR)$(CGI_BIN)/app_elife_register
	-rm -rf $(DESTDIR)$(CGI_BIN)/
	-rm -f $(DESTDIR)$(INITDIR)/S88app.sh

.PHONY:all clean install uninstall $(TARGETS)
