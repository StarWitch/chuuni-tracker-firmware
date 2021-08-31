#pragma once

#include "chuuni.h"

struct Button {
    const uint8_t PIN;
    uint32_t selected;
    bool pressed;
};

extern Button opt_button;
extern Button debug_button;

void ARDUINO_ISR_ATTR isr(void *arg);

void init_wifi();
void get_time();
void get_serial_splash();
void set_serial_debug();
void menu_selector();
float get_battery_level();
