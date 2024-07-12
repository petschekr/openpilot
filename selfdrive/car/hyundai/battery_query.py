#!/usr/bin/env python3
import struct
from openpilot.selfdrive.car.isotp_parallel_query import IsoTpParallelQuery
from openpilot.common.swaglog import cloudlog
import panda.python.uds as uds

def p16(val):
  return struct.pack("!H", val)

PID1 = 0x0101
PID2 = 0x0105

def battery_query(logcan, sendcan, bus=0, addr=0x7E4, sub_addr=None, pid=PID1, timeout=0.1, retry=10, debug=False):
  REQUEST = bytes([uds.SERVICE_TYPE.READ_DATA_BY_IDENTIFIER]) + p16(pid)
  RESPONSE = bytes([uds.SERVICE_TYPE.READ_DATA_BY_IDENTIFIER + 0x40]) + p16(pid)

  for i in range(retry):
    try:
      query = IsoTpParallelQuery(sendcan, logcan, bus, [(addr, sub_addr)], [REQUEST], [RESPONSE], debug=debug)

      for _, battery_data in query.get_data(timeout).items():
        if pid == PID1:
          print(battery_data[4] / 2)
        if pid == PID2:
          print(battery_data[31] / 2)

        soc = battery_data[4] / 2
        return soc

    except Exception:
      cloudlog.exception("battery query exception")

    cloudlog.error(f"battery query retry ({i + 1}) ...")
  cloudlog.error("battery query failed")
  return None


if __name__ == "__main__":
  import time
  import cereal.messaging as messaging
  sendcan = messaging.pub_sock('sendcan')
  logcan = messaging.sub_sock('can')
  time.sleep(1)

  battery_query(logcan, sendcan, pid=PID1, debug=False)
  battery_query(logcan, sendcan, pid=PID2, debug=False)
