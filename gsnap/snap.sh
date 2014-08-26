#!/bin/bash
#for android 
adb shell /data/gsnap /data/fb.jpg /dev/graphics/fb0
adb pull /data/fb.jpg .
gthumb fb.jpg
