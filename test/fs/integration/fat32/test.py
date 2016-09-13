#!/usr/bin/python

import sys
import os

includeos_src = os.environ.get('INCLUDEOS_SRC',
                               os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))).split('/test')[0])
sys.path.insert(0,includeos_src + "/test")

from subprocess import call

import vmrunner

# Get an auto-created VM from the vmrunner
vm = vmrunner.vms[0]

def cleanup():
  vm.make(["clean"])
  call(["./fat32_disk.sh", "clean"])

# Setup disk
call(["./fat32_disk.sh"], shell=True)

# Clean up on exit
vm.on_exit(cleanup)

# Boot the VM
vm.make().boot(30)
