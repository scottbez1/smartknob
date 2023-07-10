#pragma once

#include <FFat.h>
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
        bool setStrainCalibrationAndSave(PB_StrainCalibration& strain_calibration);

    private:
        SemaphoreHandle_t mutex_;

        Logger* logger_ = nullptr;
        bool loaded_ = false;
        PB_PersistentConfiguration pb_buffer_ = {};

        uint8_t buffer_[PB_PersistentConfiguration_size];

        void log(const char* msg);
};
class FatGuard {
    public:
        FatGuard(Logger* logger) : logger_(logger) {
            if (!FFat.begin(true)) {
                if (logger_ != nullptr) {
                    logger_->log("Failed to mount FFat");
                }
                return;
            }
            if (logger_ != nullptr) {
                logger_->log("Mounted FFat");
            }
            mounted_ = true;
        }
        ~FatGuard() {
            if (mounted_) {
                FFat.end();
                if (logger_ != nullptr) {
                    logger_->log("Unmounted FFat");
                }
            }
        }
        FatGuard(FatGuard const&)=delete;
        FatGuard& operator=(FatGuard const&)=delete;

        bool mounted_ = false;

    private:
        Logger* logger_;
};
