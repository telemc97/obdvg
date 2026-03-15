#include "btstack.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "Config.h"
#include "can/CanFrame.h"
#include "bt/ELM327.h"
#include "util/Logger.h"

// Generated GATT header
#include "elm327.h"

extern QueueHandle_t canTxQueue;
extern QueueHandle_t canRxQueue;

static btstack_packet_callback_registration_t hci_event_callback_registration;
static ELM327 elm;
static uint16 con_handle = 0;

// Forward declarations
static void packet_handler(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size);
static uint16 att_read_callback(hci_con_handle_t connection_handle, uint16 att_handle, uint16 offset, uint8 * buffer, uint16 buffer_size);
static int att_write_callback(hci_con_handle_t connection_handle, uint16 att_handle, uint16 transaction_mode, uint16 offset, uint8 *buffer, uint16 buffer_size);

void bluetoothTask(void* pvParameters) {
    (void)pvParameters;

    if (cyw43_arch_init()) {
        Logger::instance().log("BT Task: CYW43 init failed");
        vTaskDelete(nullptr);
    }
    
    l2cap_init();
    sm_init();
    att_server_init(profile_data, att_read_callback, att_write_callback);

    // Inform BTstack about our packet handler
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Turn on Bluetooth
    hci_power_control(HCI_POWER_ON);
    
    Logger::instance().log("BT Task: BLE ELM327 Initialized");

    CanFrame frame;
    while (true) {
        // 1. Check for vehicle responses in the queue
        // We drain the queue to find a match for our pending request
        while (xQueueReceive(canRxQueue, &frame, 0) == pdTRUE) {
            String resp = elm.formatCanResponse(frame);
            
            // If formatCanResponse returns a non-empty string, it means the frame 
            // matched our last request and we should send it to the phone.
            if (!resp.empty() && con_handle != 0) {
                att_server_notify(con_handle, ATT_CHARACTERISTIC_0000FFE1_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE, (uint8*)resp.c_str(), resp.length());
            }
        }

        vTaskDelay(pdMS_TO_TICKS(Config::BT_TASK_DELAY_MS));
    }
}

static void packet_handler(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size) {
    (void)channel;
    (void)size;

    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) == HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                Logger::instance().log("BT Task: Connected");
            }
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = 0;
            Logger::instance().log("BT Task: Disconnected");
            break;
        default:
            break;
    }
}

static uint16 att_read_callback(hci_con_handle_t connection_handle, uint16 att_handle, uint16 offset, uint8 * buffer, uint16 buffer_size) {
    (void)connection_handle;
    (void)att_handle;
    (void)offset;
    (void)buffer;
    (void)buffer_size;
    return 0;
}

static int att_write_callback(hci_con_handle_t connection_handle, uint16 att_handle, uint16 transaction_mode, uint16 offset, uint8 *buffer, uint16 buffer_size) {
    (void)transaction_mode;
    (void)offset;

    if (att_handle == ATT_CHARACTERISTIC_0000FFE1_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) {
        // Data received from phone
        String input((char*)buffer, buffer_size);
        
        // 1. Process via ELM emulator
        String response = elm.processInput(input);
        
        // 2. If it was an AT command or immediate error, send it back now
        if (!response.empty()) {
             att_server_notify(connection_handle, att_handle, (uint8*)response.c_str(), response.length());
        }
        
        // 3. If the emulator generated a CAN request, push it to the TX queue
        CanFrame txFrame;
        if (elm.hasPendingCanRequest(txFrame)) {
            xQueueSend(canTxQueue, &txFrame, 0);
        }
    }
    return 0;
}
