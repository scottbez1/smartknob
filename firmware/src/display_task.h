#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "knob_data.h"
#include "task.h"

class DisplayTask : public Task<DisplayTask> {
    friend class Task<DisplayTask>; // Allow base Task to invoke protected run()

    public:
        DisplayTask(const uint8_t task_core);
        ~DisplayTask();

        QueueHandle_t getKnobStateQueue();

        void setBrightness(uint16_t brightness);

    protected:
        void run();

    private:
        TFT_eSPI tft_ = TFT_eSPI();

        /** Full-size sprite used as a framebuffer */
        TFT_eSprite spr_ = TFT_eSprite(&tft_);

        QueueHandle_t knob_state_queue_;

        KnobState state_;

        SemaphoreHandle_t mutex_;

        uint16_t brightness_;
};

#else

class DisplayTask {};

#endif
