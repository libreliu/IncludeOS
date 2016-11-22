#! /usr/bin/python
import sys
import subprocess
import os

includeos_src = os.environ.get('INCLUDEOS_SRC',
                               os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))).split('/test')[0])
sys.path.insert(0,includeos_src + "/test")

import vmrunner

disks = ["memdisk", "virtio1", "virtio2"]

# Remove all data disk images
def cleanup():
  for disk in disks:
    subprocess.check_call(["rm", "-f", disk + ".disk"])

# Create all data disk images from folder names
for disk in disks:
  subprocess.check_call(["./create_disk.sh", disk])

vmrunner.vms[0].on_exit(cleanup)

vmrunner.vms[0].make().boot(50)
