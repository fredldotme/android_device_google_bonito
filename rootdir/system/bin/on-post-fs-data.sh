#!/system/bin/sh

setprop ctl.restart keymaster-4-0
setprop ctl.stop ppd
setprop ctl.stop fps_hal

bootctl mark-boot-successful
