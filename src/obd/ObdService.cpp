#include "obd/ObdService.h"
#include "can/CanFrame.h"
#include "obd/PidDecoder.h"
#include <cstring>

ObdService::ObdService(CanBus& bus) : bus_(bus) {}

void ObdService::requestPid(uint8_t pid) {
    CanFrame frame{};
    frame.id = 0x7DF; // functional broadcast
    frame.dlc = 8;
    std::memset(frame.data.data(), 0, 8);
    frame.data[0] = 0x02; // length
    frame.data[1] = 0x01; // service 01
    frame.data[2] = pid;
    bus_.send(frame);
}

bool ObdService::pollResponse(uint8_t pid, float& valueOut) {
    CanFrame rx;
    while (bus_.receive(rx)) {
        if (rx.id >= 0x7E8 && rx.id <= 0x7EF && rx.dlc >= 3) {
            uint8_t svc = rx.data[1] & 0x3F;
            uint8_t rxPid = rx.data[2];
            if (svc == 0x01 && rxPid == pid) {
                if (pid == 0x0C) valueOut = PidDecoder::decodeRpm(rx.data[3], rx.data[4]);
                else if (pid == 0x0D) valueOut = PidDecoder::decodeSpeed(rx.data[3]);
                else if (pid == 0x05) valueOut = PidDecoder::decodeTemp(rx.data[3]);
                return true;
            }
        }
    }
    return false;
}
