#ifndef OBDVG_SDLOGGING_H
#define OBDVG_SDLOGGING_H

#include "Types.h"
#include "obd/ObdLogMessage.h"
#include "ff.h"

/**
 * @class SDLogging
 * @brief Handles vehicle data logging to an SPI SD card using FatFs.
 */
class SDLogging {
public:
    SDLogging();
    ~SDLogging();

    /**
     * @brief Initializes the SD card and mounts the filesystem.
     * @return True if successful.
     */
    bool init();

    /**
     * @brief Creates a new log file on the SD card.
     *        Automatically increments filename (e.g. log_001.csv).
     * @return True if successful.
     */
    bool openNewLogFile();

    /**
     * @brief Logs a single OBD data message to the current file.
     * @param data The OBD data to log.
     * @return True if successful.
     */
    bool logData(const ObdLogMessage& data);

    /**
     * @brief Closes the current log file.
     */
    void closeLogFile();

    /**
     * @brief Flushes data to the SD card to prevent data loss.
     */
    void sync();

private:
    bool is_initialized_ = false;
    bool is_file_open_ = false;
    
    FIL file_;
    FATFS fs_;
    
    String current_filename_;
};

#endif //OBDVG_SDLOGGING_H
