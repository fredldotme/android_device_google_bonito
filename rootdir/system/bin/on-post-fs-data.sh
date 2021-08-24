#!/system/bin/sh

setprop persist.audio.fluence.voicecall false
setprop persist.audio.fluence.speaker false
bootctl mark-boot-successful
