#include "gui.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>

const struct device *display_dev;

#define NO_TAB_VIEW
#ifdef NO_TAB_VIEW
////////////////////////////////////////////////////////////

 /*********************
  *      DEFINES
  *********************/

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
static void bar_anim(lv_task_t* t);

static void btn_badge_event_cb(lv_obj_t* btn, lv_event_t e);
static void btn_config_event_cb(lv_obj_t* btn, lv_event_t e);
static void btn_analyze_event_cb(lv_obj_t* btn, lv_event_t e);
static void btn_edit_name_event_cb(lv_obj_t* btn, lv_event_t e);
static void keyboard_create(lv_obj_t* parent);

/**********************
 *  STATIC VARIABLES
 **********************/
 


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
static lv_obj_t* btn_remove;

// config widgets
static lv_obj_t* dd;
static lv_obj_t* label_rows;
static lv_obj_t* slider_rows;
static lv_obj_t* label_space;
static lv_obj_t* slider_space;
static lv_obj_t* label_rate;
static lv_obj_t* slider_rate;

// analysis widgets
// roller is shared
static lv_obj_t* bar;
static lv_obj_t* btn_prev;
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
static void update_control_visibility(void)
{
    lv_obj_set_hidden(roller, gus_mode != mode_badge && gus_mode != mode_analyze);

    lv_obj_set_hidden(btn_edit_name, gus_mode != mode_badge);
    lv_obj_set_hidden(cb_virus, gus_mode != mode_badge);
    lv_obj_set_hidden(cb_mask, gus_mode != mode_badge);
    lv_obj_set_hidden(cb_vaccine, gus_mode != mode_badge);
    lv_obj_set_hidden(btn_scan, gus_mode != mode_badge);
    lv_obj_set_hidden(btn_remove, gus_mode != mode_badge);

    lv_obj_set_hidden(dd, gus_mode != mode_config);
    lv_obj_set_hidden(label_rows, gus_mode != mode_config);
    lv_obj_set_hidden(slider_rows, gus_mode != mode_config);
    lv_obj_set_hidden(label_space, gus_mode != mode_config);
    lv_obj_set_hidden(slider_space, gus_mode != mode_config);
    lv_obj_set_hidden(label_rate, gus_mode != mode_config);
    lv_obj_set_hidden(slider_rate, gus_mode != mode_config);

    lv_obj_set_hidden(bar, gus_mode != mode_analyze);
    lv_obj_set_hidden(btn_prev, gus_mode != mode_analyze);
    lv_obj_set_hidden(btn_play, gus_mode != mode_analyze);
    lv_obj_set_hidden(btn_next, gus_mode != mode_analyze);

    lv_obj_set_hidden(kb, gus_mode != mode_edit_name);
    lv_obj_set_hidden(ta, gus_mode != mode_edit_name);

}

static void mode_buttons(lv_obj_t* parent)
{
    btn_badges = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_badges, btn_badge_event_cb);
    lv_obj_t* label = lv_label_create(btn_badges, NULL);
    lv_label_set_text(label, "Badges");
    lv_obj_set_width(btn_badges, 75);
    lv_obj_set_pos(btn_badges, 90, 5);

    btn_config = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_config, btn_config_event_cb);
    label = lv_label_create(btn_config, NULL);
    lv_label_set_text(label, "Config");
    lv_obj_set_width(btn_config, 75);
    lv_obj_set_pos(btn_config, 165, 5);

    btn_analysis = lv_btn_create(parent, NULL);
    lv_obj_set_event_cb(btn_analysis, btn_analyze_event_cb);
    label = lv_label_create(btn_analysis, NULL);
    lv_label_set_text(label, "Analysis");
    lv_obj_set_width(btn_analysis, 75);
    lv_obj_set_pos(btn_analysis, 240, 5);


}

static void badges_create(lv_obj_t* parent)
{
    const int zoff = 30;

    roller = lv_roller_create(parent, NULL);
    lv_obj_add_style(roller, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(roller, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Roller");
    lv_roller_set_auto_fit(roller, false);
    lv_roller_set_visible_row_count(roller, 5);
    lv_obj_set_width(roller, 80);
    lv_obj_set_pos(roller, 10, 5);

    lv_roller_set_options(roller, "Alpha\nBravo\nCharlie\nDelta\nEcho\nZulu", LV_ROLLER_MODE_NORMAL);

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

    cb_mask = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb_mask, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb_mask, "Mask");
    lv_obj_set_pos(cb_mask, 105, 80 + zoff);

    cb_vaccine = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb_vaccine, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb_vaccine, "Vaccine");
    lv_obj_set_pos(cb_vaccine, 105, 110 + zoff);

    btn_scan = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_scan, NULL);
    lv_label_set_text(label, "Scan");
    lv_obj_set_width(btn_scan, 75);
    lv_obj_set_pos(btn_scan, 110, 150 + zoff);

    btn_remove = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_remove, NULL);
    lv_label_set_text(label, "Remove");
    lv_obj_set_width(btn_remove, 75);
    lv_obj_set_pos(btn_remove, 210, 150 + zoff);
}




static void configure_create(lv_obj_t* parent)
{
    const int zoff = 40;
    dd = lv_dropdown_create(parent, NULL);
    lv_obj_add_style(dd, LV_CONT_PART_MAIN, &style_box);
    //    lv_obj_set_style_local_value_str(dd, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Dropdown");
    lv_obj_set_width(dd, 200);
    lv_dropdown_set_options(dd, "Proximity\nSimulate");
    lv_obj_set_pos(dd, 4, 35 + zoff);

    label_rows = lv_label_create(parent, NULL);
    lv_label_set_text(label_rows, "Rows");
    lv_obj_set_pos(label_rows, 4, 70 + zoff);

    slider_rows = lv_slider_create(parent, NULL);
    lv_slider_set_value(slider_rows, 40, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider_rows, slider_event_cb);
    lv_obj_set_pos(slider_rows, 55, 75 + zoff);


    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(slider_rows, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider_rows, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    label_space = lv_label_create(parent, NULL);
    lv_label_set_text(label_space, "Space");
    lv_obj_set_pos(label_space, 4, 110 + zoff);

    slider_space = lv_slider_create(parent, NULL);
    lv_slider_set_value(slider_space, 40, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider_space, slider_event_cb);
    lv_obj_set_pos(slider_space, 55, 115 + zoff);


    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(slider_space, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider_space, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    label_rate = lv_label_create(parent, NULL);
    lv_label_set_text(label_rate, "Rate");
    lv_obj_set_pos(label_rate, 4, 150 + zoff);

    slider_rate = lv_slider_create(parent, NULL);
    lv_slider_set_value(slider_rate, 40, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider_rate, slider_event_cb);
    lv_obj_set_pos(slider_rate, 55, 155 + zoff);


    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(slider_rate, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider_rate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);



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

    btn_prev = lv_btn_create(parent, NULL);
    lv_obj_t* label = lv_label_create(btn_prev, NULL);
    lv_label_set_text(label, LV_SYMBOL_PREV);
    lv_obj_set_width(btn_prev, 50);
    lv_obj_set_pos(btn_prev, 140, 100);

    btn_play = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_play, NULL);
    lv_label_set_text(label, LV_SYMBOL_PLAY);
    lv_obj_set_width(btn_play, 50);
    lv_obj_set_pos(btn_play, 200, 100);

    btn_next = lv_btn_create(parent, NULL);
    label = lv_label_create(btn_next, NULL);
    lv_label_set_text(label, LV_SYMBOL_NEXT);
    lv_obj_set_width(btn_next, 50);
    lv_obj_set_pos(btn_next, 260, 100);
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


static void slider_event_cb(lv_obj_t* slider, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED) {
        if (lv_slider_get_type(slider) == LV_SLIDER_TYPE_NORMAL) {
            static char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
        }
        else {
            static char buf[32];
            lv_snprintf(buf, sizeof(buf), "%d-%d", lv_slider_get_left_value(slider), lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, buf);
        }

    }

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


static void bar_anim(lv_task_t* t)
{
    static uint32_t x = 0;
    lv_obj_t* bar = t->user_data;

    static char buf[64];
    lv_snprintf(buf, sizeof(buf), "Copying %d/%d", x, lv_bar_get_max_value(bar));
    lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);

    lv_bar_set_value(bar, x, LV_ANIM_OFF);
    x++;
    if (x > lv_bar_get_max_value(bar)) x = 0;
}


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







#else //NO_TAB_VIEW
////////////////////////////////////////////////////////////
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void badges_create(lv_obj_t * parent);
static void configure_create(lv_obj_t * parent);
static void analysis_create(lv_obj_t * parent);
static void slider_event_cb(lv_obj_t * slider, lv_event_t e);
static void ta_event_cb(lv_obj_t * ta, lv_event_t e);
static void kb_event_cb(lv_obj_t * ta, lv_event_t e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * t2;
static lv_obj_t * t3;
static lv_obj_t * kb;

static lv_style_t style_box;
static lv_style_t style_tab;





void lv_demo_widgets(void)
{
    tv = lv_tabview_create(lv_scr_act(), NULL);

    lv_style_init(&style_tab);

    lv_style_set_pad_top(&style_tab, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_bottom(&style_tab, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_pad_left(&style_tab, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_right(&style_tab, LV_STATE_DEFAULT, LV_DPX(1));
//    lv_style_set_text_font(&style_tab, LV_STATE_DEFAULT, &lv_font_montserrat_14);
    lv_obj_reset_style_list(tv, LV_TABVIEW_PART_TAB_BTN); /*Remove the styles,coming from the theme*/
    lv_obj_add_style(tv, LV_TABVIEW_PART_TAB_BTN, &style_tab);

    t1 = lv_tabview_add_tab(tv, "Badges");
    t2 = lv_tabview_add_tab(tv, "Configure");
    t3 = lv_tabview_add_tab(tv, "Analysis");


    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, - LV_DPX(25));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_top(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_bottom(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_left(&style_box, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_pad_right(&style_box, LV_STATE_DEFAULT, LV_DPX(1));

//    lv_style_set_text_font(&style_box, LV_STATE_DEFAULT, &lv_font_montserrat_14);

    badges_create(t1);
    configure_create(t2);
//    analysis_create(t3);



}

static void badges_create(lv_obj_t * parent)
{
    lv_obj_t* roller = lv_roller_create(parent, NULL);
    lv_obj_add_style(roller, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(roller, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Roller");
    lv_roller_set_auto_fit(roller, false);
    lv_roller_set_visible_row_count(roller, 5);
    lv_obj_set_width(roller, 80);
    lv_obj_set_pos(roller, 10, 5);

    lv_roller_set_options(roller, "Alpha\nBravo\nCharlie\nDelta\nEcho\nZulu", LV_ROLLER_MODE_NORMAL);


    lv_obj_t * ta = lv_textarea_create(parent, NULL);
    lv_obj_add_style(ta, LV_CONT_PART_MAIN, &style_box);

    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, "Name");
    lv_obj_set_event_cb(ta, ta_event_cb);
    lv_obj_set_width(ta, 180);
    lv_obj_set_pos(ta, 105, 10);


    lv_obj_t* cb = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb, "Virus");
    lv_obj_set_pos(cb, 105, 40);

    cb = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb, "Mask");
    lv_obj_set_pos(cb, 105, 70);

    cb = lv_checkbox_create(parent, NULL);
    lv_obj_add_style(cb, LV_CONT_PART_MAIN, &style_box);
    lv_checkbox_set_text(cb, "Vaccine");
    lv_obj_set_pos(cb, 105, 100);

    lv_obj_t* btn = lv_btn_create(parent, NULL);
    lv_obj_t* label = lv_label_create(btn, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "Scan");
    lv_obj_set_width(btn, 75);
    lv_obj_set_pos(btn, 110, 150);

    btn = lv_btn_create(parent, NULL);
    label = lv_label_create(btn, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "Remove");
    lv_obj_set_width(btn, 75);
    lv_obj_set_pos(btn, 210, 150);
}

static void configure_create(lv_obj_t * parent)
{
    lv_obj_t* dd = lv_dropdown_create(parent, NULL);
    lv_obj_add_style(dd, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(dd, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Dropdown");
    lv_obj_set_width(dd, 200);
    lv_dropdown_set_options(dd, "Proximity\nSimulate");
    lv_obj_set_pos(dd, 4, 5);

    lv_obj_t* label = lv_label_create(parent, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "Rows");
    lv_obj_set_pos(label, 4, 40);

    lv_obj_t* slider = lv_slider_create(parent, NULL);
    lv_slider_set_value(slider, 40, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider, slider_event_cb);
    lv_obj_set_pos(slider, 55, 45);


    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    label = lv_label_create(parent, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "Space");
    lv_obj_set_pos(label, 4, 90);

    slider = lv_slider_create(parent, NULL);
    lv_slider_set_value(slider, 40, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider, slider_event_cb);
    lv_obj_set_pos(slider, 55, 95);


    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    label = lv_label_create(parent, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "Rate");
    lv_obj_set_pos(label, 4, 140);

    slider = lv_slider_create(parent, NULL);
    lv_slider_set_value(slider, 40, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider, slider_event_cb);
    lv_obj_set_pos(slider, 55, 145);


    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);
}

static void analysis_create(lv_obj_t * parent)
{

    lv_obj_t* btn;
    lv_obj_t* list = lv_list_create(parent, NULL);
    lv_obj_add_style(list, LV_CONT_PART_MAIN, &style_box);
    lv_list_set_scroll_propagation(list, true);
    lv_obj_set_size(list, 120, 190);
    lv_obj_set_pos(list, 5, 5);

    const char* txts[] = { LV_SYMBOL_SAVE, "Save", LV_SYMBOL_CUT, "Cut", LV_SYMBOL_COPY, "Copy",
            LV_SYMBOL_OK, "This", LV_SYMBOL_EDIT, "Edit", LV_SYMBOL_WIFI, "Wifi",
            LV_SYMBOL_BLUETOOTH, "Bluetooth",  LV_SYMBOL_GPS, "GPS", LV_SYMBOL_USB, "USB",
            LV_SYMBOL_SD_CARD, "SD card", LV_SYMBOL_CLOSE, "Close", NULL };

    uint32_t i;
    for (i = 0; txts[i] != NULL; i += 2) {
        btn = lv_list_add_btn(list, txts[i], txts[i + 1]);
    }

    lv_obj_t* bar = lv_bar_create(parent, NULL);
    lv_obj_set_width(bar, 150);
    lv_obj_set_style_local_value_font(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_align(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
    lv_obj_set_style_local_value_ofs_y(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_obj_set_style_local_margin_bottom(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);
    lv_obj_align(bar, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(bar, 30, LV_ANIM_OFF);
    lv_obj_set_pos(bar, 150, 150);

    btn = lv_btn_create(parent, NULL);
    lv_obj_add_style(btn, LV_CONT_PART_MAIN, &style_box);
    lv_obj_t* label = lv_label_create(btn, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "RESET");
    //lv_label_set_text(label, LV_SYMBOL_PREV);
    lv_obj_set_width(btn, 50);
    lv_obj_set_pos(btn, 140, 100);

    btn = lv_btn_create(parent, NULL);
    lv_obj_add_style(btn, LV_CONT_PART_MAIN, &style_box);
    label = lv_label_create(btn, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "PLAY");
    //lv_label_set_text(label, LV_SYMBOL_PLAY);
    lv_obj_set_width(btn, 50);
    lv_obj_set_pos(btn, 200, 100);

    btn = lv_btn_create(parent, NULL);
    lv_obj_add_style(btn, LV_CONT_PART_MAIN, &style_box);
    label = lv_label_create(btn, NULL);
    lv_obj_add_style(label, LV_CONT_PART_MAIN, &style_box);
    lv_label_set_text(label, "NEXT");
//    lv_label_set_text(label, LV_SYMBOL_NEXT);
    lv_obj_set_width(btn, 50);
    lv_obj_set_pos(btn, 260, 100);
}




static void slider_event_cb(lv_obj_t * slider, lv_event_t e)
{
    if(e == LV_EVENT_VALUE_CHANGED) {
        if(lv_slider_get_type(slider) == LV_SLIDER_TYPE_NORMAL) {
            static char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
        } else {
            static char buf[32];
            lv_snprintf(buf, sizeof(buf), "%d-%d", lv_slider_get_left_value(slider), lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, buf);
        }

    }
}



static void ta_event_cb(lv_obj_t * ta, lv_event_t e)
{
    if(e == LV_EVENT_RELEASED) {
        if(kb == NULL) {
            lv_coord_t kb_height = LV_MATH_MIN(LV_VER_RES / 2, LV_DPI * 4 / 3);
            lv_obj_set_height(tv, LV_VER_RES - kb_height);
            kb = lv_keyboard_create(lv_scr_act(), NULL);
    lv_obj_add_style(kb, LV_CONT_PART_MAIN, &style_box);
            lv_obj_set_height(kb, kb_height);
            lv_obj_align(kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
            lv_obj_set_event_cb(kb, kb_event_cb);

            lv_indev_wait_release(lv_indev_get_act());
        }
        lv_textarea_set_cursor_hidden(ta, false);
        lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
        lv_keyboard_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

static void kb_event_cb(lv_obj_t * _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if(e == LV_EVENT_CANCEL) {
        if(kb) {
            lv_obj_set_height(tv, LV_VER_RES);
            lv_obj_del(kb);
            kb = NULL;
        }
    }
}

#endif //NO_TAB_VIEW

//////////////////////////////////////////////////////////////////



void gui_init(gui_config_t * config)
{
	
}

void gui_set_bt_state(gui_bt_state_t state)
{

}

void gui_set_bt_led_state(bool led_is_on)
{
	
}


void gui_run(void)
{
	display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

	if (display_dev == NULL) {
//		LOG_ERR("Display device not found!");
		return;
	}

        lv_demo_widgets();

	display_blanking_off(display_dev);

	while(1){
//		process_cmd_msg_queue();
		lv_task_handler();
		k_sleep(K_MSEC(20));
	}
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(gui_thread, 4096, gui_run, NULL, NULL, NULL, 7, 0, 0);