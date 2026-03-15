#include "sd/SDLogging.h"
#include "util/Logger.h"
#include "Config.h"
#include "tf_card.h"
#include <cstdio>
#include <cstring>

SDLogging::SDLogging() : is_initialized_(false), is_file_open_(false) {}

SDLogging::~SDLogging() {
    closeLogFile();
    f_unmount("");
}

bool SDLogging::init() {
    Logger::instance().log("SD: Configuring SPI and mounting filesystem...");
    
    pico_fatfs_spi_config_t config = {
        SD_SPI_INST,
        100 * 1000,
        30 * 1000 * 1000,
        Config::SD_SPI_MISO_PIN,
        Config::SD_SPI_CS_PIN,
        Config::SD_SPI_SCK_PIN,
        Config::SD_SPI_MOSI_PIN,
        true
    };
    pico_fatfs_set_config(&config);

    FRESULT fr = f_mount(&fs_, "", 1);
    if (fr != FR_OK) {
        Logger::instance().log("SD: Mount failed (%d)", fr);
        return false;
    }

    is_initialized_ = true;
    Logger::instance().log("SD: Filesystem mounted successfully");
    return true;
}

bool SDLogging::openNewLogFile() {
    if (!is_initialized_) return false;

    char filename[16];
    int file_index = 1;
    FILINFO fno;
    
    while (file_index < 1000) {
        snprintf(filename, sizeof(filename), "log_%03d.csv", file_index);
        FRESULT fr = f_stat(filename, &fno);
        if (fr == FR_NO_FILE) {
            break;
        }
        file_index++;
    }

    current_filename_ = filename;
    Logger::instance().log("SD: Creating file %s", current_filename_.c_str());

    FRESULT fr = f_open(&file_, current_filename_.c_str(), FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        Logger::instance().log("SD: Open failed (%d)", fr);
        return false;
    }

    // Comprehensive CSV Header
    const char* header = "timestamp,rpm,speed,coolant,oil_temp,throttle,load,map,intake_temp,ambient_temp,catalyst_temp,st_trim,lt_trim,fuel_rate\n";
    UINT bw;
    f_write(&file_, header, strlen(header), &bw);
    f_sync(&file_);

    is_file_open_ = true;
    return true;
}

bool SDLogging::logData(const ObdLogMessage& data) {
    if (!is_file_open_) return false;

    char buffer[512];
    int len = snprintf(buffer, sizeof(buffer), 
        "%u,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
        (unsigned int)data.timestamp, 
        data.engine_speed, 
        data.vehicle_speed, 
        data.engine_coolant_temp,
        data.engine_oil_temp,
        data.throttle_pos, 
        data.engine_load,
        data.intake_map,
        data.intake_air_temp,
        data.ambient_air_temp,
        data.catalyst_temp,
        data.st_fuel_trim_1,
        data.lt_fuel_trim_1,
        data.engine_fuel_rate
    );

    if (len > 0) {
        UINT bw;
        FRESULT fr = f_write(&file_, buffer, len, &bw);
        if (fr != FR_OK) {
            Logger::instance().log("SD: Write error (%d)", fr);
            return false;
        }
        return true;
    }

    return false;
}

void SDLogging::closeLogFile() {
    if (is_file_open_) {
        f_close(&file_);
        is_file_open_ = false;
        Logger::instance().log("SD: File %s closed", current_filename_.c_str());
    }
}

void SDLogging::sync() {
    if (is_file_open_) {
        f_sync(&file_);
    }
}
