#! /usr/bin/env python

import sys
import os
import subprocess

includeos_src = os.environ.get('INCLUDEOS_SRC',
                               os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))).split('/test')[0])
sys.path.insert(0,includeos_src)

from vmrunner import vmrunner
from vmrunner.prettify import color

import socket

# Get an auto-created VM from the vmrunner
vm = vmrunner.vms[0]

# 1. Check that a ping request is received from google.com (193.90.147.109)
# 2. Check that sending a udp packet to 10.0.0.45 (the IncludeOS service's IP) and port 8080 returns
# an ICMP message with Destination unreachable (type 3), port unreachable (code 3).
# sudo hping 10.0.0.45 --udp -p 8080 -c 1
# (count = 1)
# 3. Check that sending an ip packet to 10.0.0.45 (the IncludeOS service's IP) with protocol 16 f.ex.
# returns an ICMP message with Destination unreachable (type 3), protocol unreachable (code 2).
# sudo hping 10.0.0.45 -d 20 -0 --ipproto 16 -c 1
# (count = 1)

num_successes = 0

def start_icmp_test(trigger_line):
  global num_successes

  # 1 Ping: Checking output from callback in service.cpp
  print color.INFO("<Test.py>"), "Performing ping test"

  output_data = ""
  for x in range(0, 9):
    output_data += vm.readline()

  if "Received packet" in output_data and \
    "Identifier: 0" in output_data and \
    "Sequence number: 0" in output_data and \
    "Source: 193.90.147.109" in output_data and \
    "Destination: 10.0.0.45" in output_data and \
    "Type: ECHO REPLY" in output_data and \
    "Code: 0" in output_data:
    num_successes += 1
    print color.INFO("<Test.py>"), "Ping test succeeded"
  else:
    print color.FAIL("<Test.py>"), "Ping test FAILED"

  # 2 Port unreachable
  print color.INFO("<Test.py>"), "Performing Destination Unreachable (port) test"
  # Sending 1 udp packet to 10.0.0.45 to port 8080
  udp_port_output = subprocess.check_output(["sudo", "hping", "10.0.0.45", "--udp", "-p", "8080", "-c", "1"])
  print udp_port_output

  # Validate content in udp_port_output:
  if "ICMP Port Unreachable from ip=10.0.0.45" in udp_port_output:
    print color.INFO("<Test.py>"), "Port Unreachable test succeeded"
    num_successes += 1
  else:
    print color.FAIL("<Test.py>"), "Port Unreachable test FAILED"

  # 3 Protocol unreachable
  print color.INFO("<Test.py>"), "Performing Destination Unreachable (protocol) test"
  # Sending 1 raw ip packet to 10.0.0.45 with protocol 16
  rawip_protocol_output = subprocess.check_output(["sudo", "hping", "10.0.0.45", "-d", "20", "-0", "--ipproto", "16", "-c", "1"])
  print rawip_protocol_output

  # Validate content in rawip_protocol_output:
  if "ICMP Protocol Unreachable from ip=10.0.0.45" in rawip_protocol_output:
    print color.INFO("<Test.py>"), "Protocol Unreachable test succeeded"
    num_successes += 1
  else:
    print color.FAIL("<Test.py>"), "Protocol Unreachable test FAILED"

  # 4 Check result of tests
  if num_successes == 3:
    vm.exit(0, "<Test.py> All ICMP tests succeeded. Process returned 0 exit status")
  else:
    num_fails = 3 - num_successes
    res = "<Test.py> " + str(num_fails) + " ICMP test(s) failed"
    vm.exit(1, res)

vm.on_output("Service IP address is 10.0.0.45", start_icmp_test);

# Boot the VM, taking a timeout as parameter
vm.cmake().boot(20).clean()
