/*
 * Copyright (C) 2021 Alfred Neumayer <dev.beidl@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dirent.h> 
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>

#include <string>
#include <utils/Log.h>

#include "booster.h"

#define LOG_TAG "powerswitch"

using namespace std;

/* Code is largely taken from ../power, have a look at it for more information. */

static const string CPUFREQ_POLICY_ROOT = "/sys/devices/system/cpu/cpufreq/";
static const string CPUFREQ_GOVERNOR_FILE = "scaling_governor";

static const string CPUQUIET_NODE = "/sys/devices/system/cpu/cpuquiet/";
static const string RQBALANCE_NODE = CPUQUIET_NODE + string("rqbalance/");

static const string SYS_MIN_CPUS = CPUQUIET_NODE + string("nr_min_cpus");
static const string SYS_MAX_CPUS = CPUQUIET_NODE + string("nr_power_max_cpus");
static const string SYS_THERM_CPUS = CPUQUIET_NODE + string("nr_thermal_max_cpus");

static const string SYS_BALANCE_LVL = RQBALANCE_NODE + string("balance_level");
static const string SYS_UPCORE_THRESH = RQBALANCE_NODE + string("nr_run_thresholds");
static const string SYS_DNCORE_THRESH = RQBALANCE_NODE + string("nr_down_run_thresholds");

static const string RQBHAL_CONF_FILE = "/system/etc/rqbalance_config.xml";

static bool param_perf_supported = true;
static rqb_pwr_mode_t cur_pwrmode;

static struct rqbalance_params *rqb = nullptr;

extern int parse_xml_data(const char* filepath, char* node, struct rqbalance_params *therqb);

static bool sysfs_write(const char *path, const char *s)
{
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);
    bool ret = true;

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return false;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);

        ret = false;
    }

    close(fd);

    return ret;
}

static void set_governor(char* governor_name) {
    struct dirent *dir;
    DIR* d = opendir(CPUFREQ_POLICY_ROOT.c_str());
    
    if (!d) {
        ALOGE("Failed to opendir() '%s'\n", CPUFREQ_POLICY_ROOT.c_str());
        return;
    }

    while ((dir = readdir(d)) != NULL) {
        const string dname(dir->d_name);
        if (dname.find("policy") != 0)
            continue;

        const string governor_file = CPUFREQ_POLICY_ROOT + dname + string("/") + 
                                     CPUFREQ_GOVERNOR_FILE;

        const bool success = sysfs_write(governor_file.c_str(), governor_name);
        if (!success) {
            ALOGE("Failed to write governor '%s' to '%s'\n", 
                  governor_name, governor_file.c_str());
        }
    }
    closedir(d);
}

static char* rqb_param_string(rqb_pwr_mode_t pwrmode, bool compat)
{
    char* type_string;
    char* compat_string;

    switch (pwrmode) {
        case POWER_MODE_BATTERYSAVE:
            type_string = "batterysave";
            compat_string = "low";
            break;
        case POWER_MODE_BALANCED:
            type_string = "balanced";
            compat_string = "normal";
            break;
        case POWER_MODE_PERFORMANCE:
            type_string = "performance";
            compat_string = "perf";
            break;
        case POWER_MODE_OMXDECODE:
            type_string = "video_decoding";
            compat_string = "vdec";
            break;
        case POWER_MODE_OMXENCODE:
            type_string = "video_encoding";
            compat_string = "venc";
        default:
            return "unknown";
    }

    if (compat)
        return compat_string;

    return type_string;
}

static void __set_power_mode(struct rqbalance_params *rqparm)
{
    bool ret, cpus_error;
    short retry = 0;


    sysfs_write(SYS_UPCORE_THRESH.c_str(), rqparm->up_thresholds);
    sysfs_write(SYS_DNCORE_THRESH.c_str(), rqparm->down_thresholds);
    sysfs_write(SYS_BALANCE_LVL.c_str(), rqparm->balance_level);

set_cpu:
    ret = sysfs_write(SYS_MAX_CPUS.c_str(), rqparm->max_cpus);
    if (!ret)
        cpus_error = true;

    ret = sysfs_write(SYS_MIN_CPUS.c_str(), rqparm->min_cpus);
    if (!ret)
        cpus_error = true;

    if (cpus_error) {
        cpus_error = false;
        retry++;

        if (retry < 2)
            goto set_cpu;
    }


    return;
}

void set_power_mode(rqb_pwr_mode_t mode)
{
    char* mode_string = rqb_param_string(mode, false);

    if (mode == POWER_MODE_PERFORMANCE && !param_perf_supported)
        return;

    ALOGI("Setting %s mode", mode_string);

    __set_power_mode(&rqb[mode]);

    cur_pwrmode = mode;
}

static bool init_all_rqb_params(void)
{
    int i, ret = 0;

    if (rqb) {
        return true;
    }

    rqb = (struct rqbalance_params*)malloc(sizeof(struct rqbalance_params) * POWER_MODE_MAX);
    if (!rqb) {
        ALOGE("Failed to allocate memory for configuration");
        return false;
    }

    for (i = 0; i < POWER_MODE_MAX; i++)
    {
        ret = parse_xml_data(RQBHAL_CONF_FILE.c_str(),
                rqb_param_string((rqb_pwr_mode_t)i, false), &rqb[i]);
        if (ret < 0) {
            ALOGE("Cannot parse configuration for %s mode!!!",
                  rqb_param_string((rqb_pwr_mode_t)i, false));
        }
    }

    return (ret == 0);
}

extern "C" {
void set_screen_on(int) {
    if (!init_all_rqb_params()) {
        ALOGE("Failed to parse configuration file");
        return;
    }
    set_power_mode(POWER_MODE_BALANCED);
}

void set_screen_off(int) {
    if (!init_all_rqb_params()) {
        ALOGE("Failed to parse configuration file");
        return;
    }
    set_power_mode(POWER_MODE_BATTERYSAVE);
}
}
