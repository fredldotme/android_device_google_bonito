#!/system/bin/sh

setprop ro.qc.sdk.audio.fluencetype fluencepro
setprop persist.audio.fluence.voicecall true
setprop persist.audio.fluence.speaker true
bootctl mark-boot-successful
