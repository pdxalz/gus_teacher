#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "gui.h"
#include "gus_config.h"
#include "gus_data.h"
#include "simulate.h"
#include "model_handler.h"
#include "contacts.h"

const struct device *display_dev;

////////////////////////////////////////////////////////////

/*********************
  *      DEFINITIONS
  *********************/
#define NAMELIST_LEN (MAX_GUS_BADGES * (MAX_NAME_LENGTH + 3))

/**********************
 *  STATIC VARIABLES
 **********************/
static gui_event_t m_gui_event;
static gui_callback_t m_gui_callback = 0;

// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(m_gui_cmd_queue, sizeof(gui_message_t), 8, 4);

static lv_style_t _style_box;

// mode controls
static lv_obj_t *_btn_badges;
static lv_obj_t *_btn_config;
static lv_obj_t *_btn_analysis;

// badges widgets
static lv_obj_t *_roller_names;
static lv_obj_t *_btn_edit_name;
static lv_obj_t *_cb_virus;
static lv_obj_t *_cb_mask;
static lv_obj_t *_cb_vaccine;
static lv_obj_t *_btn_scan;
static lv_obj_t *_btn_identify;

// config widgets
static lv_obj_t *_dd_sim_mode;
static lv_obj_t *_label_rows;
static lv_obj_t *_spinbox_rows;
static lv_obj_t *_btn_row_up;
static lv_obj_t *_btn_row_down;
static lv_obj_t *_spinbox_space;
static lv_obj_t *_btn_space_up;
static lv_obj_t *_btn_space_down;
static lv_obj_t *_spinbox_rate;
static lv_obj_t *_btn_rate_up;
static lv_obj_t *_btn_rate_down;

static lv_obj_t *_label_space;
static lv_obj_t *_label_rate;

// analysis widgets
// name roller is shared
static lv_obj_t *_bar_progress;
static lv_obj_t *_btn_record;
static lv_obj_t *_btn_rewind;
static lv_obj_t *_btn_play;
static lv_obj_t *_btn_next;
static lv_obj_t *_label_infections;

// keyboard widgets
static lv_obj_t *_ta_new_name;
static lv_obj_t *_keyboard;

// used to simulate a tabbed dialog.  Different controls are hidden
// in different modes
enum gus_mode
{
    mode_badge,
    mode_config,
    mode_analyze,
    mode_edit_name
} gus_mode;

/**********************
 *      MACROS
 **********************/
// defines how quickly the play operation steps through the simulation
#define PLAY_TIMER_VALUE K_SECONDS(2)
//extern void play_expiry_function(struct k_timer *timer_id);

// defines how often a report request is sent to the badges.  One request is
// sent to one badge per timer event, so if there are 10 badges it will take
// 10 times the timer value for all badges to get a report request.
// Decreasing the timer interval can cause too much traffic to occur on
// the mesh network.
#define RECORD_TIMER_VALUE K_SECONDS(5)
//extern void record_expiry_function(struct k_timer *timer_id);

static void play_expiry_function(struct k_timer *timer_id);
static void record_expiry_function(struct k_timer *timer_id);

K_TIMER_DEFINE(play_timer, play_expiry_function, NULL);
K_TIMER_DEFINE(record_timer, record_expiry_function, NULL);

/**********************
 *   STATIC FUNCTIONS
 **********************/
// returns : true if in gus tag mode, false if in classroom mode
static bool gus_tag_mode(void)
{
    //printk("tag: %d\n", lv_dropdown_get_selected(dd_sim_mode));
    return lv_dropdown_get_selected(_dd_sim_mode) == 1;
}

static uint16_t btnstate(enum gus_mode mode)
{
    return gus_mode == mode ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_RELEASED;
}

// shows or hides the controls base on the current mode.
static void update_control_visibility(void)
{
    lv_obj_set_hidden(_roller_names, gus_mode != mode_badge && gus_mode != mode_analyze);

    lv_btn_set_state(_btn_badges, btnstate(mode_badge));
    lv_btn_set_state(_btn_config, btnstate(mode_config));
    lv_btn_set_state(_btn_analysis, btnstate(mode_analyze));

    lv_obj_set_hidden(_btn_edit_name, gus_mode != mode_badge);
    lv_obj_set_hidden(_cb_virus, gus_mode != mode_badge);
    lv_obj_set_hidden(_cb_mask, gus_mode != mode_badge);
    lv_obj_set_hidden(_cb_vaccine, gus_mode != mode_badge);
    lv_obj_set_hidden(_btn_scan, gus_mode != mode_badge);
    lv_obj_set_hidden(_btn_identify, gus_mode != mode_badge);

    lv_obj_set_hidden(_dd_sim_mode, gus_mode != mode_config);

    lv_obj_set_hidden(_label_rows, gus_mode != mode_config);
    lv_obj_set_hidden(_spinbox_rows, gus_mode != mode_config);
    lv_obj_set_hidden(_btn_row_up, gus_mode != mode_config);
    lv_obj_set_hidden(_btn_row_down, gus_mode != mode_config);

    lv_obj_set_hidden(_label_space, gus_mode != mode_config);
    lv_obj_set_hidden(_spinbox_space, gus_mode != mode_config);
    lv_obj_set_hidden(_btn_space_up, gus_mode != mode_config);
    lv_obj_set_hidden(_btn_space_down, gus_mode != mode_config);

    lv_obj_set_hidden(_label_rate, gus_mode != mode_config);
    lv_obj_set_hidden(_spinbox_rate, gus_mode != mode_config);
    lv_obj_set_hidden(_btn_rate_up, gus_mode != mode_config);
    lv_obj_set_hidden(_btn_rate_down, gus_mode != mode_config);

    lv_obj_set_hidden(_label_rate, gus_mode != mode_config);

    lv_obj_set_hidden(_bar_progress, gus_mode != mode_analyze);
    lv_obj_set_hidden(_btn_record, gus_mode != mode_analyze || !gus_tag_mode());
    lv_obj_set_hidden(_btn_rewind, gus_mode != mode_analyze);
    lv_obj_set_hidden(_btn_play, gus_mode != mode_analyze);
    lv_obj_set_hidden(_btn_next, gus_mode != mode_analyze);
    lv_obj_set_hidden(_label_infections, gus_mode != mode_analyze);

    lv_obj_set_hidden(_keyboard, gus_mode != mode_edit_name);
    lv_obj_set_hidden(_ta_new_name, gus_mode != mode_edit_name);
}

// updates the state of the virus, mask and vaccine checkboxes based
// on the state of the gus data.
void update_checkboxes(void)
{
    int item = lv_roller_get_selected(_roller_names);
    if (item >= 0)
    {
        lv_checkbox_set_checked(_cb_virus, get_patient_zero(item));
        lv_checkbox_set_checked(_cb_mask, get_mask(item));
        lv_checkbox_set_checked(_cb_vaccine, get_vaccine(item));
    }
}

// reloads the namelist roller
static void update_namelist(void)
{
    char namelist[NAMELIST_LEN];

    int item = lv_roller_get_selected(_roller_names);
    get_badge_namelist(namelist, NAMELIST_LEN);
    lv_roller_set_options(_roller_names, namelist, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(_roller_names, item, LV_ANIM_OFF);
}

// returns the item selected in the name roller control
static uint16_t selected_address()
{
    return get_badge_address(lv_roller_get_selected(_roller_names));
}

static void restart_simulation(void)
{
    uint8_t rows = lv_spinbox_get_value(_spinbox_rows);
    uint8_t space = lv_spinbox_get_value(_spinbox_space);
    uint8_t _infection_rate = lv_spinbox_get_value(_spinbox_rate);
    sim_msg_restart(rows, space, _infection_rate, gus_tag_mode());
}

static void stop_playback(void)
{
    k_timer_stop(&play_timer);
}

static void stop_recording(void)
{
    lv_btn_set_state(_btn_record, LV_BTN_STATE_RELEASED);
    k_timer_stop(&record_timer);
}

/**********************
 *  timer event handlers
 **********************/

// function called when play timer expires.  Timer is restarted until
// progress reaches 100
static void play_expiry_function(struct k_timer *timer_id)
{
    if (lv_bar_get_value(_bar_progress) < 100)
    {
        sim_msg_next(gus_tag_mode());
        k_timer_start(&play_timer, PLAY_TIMER_VALUE, K_NO_WAIT);
    }
}

// function called when record timer expires.  Sends a report request message
// to the badge on the list pointed to by badge_index. It will send the message
// to the next badge the next time this function is called.  This gives each
// badge its own time slot to reply to the report request.
static void record_expiry_function(struct k_timer *timer_id)
{
    static uint16_t badge_index = 0;
    if (m_gui_callback)
    {
        badge_index = badge_index % get_badge_count();
        m_gui_event.addr = get_badge_address(badge_index++);
        m_gui_event.evt_type = GUI_EVT_RECORD;
        m_gui_callback(&m_gui_event);
        k_timer_start(&record_timer, RECORD_TIMER_VALUE, K_NO_WAIT);
    }
    //    }
}

/**********************
 *  GUI event handlers
 **********************/
static void roller_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        update_checkboxes();
    }
}

static void cb_virus_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        set_patient_zero(lv_roller_get_selected(_roller_names), lv_checkbox_is_checked(obj));
        update_namelist();
    }
}

static void cb_mask_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        set_mask(lv_roller_get_selected(_roller_names), lv_checkbox_is_checked(obj));
        update_namelist();
    }
}

static void cb_vaccine_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        set_vaccine(lv_roller_get_selected(_roller_names), lv_checkbox_is_checked(obj));
        update_namelist();
    }
}

static void btn_scan_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (obj == _btn_scan && event == LV_EVENT_CLICKED)
    {
        if (m_gui_callback)
        {
            clear_badge_list();
            m_gui_event.evt_type = GUI_EVT_SCAN;
            m_gui_callback(&m_gui_event);
        }
    }
    else if (obj == _btn_identify && event == LV_EVENT_CLICKED)
    {
        if (m_gui_callback)
        {
            m_gui_event.evt_type = GUI_EVT_IDENTIFY;
            m_gui_event.addr = selected_address();
            m_gui_callback(&m_gui_event);
            printk("%d\n", m_gui_event.addr);
        }
    }
}

static void btn_playback_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (m_gui_callback && event == LV_EVENT_CLICKED)
    {
        if (obj == _btn_rewind)
        {
            stop_playback();
            stop_recording();
            restart_simulation();
        }
        else if (obj == _btn_play)
        {
            stop_recording();
            sim_msg_next(gus_tag_mode());
            k_timer_start(&play_timer, PLAY_TIMER_VALUE, K_NO_WAIT);
        }
        else if (obj == _btn_next)
        {
            stop_playback();
            stop_recording();
            sim_msg_next(gus_tag_mode());
        }
    }
    else if (event == LV_EVENT_VALUE_CHANGED)
    {
        if (obj == _btn_record)
        {
            stop_playback();
            if (lv_btn_get_state(_btn_record) == LV_BTN_STATE_CHECKED_RELEASED)
            {
                reset_proximity_contacts();
                k_timer_start(&record_timer, RECORD_TIMER_VALUE, K_NO_WAIT);
            }
            else
            {
                k_timer_stop(&record_timer);
            }
        }
    }
}

static void lv_spinbox_rows_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_increment(_spinbox_rows);
    }
}

static void lv_spinbox_rows_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_decrement(_spinbox_rows);
    }
}

static void lv_spinbox_space_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_increment(_spinbox_space);
    }
}

static void lv_spinbox_space_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_decrement(_spinbox_space);
    }
}

static void lv_spinbox_rate_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_increment(_spinbox_rate);
    }
}

static void lv_spinbox_rate_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_decrement(_spinbox_rate);
    }
}

static void ta_event_cb(lv_obj_t *_ta_new_name, lv_event_t e)
{
    if (e == LV_EVENT_RELEASED)
    {

        if (e == LV_EVENT_DEFOCUSED)
        {
            lv_textarea_set_cursor_hidden(_ta_new_name, true);
        }
    }
}

static void kb_event_cb(lv_obj_t *_kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(_keyboard, e);

    if (e == LV_EVENT_CANCEL)
    {
        if (_keyboard)
        {
            gus_mode = mode_badge;
            update_control_visibility();
        }
    }
    else if (e == LV_EVENT_APPLY)
    {
        if (_keyboard)
        {
            const char *name = lv_textarea_get_text(_ta_new_name);
            printk("name: %s\n", name);
            int item = lv_roller_get_selected(_roller_names);
            set_badge_name(item, name);
            model_set_name(get_badge_address(item), name);
            update_namelist();

            gus_mode = mode_badge;
            update_control_visibility();
        }
    }
}

static void btn_badge_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED)
    {
        stop_playback();
        stop_recording();
        gus_mode = mode_badge;
        model_handler_set_state(0, BT_MESH_GUS_OFF);
        update_control_visibility();
    }
}

static void btn_config_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED)
    {
        stop_playback();
        stop_recording();
        gus_mode = mode_config;
        model_handler_set_state(0, BT_MESH_GUS_OFF);

        update_control_visibility();
    }
}

static void btn_analyze_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED)
    {
        gus_mode = mode_analyze;
        stop_playback();
        restart_simulation();
        update_control_visibility();
    }
}

static void btn_edit_name_event_cb(lv_obj_t *btn, lv_event_t e)
{
    gus_mode = mode_edit_name;
    update_control_visibility();
}

/*************************
 *  Control initialization
 *************************/

// initialize all controls on the badges "page"
static void badges_create(lv_obj_t *parent)
{
    const int zoff = 30;

    _roller_names = lv_roller_create(parent, NULL);
    lv_obj_add_style(_roller_names, LV_CONT_PART_MAIN, &_style_box);
    lv_roller_set_align(_roller_names, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_style_local_value_str(_roller_names, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Roller");
    lv_roller_set_auto_fit(_roller_names, false);
    lv_roller_set_visible_row_count(_roller_names, 7);
    lv_obj_set_width(_roller_names, 88);
    lv_obj_set_height(_roller_names, 230);
    lv_obj_set_pos(_roller_names, 2, 5);
    lv_obj_set_event_cb(_roller_names, roller_event_cb);

    update_namelist();

    _btn_edit_name = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(_btn_edit_name, btn_edit_name_event_cb);
    lv_obj_t *label = lv_label_create(_btn_edit_name, NULL);
    lv_label_set_text(label, "Edit Name");
    lv_obj_set_width(_btn_edit_name, 100);
    lv_obj_set_height(_btn_edit_name, 30);
    lv_obj_set_pos(_btn_edit_name, 200, 40 + zoff);

    _cb_virus = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(_cb_virus, LV_CONT_PART_MAIN, &_style_box);
    lv_checkbox_set_text(_cb_virus, "Virus");
    lv_obj_set_pos(_cb_virus, 105, 46 + zoff);
    lv_obj_set_event_cb(_cb_virus, cb_virus_event_cb);

    _cb_mask = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(_cb_mask, LV_CONT_PART_MAIN, &_style_box);
    lv_checkbox_set_text(_cb_mask, "Mask");
    lv_obj_set_pos(_cb_mask, 105, 80 + zoff);
    lv_obj_set_event_cb(_cb_mask, cb_mask_event_cb);

    _cb_vaccine = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(_cb_vaccine, LV_CONT_PART_MAIN, &_style_box);
    lv_checkbox_set_text(_cb_vaccine, "Vaccine");
    lv_obj_set_pos(_cb_vaccine, 105, 110 + zoff);
    lv_obj_set_event_cb(_cb_vaccine, cb_vaccine_event_cb);

    _btn_scan = lv_btn_create(parent, NULL);
    label = lv_label_create(_btn_scan, NULL);
    lv_label_set_text(label, "Scan");
    lv_obj_set_width(_btn_scan, 75);
    lv_obj_set_pos(_btn_scan, 110, 150 + zoff);
    lv_obj_set_event_cb(_btn_scan, btn_scan_event_cb);

    _btn_identify = lv_btn_create(parent, NULL);
    label = lv_label_create(_btn_identify, NULL);
    lv_label_set_text(label, "Identify");
    lv_obj_set_width(_btn_identify, 75);
    lv_obj_set_pos(_btn_identify, 210, 150 + zoff);
    lv_obj_set_event_cb(_btn_identify, btn_scan_event_cb);
}

// initialize all controls on the config "page"
static void configure_create(lv_obj_t *parent)
{
    const int zoff = 40;
    lv_obj_t *label;
    lv_coord_t h;

    _dd_sim_mode = lv_dropdown_create(parent, NULL);
    lv_obj_set_style_local_value_str(_dd_sim_mode, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Dropdown");
    lv_dropdown_set_options(_dd_sim_mode, " Classroom\n GUS Tag");
    lv_obj_set_pos(_dd_sim_mode, 110, 20 + zoff);

    // rows spinner
    _label_rows = lv_label_create(parent, NULL);
    lv_label_set_text(_label_rows, "Rows");
    lv_obj_set_pos(_label_rows, 4, 70 + zoff);

    _spinbox_rows = lv_spinbox_create(parent, NULL);
    lv_textarea_set_cursor_blink_time(_spinbox_rows, 0);
    lv_spinbox_set_range(_spinbox_rows, 1, 10);
    lv_spinbox_set_digit_format(_spinbox_rows, 1, 0);
    lv_spinbox_set_value(_spinbox_rows, 1);
    lv_obj_set_width(_spinbox_rows, 50);
    lv_obj_set_pos(_spinbox_rows, 150, 64 + zoff);

    h = lv_obj_get_height(_spinbox_rows);
    _btn_row_up = lv_btn_create(parent, NULL);
    lv_obj_set_size(_btn_row_up, h + 12, h);
    lv_obj_align(_btn_row_up, _spinbox_rows, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_theme_apply(_btn_row_up, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(_btn_row_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(_btn_row_up, lv_spinbox_rows_increment_event_cb);

    _btn_row_down = lv_btn_create(parent, NULL);
    lv_obj_set_size(_btn_row_down, h + 12, h);
    lv_obj_align(_btn_row_down, _spinbox_rows, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_theme_apply(_btn_row_down, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(_btn_row_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(_btn_row_down, lv_spinbox_rows_decrement_event_cb);

    // space spinner
    _label_space = lv_label_create(parent, NULL);
    lv_label_set_text(_label_space, "Space");
    lv_obj_set_pos(_label_space, 4, 110 + zoff);

    _spinbox_space = lv_spinbox_create(parent, NULL);
    lv_textarea_set_cursor_blink_time(_spinbox_space, 0);
    lv_spinbox_set_range(_spinbox_space, 1, 10);
    lv_spinbox_set_digit_format(_spinbox_space, 1, 0);
    lv_spinbox_set_value(_spinbox_space, 4);
    lv_obj_set_width(_spinbox_space, 50);
    lv_obj_set_pos(_spinbox_space, 150, 100 + zoff);

    h = lv_obj_get_height(_spinbox_space);
    _btn_space_up = lv_btn_create(parent, NULL);
    lv_obj_set_size(_btn_space_up, h + 12, h);
    lv_obj_align(_btn_space_up, _spinbox_space, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_theme_apply(_btn_space_up, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(_btn_space_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(_btn_space_up, lv_spinbox_space_increment_event_cb);

    _btn_space_down = lv_btn_create(parent, NULL);
    lv_obj_set_size(_btn_space_down, h + 12, h);
    lv_obj_align(_btn_space_down, _spinbox_space, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_theme_apply(_btn_space_down, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(_btn_space_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(_btn_space_down, lv_spinbox_space_decrement_event_cb);

    // rate spinner
    _label_rate = lv_label_create(parent, NULL);
    lv_label_set_text(_label_rate, "Rate");
    lv_obj_set_pos(_label_rate, 4, 150 + zoff);

    _spinbox_rate = lv_spinbox_create(parent, NULL);
    lv_textarea_set_cursor_blink_time(_spinbox_rate, 0);
    lv_spinbox_set_range(_spinbox_rate, 5, 50);
    lv_spinbox_set_digit_format(_spinbox_rate, 2, 1);
    lv_spinbox_set_value(_spinbox_rate, 12);
    lv_obj_set_width(_spinbox_rate, 50);
    lv_obj_set_pos(_spinbox_rate, 150, 140 + zoff);

    h = lv_obj_get_height(_spinbox_rate);
    _btn_rate_up = lv_btn_create(parent, NULL);
    lv_obj_set_size(_btn_rate_up, h + 12, h);
    lv_obj_align(_btn_rate_up, _spinbox_rate, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_theme_apply(_btn_rate_up, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(_btn_rate_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(_btn_rate_up, lv_spinbox_rate_increment_event_cb);

    _btn_rate_down = lv_btn_create(parent, NULL);
    lv_obj_set_size(_btn_rate_down, h + 12, h);
    lv_obj_align(_btn_rate_down, _spinbox_rate, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_theme_apply(_btn_rate_down, LV_THEME_SPINBOX_BTN);
    label = lv_label_create(_btn_rate_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(_btn_rate_down, lv_spinbox_rate_decrement_event_cb);
}

// initialize all controls on the analysis "page"
static void analysis_create(lv_obj_t *parent)
{
    // roller is shared and not created here

    _bar_progress = lv_bar_create(parent, NULL);
    lv_obj_set_width(_bar_progress, 150);
    lv_obj_set_style_local_value_font(_bar_progress, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_align(_bar_progress, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
    lv_obj_set_style_local_value_ofs_y(_bar_progress, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_obj_set_style_local_margin_bottom(_bar_progress, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);

    lv_obj_set_style_local_bg_color(_bar_progress, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x40, 0x40, 0x40));
    lv_obj_align(_bar_progress, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(_bar_progress, 100, LV_ANIM_OFF);
    lv_obj_set_pos(_bar_progress, 135, 180);

    _btn_record = lv_btn_create(parent, NULL);
    lv_btn_set_checkable(_btn_record, true);
    lv_obj_t *label = lv_label_create(_btn_record, NULL);
    lv_label_set_text(label, "Record");
    lv_obj_set_width(_btn_record, 120);
    lv_obj_set_pos(_btn_record, 155, 60);
    lv_obj_set_event_cb(_btn_record, btn_playback_event_cb);

    _btn_rewind = lv_btn_create(parent, NULL);
    label = lv_label_create(_btn_rewind, NULL);
    lv_label_set_text(label, LV_SYMBOL_LEFT LV_SYMBOL_LEFT);
    lv_obj_set_width(_btn_rewind, 50);
    lv_obj_set_pos(_btn_rewind, 125, 120);
    lv_obj_set_event_cb(_btn_rewind, btn_playback_event_cb);

    _btn_play = lv_btn_create(parent, NULL);
    label = lv_label_create(_btn_play, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLAY);
    lv_obj_set_width(_btn_play, 50);
    lv_obj_set_pos(_btn_play, 185, 120);
    lv_obj_set_event_cb(_btn_play, btn_playback_event_cb);

    _btn_next = lv_btn_create(parent, NULL);
    label = lv_label_create(_btn_next, NULL);
    lv_label_set_text(label, LV_SYMBOL_NEXT);
    lv_obj_set_width(_btn_next, 50);
    lv_obj_set_pos(_btn_next, 245, 120);
    lv_obj_set_event_cb(_btn_next, btn_playback_event_cb);

    _label_infections = lv_label_create(parent, NULL);
    lv_label_set_text(_label_infections, "0");
    lv_obj_set_pos(_label_infections, 170, 210);
}

// initialize the keyboard control
static void keyboard_create(lv_obj_t *parent)
{

    _ta_new_name = lv_textarea_create(parent, NULL);
    lv_textarea_set_text_align(_ta_new_name, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style(_ta_new_name, LV_CONT_PART_MAIN, &_style_box);

    lv_textarea_set_one_line(_ta_new_name, true);
    lv_textarea_set_text(_ta_new_name, "");
    lv_textarea_set_placeholder_text(_ta_new_name, "Name");
    lv_obj_set_event_cb(_ta_new_name, ta_event_cb);
    lv_obj_set_width(_ta_new_name, 180);
    lv_obj_set_height(_ta_new_name, 30);
    lv_obj_set_pos(_ta_new_name, 70, 60);

    if (_keyboard == NULL)
    {
        lv_coord_t kb_height = LV_MATH_MIN(LV_VER_RES / 2, LV_DPI * 4 / 3);
        _keyboard = lv_keyboard_create(lv_scr_act(), NULL);
        //        lv_obj_add_style(kb, LV_KEYBOARD_PART_BTN, &style_box);
        lv_obj_set_height(_keyboard, kb_height);
        lv_obj_align(_keyboard, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_event_cb(_keyboard, kb_event_cb);

        lv_indev_wait_release(lv_indev_get_act());
    }
    //    lv_textarea_set_cursor_hidden(ta, false);
    //        lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
    lv_keyboard_set_textarea(_keyboard, _ta_new_name);
}

// initialize the mode buttons which simulate a tabbed dialog
static void mode_buttons(lv_obj_t *parent)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, LV_STATE_DEFAULT, 5);

    _btn_badges = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(_btn_badges, btn_badge_event_cb);
    lv_obj_t *label = lv_label_create(_btn_badges, NULL);
    lv_label_set_text(label, "Badges");
    lv_obj_set_width(_btn_badges, 75);
    lv_obj_set_pos(_btn_badges, 92, 5);
    lv_obj_add_style(_btn_badges, LV_OBJ_PART_MAIN, &style);

    _btn_config = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(_btn_config, btn_config_event_cb);
    label = lv_label_create(_btn_config, NULL);
    lv_label_set_text(label, "Config");
    lv_obj_set_width(_btn_config, 75);
    lv_obj_set_pos(_btn_config, 167, 5);
    lv_obj_add_style(_btn_config, LV_OBJ_PART_MAIN, &style);

    _btn_analysis = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(_btn_analysis, btn_analyze_event_cb);
    label = lv_label_create(_btn_analysis, NULL);
    lv_label_set_text(label, "Analysis");
    lv_obj_set_width(_btn_analysis, 75);
    lv_obj_set_pos(_btn_analysis, 242, 5);
    lv_obj_add_style(_btn_analysis, LV_OBJ_PART_MAIN, &style);
}

// Initialize all widgets in the user interface
static void init_gus_widgets(void)
{
    lv_style_init(&_style_box);
    lv_style_set_value_align(&_style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&_style_box, LV_STATE_DEFAULT, -LV_DPX(25));
    lv_style_set_margin_top(&_style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_top(&_style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_bottom(&_style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_left(&_style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_right(&_style_box, LV_STATE_DEFAULT, LV_DPX(1));

    mode_buttons(lv_scr_act());

    badges_create(lv_scr_act());
    configure_create(lv_scr_act());
    analysis_create(lv_scr_act());
    keyboard_create(lv_scr_act());

    gus_mode = mode_badge;
    update_control_visibility();
}

//////////////////////////////////////////////////////////////////

void gui_init(gui_config_t *config)
{
    m_gui_callback = config->event_callback;
}

void gui_update_namelist(void)
{
    static gui_message_t msg;
    msg.type = GUI_MSG_UPDATE_LIST;
    k_msgq_put(&m_gui_cmd_queue, &msg, K_NO_WAIT);
}

void gui_update_progress(uint8_t progress, uint16_t infections)
{
    static gui_message_t msg;
    msg.type = GUI_MSG_PROGRESS;
    msg.params.progress = progress;
    msg.params.param = infections;

    k_msgq_put(&m_gui_cmd_queue, &msg, K_NO_WAIT);
}

static void process_cmd_msg_queue(void)
{
    char buf[20];
    gui_message_t cmd_message;
    while (k_msgq_get(&m_gui_cmd_queue, &cmd_message, K_NO_WAIT) == 0)
    {
        // Process incoming commands depending on type
        switch (cmd_message.type)
        {
        case GUI_MSG_UPDATE_LIST:
            update_namelist();
            break;

        case GUI_MSG_PROGRESS:
            lv_bar_set_value(_bar_progress, cmd_message.params.progress, LV_ANIM_OFF);
            sprintf(buf, "infections: %d", cmd_message.params.param);
            lv_label_set_text(_label_infections, buf);
            break;

        default:
            printk("m: %d\n", cmd_message.type);
        }
    }
}

uint16_t gui_get_selected_addr(void)
{
    int item = lv_roller_get_selected(_roller_names);
    return get_badge_address(item);
}

/**********************
  *   GLOBAL FUNCTIONS
  **********************/
void gui_run(void)
{
    init_badge_data();

    display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

    if (display_dev == NULL)
    {
        //		LOG_ERR("Display device not found!");
        return;
    }

    init_gus_widgets();

    display_blanking_off(display_dev);

    while (1)
    {
        process_cmd_msg_queue();
        lv_task_handler();
        k_sleep(K_MSEC(20));
    }
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(gui_thread, 4096, gui_run, NULL, NULL, NULL, 7, 0, 0);