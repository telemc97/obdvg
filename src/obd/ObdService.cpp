#include "obd/ObdService.h"
#include "obd/ObdPids.h"
#include "obd/PidDecoder.h"

#include "can/CanFrame.h"

#include <cstring>

void ObdService::buildCanFrameForPID(uint8 const pid, CanFrame &tx) {
  tx.id = 0x7DF; // functional broadcast
  tx.dlc = 8;
  std::memset(tx.data.data(), 0, 8);
  tx.data[0] = 0x02; // length
  tx.data[1] = 0x01; // service 01
  tx.data[2] = pid;
}

bool ObdService::pollResponse(const CanFrame &rx, const uint8 pid, float32 &valueOut) {
  if (isValidResponse(rx, pid)) {
    switch (pid) {

    case PID_ENGINE_RPM:
      valueOut = PidDecoder::decodeRpm(rx.data[3], rx.data[4]);
      break;

    case PID_ENGINE_TEMP:
      valueOut = PidDecoder::decodeTemp(rx.data[3]);
      break;

    case PID_VEHICLE_SPEED:
      valueOut = PidDecoder::decodeSpeed(rx.data[3]);

    default:
      return false;
      break;
    }
    return true;
  }
  return false;
}

bool ObdService::isValidResponse(const CanFrame &frame, const uint8_t requestedPid) {
  bool valid = false;
  // Check if frame ID is in OBD-II response range and has enough data
  if (frame.id >= 0x7E8 && frame.id <= 0x7EF && frame.dlc >= 3) {
    const uint8 svc = frame.data[1] & 0x3F;
    const uint8 rxPid = frame.data[2];
    // Service must be 0x01 and PID must match the requested PID
    valid = (svc == 0x01 && rxPid == requestedPid);
  }
  return valid;
}
