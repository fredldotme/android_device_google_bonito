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

#define LOG_TAG "powerswitch"

using namespace std;

static const string TOUCHSCREEN_RESET = "/sys/devices/platform/soc/a84000.i2c/i2c-2/2-0020/input/input1/hw_reset";

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
extern "C" {
void set_screen_on(int) {
    sysfs_write(TOUCHSCREEN_RESET.c_str(), "1");
}

void set_screen_off(int) {
}
}
