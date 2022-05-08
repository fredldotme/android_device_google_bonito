#!/system/bin/sh

for i in $(seq 0 7);
do
    echo -6 > /sys/devices/system/cpu/cpu${i}/sched_load_boost
    HIGHEST_FREQ=$(cat /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_available_frequencies | awk '{print $(NF)}')
    echo $HIGHEST_FREQ > /sys/devices/system/cpu/cpu${i}/cpufreq/schedutil/hispeed_freq
done

bootctl mark-boot-successful
