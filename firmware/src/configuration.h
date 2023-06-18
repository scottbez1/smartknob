#pragma once

#include <SPIFFS.h>
#include <PacketSerial.h>

#include "proto_gen/smartknob.pb.h"

#include "logger.h"

const uint32_t PERSISTENT_CONFIGURATION_VERSION = 1;

class Configuration {
    public:
        Configuration();
        ~Configuration();

        void setLogger(Logger* logger);
        bool loadFromDisk();
        bool saveToDisk();
        PB_PersistentConfiguration get();
        bool setMotorCalibrationAndSave(PB_MotorCalibration& motor_calibration);

    private:
        SemaphoreHandle_t mutex_;

        Logger* logger_ = nullptr;
        bool loaded_ = false;
        PB_PersistentConfiguration pb_buffer_ = {};

        uint8_t buffer_[PB_PersistentConfiguration_size];

        void log(const char* msg);
};
class SPIFFSGuard {
    public:
        SPIFFSGuard(Logger* logger) : logger_(logger) {
            if (!SPIFFS.begin(true)) {
                if (logger != nullptr) {
                    logger->log("Failed to mount SPIFFS");
                }
                return;
            }
            ok_ = true;
        }
        ~SPIFFSGuard() {
            if (ok_) {
                SPIFFS.end();
                logger_->log("Unmounted SPIFFS");
            }
        }
        SPIFFSGuard(SPIFFSGuard const&)=delete;
        SPIFFSGuard& operator=(SPIFFSGuard const&)=delete;

        bool ok_ = false;

    private:
        Logger* logger_;
};
