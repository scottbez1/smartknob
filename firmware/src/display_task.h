#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "logger.h"
#include "proto_gen/smartknob.pb.h"
#include "task.h"
#include "ui/ui_state.h"

class DisplayTask : public Task<DisplayTask> {
    friend class Task<DisplayTask>; // Allow base Task to invoke protected run()

    public:
        DisplayTask(const uint8_t task_core);
        ~DisplayTask();

        QueueHandle_t getKnobStateQueue();

        void setBrightness(uint16_t brightness);
        void setLogger(Logger* logger);

        /** Called from the interface task whenever the menu/app state changes. */
        void setUiState(const UiState& ui_state);

    protected:
        void run();

    private:
        TFT_eSPI tft_ = TFT_eSPI();

        /** Full-size sprite used as a framebuffer */
        TFT_eSprite spr_ = TFT_eSprite(&tft_);

        QueueHandle_t knob_state_queue_;

        PB_SmartKnobState state_;
        UiState ui_state_;
        SemaphoreHandle_t mutex_;
        uint16_t brightness_;
        Logger* logger_;
        void log(const char* msg);
};

#else

class DisplayTask {};

#endif
