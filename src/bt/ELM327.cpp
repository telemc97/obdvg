#include "bt/ELM327.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cctype>

ELM327::ELM327() : echo_(true), linefeeds_(false), showHeaders_(false), pendingRequest_(false), 
                   headerId_(0x7DF), isExtendedId_(false),
                   lastMode_(0), lastPid_(0), waitingForResponse_(false) {}

String ELM327::processInput(const String& input) {
    String cleanInput = "";
    for (char c : input) {
        if (c == '\r' || c == '\n') break;
        if (c != ' ') {
            cleanInput += (char)toupper(c);
        }
    }

    if (cleanInput.empty()) {
        return ">";
    }

    String response = "";
    if (echo_) {
        response += cleanInput + "\r";
    }

    if (cleanInput.length() >= 2 && cleanInput.substr(0, 2) == "AT") {
        response += handleAtCommand(cleanInput.substr(2));
        response += "\r>";
    } else {
        String obdResult = handleObdRequest(cleanInput);
        if (obdResult == "?") {
            response += "?\r>";
        } else {
            // It was a valid OBD request, we return ONLY the echo (if enabled)
            if (!echo_) return ""; 
        }
    }

    return response;
}

String ELM327::handleAtCommand(const String& cmd) {
    if (cmd == "Z") { // Reset
        echo_ = true;
        linefeeds_ = false;
        showHeaders_ = false;
        headerId_ = 0x7DF;
        isExtendedId_ = false;
        waitingForResponse_ = false;
        return "ELM327 v1.5\rOK";
    } else if (cmd == "E0") {
        echo_ = false;
        return "OK";
    } else if (cmd == "E1") {
        echo_ = true;
        return "OK";
    } else if (cmd == "H0") { // Headers Off
        showHeaders_ = false;
        return "OK";
    } else if (cmd == "H1") { // Headers On
        showHeaders_ = true;
        return "OK";
    } else if (cmd == "L0") {
        linefeeds_ = false;
        return "OK";
    } else if (cmd == "L1") {
        linefeeds_ = true;
        return "OK";
    } else if (cmd.substr(0, 2) == "SH") { // Set Header (e.g. ATSH 7E0)
        String hexId = cmd.substr(2);
        unsigned int id;
        if (sscanf(hexId.c_str(), "%x", &id) == 1) {
            headerId_ = (uint32)id;
            isExtendedId_ = (hexId.length() > 3);
            return "OK";
        }
        return "?";
    } else if (cmd.substr(0, 2) == "SP") { // Set Protocol
        return "OK";
    } else if (cmd == "I") {
        return "ELM327 v1.5";
    } else if (cmd == "DP") {
        return "AUTO, CAN (11/500)";
    }
    return "OK";
}

String ELM327::handleObdRequest(const String& req) {
    // Check if it's a hex request (Mode + PID, e.g. "010C")
    if (req.length() >= 2) {
        unsigned int mode = 0, pid = 0;
        int parsed = sscanf(req.c_str(), "%02x%02x", &mode, &pid);
        
        if (parsed >= 1) {
            nextFrame_.id = headerId_;
            nextFrame_.dlc = 8;
            std::memset(nextFrame_.data.data(), 0, 8);
            
            if (parsed == 2) { // Mode + PID
                nextFrame_.data[0] = 0x02;
                nextFrame_.data[1] = (uint8)mode;
                nextFrame_.data[2] = (uint8)pid;
                lastPid_ = (uint8)pid;
            } else { // Mode only
                nextFrame_.data[0] = 0x01;
                nextFrame_.data[1] = (uint8)mode;
                lastPid_ = 0xFF; // Special marker
            }
            
            lastMode_ = (uint8)mode;
            pendingRequest_ = true;
            waitingForResponse_ = true;
            return "OK";
        }
    }
    return "?";
}

bool ELM327::hasPendingCanRequest(CanFrame& outFrame) {
    if (pendingRequest_) {
        outFrame = nextFrame_;
        pendingRequest_ = false;
        return true;
    }
    return false;
}

String ELM327::formatCanResponse(const CanFrame& rx) {
    if (!waitingForResponse_) return "";

    // Standard OBD response check: ID should match RequestHeader + 8 
    // or be in the range 0x7E8-0x7EF
    bool match = false;
    if (headerId_ == 0x7DF) {
        match = (rx.id >= 0x7E8 && rx.id <= 0x7EF);
    } else {
        match = (rx.id == (headerId_ + 8));
    }

    if (match && rx.dlc >= 2) {
        uint8 rxMode = rx.data[1];
        
        // Response mode is Request mode + 0x40
        if (rxMode == (lastMode_ + 0x40)) {
            char buf[128];
            int offset = 0;

            if (showHeaders_) {
                offset += snprintf(buf + offset, sizeof(buf) - offset, "%03X ", rx.id);
            }

            // Standard ELM format: Space separated hex bytes
            // Note: Data[0] is length in ISO 15765-2
            int dataLen = rx.data[0];
            for (int i = 0; i < dataLen && (i + 1) < 8; i++) {
                offset += snprintf(buf + offset, sizeof(buf) - offset, "%02X ", rx.data[i + 1]);
            }
            
            waitingForResponse_ = false;
            String result(buf);
            // Remove trailing space
            if (!result.empty() && result.back() == ' ') result.pop_back();
            
            return result + "\r>";
        }
    }
    return "";
}
