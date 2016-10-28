#!/bin/bash

#rm ./../output/mozart/app/usr/sbin/mozart
#rm ./../src/main/mozart
#rm ./../src/main/mozart.c.o

#make libaicurl-clean
#make libaitingfm-clean
#make libairadio-clean
#make libaiplayer-clean
#make libaiweather-clean
#make libaimusic-clean
#make vr-speech-clean
#make libaiserver-clean

#make libaiurl-rebuild -j
#cp ./src/asr/ai_url/libaiurl.so ./src/asr/ai_music/libs/libaiurl.so
#cp ./src/asr/ai_url/src/ai_url.h ./src/asr/ai_music/src/ai_url.h

#make libaicurl-rebuild -j
#cp ./src/asr/ai_url/libaicurl.so ./src/asr/ai_music/libs/
#cp ./src/asr/ai_url/libaicurl.so ./src/asr/ai_radio/libs/

#make libaiplayer-rebuild -j
#cp ./src/asr/ai_player/libaiplayer.so ./src/asr/ai_server/libs/
#cp ./src/asr/ai_player/src/*.h ./src/asr/include/

#make libaiweather-rebuild -j
#cp ./src/asr/ai_weather/libaiweather.so ./src/asr/ai_server/libs/
#cp ./src/asr/ai_weather/src/*.h ./src/asr/include/

#make libaimusic-rebuild -j
#cp ./src/asr/ai_music/libaimusic.so ./src/asr/ai_server/libs/
#cp ./src/asr/ai_music/libs/*.so ./src/asr/ai_server/libs/
#cp ./src/asr/ai_music/src/*.h ./src/asr/include/
#cp ./src/asr/ai_music/include/*.h ./src/asr/ai_server/include/

#make libaitingfm-rebuild -j
#cp ./src/asr/ai_tingfm/libaitingfm.so ./src/asr/ai_radio/libs/

#make libairadio-rebuild -j
#cp ./src/asr/ai_radio/libairadio.so ./src/asr/ai_server/libs/

make libaiserver-rebuild -j
#cp ./src/aispeech/server/libaiserver.so ./src/aispeech/speech/libs/
#cp ./src/asr/ai_server/libs/*.so ./src/asr/speech/libs/
#cp ./src/asr/ai_server/src/*.h ./src/asr/speech/x1000_1mic/
#cp ./src/asr/include/*.h ./src/asr/speech/x1000_1mic/

make libaispeech-rebuild

make mozart-rebuild
make app-config-rebuild

make -j

cp output/target/appfs.cramfs   ../update/
cp output/target/updater.cramfs ../update/
cp output/target/usrdata.jffs2  ../update/
cp output/target/nv.img ../update/
echo "============================== make libvr_speech-rebuild ok !"
date 
