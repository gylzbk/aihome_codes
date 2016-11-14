define ifexist
$(shell if [ -f $(1) ]; then echo "exist"; else :; fi;)
endef

define ifgt
$(shell file_size=`du -b $1 | cut -f1` && \
if [[ $$file_size -gt $2 ]];then echo "greater"; else :; fi;)
endef

# $1: string
define strlen
$(shell echo `expr length $(1)`)
endef

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

# remove all bt utils supported by bluez.
define remove_bluez
	-rm -rf $(APPFS_DIR)/etc/dbus-1
	-rm -rf $(APPFS_DIR)/etc/bluetooth
	-rm -rf $(APPFS_DIR)/lib/firmware/rtlbt
	-rm -rf $(APPFS_DIR)/var/run/dbus
	-rm -rf $(APPFS_DIR)/var/lib/dbus
	-rm -rf $(APPFS_DIR)/usr/bin/{dbus-*,bt_audio,hcitool}
	-rm -rf $(APPFS_DIR)/usr/bin/{ircp,irobex_palm3,irxfer}
	-rm -rf $(APPFS_DIR)/usr/bin/{obex_*,rtk_hciattach,sdptool,sndfile-resample}
	-rm -rf $(APPFS_DIR)/usr/sbin/{bt_enable,bt_disable}
	-rm -rf $(APPFS_DIR)/usr/sbin/{agent,bluetoothd,hciconfig,ofonod}
	-rm -rf $(APPFS_DIR)/usr/lib/{bluetooth,libusb*,libbluetooth.*,libdbus-1.*,libexpat.*,libglib-2.0.*,libopenobex.*}
	-rm -rf $(APPFS_DIR)/usr/share/{dbus-1,glib-2.0}
endef

# remove all bt utils supported by bsa protocol.
define remove_bsa
	-rm -rf $(APPFS_DIR)/etc/init.d/S04bsa.sh
	-rm -rf $(APPFS_DIR)/usr/lib/libbsa.so
	-rm -rf $(APPFS_DIR)/usr/sbin/bsa_server_mips
	-rm -rf $(APPFS_DIR)/usr/bin/app_avk
	-rm -rf $(APPFS_DIR)/usr/bin/app_manager
	-rm -rf $(APPFS_DIR)/usr/bin/app_hs
	-rm -rf $(APPFS_DIR)/var/run/bsa
endef

# remove all vr utils supported by baidu.
define remove_vr_baidu
	-rm -rf $(APPFS_DIR)/usr/lib/libmovr.so
ifneq ("$(SUPPORT_LAPSULE)","1")
	-rm -rf $(APPFS_DIR)/usr/lib/{libcurl*}
endif
	-rm -rf $(APPFS_DIR)/usr/bin/httpsget
	-rm -rf $(APPFS_DIR)/etc/resources.cfg
	-rm -rf $(APPFS_DIR)/etc/s_1
	-rm -rf $(APPFS_DIR)/etc/factory.cfg
	-rm -rf $(APPFS_DIR)/etc/biss.cfg
	-rm -rf $(APPFS_DIR)/etc/biss.cfg
	-rm -rf $(APPFS_DIR)/etc/jsonsim/
endef

# remove all vr utils supported by speech.
define remove_vr_speech
	-rm -rf $(APPFS_DIR)/usr/lib/libaiengine.so
	-rm -rf $(APPFS_DIR)/usr/lib/libvr_speech.so
	-rm -rf $(APPFS_DIR)/usr/share/vr/bin/
endef

# remove all vr utils supported by iflytek.
define remove_vr_iflytek
	-rm -rf $(APPFS_DIR)/usr/lib/libmsc.so
	-rm -rf $(APPFS_DIR)/usr/lib/libvr_iflytek.so
endef

# remove all vr utils supported by unisound.
define remove_vr_unisound
	@: #TODO
endef

# remove all vr utils supported by atalk.
define remove_vr_atalk
	-rm -rf $(APPFS_DIR)/usr/lib/libvr_atalk.so
	-rm -rf $(APPFS_DIR)/usr/lib/{libopus.so,librecognizer.so,libvad.so,libvoicesender_dynamic.so,libwebsockets.so}
	-rm -rf $(APPFS_DIR)/etc/atalk/sdk-config.json
endef

# remove all alsa utils.
define remove_audio_alsa
	-rm -rf $(UPDATERFS_DIR)/usr/lib/alsa-lib
	-rm -rf $(APPFS_DIR)/usr/share/{alsa,sounds,locale}
	-rm -rf $(APPFS_DIR)/lib/udev/rules.d/90-alsa-restore.rules
	-rm -f $(APPFS_DIR)/usr/lib/libasound.so*
	-rm -f $(APPFS_DIR)/usr/bin/{aconnect,alsaloop,alsaucm,amidi,amixer,aplay,aplaymidi,arecord,arecordmidi,aseqdump,aseqnet,iecset,aserver,speaker-test}
	-rm -f $(UPDATERFS_DIR)/usr/bin/aserver
	-rm -f $(APPFS_DIR)/usr/sbin/{alsaconf,alsactl}
	-rm -rf $(UPDATERFS_DIR)/etc/alsa/
	-rm -f $(UPDATERFS_DIR)/usr/lib/libasound.so*
	mv $(UPDATERFS_DIR)/usr/lib/libfake_alsalib.so $(UPDATERFS_DIR)/usr/lib/libasound.so.2
endef

rootfs:
	@echo
	@echo
	@echo
	@$(call MESSAGE,"Targets:")
	@for file in $(TARGET_DIR)/*; do \
		echo -e "$$file: `$(TOPDIR)/configs/scripts/get_readable_filesize.sh $$file`"; \
	done \

update:
	@echo
	@$(call MESSAGE,"Ota Packages:")
	@echo
	@for product in output/updatepkg/*; do \
		echo "product: `basename $$product`"; \
		for version in $$product/v*; do \
			echo "    version: `basename $$version`"; \
			for file in $$version/*; do \
				echo -e "        $$file: `$(TOPDIR)/configs/scripts/get_readable_filesize.sh $$file`"; \
			done \
		done \
	done
	@echo


##############################################
#         install rootfs rules           #
##############################################
fakeroot_bin=$(OUTPUT_DIR)/host/usr/bin/fakeroot
FAKEROOT_SCRIPT = $(TARGET_DIR)/fakeroot.fs
DEVICE_TABLES = $(TARGET_DIR)/devs.txt

usrdata:

##############################################
#         install usrdata rules           #
##############################################
define install_usrdata_rules
# Params 1: fs type:
#      jffs2: make jffs2

$(TARGET_DIR)/usrdata.$(1): usrdata-$(1)-prepare
	@$(call MESSAGE,"Build usrdata.$(1)")
	$(call USRDATA_$(1)_CMDS,$(USRDATAFS_DIR),$$@)

usrdata-$(1)-prepare: $$(USRDATA_$(1)_DEPENDENCIES)
	-rm -rf $(USRDATAFS_DIR); mkdir -p $(USRDATAFS_DIR)
	cp $(UPDATERFS_DIR)/usr/share/data/* $(USRDATAFS_DIR)/

usrdata: $(TARGET_DIR)/usrdata.$(1)
endef

nvimage:

##############################################
#         install nvimage rules           #
##############################################
define install_nvimage_rules

$(TARGET_DIR)/nv.img:
	@$(call MESSAGE,"Build nv.img")
	$(call NV_IMAGE_CMDS,$$@)

nvimage: $(TARGET_DIR)/nv.img
endef

define install_fs_rules
# Params 1: fs type:
#   ramdisk: make ramdisk
#      ext4: make ext4
#    cramfs: make cramfs
# Params 2: need compress the target after make fs done?
#    1: compress with gzip -9
#    0: do not compress

ROOTFS_$(1)_DEPENDENCIES += host-fakeroot host-makedevs host-inirw

rootfs-$(1)-show-depends:
	@echo $$(ROOTFS_$(1)_DEPENDENCIES)

$(TARGET_DIR)/updater.$(1)::
	@$(call MESSAGE,"Build updater.$(1)")
	echo "chown -R 0:0 $(APPFS_DIR)" > $(FAKEROOT_SCRIPT)
	echo "makedevs -d $(DEVICE_TABLES) $(UPDATERFS_DIR) 2>/dev/null" >> $(FAKEROOT_SCRIPT)
	echo "$(call ROOTFS_UPDATER_$(1)_CMDS,$(UPDATERFS_DIR),$$@)" >> $(FAKEROOT_SCRIPT)
	chmod a+x $(FAKEROOT_SCRIPT)
	PATH=$(OUTPUT_DIR)/host/usr/bin:$(OUTPUT_DIR)/host/usr/sbin:$(PATH) \
	     $(fakeroot_bin) -l $(OUTPUT_DIR)/host/usr/lib/libfakeroot.so -f $(OUTPUT_DIR)/host/usr/bin/faked $(FAKEROOT_SCRIPT)
ifeq ($(2),1)
	gzip -9 -c $$@ > $$@.gz
endif
	rm -f $(FAKEROOT_SCRIPT) $(DEVICE_TABLES)

$(TARGET_DIR)/appfs.$(1)::
	@$(call MESSAGE,"Build appfs.$(1)")
	echo "chown -R 0:0 $(APPFS_DIR)" > $(FAKEROOT_SCRIPT)
	echo "$(call ROOTFS_APP_$(1)_CMDS,$(APPFS_DIR),$$@)" >> $(FAKEROOT_SCRIPT)
	chmod a+x $(FAKEROOT_SCRIPT)
	PATH=$(OUTPUT_DIR)/host/usr/bin:$(OUTPUT_DIR)/host/usr/sbin:$(PATH) \
	     $(fakeroot_bin) -l $(OUTPUT_DIR)/host/usr/lib/libfakeroot.so -f $(OUTPUT_DIR)/host/usr/bin/faked $(FAKEROOT_SCRIPT)
ifeq ($(2),1)
	gzip -9 -c $$@ > $$@.gz
endif
	rm -f $(FAKEROOT_SCRIPT)

rootfs-$(1)-prepare: pre-prepare rootfs-$(1)-updater-prepare rootfs-$(1)-appfs-prepare
	mkdir -p $(UPDATERFS_DIR)/usr/share/data/
	cp -rf $(UPDATERFS_DIR)/usr/data/ $(UPDATERFS_DIR)/usr/share/
	-cp -rf $(APPFS_DIR)/usr/data/ $(UPDATERFS_DIR)/usr/share/
	rm -rf $(UPDATERFS_DIR)/usr/data/* $(APPFS_DIR)/usr/data

rootfs-$(1)-fs: $(TARGET_DIR)/updater.$(1) $(TARGET_DIR)/appfs.$(1)

rootfs-$(1):$$(ROOTFS_$(1)_DEPENDENCIES) host-$(1)-fakeroot-check rootfs-$(1)-prepare rootfs-$(1)-fs

rootfs:rootfs-$(1) usrdata nvimg

rootfs-$(1)-updater-prepare:
	cp -f $(TOPDIR)/configs/devs.txt $(DEVICE_TABLES)
	@$(call MESSAGE,"Prepare updater directory: $(UPDATERFS_DIR)")
	-rm -rf $(UPDATERFS_DIR); mkdir -p $(UPDATERFS_DIR)
	mkdir -p $(UPDATERFS_DIR)/{bin,lib,sbin,usr}
	mkdir -p $(UPDATERFS_DIR)/lib/firmware
	mkdir -p $(UPDATERFS_DIR)/{dev,root,var,tmp,proc,mnt,sys}
	mkdir -p $(UPDATERFS_DIR)/{usr/lib,usr/bin,usr/sbin}
	ln -sf /tmp $(UPDATERFS_DIR)/run
	@$(call MESSAGE,"Prepare rootfs base environment")
	cp $(TOPDIR)/rootfs/updater/* $(UPDATERFS_DIR)/ -a
	cp $(MOZART_UPDATER_DIR)/* $(UPDATERFS_DIR)/ -a
	cp $(MOLIB_UPDATER_DIR)/* $(UPDATERFS_DIR)/ -a

	@$(call MESSAGE,"Remove VCS files")
	find $(UPDATERFS_DIR) -iname .svn | xargs rm -rf
	@$(call MESSAGE,"Remove useless files")
	find $(UPDATERFS_DIR) \( -name '*.a' -o -name '*.la' \) | xargs rm -rf
	find $(UPDATERFS_DIR) \( -name '.svn' -o -name '*.git' -o -name '.gitignore' \) | xargs rm -rf
	-rm -rf $(UPDATERFS_DIR)/usr/sbin/ppp*
	-rm -rf $(UPDATERFS_DIR)/usr/sbin/chat
	-rm -rf $(UPDATERFS_DIR)/usr/sbin/usb_modeswitch
	-rm -rf $(UPDATERFS_DIR)/usr/include
	-rm -rf $(UPDATERFS_DIR)/usr/man
	-rm -rf $(UPDATERFS_DIR)/usr/lib/pkgconfig
	-rm -rf $(UPDATERFS_DIR)/usr/share/{gdb,man,info,doc}
	-rm -rf $(UPDATERFS_DIR)/usr/share/aclocal
	ln -sf /tmp/resolv.conf $(UPDATERFS_DIR)/etc/resolv.conf
	ln -sf /usr/fs/var/www $(UPDATERFS_DIR)/var/www
	ln -sf /usr/fs/usr/share/render $(UPDATERFS_DIR)/usr/share/render
#ifeq ("$(SUPPORT_VR)", "vr_speech")
#	ln -sf /usr/fs/usr/share/vr/bin $(UPDATERFS_DIR)/usr/share/vr/bin
#endif
ifeq ("$(SUPPORT_LAPSULE)","1")
	ln -sf /usr/lib/libwifi.so $(UPDATERFS_DIR)/usr/lib/libwifi_interface.so
endif

ifneq ("$(SUPPORT_LCD)","1")
	-rm -rf $(UPDATERFS_DIR)/usr/lib/{liblcd,liblcdshow,libpng}
	-rm -rf $(UPDATERFS_DIR)/usr/bin/uni2gbk.bin
	-rm -rf $(UPDATERFS_DIR)/usr/share/vr/tone/HZK16
endif

ifneq ("$(SUPPORT_SMARTUI)","1")
	-rm -rf $(UPDATERFS_DIR)/usr/share/ui/uni2gbk.bin
	-rm -rf $(UPDATERFS_DIR)/usr/share/ui/HZK16
	-rm -rf $(UPDATERFS_DIR)/usr/lib/libsmartui.so
endif

	@$(call MESSAGE,"Prepare nv settings")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k storage -v $(FLASH_TYPE)
ifeq ("$(FLASH_TYPE)","spinor")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k blkname -v /dev/mtdblock1
endif
ifeq ("$(FLASH_TYPE)","spinand")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k blkname -v /dev/mtdblock1
endif
ifeq ("$(FLASH_TYPE)","emmc")
	# reset nv blkname.
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s nv -k blkname -v /dev/mmcblk0p2
endif


	@$(call MESSAGE,"Prepare device info")
# correct fs type
	sed -i 's/USRFSFS/$(SUPPORT_FS)/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
	sed -i 's/USRDATAFS/$(SUPPORT_USRDATA)/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
ifeq ("$(1)","ext4")
	# auto-mount true mmc card.
	sed -i 's/^mmcblk\[0/^mmcblk\[1/g' $(UPDATERFS_DIR)/etc/mdev/mmc.sh
	sed -i 's/YesOrNo/0/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
else
ifeq ("$(1)","ubifs")
	sed -i 's/YesOrNo/0/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
	sed -i 's/\(mount -t ubifs\ \)\(.*\)\(\ \/usr\/fs\)/\1ubi1:app\3/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
else
	sed -i 's/YesOrNo/1/g' $(UPDATERFS_DIR)/etc/init.d/S00system_init.sh
endif
endif # correct fs type
# cpu model filter.
ifneq ("$(CPU_MODEL)","")
		$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k cpu -v $(CPU_MODEL)
endif
# product filter.
ifneq ("$(PRODUCT_NAME)","")
		$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k name -v $(PRODUCT_NAME)
endif


# audio device filter.
ifneq ("$(AUDIO_DEV_PLAYBACK)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_playback -v $(AUDIO_DEV_PLAYBACK)
endif

ifneq ("$(AUDIO_DEV_RECORD)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_record -v $(AUDIO_DEV_RECORD)
endif

ifneq ("$(AUDIO_DEV_PCM)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_pcm -v $(AUDIO_DEV_PCM)
endif

ifneq ("$(AUDIO_DEV_SPDIFOUT)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k dev_spdifout -v $(AUDIO_DEV_SPDIFOUT)
endif


# wifi filter.
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "rtk") # realtek's wifi module.
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k wifi -v rtk
	mv $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_realtek $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant
	mv $(UPDATERFS_DIR)/usr/sbin/wpa_cli_realtek $(UPDATERFS_DIR)/usr/sbin/wpa_cli
	mv $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_realtek $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_broadcom
	rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_cli_broadcom
	rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_broadcom
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "bcm") # broadcom's wifi module.
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k wifi -v bcm
	mv $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant
	mv $(UPDATERFS_DIR)/usr/sbin/wpa_cli_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_cli
	mv $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_broadcom $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_supplicant_realtek
	rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_cli_realtek
	rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase_realtek
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6181") # ap6181 module, bcm43362 inside.
	cp -r $(TOPDIR)/configs/firmwares/ap6181/wifi/* $(UPDATERFS_DIR)/lib/firmware/
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "bcm43438") # bcm43438, core on broad.
	cp -r $(TOPDIR)/configs/firmwares/43438/wifi/* $(UPDATERFS_DIR)/lib/firmware/
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6212") # ap6212 module, bcm43438a0 inside.
	cp -r $(TOPDIR)/configs/firmwares/ap6212/wifi/* $(UPDATERFS_DIR)/lib/firmware/
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "ap6212a") # ap6212a module, bcm43438a1 inside.
	cp -r $(TOPDIR)/configs/firmwares/ap6212a/wifi/* $(UPDATERFS_DIR)/lib/firmware/
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "bcm43362") # bcm43362, core on broad.
	cp -r $(TOPDIR)/configs/firmwares/43362/wifi/* $(UPDATERFS_DIR)/lib/firmware/
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),5,$(call strlen,$(SUPPORT_WIFI_MODULE)))", "iw8101") # iw8101 module, bcm43362 inside.
	cp -r $(TOPDIR)/configs/firmwares/iw8101/wifi/* $(UPDATERFS_DIR)/lib/firmware/
endif
endif
endif
endif
endif
endif
endif
endif

# wifi smart config filter
ifneq ("$(SUPPORT_WIFI_SC)", "")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/network_manager.ini -w -s network-configure -k target -v $(SUPPORT_WIFI_SC)
endif

# cutdown fs size filter
ifeq ("$(SUPPORT_CUT_CONTINUE)","1")
	-rm -f $(UPDATERFS_DIR)/lib/{libanl*,libBrokenLocale*,libcidn*,libmemusage.so,libnsl*,libnss*,libpcprofile.so,libSegFault.so,libutil*,libthread_db*}
	-rm -rf $(APPFS_DIR)/usr/test
	-rm -f $(UPDATERFS_DIR)/usr/sbin/wpa_passphrase
	-rm -f $(UPDATERFS_DIR)/usr/sbin/{ifrename,iwevent,iwspy,iwconfig,iwgetid,iwpriv}
	-rm -f $(UPDATERFS_DIR)/usr/bin/{linuxrw}
endif

# tone
ifeq ("$(SUPPORT_ATALK)","1")
	-rm -rf $(UPDATERFS_DIR)/usr/share/vr/tone/*.mp3
#	cp $(MOLIB_UPDATER_DIR)/usr/share/vr/tone/welcome.mp3 $(UPDATERFS_DIR)/usr/share/vr/tone/
#	cp $(MOLIB_UPDATER_DIR)/usr/share/vr/tone/charge_prompt.mp3 $(UPDATERFS_DIR)/usr/share/vr/tone/
#	cp $(MOLIB_UPDATER_DIR)/usr/share/vr/tone/bluetooth_mode.mp3 $(UPDATERFS_DIR)/usr/share/vr/tone/
#	cp $(MOLIB_UPDATER_DIR)/usr/share/vr/tone/linein_mode.mp3 $(UPDATERFS_DIR)/usr/share/vr/tone/
#	cp $(MOLIB_UPDATER_DIR)/usr/share/vr/tone/airkiss_config_quit.mp3 $(UPDATERFS_DIR)/usr/share/vr/tone/
endif

# strip all ELF files.
	@$(call MESSAGE,"Strip ELF files")
	@-$(STRIP) $(UPDATERFS_DIR)/bin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/lib/*so* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/usr/bin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/usr/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(UPDATERFS_DIR)/usr/lib/*so* > /dev/null 2>&1


rootfs-$(1)-appfs-prepare:
	@$(call MESSAGE,"Prepare target directory: $(APPFS_DIR)")
	-rm -rf $(APPFS_DIR); mkdir -p $(APPFS_DIR)
	@$(call MESSAGE,"Prepare applications")
	cp $(TOPDIR)/rootfs/app/* $(APPFS_DIR)/ -a
	cp $(MOLIB_APP_DIR)/* $(APPFS_DIR)/ -a
	cp $(MOZART_APP_DIR)/* $(APPFS_DIR)/ -a
	@$(call MESSAGE,"Remove useless files")
	find $(APPFS_DIR) \( -name '*.a' -o -name '*.la' \) | xargs rm -rf
	find $(APPFS_DIR) \( -name '.svn' -o -name '*.git' -o -name '.gitignore' \) | xargs rm -rf
	-rm -rf $(APPFS_DIR)/usr/include
	-rm -rf $(APPFS_DIR)/usr/man
	-rm -rf $(APPFS_DIR)/usr/lib/pkgconfig
	-rm -rf $(APPFS_DIR)/usr/share/{gdb,man,info,doc}
	-rm -rf $(APPFS_DIR)/usr/share/aclocal
	-rm -rf $(APPFS_DIR)/usr/lib/{dbus-1.0,glib-2.0}
	-rm -rf $(APPFS_DIR)/usr/lib/libfoo.so
	-rm -rf $(APPFS_DIR)/firmware
ifeq ("$(SUPPORT_LAPSULE)","1")
	ln -sf /usr/fs/usr/lib/libevent.so $(APPFS_DIR)/usr/lib/libkey_interface.so
endif

	@$(call MESSAGE,"Check all modules")
ifneq ("$(SUPPORT_ADB_DEBUG)","1")
	rm $(APPFS_DIR)/usr/sbin/adbd
	rm $(APPFS_DIR)/etc/init.d/S99adbd
endif


# mplayer filter.
ifeq ("$(SUPPORT_MPLAYER)","float") # mplayer_float
	mv $(APPFS_DIR)/usr/bin/mplayer-float $(APPFS_DIR)/usr/bin/mplayer
	rm $(APPFS_DIR)/usr/bin/mplayer-fixed
else
ifeq ("$(SUPPORT_MPLAYER)","fixed") # mplayer_fixed
	mv $(APPFS_DIR)/usr/bin/mplayer-fixed $(APPFS_DIR)/usr/bin/mplayer
	rm $(APPFS_DIR)/usr/bin/mplayer-float
else                                # mplayer_float(default)
	mv  $(APPFS_DIR)/usr/bin/mplayer-float $(APPFS_DIR)/usr/bin/mplayer
	rm $(APPFS_DIR)/usr/bin/mplayer-fixed
endif
endif


# audio type filter.
ifeq ("$(SUPPORT_AUDIO)", "alsa") # alsa.
		$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k type -v $(SUPPORT_AUDIO)
		-rm -f $(UPDATERFS_DIR)/usr/lib/libfake_alsa.so
else
ifeq ("$(SUPPORT_AUDIO)", "oss") # oss.
		$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k type -v $(SUPPORT_AUDIO)
		$(call remove_audio_alsa)
else
ifeq ("$(SUPPORT_AUDIO)", "") #default: oss.
		$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s audio -k type -v oss
		$(call remove_audio_alsa)
else
	@echo "$(SUPPORT_AUDIO): NOT supported audio type!!!!" && exit 1
endif
endif
endif


# wifi filter.
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "rtk") # realtek's wifi module.
	mv $(APPFS_DIR)/usr/sbin/hostapd_realtek $(APPFS_DIR)/usr/sbin/hostapd
	mv $(APPFS_DIR)/usr/sbin/hostapd_cli_realtek $(APPFS_DIR)/usr/sbin/hostapd_cli
	rm -f $(APPFS_DIR)/usr/sbin/hostapd_broadcom
	rm -f $(APPFS_DIR)/usr/sbin/hostapd_cli_broadcom
else
ifeq ("$(call substring,$(SUPPORT_WIFI_MODULE),1,3)", "bcm") # broadcom's wifi module.
	mv $(APPFS_DIR)/usr/sbin/hostapd_broadcom $(APPFS_DIR)/usr/sbin/hostapd
	mv $(APPFS_DIR)/usr/sbin/hostapd_cli_broadcom $(APPFS_DIR)/usr/sbin/hostapd_cli
	rm -f $(APPFS_DIR)/usr/sbin/hostapd_realtek
	rm -f $(APPFS_DIR)/usr/sbin/hostapd_cli_realtek
endif
endif


# bt filter.
ifeq ("$(SUPPORT_BT_MODULE)", "0") # bt disabled.
	$(call remove_bluez)
	$(call remove_bsa)
else
ifeq ("$(call substring,$(SUPPORT_BT_MODULE),1,3)", "rtk") # realtek's bt module.
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k bt -v rtk
	$(call remove_bsa)
ifeq ("$(call substring,$(SUPPORT_BT_MODULE),5,$(call strlen,$(SUPPORT_BT_MODULE)))", "rtl8723bs") # RTL8723BS
	cp -a $(TOPDIR)/configs/firmwares/rtl8723bs/bt/* $(UPDATERFS_DIR)/lib/firmware/
endif
else
ifeq ("$(call substring,$(SUPPORT_BT_MODULE),1,3)", "bcm") # broadcom's bt module.
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k bt -v bcm
	sed -i 's!-d /dev/ttyS1!-d $(SUPPORT_BT_HCI_DEV)!g' $(APPFS_DIR)/etc/init.d/S04bsa.sh
	$(call remove_bluez)
ifeq ("$(call substring,$(SUPPORT_BT_MODULE),5,$(call strlen,$(SUPPORT_BT_MODULE)))", "ap6212") # ap6212 module, bcm43438a0 inside.
	cp -a $(TOPDIR)/configs/firmwares/ap6212/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
else
ifeq ("$(call substring,$(SUPPORT_BT_MODULE),5,$(call strlen,$(SUPPORT_BT_MODULE)))", "ap6212a") # ap6212a module, bcm43438a1 inside.
	cp -a $(TOPDIR)/configs/firmwares/ap6212a/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
else
ifeq ("$(call substring,$(SUPPORT_BT_MODULE),5,$(call strlen,$(SUPPORT_BT_MODULE)))", "bcm43438") # bcm43438, core on broad.
	cp -a $(TOPDIR)/configs/firmwares/43438/bt/* $(UPDATERFS_DIR)/lib/firmware/BCM_bt_firmware.hcd
endif
endif
endif
endif
endif
endif

# lapsule filter
ifneq ("$(SUPPORT_LAPSULE)","1")
	-rm -rf $(APPFS_DIR)/usr/bin/{lapsule,lapsule.sh}
	-rm -rf $(APPFS_DIR)/mnt/vendor/
endif

ifneq ("$(SUPPORT_ATALK)","1")
	-rm -rf $(APPFS_DIR)/usr/bin/atalk_vendor
	-rm -rf $(APPFS_DIR)/etc/atalk/
endif

# dmr filter
ifneq ("$(SUPPORT_DMR)","1")
	-rm -rf $(APPFS_DIR)/usr/data/render.ini
	-rm -rf $(APPFS_DIR)/usr/lib/librender.so
endif

# WEBRTC filter
ifneq ("$(SUPPORT_WEBRTC)","1")
	-rm -rf $(APPFS_DIR)/usr/lib/libwebrtc_audio_processing.so*
	-rm -rf $(APPFS_DIR)/usr/lib/libwebrtc.so
endif

# dms filter
ifneq ("$(SUPPORT_DMS)","1")
	-rm -rf $(APPFS_DIR)/etc/ushare.conf
	-rm -rf $(APPFS_DIR)/usr/bin/ushare
	-rm -rf $(APPFS_DIR)/usr/dms_refresh_on_storage_hotplug
ifneq ("$(SUPPORT_DMR)","1")
ifneq ("$(SUPPORT_LAPSULE)","1")
	-rm -rf $(APPFS_DIR)/usr/lib/libupnp.so*
	-rm -rf $(APPFS_DIR)/usr/lib/libthreadutil.so*
	-rm -rf $(APPFS_DIR)/usr/lib/libixml.so*
endif
endif
endif

# atalk filter
ifneq ("$(SUPPORT_ATALK)","1")
	-rm -rf $(APPFS_DIR)/usr/bin/atalk_vendor
	-rm -rf $(APPFS_DIR)/etc/atalk/{ca.pem,prodconf.json}
endif

# airplay filter
ifneq ("$(SUPPORT_AIRPLAY)","1")
	-rm -rf $(APPFS_DIR)/usr/bin/shairport
endif

# airplay filter
ifneq ("$(SUPPORT_LOCALPLAYER)","1")
	-rm -rf $(APPFS_DIR)/usr/lib/liblocalplayer.so
endif

# voice recognition filter
ifeq ("$(SUPPORT_VR)", "0")
	$(call remove_vr_baidu)
	$(call remove_vr_speech)
	$(call remove_vr_iflytek)
	$(call remove_vr_unisound)
	$(call remove_vr_atalk)
else
ifeq ("$(SUPPORT_VR)", "vr_baidu")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
	$(call remove_vr_speech)
	$(call remove_vr_iflytek)
	$(call remove_vr_unisound)
	$(call remove_vr_atalk)
else
ifeq ("$(SUPPORT_VR)", "vr_speech")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
	$(call remove_vr_baidu)
	$(call remove_vr_iflytek)
	$(call remove_vr_unisound)
	$(call remove_vr_atalk)
else
ifeq ("$(SUPPORT_VR)", "vr_iflytek")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
	$(call remove_vr_baidu)
	$(call remove_vr_speech)
	$(call remove_vr_unisound)
	$(call remove_vr_atalk)
else
ifeq ("$(SUPPORT_VR)", "vr_unisound")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
	$(call remove_vr_baidu)
	$(call remove_vr_speech)
	$(call remove_vr_iflytek)
	$(call remove_vr_atalk)
else
ifeq ("$(SUPPORT_VR)", "vr_atalk")
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(UPDATERFS_DIR)/usr/data/system.ini -w -s product -k vr -v $(SUPPORT_VR)
	$(call remove_vr_baidu)
	$(call remove_vr_speech)
	$(call remove_vr_iflytek)
	$(call remove_vr_unisound)
else
	@echo "$(SUPPORT_VR): NOT supported vr!!!!" && exit 1
endif
endif
endif
endif
endif
endif

# alarm
	-rm -rf $(APPFS_DIR)/usr/sbin/alarm_manager
	-rm -rf $(APPFS_DIR)/etc/init.d/S01alarm.sh
# cgi
ifeq ("$(SUPPORT_CGI)", "0")
	-rm -rf $(APPFS_DIR)/var/www/
	-rm -rf $(APPFS_DIR)/etc/init.d/S88app.sh
	-rm -rf $(UPDATERFS_DIR)/var/www
endif

# strip all ELF files.
	@$(call MESSAGE,"Strip ELF files")
	@-$(STRIP) $(APPFS_DIR)/bin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/lib/*so* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/usr/bin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/usr/sbin/* > /dev/null 2>&1
	@-$(STRIP) $(APPFS_DIR)/usr/lib/*so* > /dev/null 2>&1

pre-prepare:

host-$(1)-fakeroot-check:
	@test -x $(fakeroot_bin) || \
		(echo "fakeroot is broken, please run 'make host-fakeroot-rebuild' Firstly!!" && exit 1)

update: update-prepare update-$(1)-updater update-$(1)-appfs update-nvimg updatepkg-gen

updatepkg-gen:
	@$(TOPDIR)/tools/host-tools/update_pack/tool/gen_updatepkg.sh -v $(MOZART_VERSION) \
		-m $(TOPDIR) -o $(UPDATEPKG_DIR)
	@rm $(UPDATEPKG_DIR)/temp -rf

update-prepare:
	@mkdir -p $(UPDATEPKG_DIR)/temp
update-$(1)-updater:
	@$(call UPDATE_UPDATER_$(1)_CMDS)
update-$(1)-appfs:
	@$(call UPDATE_APP_$(1)_CMDS)

endef

nvimg: nvimg-prepare
	@$(call MESSAGE,"Build nv.img")
	@$(TOPDIR)/output/host/usr/bin/nvgen \
		-c $(TOPDIR)/configs/nvimage.ini \
		-o $(TARGET_DIR)/nv.img \
		-p $(call mul,2,$(FLASH_ERASE_BLOCK_SIZE))

nvimg-prepare:
	@cp $(TOPDIR)/configs/ota.ini $(TOPDIR)/configs/nvimage.ini
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k product -v $(PRODUCT_NAME)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k storage -v $(FLASH_TYPE)
	$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k current_version -v $(MOZART_VERSION)
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k url -v \
		`$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/ota.ini -r -s ota -k url`
ifeq ("$(FLASH_TYPE)","spinor")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k method -v update_times
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k location -v /dev/mtdblock5
endif
ifeq ("$(FLASH_TYPE)","spinand")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k method -v update_once
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k location -v /dev/mtdblock5
endif
ifeq ("$(FLASH_TYPE)","emmc")
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k method -v update_once
	@$(OUTPUT_DIR)/host/usr/bin/inirw -f $(TOPDIR)/configs/nvimage.ini -w -s ota -k location -v /dev/mmcblk0p6
endif


update-nvimg:
	@#cp -f $(TARGET_DIR)/nv.img $(UPDATEPKG_DIR)/temp
