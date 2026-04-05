#include "sd/SDLogging.h"
#include "util/Logger.h"
#include "Config.h"
#include "tf_card.h"
#include <cstdio>
#include <cstring>

SDLogging::SDLogging() : is_initialized_(false), is_file_open_(false) {
    mutex_ = xSemaphoreCreateMutex();
}

SDLogging::~SDLogging() {
    closeLogFile();
    f_unmount("");
    if (mutex_) {
        vSemaphoreDelete(mutex_);
    }
}

bool SDLogging::init() {
    Logger::instance().log("SD: Configuring SPI and mounting filesystem...");
    
    pico_fatfs_spi_config_t config = {
        SD_SPI_INST,
        Config::SD_SPI_FREQ_INIT,
        Config::SD_SPI_FREQ_OP,
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
    if (!is_initialized_ || !mutex_) return false;

    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE) {
        return false;
    }

    char filename[16];
    int32 file_index = 1;
    FILINFO fno;
    
    while (file_index < 1000) {
        std::snprintf(filename, sizeof(filename), "log_%03d.csv", (int32)file_index);
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
        xSemaphoreGive(mutex_);
        return false;
    }

    // Comprehensive CSV Header
    const char* header = "timestamp,rpm,speed,coolant,oil_temp,throttle,load,map,intake_temp,ambient_temp,catalyst_temp,st_trim,lt_trim,fuel_rate\n";
    UINT bw;
    UINT headerLen = (UINT)std::strlen(header);
    fr = f_write(&file_, header, headerLen, &bw);
    
    if (fr != FR_OK || bw != headerLen) {
        Logger::instance().log("SD: Header write failed (Disk full?)");
        f_close(&file_);
        xSemaphoreGive(mutex_);
        return false;
    }

    f_sync(&file_);

    is_file_open_ = true;
    xSemaphoreGive(mutex_);
    return true;
}

bool SDLogging::logData(const ObdLogMessage& data) {
    if (!is_file_open_ || !mutex_) return false;

    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) {
        return false;
    }

    char buffer[512];
    int32 len = std::snprintf(buffer, sizeof(buffer), 
        "%u,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
        static_cast<uint32>(data.timestamp), 
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
        FRESULT fr = f_write(&file_, buffer, (UINT)len, &bw);
        if (fr != FR_OK || bw != (UINT)len) {
            Logger::instance().log("SD: Write error (Disk full?)");
            xSemaphoreGive(mutex_);
            return false;
        }
        xSemaphoreGive(mutex_);
        return true;
    }

    xSemaphoreGive(mutex_);
    return false;
}

void SDLogging::closeLogFile() {
    if (!mutex_) return;
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (is_file_open_) {
            f_close(&file_);
            is_file_open_ = false;
            Logger::instance().log("SD: File %s closed", current_filename_.c_str());
        }
        xSemaphoreGive(mutex_);
    }
}

void SDLogging::sync() {
    if (!mutex_) return;
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (is_file_open_) {
            f_sync(&file_);
        }
        xSemaphoreGive(mutex_);
    }
}
