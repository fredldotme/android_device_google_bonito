#!/usr/bin/env python3

import os
from subprocess import check_output

def get_pid(name):
    return map(int,check_output(["pidof",name]).split())

with open("/dev/kmsg") as f:
    for line in f:
        if "binder" in line and "-28" in line:
            is_qcrild = False

            for pid in get_pid("qcrild"):
                if str(pid) in line:
                    is_qcrild = True
                    break

            if is_qcrild:
                os.system("restart ofono")
