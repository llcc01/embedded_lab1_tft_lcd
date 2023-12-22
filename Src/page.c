#include "page.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "24c64.h"
#include "lvgl.h"
#include "radio.h"

#define SCAN_MAX 16
#define SCAN_STEP_KHZ 10

lv_obj_t* freq_input = NULL;
lv_obj_t* freq_input_label = NULL;
lv_obj_t* list_btns[SCAN_MAX];

// float freq_display = 100.0;
uint32_t freq_display_khz = 100000;

float freq_list[SCAN_MAX] = {};
// float freq_list[SCAN_MAX] = {87.6,  88.7,  90.0,  90.5,  91.5,
//                              94.5,  96.6,  97.4,  100.6, 101.8,
//                              102.5, 103.9, 106.1, 106.6, 107.3};

lv_timer_t* timer_scan = NULL;

uint8_t scan_counter = 0;
uint32_t scan_last_freq_khz = 0;
uint8_t scan_last_level = 0;
uint8_t scan_searching_max = 0;

void data_read() {
  uint16_t eepromAddress = 0;
  for (int i = 0; i < 16; i++) {
    freq_list[i] = readFloatFromEEPROM(eepromAddress);
    eepromAddress += sizeof(float);
  }
}

void data_write() {
  uint16_t eepromAddress = 0;
  for (int i = 0; i < 16; i++) {
    writeFloatToEEPROM(freq_list[i], eepromAddress);
    eepromAddress += sizeof(float);
  }
}

void scan_stop(lv_timer_t* timer) {
  lv_timer_del(timer_scan);
  lv_obj_t* freq_scan_btn_label = timer->user_data;
  lv_label_set_text(freq_scan_btn_label, "scan");

  for (int i = scan_counter; i < SCAN_MAX; i++) {
    freq_list[i] = 0;
  }

  list_widgets_update();
  freq_display_khz = freq_list[0] * 1000;
  freq_input_update();
  data_write();
  radio_set_frequency();
}

void timer_scan_event_cb(lv_timer_t* timer) {
  radio_set_frequency();
  struct strength strength = radio_get_strength();
  if (freq_display_khz % 100 == 0) {
    freq_input_update();
  }

  if ((freq_display_khz > (scan_last_freq_khz + 200)) && scan_searching_max) {
    uint32_t new_freq_khz = (scan_last_freq_khz / 100) * 100;
    uint32_t new_freq_khz_remainder = scan_last_freq_khz % 100;
    if (new_freq_khz_remainder > 50) {
      new_freq_khz += 100;
    }
    scan_last_freq_khz = new_freq_khz;
    freq_list[scan_counter] = scan_last_freq_khz / 1000.0;
    scan_counter++;
    scan_searching_max = 0;
    scan_last_level = 0;
  } else {
    if (strength.level > scan_last_level &&
        (strength.level > 10 && strength.if_counter > 0x31 &&
         strength.if_counter < 0x3e)) {
      scan_last_level = strength.level;
      scan_last_freq_khz = freq_display_khz;
      scan_searching_max = 1;
    }
  }

  freq_display_khz += SCAN_STEP_KHZ;
  if (freq_display_khz > max_frequency || scan_counter >= SCAN_MAX) {
    scan_stop(timer);
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
    float freq;
    sscanf(text, "%f", &freq);
    freq_display_khz = freq * 1000;
    if (freq_display_khz > max_frequency) {
      freq_display_khz = max_frequency;
    } else if (freq_display_khz < min_frequency) {
      freq_display_khz = min_frequency;
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
  sprintf(freq_str, "%.1f", freq_display_khz / 1000.0);
  lv_textarea_set_text(freq_input, freq_str);
  // struct strength strength = radio_get_strength();
  // lv_label_set_text_fmt(freq_input_label, "%d,%d,%d,%d", strength.rf,
  //                       strength.blf, strength.if_counter, strength.level);
}

void freq_up_event_cb(lv_event_t* e) {
  freq_display_khz += 100;
  if (freq_display_khz > max_frequency) {
    freq_display_khz = max_frequency;
  }
  freq_input_update();
  radio_set_frequency();
}

void freq_down_event_cb(lv_event_t* e) {
  freq_display_khz -= 100;
  if (freq_display_khz < min_frequency) {
    freq_display_khz = min_frequency;
  }
  freq_input_update();
  radio_set_frequency();
}

void freq_list_event_cb(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_target(e);
  lv_obj_t* label = lv_obj_get_child(obj, NULL);
  float* freq_ptr = lv_event_get_user_data(e);
  freq_display_khz = *freq_ptr * 1000;
  if (freq_display_khz > max_frequency) {
    freq_display_khz = max_frequency;
  } else if (freq_display_khz < min_frequency) {
    freq_display_khz = min_frequency;
  }
  freq_input_update();
  radio_set_frequency();
}

void freq_scan_event_cb(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_target(e);
  lv_obj_t* label = lv_obj_get_child(obj, NULL);
  if (strcmp(lv_label_get_text(label), "scan") == 0) {
    lv_label_set_text(label, "stop");
    scan_counter = 0;
    scan_last_level = 0;
    freq_display_khz = min_frequency;
    scan_last_freq_khz = min_frequency;
    scan_searching_max = 0;
    timer_scan = lv_timer_create(timer_scan_event_cb, 10, label);
  } else {
    scan_stop(timer_scan);
  }
}

void list_widgets(lv_obj_t* parent) {
  lv_obj_t* list = lv_obj_create(parent);
  lv_obj_set_size(list, 400, 160);
  lv_obj_align_to(list, freq_input, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  static lv_coord_t col_dsc[] = {80, 80, 80, 80, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 40, 40, 40, LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(list, col_dsc, row_dsc);

  data_read();
  for (int i = 0; i < SCAN_MAX; i++) {
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

void list_widgets_update(void) {
  for (int i = 0; i < SCAN_MAX; i++) {
    lv_obj_t* label = lv_obj_get_child(list_btns[i], NULL);
    lv_label_set_text_fmt(label, "%.1f", freq_list[i]);
  }
}

void lv_lc_widgets(void) {
  lv_obj_t* scr = lv_disp_get_scr_act(NULL); /*Get the current screen*/

  lv_obj_t* frame = lv_obj_create(scr);
  lv_obj_set_size(frame, 460, 260);
  lv_obj_align(frame, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* kb = lv_keyboard_create(scr);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);

  freq_input = lv_textarea_create(frame);
  lv_obj_set_size(freq_input, 80, 48);
  lv_obj_align_to(freq_input, frame, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_add_event_cb(freq_input, freq_input_event_cb, LV_EVENT_ALL, kb);
  lv_textarea_set_accepted_chars(freq_input, "0123456789.");
  lv_textarea_set_one_line(freq_input, true);
  lv_textarea_set_max_length(freq_input, 5);
  freq_input_update();

  freq_input_label = lv_label_create(frame);
  lv_label_set_text(freq_input_label, "MHz");
  lv_obj_align_to(freq_input_label, freq_input, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

  lv_obj_t* freq_scan_btn = lv_btn_create(frame);
  lv_obj_t* freq_scan_label = lv_label_create(freq_scan_btn);
  lv_obj_add_event_cb(freq_scan_btn, freq_scan_event_cb, LV_EVENT_CLICKED,
                      NULL);
  lv_label_set_text(freq_scan_label, "scan");
  lv_obj_align_to(freq_scan_btn, freq_input, LV_ALIGN_OUT_LEFT_MID, -20, 0);

  lv_obj_t* freq_up_btn = lv_btn_create(frame);
  lv_obj_t* freq_up_label = lv_label_create(freq_up_btn);
  lv_label_set_text(freq_up_label, "->");
  lv_obj_add_event_cb(freq_up_btn, freq_up_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_align_to(freq_up_btn, freq_input, LV_ALIGN_OUT_RIGHT_MID, 100, 0);

  lv_obj_t* freq_down_btn = lv_btn_create(frame);
  lv_obj_t* freq_down_label = lv_label_create(freq_down_btn);
  lv_label_set_text(freq_down_label, "<-");
  lv_obj_add_event_cb(freq_down_btn, freq_down_event_cb, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_align_to(freq_down_btn, freq_input, LV_ALIGN_OUT_LEFT_MID, -100, 0);

  list_widgets(frame);
}
