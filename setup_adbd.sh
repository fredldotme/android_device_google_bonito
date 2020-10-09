#!/sbin/sh

CONFIG_DIR=/config
GADGET_DIR=$CONFIG_DIR/usb_gadget
SERIALNUMBER=`getprop ro.serialno`
MANUFACTURER=`getprop ro.product.manufacturer`
PRODUCT=`getprop ro.product.model`
PRODUCT_ID_SUFFIX=`getprop ro.usb.pid_suffix`
CONTROLLER=`getprop sys.usb.controller`

write() {
    if [ ! -e "$1" ]; then
        return
    fi
	echo -n "$2" >"$1"
}

symlink() {
    ln -s "$1" "$2"
}

setup_adb() {
    write $GADGET_DIR/gadget1/configs/config.1/strings/0x409/configuration "adb"
    symlink $GADGET_DIR/gadget1/functions/ffs.adb $GADGET_DIR/gadget1/configs/config.1
    write $GADGET_DIR/gadget1/UDC ${CONTROLLER}
    setprop sys.usb.state adb
}

setup_boot() {
    if [ -e /dev/.usb_setup_done ]; then
        echo "Boot setup done"
        return
    fi
    write /sys/class/android_usb/android0/iManufacturer ${MANUFACTURER}
    write /sys/class/android_usb/android0/f_rndis/manufacturer ${MANUFACTURER}
    write /sys/class/android_usb/android0/iProduct ${PRODUCT}
    mkdir /dev/usb-ffs
    mkdir /dev/usb-ffs/adb
    mkdir -p $CONFIG_DIR
    mount -t configfs none $CONFIG_DIR
    mkdir -p $GADGET_DIR/gadget1
    mkdir -p $GADGET_DIR/gadget1/strings/0x409
    write $GADGET_DIR/gadget1/idVendor 0x18D1
    write $GADGET_DIR/gadget1/bcdDevice 0x0223
    write $GADGET_DIR/gadget1/bcdUSB 0x0200
    write $GADGET_DIR/gadget1/os_desc/use 1
    write $GADGET_DIR/gadget1/strings/0x409/serialnumber ${SERIALNUMBER}
    write $GADGET_DIR/gadget1/strings/0x409/manufacturer ${MANUFACTURER}
    write $GADGET_DIR/gadget1/strings/0x409/product ${PRODUCT}
    mkdir -p $GADGET_DIR/gadget1/functions/mtp.gs0
    mkdir -p $GADGET_DIR/gadget1/functions/ptp.gs1
    mkdir -p $GADGET_DIR/gadget1/functions/accessory.gs2
    mkdir -p $GADGET_DIR/gadget1/functions/audio_source.gs2
    mkdir -p $GADGET_DIR/gadget1/functions/audio_source.gs3
    mkdir -p $GADGET_DIR/gadget1/functions/midi.gs5
    mkdir -p $GADGET_DIR/gadget1/functions/ffs.adb
    mkdir -p $GADGET_DIR/gadget1/functions/cser.dun.0
    mkdir -p $GADGET_DIR/gadget1/functions/cser.nmea.1
    mkdir -p $GADGET_DIR/gadget1/functions/gsi.rmnet
    mkdir -p $GADGET_DIR/gadget1/functions/gsi.rndis
    mkdir -p $GADGET_DIR/gadget1/functions/gsi.dpl
    mkdir -p $GADGET_DIR/gadget1/functions/rndis_bam.rndis
    mkdir -p $GADGET_DIR/gadget1/functions/rmnet_bam.rmnet
    mkdir -p $GADGET_DIR/gadget1/functions/rmnet_bam.dpl
    mkdir -p $GADGET_DIR/gadget1/functions/ncm.0
    mkdir -p $GADGET_DIR/gadget1/configs/config.1
    mkdir -p $GADGET_DIR/gadget1/configs/config.1/strings/0x409
    write $GADGET_DIR/gadget1/os_desc/b_vendor_code 0x1
    write $GADGET_DIR/gadget1/os_desc/qw_sign "MSFT100"
    write $GADGET_DIR/gadget1/configs/config.1/MaxPower 500
    write $GADGET_DIR/gadget1/idProduct 0x0${PRODUCT_ID_SUFFIX}
    symlink $GADGET_DIR/gadget1/configs/config.1 $GADGET_DIR/gadget1/os_desc/config.1
    mount -t functionfs -o uid=2000,gid=2000 adb /dev/usb-ffs/adb
    chown system:system /dev/usb-ffs
    chmod 775 /dev/usb-ffs
    chown system:system /dev/usb-ffs/adb
    chmod 775 /dev/usb-ffs/adb
    chown system:system /dev/mtp_usb
    chmod 660 /dev/mtp_usb
    write /sys/class/android_usb/android0/f_ffs/aliases adb
    touch /dev/.usb_setup_done
}

setup_boot
setup_adb

exit 0
