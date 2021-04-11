#include "gui.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "gus_config.h"
#include "gus_data.h"

const struct device *display_dev;

////////////////////////////////////////////////////////////

 /*********************
  *      DEFINES
  *********************/
#define NAMELIST_LEN (MAX_GUS_NODES * (MAX_NAME_LENGTH+3))

  /**********************
   *      TYPEDEFS
   **********************/

   /**********************
    *  STATIC PROTOTYPES
    **********************/
static void update_control_visibility(void);
static void mode_buttons(lv_obj_t* parent);
static void badges_create(lv_obj_t* parent);
static void configure_create(lv_obj_t* parent);
static void analysis_create(lv_obj_t* parent);
static void slider_event_cb(lv_obj_t* slider, lv_event_t e);
static void ta_event_cb(lv_obj_t* ta, lv_event_t e);
static void kb_event_cb(lv_obj_t* ta, lv_event_t e);
//static void bar_anim(lv_task_t* t);

static void btn_badge_event_cb(lv_obj_t* btn, lv_event_t e);
static void btn_config_event_cb(lv_obj_t* btn, lv_event_t e);
static void btn_analyze_event_cb(lv_obj_t* btn, lv_event_t e);
static void btn_edit_name_event_cb(lv_obj_t* btn, lv_event_t e);
static void keyboard_create(lv_obj_t* parent);

/**********************
 *  STATIC VARIABLES
 **********************/
char namelist[NAMELIST_LEN];
 
static gui_event_t m_gui_event;
static gui_callback_t m_gui_callback = 0;
// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(m_gui_cmd_queue, sizeof(gui_message_t), 8, 4);



static lv_style_t style_box;
//static lv_style_t style_tab;


// mode controls
static lv_obj_t* btn_badges;
static lv_obj_t* btn_config;
static lv_obj_t* btn_analysis;

// badges widgets
static lv_obj_t* roller;
static lv_obj_t* btn_edit_name;
static lv_obj_t* cb_virus;
static lv_obj_t* cb_mask;
static lv_obj_t* cb_vaccine;
static lv_obj_t* btn_scan;
static lv_obj_t* btn_id;

// config widgets
static lv_obj_t* dd;
static lv_obj_t* label_rows;
static lv_obj_t * spinbox_rows;
static lv_obj_t * btn_row_up;
static lv_obj_t * btn_row_down;
static lv_obj_t * spinbox_space;
static lv_obj_t * btn_space_up;
static lv_obj_t * btn_space_down;
static lv_obj_t * spinbox_rate;
static lv_obj_t * btn_rate_up;
static lv_obj_t * btn_rate_down;


//static lv_obj_t* slider_rows;
static lv_obj_t* label_space;
//static lv_obj_t* slider_space;
static lv_obj_t* label_rate;
//static lv_obj_t* slider_rate;

// analysis widgets
// roller is shared
static lv_obj_t* bar;
static lv_obj_t* btn_rew;
static lv_obj_t* btn_play;
static lv_obj_t* btn_next;

// keyboard widgets
static lv_obj_t* ta;
static lv_obj_t* kb;


enum gus_mode { mode_badge, mode_config, mode_analyze, mode_edit_name} gus_mode;

/**********************
 *      MACROS
 **********************/

 /**********************
  *   GLOBAL FUNCTIONS
  **********************/

void lv_demo_widgets(void)
{
    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(25));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_top(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_bottom(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_left(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_right(&style_box, LV_STATE_DEFAULT, LV_DPX(1));


    mode_buttons(lv_scr_act());

    badges_create(lv_scr_act());
    configure_create(lv_scr_act());
    analysis_create(lv_scr_act());
    keyboard_create(lv_scr_act());

    gus_mode = mode_badge;
    //gus_mode = mode_config;
    //gus_mode = mode_analyze;
    update_control_visibility();



}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint16_t btnstate(enum gus_mode mode) 
{
    return gus_mode == mode ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_RELEASED;
}

static void update_control_visibility(void)
{
    lv_obj_set_hidden(roller, gus_mode != mode_badge && gus_mode != mode_analyze);


    lv_btn_set_state(btn_badges, btnstate(mode_badge));
    lv_btn_set_state(btn_config, btnstate(mode_config));
    lv_btn_set_state(btn_analysis, btnstate(mode_analyze));

    lv_obj_set_hidden(btn_edit_name, gus_mode != mode_badge);
    lv_obj_set_hidden(cb_virus, gus_mode != mode_badge);
    lv_obj_set_hidden(cb_mask, gus_mode != mode_badge);
    lv_obj_set_hidden(cb_vaccine, gus_mode != mode_badge);
    lv_obj_set_hidden(btn_scan, gus_mode != mode_badge);
    lv_obj_set_hidden(btn_id, gus_mode != mode_badge);

    lv_obj_set_hidden(dd, gus_mode != mode_config);

    lv_obj_set_hidden(label_rows, gus_mode != mode_config);
    lv_obj_set_hidden(spinbox_rows, gus_mode != mode_config);
    lv_obj_set_hidden(btn_row_up, gus_mode != mode_config);
    lv_obj_set_hidden(btn_row_down, gus_mode != mode_config);

    lv_obj_set_hidden(label_space, gus_mode != mode_config);
    lv_obj_set_hidden(spinbox_space, gus_mode != mode_config);
    lv_obj_set_hidden(btn_space_up, gus_mode != mode_config);
    lv_obj_set_hidden(btn_space_down, gus_mode != mode_config);

    lv_obj_set_hidden(label_rate, gus_mode != mode_config);
    lv_obj_set_hidden(spinbox_rate, gus_mode != mode_config);
    lv_obj_set_hidden(btn_rate_up, gus_mode != mode_config);
    lv_obj_set_hidden(btn_rate_down, gus_mode != mode_config);

    lv_obj_set_hidden(label_rate, gus_mode != mode_config);

    lv_obj_set_hidden(bar, gus_mode != mode_analyze);
    lv_obj_set_hidden(btn_rew, gus_mode != mode_analyze);
    lv_obj_set_hidden(btn_play, gus_mode != mode_analyze);
    lv_obj_set_hidden(btn_next, gus_mode != mode_analyze);

    lv_obj_set_hidden(kb, gus_mode != mode_edit_name);
    lv_obj_set_hidden(ta, gus_mode != mode_edit_name);

}

static void mode_buttons(lv_obj_t* parent)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, LV_STATE_DEFAULT, 5);

    btn_badges = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_badges, btn_badge_event_cb);
    lv_obj_t* label = lv_label_create(btn_badges, NULL);
    lv_label_set_text(label, "Badges");
    lv_obj_set_width(btn_badges, 75);
    lv_obj_set_pos(btn_badges, 92, 5);
    lv_obj_add_style(btn_badges, LV_OBJ_PART_MAIN, &style);

    btn_config = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_config, btn_config_event_cb);
    label = lv_label_create(btn_config, NULL);
    lv_label_set_text(label, "Config");
    lv_obj_set_width(btn_config, 75);
    lv_obj_set_pos(btn_config, 167, 5);
    lv_obj_add_style(btn_config, LV_OBJ_PART_MAIN, &style);

    btn_analysis = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_analysis, btn_analyze_event_cb);
    label = lv_label_create(btn_analysis, NULL);
    lv_label_set_text(label, "Analysis");
    lv_obj_set_width(btn_analysis, 75);
    lv_obj_set_pos(btn_analysis, 242, 5);
    lv_obj_add_style(btn_analysis, LV_OBJ_PART_MAIN, &style);


}

static void update_namelist(void) 
{
    int item = lv_roller_get_selected(roller);
    gd_get_namelist(namelist, NAMELIST_LEN);
    lv_roller_set_options(roller, namelist, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(roller, item, LV_ANIM_OFF);
}

static void update_checkboxes(void)
{
    int item = lv_roller_get_selected(roller);
    lv_checkbox_set_checked(cb_virus, is_patient_zero(item));
    lv_checkbox_set_checked(cb_mask, has_mask(item));
    lv_checkbox_set_checked(cb_vaccine, has_vaccine(item));
}

static void roller_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        update_checkboxes();
    }
}

static void cb_virus_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        set_patient_zero(lv_roller_get_selected(roller), lv_checkbox_is_checked(obj));
        update_namelist();
    }    
}

static void cb_mask_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        set_masked(lv_roller_get_selected(roller), lv_checkbox_is_checked(obj));
        update_namelist();
    }    
}

static void cb_vaccine_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        set_vaccine(lv_roller_get_selected(roller), lv_checkbox_is_checked(obj));
        update_namelist();
    }    
}

static uint16_t selected_element()
{
    return get_element(lv_roller_get_selected(roller));
}

static void btn_scan_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(obj == btn_scan && event == LV_EVENT_CLICKED) {
        if(m_gui_callback) { 
                m_gui_event.evt_type = GUI_EVT_SCAN;
                m_gui_callback(&m_gui_event);
        } 
    } else  if(obj == btn_id && event == LV_EVENT_CLICKED) {
        if(m_gui_callback) { 
                m_gui_event.evt_type = GUI_EVT_IDENTIFY;
                m_gui_event.element = selected_element();
                m_gui_callback(&m_gui_event);
        } 
    }   
}

static void btn_playback_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(m_gui_callback && event == LV_EVENT_CLICKED) {
        if(obj == btn_rew) {
                m_gui_event.evt_type = GUI_EVT_SIM_RESTART;
        } else if(obj == btn_play) {
                m_gui_event.evt_type = GUI_EVT_SIM_STEP;
        } else if(obj == btn_next) {
                m_gui_event.evt_type = GUI_EVT_SIM_STEP;
        } 
        m_gui_callback(&m_gui_event);
    }   
}
static void badges_create(lv_obj_t* parent)
{
    const int zoff = 30;

    roller = lv_roller_create(parent, NULL);
    lv_obj_add_style(roller, LV_CONT_PART_MAIN, &style_box);
    lv_roller_set_align(roller, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_style_local_value_str(roller, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Roller");
    lv_roller_set_auto_fit(roller, false);
    lv_roller_set_visible_row_count(roller, 6);
    lv_obj_set_width(roller, 88);
    lv_obj_set_pos(roller, 2, 5);
    lv_obj_set_event_cb(roller, roller_event_cb);

    update_namelist();

    btn_edit_name = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_edit_name, btn_edit_name_event_cb);
    lv_obj_t* label = lv_label_create(btn_edit_name, NULL);
    lv_label_set_text(label, "Edit Name");
    lv_obj_set_width(btn_edit_name, 100);
    lv_obj_set_height(btn_edit_name, 30);
    lv_obj_set_pos(btn_edit_name, 200, 40 + zoff);



    cb_virus = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb_virus, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb_virus, "Virus");
    lv_obj_set_pos(cb_virus, 105, 45 + zoff);
    lv_obj_set_event_cb(cb_virus, cb_virus_event_cb);

    cb_mask = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb_mask, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb_mask, "Mask");
    lv_obj_set_pos(cb_mask, 105, 80 + zoff);
    lv_obj_set_event_cb(cb_mask, cb_mask_event_cb);

    cb_vaccine = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb_vaccine, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb_vaccine, "Vaccine");
    lv_obj_set_pos(cb_vaccine, 105, 110 + zoff);
    lv_obj_set_event_cb(cb_vaccine, cb_vaccine_event_cb);

    btn_scan = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_scan, NULL);
    lv_label_set_text(label, "Scan");
    lv_obj_set_width(btn_scan, 75);
    lv_obj_set_pos(btn_scan, 110, 150 + zoff);
    lv_obj_set_event_cb(btn_scan, btn_scan_event_cb);

    btn_id = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_id, NULL);
    lv_label_set_text(label, "Identify");
    lv_obj_set_width(btn_id, 75);
    lv_obj_set_pos(btn_id, 210, 150 + zoff);
    lv_obj_set_event_cb(btn_id, btn_scan_event_cb);
}









static void lv_spinbox_rows_increment_event_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_rows);
    }
}

static void lv_spinbox_rows_decrement_event_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_rows);
    }
}

static void lv_spinbox_space_increment_event_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_space);
    }
}

static void lv_spinbox_space_decrement_event_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_space);
    }
}

static void lv_spinbox_rate_increment_event_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_rate);
    }
}

static void lv_spinbox_rate_decrement_event_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_rate);
    }
}









static void configure_create(lv_obj_t* parent)
{
    const int zoff = 40;
    lv_obj_t* label;
    lv_coord_t h;

    dd = lv_dropdown_create(parent, NULL);
    lv_obj_set_style_local_value_str(dd, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Dropdown");
    lv_dropdown_set_options(dd, " Simulate\n Proximity");
    lv_obj_set_pos(dd, 110, 20 + zoff);

    // rows spinner
    label_rows = lv_label_create(parent, NULL);
    lv_label_set_text(label_rows, "Rows");
    lv_obj_set_pos(label_rows, 4, 70 + zoff);

    spinbox_rows = lv_spinbox_create(parent, NULL);
    lv_spinbox_set_range(spinbox_rows, 1, 10);
    lv_spinbox_set_digit_format(spinbox_rows, 1, 0);
    lv_spinbox_set_value(spinbox_rows, 3);
//    lv_spinbox_step_prev(spinbox_rows);
    lv_obj_set_width(spinbox_rows, 50);
    lv_obj_set_pos(spinbox_rows, 150, 64 + zoff);

    h = lv_obj_get_height(spinbox_rows);
    btn_row_up = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn_row_up, h+12, h);
    lv_obj_align(btn_row_up, spinbox_rows, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_theme_apply(btn_row_up, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(btn_row_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn_row_up, lv_spinbox_rows_increment_event_cb);

    btn_row_down = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn_row_down, h+12, h);
    lv_obj_align(btn_row_down, spinbox_rows, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_theme_apply(btn_row_down, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(btn_row_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(btn_row_down, lv_spinbox_rows_decrement_event_cb);

    // space spinner
    label_space = lv_label_create(parent, NULL);
    lv_label_set_text(label_space, "Space");
    lv_obj_set_pos(label_space, 4, 110 + zoff);

    spinbox_space = lv_spinbox_create(parent, NULL);
    lv_spinbox_set_range(spinbox_space, 1, 10);
    lv_spinbox_set_digit_format(spinbox_space, 1, 0);
    lv_spinbox_set_value(spinbox_space, 4);
//    lv_spinbox_step_prev(spinbox_space);
    lv_obj_set_width(spinbox_space, 50);
    lv_obj_set_pos(spinbox_space, 150, 100 + zoff);

    h = lv_obj_get_height(spinbox_space);
    btn_space_up = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn_space_up, h+12, h);
    lv_obj_align(btn_space_up, spinbox_space, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_theme_apply(btn_space_up, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(btn_space_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn_space_up, lv_spinbox_space_increment_event_cb);

    btn_space_down = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn_space_down, h+12, h);
    lv_obj_align(btn_space_down, spinbox_space, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_theme_apply(btn_space_down, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(btn_space_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(btn_space_down, lv_spinbox_space_decrement_event_cb);


    // rate spinner
    label_rate = lv_label_create(parent, NULL);
    lv_label_set_text(label_rate, "Rate");
    lv_obj_set_pos(label_rate, 4, 150 + zoff);

    spinbox_rate = lv_spinbox_create(parent, NULL);
    lv_spinbox_set_range(spinbox_rate, 5, 50);
    lv_spinbox_set_digit_format(spinbox_rate, 2, 1);
    lv_spinbox_set_value(spinbox_rate, 12);
//    lv_spinbox_step_prev(spinbox_rate);
    lv_obj_set_width(spinbox_rate, 50);
    lv_obj_set_pos(spinbox_rate, 150, 140 + zoff);

    h = lv_obj_get_height(spinbox_rate);
    btn_rate_up = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn_rate_up, h+12, h);
    lv_obj_align(btn_rate_up, spinbox_rate, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_theme_apply(btn_rate_up, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(btn_rate_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn_rate_up, lv_spinbox_rate_increment_event_cb);

    btn_rate_down = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn_rate_down, h+12, h);
    lv_obj_align(btn_rate_down, spinbox_rate, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_theme_apply(btn_rate_down, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(btn_rate_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(btn_rate_down, lv_spinbox_rate_decrement_event_cb);





}

static void analysis_create(lv_obj_t* parent)
{
    // roller is shared and not created here

    bar = lv_bar_create(parent, NULL);
    lv_obj_set_width(bar, 150);
    lv_obj_set_style_local_value_font(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_align(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
    lv_obj_set_style_local_value_ofs_y(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_obj_set_style_local_margin_bottom(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);
    lv_obj_align(bar, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(bar, 30, LV_ANIM_OFF);
    lv_obj_set_pos(bar, 150, 150);

    btn_rew = lv_btn_create(parent, NULL);
    lv_obj_t* label = lv_label_create(btn_rew, NULL);
    lv_label_set_text(label, LV_SYMBOL_LEFT LV_SYMBOL_LEFT);
    lv_obj_set_width(btn_rew, 50);
    lv_obj_set_pos(btn_rew, 140, 100);
    lv_obj_set_event_cb(btn_rew, btn_playback_event_cb);

    btn_play = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_play, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLAY);
    lv_obj_set_width(btn_play, 50);
    lv_obj_set_pos(btn_play, 200, 100);
    lv_obj_set_event_cb(btn_play, btn_playback_event_cb);

    btn_next = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_next, NULL);
    lv_label_set_text(label, LV_SYMBOL_NEXT);
    lv_obj_set_width(btn_next, 50);
    lv_obj_set_pos(btn_next, 260, 100);
    lv_obj_set_event_cb(btn_next, btn_playback_event_cb);
}

static void keyboard_create(lv_obj_t* parent)
{

    ta = lv_textarea_create(parent, NULL);
    lv_obj_add_style(ta, LV_CONT_PART_MAIN, &style_box);

    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, "Name");
    lv_obj_set_event_cb(ta, ta_event_cb);
    lv_obj_set_width(ta, 180);
    lv_obj_set_pos(ta, 110, 60);

    if (kb == NULL) {
        lv_coord_t kb_height = LV_MATH_MIN(LV_VER_RES / 2, LV_DPI * 4 / 3);
        kb = lv_keyboard_create(lv_scr_act(), NULL);
//        lv_obj_add_style(kb, LV_KEYBOARD_PART_BTN, &style_box);
        lv_obj_set_height(kb, kb_height);
        lv_obj_align(kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_event_cb(kb, kb_event_cb);

        lv_indev_wait_release(lv_indev_get_act());
    }
//    lv_textarea_set_cursor_hidden(ta, false);
    //        lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
    lv_keyboard_set_textarea(kb, ta);

}




static void ta_event_cb(lv_obj_t* ta, lv_event_t e)
{
    if (e == LV_EVENT_RELEASED) {

        if (e == LV_EVENT_DEFOCUSED) {
            lv_textarea_set_cursor_hidden(ta, true);
        }
    }
}


static void kb_event_cb(lv_obj_t* _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if (e == LV_EVENT_CANCEL) {
        if (kb) {
            gus_mode = mode_badge;
            update_control_visibility();
        }
    }
}


//static void bar_anim(lv_task_t* t)
//{
//    static uint32_t x = 0;
//    lv_obj_t* bar = t->user_data;

//    static char buf[64];
//    lv_snprintf(buf, sizeof(buf), "Copying %d/%d", x, lv_bar_get_max_value(bar));
//    lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);

//    lv_bar_set_value(bar, x, LV_ANIM_OFF);
//    x++;
//    if (x > lv_bar_get_max_value(bar)) x = 0;
//}


static void btn_badge_event_cb(lv_obj_t* btn, lv_event_t e)
{
    gus_mode = mode_badge;
    update_control_visibility();
}

static void btn_config_event_cb(lv_obj_t* btn, lv_event_t e)
{
    gus_mode = mode_config;
    update_control_visibility();
}

static void btn_analyze_event_cb(lv_obj_t* btn, lv_event_t e)
{
    gus_mode = mode_analyze;
    update_control_visibility();
}

static void btn_edit_name_event_cb(lv_obj_t* btn, lv_event_t e)
{
    gus_mode = mode_edit_name;
    update_control_visibility();
}






//////////////////////////////////////////////////////////////////



void gui_init(gui_config_t * config)
{
    m_gui_callback = config->event_callback;	
}

void gui_update_namelist(void)
{
    static gui_message_t msg;
    msg.type = GUI_MSG_UPDATE_LIST;
    k_msgq_put(&m_gui_cmd_queue, &msg, K_NO_WAIT);
}


static void process_cmd_msg_queue(void)
{
    gui_message_t cmd_message;
    while(k_msgq_get(&m_gui_cmd_queue, &cmd_message, K_NO_WAIT) == 0){
        // Process incoming commands depending on type
        switch(cmd_message.type){
            case GUI_MSG_UPDATE_LIST:
                update_namelist();
                break;
            default:
                printk("m: %d\n",cmd_message.type);
        }
    }
}


void gui_run(void)
{
    gd_init();

    display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

    if (display_dev == NULL) {
//		LOG_ERR("Display device not found!");
            return;
    }

    lv_demo_widgets();

    display_blanking_off(display_dev);

    while(1){
        process_cmd_msg_queue();
        lv_task_handler();
        k_sleep(K_MSEC(20));
    }
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(gui_thread, 4096, gui_run, NULL, NULL, NULL, 7, 0, 0);