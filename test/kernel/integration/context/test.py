#! /usr/bin/env python

import sys
import os

from vmrunner import vmrunner

if len(sys.argv) > 1:
    vmrunner.vms[0].boot(image_name=str(sys.argv[1]))
else:
    vmrunner.vms[0].cmake().boot(20,image_name='kernel_context').clean()
