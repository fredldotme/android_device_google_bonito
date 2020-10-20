#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <linux/limits.h>
#include <limits>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    static const char* targetName = "uinput-fpc";
    int fd = -1;
    int ioctlResult;
    char name[256];

    for (int i = 1; i < 9; i++) {
        char eventPath[PATH_MAX];

        memset(name, 0, sizeof(name));
        sprintf(eventPath, "/dev/input/event%d", i);
        std::cout << "Trying " << eventPath << std::endl;

        fd = open(eventPath, O_RDONLY);
        ioctlResult = ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        std::cout << "Device name is '" << name << "'" << std::endl;

        if (ioctlResult < 0) {
            close(fd);
            fd = -1;
            continue;
        }

        if (strcmp(targetName, name) == 0)
            break;
    }

    if (fd < 0) {
        std::cerr << "Failed to find uinput-fpc device" << std::endl;
        return 1;
    }

    std::cout << "Grabbing " << targetName << std::endl;
    ioctlResult = ioctl(fd, EVIOCGRAB, 1);

    if (ioctlResult < 0) {
        std::cerr << "Failed to grab fpc input device" << std::endl;
        return 2;
    }

    while (true) {
        sleep(std::numeric_limits<unsigned int>::max());
    }

    close(fd);
    return 0;
}
