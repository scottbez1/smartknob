#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"

#include "font/roboto_light_60.h"

static const uint8_t LEDC_CHANNEL_LCD_BACKLIGHT = 0;

TFT_eSPI tft_ = TFT_eSPI();

static lv_disp_draw_buf_t disp_buf;
uint32_t size_in_px = DISP_BUF_SIZE;
static lv_color_t buf[DISP_BUF_SIZE];

static lv_point_t points_left_bound[2] = {{0,0},{0,0}};
static lv_point_t points_right_bound[2] = {{0,0},{0,0}};

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 4096, 1, task_core} {
  knob_state_queue_ = xQueueCreate(1, sizeof(PB_SmartKnobState));
  assert(knob_state_queue_ != NULL);

  mutex_ = xSemaphoreCreateMutex();
  assert(mutex_ != NULL);
}

DisplayTask::~DisplayTask() {
  vQueueDelete(knob_state_queue_);
  vSemaphoreDelete(mutex_);
}

void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    uint32_t wh = w*h;

    tft_.startWrite();
    tft_.setAddrWindow(area->x1, area->y1, w, h);
    tft_.pushColors(&color_p->full, w * h, true);
    tft_.endWrite();

    lv_disp_flush_ready(disp);
}

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(0);
    tft_.fillScreen(TFT_DARKGREEN);

    ledcSetup(LEDC_CHANNEL_LCD_BACKLIGHT, 5000, 16);
    ledcAttachPin(PIN_LCD_BACKLIGHT, LEDC_CHANNEL_LCD_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, UINT16_MAX);

    lv_init();

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
      
    log("Display Driver Init");
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);
      
    log("Display Driver Register");

      

        //Create lvgl objects
    screen = lv_scr_act();
    
    lv_obj_set_style_bg_color(screen,lv_color_hex(0x383838),0);       
    lv_obj_set_style_bg_opa( screen,LV_OPA_COVER,0);
    lv_obj_set_style_bg_grad_color(screen,lv_color_hex(0x7e4dfa),0);   
    lv_obj_set_style_bg_grad_dir(screen,LV_GRAD_DIR_VER,0);   
    lv_obj_set_style_bg_grad_stop(screen,TFT_HEIGHT,0);
    lv_obj_set_style_bg_main_stop(screen,TFT_HEIGHT,0);       //Same value for gradient and main gives sharp line on Gradient


    //Testing with gauge. Not really suitable after a short test. Maybe create own lvgl Widget that is more usable with the smartknob
    /**gauge = lv_gauge_create(screen,NULL);
    lv_obj_clean_style_list(gauge,LV_GAUGE_PART_MAIN);
    lv_obj_set_size(gauge,210,210);
    lv_obj_align(gauge,screen,LV_ALIGN_CENTER,0,0);
    lv_gauge_set_range(gauge,0,10);
    lv_gauge_set_scale(gauge,360,10,10);
    lv_gauge_set_value(gauge,0,4);**/

    /*Create line style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 4);
    lv_style_set_line_color(&style_line, lv_color_white());
    lv_style_set_line_rounded(&style_line, true);

    line_left_bound = lv_line_create(screen);
    lv_obj_add_style(line_left_bound,&style_line, 0);
    lv_obj_set_pos(line_left_bound,0,0);
    

    line_right_bound = lv_line_create(screen);
    lv_obj_add_style(line_right_bound,&style_line, 0);
    lv_obj_set_pos(line_right_bound,0,0);

    arc_dot = lv_arc_create(screen);//use this arc only for the knob
    //lv_style_reset(arc_dot,LV_PART_MAIN);//we dont need a background
    //lv_obj_set_style_bg_opa();
    lv_obj_remove_style(arc_dot,NULL,0);
    lv_arc_set_bg_angles(arc_dot,0,0);
    lv_obj_set_style_line_width(arc_dot,20,0);
    lv_obj_set_style_pad_all(arc_dot,10,0);//line width and padding are used in calculation for the knob and arc so we have to set them
    //lv_obj_set_style_bg_opa(arc_dot,LV_ARC_PART_BG,LV_STATE_DEFAULT,LV_OPA_TRANSP);
    //lv_obj_set_style_bg_opa(arc_dot,LV_ARC_PART_INDIC,LV_STATE_DEFAULT,LV_OPA_TRANSP);
    lv_obj_remove_style(arc_dot,NULL,LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_size(arc_dot,TFT_WIDTH,TFT_HEIGHT);
    lv_obj_align(arc_dot,LV_ALIGN_CENTER,0,0);
    //lv_arc_set_adjustable(arc_dot,true);//knob showing
    lv_obj_add_flag(arc_dot,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(arc_dot, LV_OBJ_FLAG_CLICKABLE);

    lv_arc_set_angles(arc_dot,0,0);
    //lv_arc_set_bg_angles(arc_dot,150,260);
    //lv_arc_set_value(arc_dot,20);
    lv_obj_set_style_line_width(arc_dot,10,LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(arc_dot,5,LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_arc_set_rotation(arc_dot, 270);

    arc = lv_arc_create(screen);
    //lv_obj_clean_style_list(arc,LV_ARC_PART_BG);
    lv_obj_remove_style(arc,NULL,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(arc,0,0);
    lv_obj_set_style_line_width(arc,20,0);
    lv_obj_set_style_pad_all(arc,10,0);
    lv_obj_set_size(arc,TFT_WIDTH,TFT_HEIGHT);
    lv_obj_align(arc,LV_ALIGN_CENTER,0,0);
    //lv_arc_set_adjustable(arc,false);//knob hidden 
    lv_obj_add_flag(arc,LV_OBJ_FLAG_SCROLLABLE);
    lv_arc_set_angles(arc,0,0);
    lv_obj_set_style_line_width(arc,10,LV_PART_INDICATOR|LV_STATE_DEFAULT);
    //lv_obj_set_style_pad_all(arc,LV_ARC_PART_KNOB,LV_STATE_DEFAULT,5);
  lv_arc_set_rotation(arc, 270);

    //lv_obj_set_style_line_color(arc_dot,LV_ARC_PART_INDIC,LV_STATE_DEFAULT,LV_COLOR_TEAL);
    //lv_obj_set_style_line_color(arc_dot,LV_ARC_PART_KNOB,LV_STATE_DEFAULT,LV_COLOR_CYAN);
    //lv_obj_set_style_pad_all(arc_dot,LV_ARC_PART_INDIC,LV_STATE_DEFAULT,8);
    //lv_obj_set_style_size(arc_dot,LV_ARC_PART_KNOB,LV_STATE_DEFAULT,10);
    
    

    label_cur_pos = lv_label_create(screen);
    //lv_obj_set_pos(label_cur_pos,TFT_WIDTH/2,TFT_HEIGHT/2-VALUE_OFFSET);
    //lv_label_set_align(label_cur_pos,LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_text_align(label_cur_pos,LV_ALIGN_CENTER,LV_PART_MAIN);
    lv_obj_align(label_cur_pos,LV_ALIGN_CENTER,0,-VALUE_OFFSET);
    lv_label_set_text(label_cur_pos,"0");
    lv_obj_set_style_text_color(label_cur_pos,lv_color_white(),0);
    lv_obj_set_style_text_font(label_cur_pos,&lv_font_montserrat_28,0);

    label_desc = lv_label_create(screen);
    //lv_obj_set_pos(label_desc,TFT_WIDTH/2,TFT_HEIGHT / 2 + DESCRIPTION_Y_OFFSET);
    lv_obj_set_style_text_align(label_desc,LV_ALIGN_CENTER,LV_PART_MAIN);
    lv_obj_align(label_desc,LV_ALIGN_CENTER,0,DESCRIPTION_Y_OFFSET);
    lv_label_set_text(label_desc,"");
    lv_obj_set_style_text_color(label_desc,lv_color_white(),0);
    lv_obj_set_style_text_font(label_desc,&lv_font_montserrat_16,0);

    
    PB_SmartKnobState state;

    const int RADIUS = TFT_WIDTH / 2;


    while(1) {
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) == pdFALSE) {
          continue;
        }

        if (state.config.num_positions > 1) {
          int32_t height = state.current_position * TFT_HEIGHT / (state.config.num_positions-1);
          lv_obj_set_style_bg_grad_stop(screen,TFT_HEIGHT-height,0);
          lv_obj_set_style_bg_main_stop(screen,TFT_HEIGHT-height,0);       //Same value for gradient and main gives sharp line on Gradient
        }

        lv_label_set_text_fmt(label_cur_pos,"%d" , state.current_position);
        lv_obj_set_style_text_align(label_cur_pos,LV_ALIGN_CENTER,LV_PART_MAIN);
        lv_obj_align(label_cur_pos,LV_ALIGN_CENTER,0,-VALUE_OFFSET);
        
        lv_label_set_text(label_desc,state.config.text);
        lv_obj_set_style_text_align(label_desc,LV_ALIGN_CENTER,LV_PART_MAIN);
        lv_obj_align(label_desc,LV_ALIGN_CENTER,0,DESCRIPTION_Y_OFFSET);

        float left_bound = PI / 2;

        if (state.config.num_positions > 0) {
          float range_radians = (state.config.num_positions - 1) * state.config.position_width_radians;
          left_bound = PI / 2 + range_radians / 2;
          float right_bound = PI / 2 - range_radians / 2;
          points_left_bound[0] = {
                                  (lv_coord_t)(TFT_WIDTH/2 + RADIUS * cosf(left_bound)),
                                  (lv_coord_t)(TFT_HEIGHT/2 - RADIUS * sinf(left_bound))
                                };
          points_left_bound[1] = {
                                  (lv_coord_t)(TFT_WIDTH/2 + (RADIUS - 10) * cosf(left_bound)),
                                  (lv_coord_t)(TFT_HEIGHT/2 - (RADIUS - 10) * sinf(left_bound))
                                };
          lv_line_set_points(line_left_bound,points_left_bound,2);
          points_right_bound[0] = {
                                  (lv_coord_t)(TFT_WIDTH/2 + RADIUS * cosf(right_bound)),
                                  (lv_coord_t)(TFT_HEIGHT/2 - RADIUS * sinf(right_bound))
                                };
          points_right_bound[1] = {
                                  (lv_coord_t)(TFT_WIDTH/2 + (RADIUS - 10) * cosf(right_bound)),
                                  (lv_coord_t)(TFT_HEIGHT/2 - (RADIUS - 10) * sinf(right_bound))
                                };                           
          lv_line_set_points(line_right_bound,points_right_bound,2);
        }
        else{
          points_left_bound[0] = {0,0};
          points_left_bound[1] = {0,0};
          lv_line_set_points(line_left_bound,points_left_bound,2);
          points_right_bound[0] = {0,0};
          points_right_bound[1] = {0,0};
          lv_line_set_points(line_right_bound,points_right_bound,2);
        }

        float adjusted_sub_position = state.sub_position_unit * state.config.position_width_radians;
        if (state.config.num_positions > 0) {
          if (state.current_position == 0 && state.sub_position_unit < 0) {
            adjusted_sub_position = -logf(1 - state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180) * 5 * PI / 180;
          } else if (state.current_position == state.config.num_positions - 1 && state.sub_position_unit > 0) {
            adjusted_sub_position = logf(1 + state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180)  * 5 * PI / 180;
          }
        }

        float raw_angle = left_bound - state.current_position * state.config.position_width_radians;
        float adjusted_angle = raw_angle - adjusted_sub_position;
        int32_t raw_angle_offset = (int)(-((raw_angle*(180/PI))-90))%360;
        int32_t adjusted_angle_offset = (int)(-((adjusted_angle*(180/PI))-90))%360;

        lv_arc_set_angles(arc_dot,raw_angle_offset,raw_angle_offset);

        if(adjusted_sub_position!=0){
          lv_obj_add_flag(arc,LV_OBJ_FLAG_HIDDEN);
          if (raw_angle_offset < adjusted_angle_offset) {
            lv_arc_set_angles(arc,raw_angle_offset,adjusted_angle_offset);
          } else {
            lv_arc_set_angles(arc,adjusted_angle_offset,raw_angle_offset);
          }
        }
        else{
          lv_obj_clear_flag(arc,LV_OBJ_FLAG_HIDDEN);
        }

        lv_task_handler();

        {
          SemaphoreGuard lock(mutex_);
          ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, brightness_);
        }
        delay(2);
    }
}

QueueHandle_t DisplayTask::getKnobStateQueue() {
  return knob_state_queue_;
}

void DisplayTask::setBrightness(uint16_t brightness) {
  SemaphoreGuard lock(mutex_);
  brightness_ = brightness;
}

void DisplayTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void DisplayTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}

#endif