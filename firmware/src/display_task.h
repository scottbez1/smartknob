#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "lvgl.h"

#include "logger.h"
#include "proto_gen/smartknob.pb.h"
#include "task.h"

#define DISP_BUF_SIZE  (TFT_WIDTH * 10)

class DisplayTask : public Task<DisplayTask> {
    friend class Task<DisplayTask>; // Allow base Task to invoke protected run()

    public:
        DisplayTask(const uint8_t task_core);
        ~DisplayTask();

        QueueHandle_t getKnobStateQueue();

        void setBrightness(uint16_t brightness);
        void setLogger(Logger* logger);

    protected:
        void run();

    private:

        lv_obj_t * screen;
        lv_obj_t * label_cur_pos;
        lv_obj_t * label_desc;
        lv_obj_t * arc;
        lv_obj_t * gauge;
        lv_obj_t * roller;
        lv_obj_t * line_left_bound;
        lv_obj_t * line_right_bound;
        lv_obj_t * arc_dot;

        QueueHandle_t knob_state_queue_;

        PB_SmartKnobState state_;
        SemaphoreHandle_t mutex_;
        uint16_t brightness_;
        Logger* logger_;
        void log(const char* msg);
};

#else

class DisplayTask {};

#endif
