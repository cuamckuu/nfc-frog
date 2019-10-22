#!/usr/bin/env python3

import sys
import subprocess
from subprocess import PIPE, DEVNULL

class DeviceNFC:
    def __init__(self):
        app = ["sudo", "./nfc-frog", "shell"]
        kwargs = {"stdin":PIPE, "stdout":PIPE, "stderr":DEVNULL}

        self.proc = subprocess.Popen(app, **kwargs)

    def execute_command(self, command_str):
        command_bytes = (command_str + "\n").encode("utf-8")

        self.proc.stdin.write(command_bytes)
        self.proc.stdin.flush()

        self.proc.stdout.readline()
        self.proc.stdout.readline()
        res = self.proc.stdout.readline().decode("utf-8").strip("\n ")

        return res

errors = [
    "6D 00", # Wrong INS
    "6E 00", # Wrong CLA
    "6F 00", # Unknown error
    "6A 81", # Function not supported
    "68 81", # Logical channel not supported
    "68 82", # Secure messaging not supported
]

if __name__ == "__main__":
    device = DeviceNFC()

    for cla in range(0, 256):
        for ins in range(0, 256):
            command = f"{hex(cla)} {hex(ins)} 00 00 00"

            SW = device.execute_command(command)

            if SW not in errors:
                print(hex(ins), hex(cla), ":", SW, flush=True)
