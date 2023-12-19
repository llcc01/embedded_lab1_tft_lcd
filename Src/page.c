#include "page.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lvgl.h"
#include "radio.h"

#define FREQ_MAX 108.0
#define FREQ_MIN 88.0

lv_obj_t* freq_input = NULL;
lv_obj_t* list_btns[16];

float freq_display = 100.0;

float freq_list[16] = {};

void timer_scan_event_cb(lv_timer_t* timer) {
  freq_input_update();
  radio_set_frequency();
  freq_display += 0.1;
  if (freq_display > FREQ_MAX) {
    lv_timer_reset(timer);
  }
}

void freq_input_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* ta = lv_event_get_target(e);
  lv_obj_t* kb = lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
  } else if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  } else if (code == LV_EVENT_READY) {
    const char* text = lv_textarea_get_text(ta);
    sscanf(text, "%f", &freq_display);
    if (freq_display > FREQ_MAX) {
      freq_display = FREQ_MAX;
    } else if (freq_display < FREQ_MIN) {
      freq_display = FREQ_MIN;
    }
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    freq_input_update();
    radio_set_frequency();
  } else if (code == LV_EVENT_CANCEL) {
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    freq_input_update();
    radio_set_frequency();
  }
}

void freq_input_update(void) {
  char freq_str[10];
  sprintf(freq_str, "%.1f", freq_display);
  lv_textarea_set_text(freq_input, freq_str);
}

void freq_up_event_cb(lv_event_t* e) {
  freq_display += 0.1;
  if (freq_display > FREQ_MAX) {
    freq_display = FREQ_MAX;
  }
  freq_input_update();
  radio_set_frequency();
}

void freq_down_event_cb(lv_event_t* e) {
  freq_display -= 0.1;
  if (freq_display < FREQ_MIN) {
    freq_display = FREQ_MIN;
  }
  freq_input_update();
  radio_set_frequency();
}

void freq_list_event_cb(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_target(e);
  lv_obj_t* label = lv_obj_get_child(obj, NULL);
  float* freq_ptr = lv_event_get_user_data(e);
  freq_display = *freq_ptr;
  freq_input_update();
  radio_set_frequency();
}

void freq_scan_event_cb(lv_event_t* e) {
  freq_display = radio_get_frequency();
  freq_input_update();
}

void list_widgets(lv_obj_t* parent) {
  lv_obj_t* list = lv_obj_create(parent);
  lv_obj_set_size(list, 400, 280);
  lv_obj_align_to(list, freq_input, LV_ALIGN_OUT_BOTTOM_MID, 0, 80);

  static lv_coord_t col_dsc[] = {80, 80, 80, 80, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {50, 50, 50, 50, LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(list, col_dsc, row_dsc);

  for (int i = 0; i < 16; i++) {
    list_btns[i] = lv_btn_create(list);
    lv_obj_set_size(list_btns[i], 70, 40);
    lv_obj_t* label = lv_label_create(list_btns[i]);
    lv_label_set_text_fmt(label, "%.1f", freq_list[i]);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(list_btns[i], freq_list_event_cb, LV_EVENT_CLICKED,
                        &freq_list[i]);
    lv_obj_set_grid_cell(list_btns[i], LV_GRID_ALIGN_CENTER, i % 4, 1,
                         LV_GRID_ALIGN_CENTER, i / 4, 1);
  }
}

void lv_lc_widgets(void) {
  lv_obj_t* scr = lv_disp_get_scr_act(NULL); /*Get the current screen*/

  lv_obj_t* frame = lv_obj_create(scr);
  lv_obj_set_size(frame, 440, 240);
  lv_obj_align(frame, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* kb = lv_keyboard_create(scr);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);

  freq_input = lv_textarea_create(frame);
  lv_obj_set_size(freq_input, 80, 48);
  lv_obj_align_to(freq_input, frame, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_add_event_cb(freq_input, freq_input_event_cb, LV_EVENT_ALL, kb);
  lv_textarea_set_accepted_chars(freq_input, "0123456789.");
  lv_textarea_set_one_line(freq_input, true);
  lv_textarea_set_max_length(freq_input, 5);
  freq_input_update();

  lv_obj_t* freq_input_label = lv_label_create(frame);
  lv_label_set_text(freq_input_label, "MHz");
  lv_obj_align_to(freq_input_label, freq_input, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  lv_obj_t* freq_up_btn = lv_btn_create(frame);
  lv_obj_t* freq_up_label = lv_label_create(freq_up_btn);
  lv_label_set_text(freq_up_label, "->");
  lv_obj_add_event_cb(freq_up_btn, freq_up_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_align_to(freq_up_btn, freq_input, LV_ALIGN_OUT_BOTTOM_MID, 150, 20);

  lv_obj_t* freq_scan_btn = lv_btn_create(frame);
  lv_obj_t* freq_scan_label = lv_label_create(freq_scan_btn);
  lv_obj_add_event_cb(freq_scan_btn, freq_scan_event_cb, LV_EVENT_CLICKED,
                      NULL);
  lv_label_set_text(freq_scan_label, "scan");
  lv_obj_align_to(freq_scan_btn, freq_input, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  lv_obj_t* freq_down_btn = lv_btn_create(frame);
  lv_obj_t* freq_down_label = lv_label_create(freq_down_btn);
  lv_label_set_text(freq_down_label, "<-");
  lv_obj_add_event_cb(freq_down_btn, freq_down_event_cb, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_align_to(freq_down_btn, freq_input, LV_ALIGN_OUT_BOTTOM_MID, -150, 20);

  list_widgets(frame);
}
