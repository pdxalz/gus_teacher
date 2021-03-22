#include "gui.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(gui);

//uint32_t count = 0U;
//char count_str[11] = {0};
const struct device *display_dev;

//static gui_event_t m_gui_event;
//static gui_callback_t m_gui_callback = 0;

// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(m_gui_cmd_queue, sizeof(gui_message_t), 8, 4);

// Define a timer to update the GUI periodically
static void on_gui_blink_timer(struct k_timer *dummy);
K_TIMER_DEFINE(gui_blink_timer, on_gui_blink_timer, NULL);

//char *on_off_strings[2] = {"On", "Off"};

// GUI objects
//lv_obj_t *top_header;
//lv_obj_t *top_header_logo;
//lv_obj_t *label_button, *label_led, *label_bt_state_hdr, *label_bt_state;
//lv_obj_t *connected_background;
//lv_obj_t *label_btn_state, *label_led_state;
//lv_obj_t *btn1, *btn1_label;
//lv_obj_t *checkbox_led;
//lv_obj_t *image_led;
//lv_obj_t *image_bg[12];

// Styles
//lv_style_t style_btn, style_label, style_label_value, style_checkbox;
//lv_style_t style_header, style_con_bg;

// Fonts
//LV_FONT_DECLARE(arial_20bold);
//LV_FONT_DECLARE(calibri_20b);
//LV_FONT_DECLARE(calibri_20);
//LV_FONT_DECLARE(calibri_24b);
//LV_FONT_DECLARE(calibri_32b);

// Images
//LV_IMG_DECLARE(nod_logo);
//LV_IMG_DECLARE(led_on);
//LV_IMG_DECLARE(led_off);
//LV_IMG_DECLARE(img_noise_background);

//static void gui_show_connected_elements(bool connected);



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
//static void bar_anim(lv_task_t * t);
//static void arc_anim(lv_obj_t * arc, lv_anim_value_t value);
//static void linemeter_anim(lv_obj_t * linemeter, lv_anim_value_t value);
//static void gauge_anim(lv_obj_t * gauge, lv_anim_value_t value);
//static void table_event_cb(lv_obj_t * table, lv_event_t e);



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
//    configure_create(t2);
//    analysis_create(t3);



}

static void badges_create(lv_obj_t * parent)
{
/*
*/
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
//        lv_btn_set_checkable(btn, true);

        ///*Make a button disabled*/
        //if (i == 4) {
        //    lv_btn_set_state(btn, LV_BTN_STATE_DISABLED);
        //}
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



//////////////////////////////////////////////////////////////////








//static void on_button1(lv_obj_t *btn, lv_event_t event)
//{
//	if(btn == btn1){
//		if(event == LV_EVENT_PRESSED) {
//			lv_label_set_text(label_btn_state, "Pressed");
//			if(m_gui_callback) { 
//				m_gui_event.evt_type = GUI_EVT_BUTTON_PRESSED;
//				m_gui_event.button_checked = true;
//				m_gui_callback(&m_gui_event);
//			}
//		}
//		else if(event == LV_EVENT_RELEASED) {
//			lv_label_set_text(label_btn_state, "Released");
//			if(m_gui_callback) { 
//				m_gui_event.evt_type = GUI_EVT_BUTTON_PRESSED;
//				m_gui_event.button_checked = false;
//				m_gui_callback(&m_gui_event);
//			}
//		}
//	}
//}

//static void init_styles(void)
//{
//	/*Create background style*/
//	static lv_style_t style_screen;
//	lv_style_set_bg_color(&style_screen, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xcb, 0xca, 0xff));
//	lv_obj_add_style(lv_scr_act(), LV_BTN_PART_MAIN, &style_screen);

//	/*Create the screen header label style*/
//	lv_style_init(&style_header);
//	lv_style_set_bg_opa(&style_header, LV_STATE_DEFAULT, LV_OPA_COVER);
//	lv_style_set_bg_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x1C, 0x11, 0xFD));
//	lv_style_set_radius(&style_header, LV_STATE_DEFAULT, 8);
//	//lv_style_set_bg_grad_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_TEAL);
//	//lv_style_set_bg_grad_dir(&style_header, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
//	lv_style_set_pad_left(&style_header, LV_STATE_DEFAULT, 70);
//	lv_style_set_pad_top(&style_header, LV_STATE_DEFAULT, 30);
//	lv_style_set_shadow_spread(&style_header, LV_STATE_DEFAULT, 1);
//	lv_style_set_shadow_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_BLACK);
//	lv_style_set_shadow_opa(&style_header, LV_STATE_DEFAULT, 255);
//	lv_style_set_shadow_width(&style_header, LV_STATE_DEFAULT, 1);
//	lv_style_set_shadow_ofs_x(&style_header, LV_STATE_DEFAULT, 1);
//	lv_style_set_shadow_ofs_y(&style_header, LV_STATE_DEFAULT, 2);
//	lv_style_set_shadow_opa(&style_header, LV_STATE_DEFAULT, LV_OPA_50);

//	/*Screen header text style*/
//	lv_style_set_text_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x7d, 0xce, 0xfd));
//	lv_style_set_text_font(&style_header, LV_STATE_DEFAULT, &calibri_32b);
	

//	lv_style_init(&style_con_bg);
//	lv_style_copy(&style_con_bg, &style_header);
//	lv_style_set_bg_color(&style_con_bg, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x69, 0xb0, 0x5a));
//	lv_style_set_bg_opa(&style_con_bg, LV_STATE_DEFAULT, LV_OPA_50);
//	lv_style_set_radius(&style_header, LV_STATE_DEFAULT, 4);
	
//	/*Create a label style*/
//	lv_style_init(&style_label);
//	lv_style_set_bg_opa(&style_label, LV_STATE_DEFAULT, LV_OPA_COVER);
//	lv_style_set_bg_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_SILVER);
//	lv_style_set_bg_grad_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_GRAY);
//	lv_style_set_bg_grad_dir(&style_label, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
//	lv_style_set_pad_left(&style_label, LV_STATE_DEFAULT, 5);
//	lv_style_set_pad_top(&style_label, LV_STATE_DEFAULT, 10);

//	/*Add a border*/
//	lv_style_set_border_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
//	lv_style_set_border_opa(&style_label, LV_STATE_DEFAULT, LV_OPA_70);
//	lv_style_set_border_width(&style_label, LV_STATE_DEFAULT, 3);

//	/*Set the text style*/
//	lv_style_set_text_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x00, 0x30));
//	lv_style_set_text_font(&style_label, LV_STATE_DEFAULT, &calibri_20b);


//	/*Create a label value style*/
//	lv_style_init(&style_label_value);
//	lv_style_set_bg_opa(&style_label_value, LV_STATE_DEFAULT, LV_OPA_20);
//	lv_style_set_bg_color(&style_label_value, LV_STATE_DEFAULT, LV_COLOR_SILVER);
//	lv_style_set_bg_grad_color(&style_label_value, LV_STATE_DEFAULT, LV_COLOR_TEAL);
//	lv_style_set_bg_grad_dir(&style_label_value, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
//	lv_style_set_pad_left(&style_label_value, LV_STATE_DEFAULT, 0);
//	lv_style_set_pad_top(&style_label_value, LV_STATE_DEFAULT, 3);

//	/*Set the text style*/
//	lv_style_set_text_color(&style_label_value, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x00, 0x30));
//	lv_style_set_text_font(&style_label_value, LV_STATE_DEFAULT, &calibri_20);


//	/*Create a simple button style*/
//	lv_style_init(&style_btn);
//	lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 10);
//	lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
//	lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_SILVER);
//	lv_style_set_bg_grad_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_GRAY);
//	lv_style_set_bg_grad_dir(&style_btn, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
//	lv_style_set_shadow_spread(&style_btn, LV_STATE_DEFAULT, 1);
//	lv_style_set_shadow_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_GRAY);
//	lv_style_set_shadow_opa(&style_btn, LV_STATE_DEFAULT, 255);
//	lv_style_set_shadow_width(&style_btn, LV_STATE_DEFAULT, 1);

//	/*Swap the colors in pressed state*/
//	lv_style_set_bg_color(&style_btn, LV_STATE_PRESSED, LV_COLOR_GRAY);
//	lv_style_set_bg_grad_color(&style_btn, LV_STATE_PRESSED, LV_COLOR_SILVER);

//	/*Add a border*/
//	lv_style_set_border_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_BLACK);
//	lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_70);
//	lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 3);

//	/*Different border color in focused state*/
//	lv_style_set_border_color(&style_btn, LV_STATE_FOCUSED, LV_COLOR_BLACK);
//	lv_style_set_border_color(&style_btn, LV_STATE_FOCUSED | LV_STATE_PRESSED, LV_COLOR_NAVY);

//	/*Set the text style*/
//	lv_style_set_text_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_TEAL);
//	lv_style_set_text_font(&style_btn, LV_STATE_DEFAULT, &calibri_24b);

//	/*Make the button smaller when pressed*/
//	lv_style_set_transform_height(&style_btn, LV_STATE_PRESSED, -4);
//	lv_style_set_transform_width(&style_btn, LV_STATE_PRESSED, -8);
//#if LV_USE_ANIMATION
//	/*Add a transition to the size change*/
//	static lv_anim_path_t path;
//	lv_anim_path_init(&path);
//	lv_anim_path_set_cb(&path, lv_anim_path_overshoot);

//	lv_style_set_transition_prop_1(&style_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);
//	lv_style_set_transition_prop_2(&style_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
//	lv_style_set_transition_time(&style_btn, LV_STATE_DEFAULT, 300);
//	lv_style_set_transition_path(&style_btn, LV_STATE_DEFAULT, &path);
//#endif
//}


//static lv_obj_t * kb;

//static void kb_event_cb(lv_obj_t * _kb, lv_event_t e)
//{
//    lv_keyboard_def_event_cb(kb, e);

//    if(e == LV_EVENT_CANCEL) {
//        if(kb) {
////            lv_obj_set_height(tv, LV_VER_RES);
//            lv_obj_del(kb);
//            kb = NULL;
//        }
//    }
//}

//static void ta_event_cb(lv_obj_t * ta, lv_event_t e)
//{
//    if(e == LV_EVENT_RELEASED) {
//        if(kb == NULL) {
//            lv_coord_t kb_height = 160;//LV_MATH_MIN(LV_VER_RES / 2, LV_DPI * 4 / 3);
////            lv_obj_set_height(tv, LV_VER_RES - kb_height);
//            kb = lv_keyboard_create(lv_scr_act(), NULL);
////    lv_obj_add_style(kb, LV_CONT_PART_MAIN, &style_box);
//            lv_obj_set_height(kb, kb_height);
//            lv_obj_align(kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
//            lv_obj_set_event_cb(kb, kb_event_cb);

//            lv_indev_wait_release(lv_indev_get_act());
//        }
//        lv_textarea_set_cursor_hidden(ta, false);
////        lv_page_focus(t1, lv_textarea_get_label(ta), LV_ANIM_ON);
//        lv_keyboard_set_textarea(kb, ta);
//    } else if(e == LV_EVENT_DEFOCUSED) {
//        lv_textarea_set_cursor_hidden(ta, true);
//    }
//}

//static void init_blinky_gui(void)
//{
///*
//	for(int i = 0; i < 12; i++){
//		int x_index = i % 4;
//		int y_index = i / 4;
//		image_bg[i] = lv_img_create(lv_scr_act(), NULL);
//		lv_obj_set_pos(image_bg[i], x_index*80, y_index*80);
//		lv_obj_set_size(image_bg[i], 80, 80);
//		lv_img_set_src(image_bg[i], &img_noise_background);		
//	}
//*/
//	// The connected header needs to be created before the top_header, to appear behind
//	connected_background = lv_label_create(lv_scr_act(), NULL);
//	lv_obj_add_style(connected_background, LV_LABEL_PART_MAIN, &style_con_bg);
//	lv_label_set_long_mode(connected_background, LV_LABEL_LONG_DOT);
//	lv_obj_set_pos(connected_background, 6, 65);
//	lv_obj_set_size(connected_background, 308, 135);
//	lv_label_set_text(connected_background, "");

//	top_header = lv_label_create(lv_scr_act(), NULL);
//	lv_obj_add_style(top_header, LV_LABEL_PART_MAIN, &style_header);
//	lv_label_set_long_mode(top_header, LV_LABEL_LONG_DOT);
//	lv_obj_set_pos(top_header, 3, 3);
//	lv_obj_set_size(top_header, 314, 68);
//	lv_label_set_text(top_header, "nRF Blinky");
//	//lv_label_set_align(top_header, LV_LABEL_ALIGN_CENTER);

//	top_header_logo = lv_img_create(lv_scr_act(), NULL);
//	lv_obj_set_pos(top_header_logo, 0, 0);
//	lv_obj_set_size(top_header_logo, 68, 68);
//	lv_img_set_src(top_header_logo, &nod_logo);

//	label_bt_state = lv_label_create(lv_scr_act(), NULL);
//	lv_label_set_long_mode(label_bt_state, LV_LABEL_LONG_CROP);
//	lv_label_set_align(label_bt_state, LV_LABEL_ALIGN_CENTER); 
//	lv_obj_set_pos(label_bt_state, 70, 210);
//	lv_obj_set_size(label_bt_state, 180, 30);
//	lv_label_set_text(label_bt_state, "Idle");
//	lv_obj_add_style(label_bt_state, LV_LABEL_PART_MAIN, &style_label_value);

//	btn1 = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
//	lv_obj_set_pos(btn1, 10, 90);                            /*Set its position*/
//	lv_obj_set_size(btn1, 120, 50);                          /*Set its size*/
//	lv_obj_reset_style_list(btn1, LV_BTN_PART_MAIN);         /*Remove the styles coming from the theme*/
//	lv_obj_add_style(btn1, LV_BTN_PART_MAIN, &style_btn);
//	lv_btn_set_checkable(btn1, true);

//	btn1_label = lv_label_create(btn1, NULL);          /*Add a label to the button*/
//	lv_label_set_text(btn1_label, "Button");                     /*Set the labels text*/
//	lv_obj_set_event_cb(btn1, on_button1);

//	label_btn_state = lv_label_create(lv_scr_act(), NULL);
//	lv_label_set_long_mode(label_btn_state, LV_LABEL_LONG_DOT);
//	lv_obj_set_pos(label_btn_state, 10, 170);
//	lv_obj_set_size(label_btn_state, 120, 25);
//	lv_label_set_text(label_btn_state, "Released");
//	lv_label_set_align(label_btn_state, LV_LABEL_ALIGN_CENTER);
//	lv_obj_add_style(label_btn_state, LV_LABEL_PART_MAIN, &style_label_value);	

//	image_led = lv_img_create(lv_scr_act(), NULL);
//	lv_obj_set_pos(image_led, 192, 68);
//	lv_obj_set_size(image_led, 68, 68);
//	lv_img_set_src(image_led, &led_off);

//	label_led_state = lv_label_create(lv_scr_act(), NULL);
//	lv_label_set_long_mode(label_led_state, LV_LABEL_LONG_DOT);
//	lv_obj_set_pos(label_led_state, 170, 170);
//	lv_obj_set_size(label_led_state, 120, 25);
//	lv_label_set_text(label_led_state, "Off");
//	lv_label_set_align(label_led_state, LV_LABEL_ALIGN_CENTER);
//	lv_obj_add_style(label_led_state, LV_LABEL_PART_MAIN, &style_label_value);

////    lv_obj_t * ta = lv_textarea_create(lv_scr_act(), NULL);
//////    lv_obj_add_style(ta, LV_CONT_PART_MAIN, &style_box);

////    lv_textarea_set_one_line(ta, true);
////    lv_textarea_set_text(ta, "");
////    lv_textarea_set_placeholder_text(ta, "Name");
////    lv_obj_set_event_cb(ta, ta_event_cb);
////    lv_obj_set_width(ta, 180);
////    lv_obj_set_pos(ta, 105, 60);


//	gui_show_connected_elements(false);
//}

//static void gui_show_connected_elements(bool connected)
//{
//	lv_obj_set_hidden(connected_background, !connected);
//	lv_obj_set_hidden(btn1, !connected);
//	lv_obj_set_hidden(label_btn_state, !connected);
//	lv_obj_set_hidden(image_led, !connected);
//	lv_obj_set_hidden(label_led_state, !connected);
//}

static void set_bt_state(gui_bt_state_t state)
{
	bool connected = false;
	switch(state){
		case GUI_BT_STATE_IDLE:
			//k_timer_stop(&gui_blink_timer);
//			lv_label_set_text(label_bt_state, "Idle");
			break;
		case GUI_BT_STATE_ADVERTISING:
			//k_timer_start(&gui_blink_timer, K_MSEC(500), K_MSEC(500));
//			lv_label_set_text(label_bt_state, "Advertising");
			break;
		case GUI_BT_STATE_CONNECTED:
			k_timer_stop(&gui_blink_timer);
//			lv_label_set_text(label_bt_state, "Connected");
			//connected = true;
			break;
	}
//	gui_show_connected_elements(connected);
}

static void on_gui_blink_timer(struct k_timer *dummy)
{
	//static bool blink_state;
	//blink_state = !blink_state;
//	lv_label_set_text(label_bt_state, blink_state ? "Advertising" : "");
}

void gui_init(gui_config_t * config)
{
	//m_gui_callback = config->event_callback;
}

void gui_set_bt_state(gui_bt_state_t state)
{
	//static gui_message_t set_bt_state_msg;
	//set_bt_state_msg.type = GUI_MSG_SET_BT_STATE;
	//set_bt_state_msg.params.bt_state = state;
	//k_msgq_put(&m_gui_cmd_queue, &set_bt_state_msg, K_NO_WAIT);
}

void gui_set_bt_led_state(bool led_is_on)
{
	//static gui_message_t set_led_state_msg;
	//set_led_state_msg.type = GUI_MSG_SET_LED_STATE;
	//set_led_state_msg.params.led_state = led_is_on;
	//k_msgq_put(&m_gui_cmd_queue, &set_led_state_msg, K_NO_WAIT);
}

static void process_cmd_msg_queue(void)
{
	gui_message_t cmd_message;
	while(k_msgq_get(&m_gui_cmd_queue, &cmd_message, K_NO_WAIT) == 0){
		// Process incoming commands depending on type
		switch(cmd_message.type){
			case GUI_MSG_SET_STATE:
				break;
			case GUI_MSG_SET_BT_STATE:
//				set_bt_state(cmd_message.params.bt_state);
				break;
			case GUI_MSG_SET_LED_STATE:
//				lv_img_set_src(image_led, cmd_message.params.led_state ? &led_on : &led_off);
//				lv_label_set_text(label_led_state, cmd_message.params.led_state ? "On" : "Off");
				break;
		}
	}
}

void gui_run(void)
{
	display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

	if (display_dev == NULL) {
		LOG_ERR("Display device not found!");
		return;
	}

//	init_styles();

//	init_blinky_gui();
        lv_demo_widgets();

	display_blanking_off(display_dev);

	while(1){
		process_cmd_msg_queue();
		lv_task_handler();
		k_sleep(K_MSEC(20));
	}
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(gui_thread, 4096*2, gui_run, NULL, NULL, NULL, 7, 0, 0);