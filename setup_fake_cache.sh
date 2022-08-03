#!/sbin/sh

DATA_MOUNT_CODE=1

while [ "$DATA_MOUNT_CODE" != "0" ]; do
    mount -t ext4 /dev/block/bootdevice/by-name/userdata /data > /dev/kmsg
    DATA_MOUNT_CODE=$?
    sleep 1
done

mkdir /data/android-data > /dev/kmsg
mkdir /data/android-data/cache > /dev/kmsg

if [ ! -L /data/cache ]; then
    ln -sf android-data/cache /data/cache > /dev/kmsg
fi

mount -o bind /data/android-data/cache /cache > /dev/kmsgs

setprop halium.datamount.done 1

exit 0
