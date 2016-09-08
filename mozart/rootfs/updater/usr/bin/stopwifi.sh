# stop already exist process
killall udhcpc > /dev/null
killall udhcpd > /dev/null
killall wpa_supplicant > /dev/null
killall hostapd  > /dev/null

ifconfig wlan0 up > /dev/null
sleep 1
ifconfig wlan0 down > /dev/null
