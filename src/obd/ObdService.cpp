#include "Types.h"

#include <cstring>

#include "can/CanFrame.h"
#include "obd/ObdPids.h"
#include "obd/ObdService.h"
#include "obd/PidDecoder.h"

// OBD-II Constants
static constexpr uint8 OBD_PCI_SINGLE_FRAME = 0x02;
static constexpr uint8 OBD_MODE_SHOW_CURRENT = 0x01;
static constexpr uint8 OBD_RESPONSE_OFFSET = 0x40;

/**
 * @brief Returns the expected data length (number of bytes after PID) for a given PID.
 */
static int32 getExpectedDataLength(ObdPid pid) {
    switch (pid) {
        case ObdPid::ENGINE_RPM:
        case ObdPid::MAF_FLOW:
        case ObdPid::RUNTIME_SINCE_START:
        case ObdPid::DISTANCE_WITH_MIL:
        case ObdPid::FUEL_RAIL_PRESSURE:
        case ObdPid::FUEL_RAIL_GAUGE_PRESSURE:
        case ObdPid::DISTANCE_SINCE_CLEARED:
        case ObdPid::EVAP_VAPOR_PRESSURE:
        case ObdPid::CATALYST_TEMP_B1S1:
        case ObdPid::CATALYST_TEMP_B2S1:
        case ObdPid::CATALYST_TEMP_B1S2:
        case ObdPid::CATALYST_TEMP_B2S2:
        case ObdPid::CONTROL_MODULE_VOLTAGE:
        case ObdPid::ABSOLUTE_LOAD_VALUE:
        case ObdPid::COMMANDED_EQUIV_RATIO:
        case ObdPid::ABS_EVAP_VAPOR_PRESSURE:
        case ObdPid::EVAP_VAPOR_PRESSURE_ALT:
        case ObdPid::FUEL_RAIL_ABS_PRESSURE:
        case ObdPid::TIME_RUN_WITH_MIL:
        case ObdPid::TIME_SINCE_CODES_CLEARED:
        case ObdPid::FUEL_INJECTION_TIMING:
        case ObdPid::ENGINE_FUEL_RATE:
        case ObdPid::ENGINE_REFERENCE_TORQUE:
            return 2; // 2-byte PIDs
        default:
            return 1; // 1-byte PIDs (default)
    }
}

void ObdService::buildCanFrameForPID(ObdPid pid, CanFrame &tx) {
  tx.id = 0x7DF;
  tx.dlc = 8;
  tx.isExtended = false;
  std::memset(tx.data.data(), 0, 8);
  tx.data[0] = OBD_PCI_SINGLE_FRAME;
  tx.data[1] = OBD_MODE_SHOW_CURRENT;
  tx.data[2] = static_cast<uint8>(pid);
}

bool ObdService::pollResponse(const CanFrame &rx, ObdPid pid, float32 &valueOut) {
  if (isValidResponse(rx, pid)) {
    switch (pid) {
    case ObdPid::ENGINE_LOAD:
      valueOut = PidDecoder::decodeEngineLoad(rx.data[3]);
      break;
    case ObdPid::ENGINE_COOLANT_TEMP:
      valueOut = PidDecoder::decodeTemperature(rx.data[3]);
      break;
    case ObdPid::SHORT_TERM_FUEL_TRIM_1:
    case ObdPid::LONG_TERM_FUEL_TRIM_1:
    case ObdPid::SHORT_TERM_FUEL_TRIM_2:
    case ObdPid::LONG_TERM_FUEL_TRIM_2:
      valueOut = PidDecoder::decodeFuelTrim(rx.data[3]);
      break;
    case ObdPid::FUEL_PRESSURE:
      valueOut = PidDecoder::decodeFuelPressure(rx.data[3]);
      break;
    case ObdPid::INTAKE_MAP:
      valueOut = PidDecoder::decodePressure(rx.data[3]);
      break;
    case ObdPid::ENGINE_RPM:
      valueOut = PidDecoder::decodeRpm(rx.data[3], rx.data[4]);
      break;
    case ObdPid::VEHICLE_SPEED:
      valueOut = PidDecoder::decodeSpeed(rx.data[3]);
      break;
    case ObdPid::TIMING_ADVANCE:
      valueOut = PidDecoder::decodeTimingAdvance(rx.data[3]);
      break;
    case ObdPid::INTAKE_AIR_TEMP:
      valueOut = PidDecoder::decodeTemperature(rx.data[3]);
      break;
    case ObdPid::MAF_FLOW:
      valueOut = PidDecoder::decodeMafFlow(rx.data[3], rx.data[4]);
      break;
    case ObdPid::THROTTLE_POS:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::RUNTIME_SINCE_START:
      valueOut = PidDecoder::decodeTime(rx.data[3], rx.data[4]);
      break;
    case ObdPid::DISTANCE_WITH_MIL:
      valueOut = PidDecoder::decodeDistance(rx.data[3], rx.data[4]);
      break;
    case ObdPid::FUEL_RAIL_PRESSURE:
      valueOut = PidDecoder::decodeFuelRailPressureRel(rx.data[3], rx.data[4]);
      break;
    case ObdPid::FUEL_RAIL_GAUGE_PRESSURE:
      valueOut = PidDecoder::decodeFuelRailGaugePressure(rx.data[3], rx.data[4]);
      break;
    case ObdPid::COMMANDED_EGR:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::EGR_ERROR:
      valueOut = PidDecoder::decodeFuelTrim(rx.data[3]);
      break;
    case ObdPid::COMMANDED_EVAP_PURGE:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::FUEL_LEVEL:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::WARMUPS_SINCE_CLEARED:
      valueOut = PidDecoder::decodeWarmups(rx.data[3]);
      break;
    case ObdPid::DISTANCE_SINCE_CLEARED:
      valueOut = PidDecoder::decodeDistance(rx.data[3], rx.data[4]);
      break;
    case ObdPid::EVAP_VAPOR_PRESSURE:
      valueOut = PidDecoder::decodeEvapVaporPressure(rx.data[3], rx.data[4]);
      break;
    case ObdPid::BAROMETRIC_PRESSURE:
      valueOut = PidDecoder::decodePressure(rx.data[3]);
      break;
    case ObdPid::CATALYST_TEMP_B1S1:
    case ObdPid::CATALYST_TEMP_B2S1:
    case ObdPid::CATALYST_TEMP_B1S2:
    case ObdPid::CATALYST_TEMP_B2S2:
      valueOut = PidDecoder::decodeCatalystTemp(rx.data[3], rx.data[4]);
      break;
    case ObdPid::CONTROL_MODULE_VOLTAGE:
      valueOut = PidDecoder::decodeControlModuleVoltage(rx.data[3], rx.data[4]);
      break;
    case ObdPid::ABSOLUTE_LOAD_VALUE:
      valueOut = PidDecoder::decodeAbsoluteLoad(rx.data[3], rx.data[4]);
      break;
    case ObdPid::COMMANDED_EQUIV_RATIO:
      valueOut = PidDecoder::decodeCommandedEquivRatio(rx.data[3], rx.data[4]);
      break;
    case ObdPid::RELATIVE_THROTTLE_POS:
    case ObdPid::ABSOLUTE_THROTTLE_POS_B:
    case ObdPid::ABSOLUTE_THROTTLE_POS_C:
    case ObdPid::ACCELERATOR_PEDAL_POS_D:
    case ObdPid::ACCELERATOR_PEDAL_POS_E:
    case ObdPid::ACCELERATOR_PEDAL_POS_F:
    case ObdPid::COMMANDED_THROTTLE_ACTUATOR:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::AMBIENT_AIR_TEMP:
      valueOut = PidDecoder::decodeTemperature(rx.data[3]);
      break;
    case ObdPid::TIME_RUN_WITH_MIL:
    case ObdPid::TIME_SINCE_CODES_CLEARED:
      valueOut = PidDecoder::decodeTime(rx.data[3], rx.data[4]);
      break;
    case ObdPid::MAX_MAF_FLOW:
      valueOut = PidDecoder::decodeMaxMafFlow(rx.data[3]);
      break;
    case ObdPid::ETHANOL_FUEL_PERCENT:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::ABS_EVAP_VAPOR_PRESSURE:
      valueOut = PidDecoder::decodeAbsEvapVaporPressure(rx.data[3], rx.data[4]);
      break;
    case ObdPid::EVAP_VAPOR_PRESSURE_ALT:
      valueOut = PidDecoder::decodeEvapVaporPressureAlt(rx.data[3], rx.data[4]);
      break;
    case ObdPid::FUEL_RAIL_ABS_PRESSURE:
      valueOut = PidDecoder::decodeFuelRailGaugePressure(rx.data[3], rx.data[4]);
      break;
    case ObdPid::RELATIVE_ACCEL_PEDAL_POS:
    case ObdPid::HYBRID_BATTERY_REMAINING:
      valueOut = PidDecoder::decodePercentage(rx.data[3]);
      break;
    case ObdPid::ENGINE_OIL_TEMP:
      valueOut = PidDecoder::decodeTemperature(rx.data[3]);
      break;
    case ObdPid::FUEL_INJECTION_TIMING:
      valueOut = PidDecoder::decodeFuelInjectionTiming(rx.data[3], rx.data[4]);
      break;
    case ObdPid::ENGINE_FUEL_RATE:
      valueOut = PidDecoder::decodeEngineFuelRate(rx.data[3], rx.data[4]);
      break;
    case ObdPid::DEMAND_ENGINE_TORQUE:
    case ObdPid::ACTUAL_ENGINE_TORQUE:
    case ObdPid::ENGINE_PERCENT_TORQUE:
      valueOut = PidDecoder::decodeTorquePercentage(rx.data[3]);
      break;
    case ObdPid::ENGINE_REFERENCE_TORQUE:
      valueOut = PidDecoder::decodeReferenceTorque(rx.data[3], rx.data[4]);
      break;
    case ObdPid::ENGINE_COOLANT_TEMP_SENSOR:
    case ObdPid::INTAKE_AIR_TEMP_SENSOR:
      valueOut = PidDecoder::decodeTemperature(rx.data[3]);
      break;
    default:
      return false;
    }
    return true;
  }
  return false;
}

bool ObdService::isValidResponse(const CanFrame &frame, ObdPid requestedPid) {
  // Check if frame ID is in OBD-II response range
  if (frame.id >= 0x7E8 && frame.id <= 0x7EF) {
    // DLC must be at least 3 (Length, Mode, PID)
    if (frame.dlc < 3) return false;

    const uint8 pciLen = frame.data[0];
    const uint8 rxMode = frame.data[1];
    const uint8 rxPid = frame.data[2];

    // ISO 15765-2 Single Frame length must match (Service + PID + Data)
    const int32 expectedDataLen = getExpectedDataLength(requestedPid);
    if (pciLen != (1 + 1 + expectedDataLen)) return false;

    // CAN DLC must be sufficient to hold the PCI length + 1 (for the length byte itself)
    if (frame.dlc < (pciLen + 1)) return false;

    // Service must be Mode 01 + Response Offset (0x40) and PID must match
    if (rxMode == (OBD_MODE_SHOW_CURRENT + OBD_RESPONSE_OFFSET) && rxPid == static_cast<uint8>(requestedPid)) {
        return true;
    }
  }
  return false;
}
