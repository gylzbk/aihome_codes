#!/bin/sh

DHCP=yes
if [ $# -ge 1 -a "$1" == "--without-dhcp" ]; then
    DHCP=no
fi

# stop already exist process
killall udhcpc > /dev/null
killall udhcpd > /dev/null
killall wpa_supplicant > /dev/null
killall hostapd  > /dev/null

# wpa_supplicant config file
if [ -f /usr/data/wpa_supplicant.conf ]; then
    WPA_CONF=/usr/data/wpa_supplicant.conf
else
    WPA_CONF=/etc/wpa_supplicant.conf
fi

# guess what wifi model we are using(light detect, may not match!!!)
INTERFACE=wlan0
DRIVER=nl80211

if [ -d /proc/net/rtl*/wlan? ]; then
    INTERFACE=`basename /proc/net/rtl*/wlan?`
    DRIVER=wext
fi

# delete default Gateway
route del default gw 0.0.0.0 dev $INTERFACE
# release ip address
ifconfig $INTERFACE 0.0.0.0
# turn up wifi interface
ifconfig $INTERFACE up

# start service
wpa_supplicant -D$DRIVER -i$INTERFACE -c$WPA_CONF -B

if [ "$DHCP" == "yes" ]; then
    udhcpc -i $INTERFACE -q
fi

# Add Multicast Router for Apple Airplay
# DHCP will reset Router, This can not work with udhcpc
route add -net 224.0.0.0 netmask 224.0.0.0 $INTERFACE

exit 0
