#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "vec.h"
#ifdef __TICE__
#include "controller.h"
#endif /* __TICE__ */

typedef struct input {
    vec2_t move;
    bool jump;
    bool attack;
    bool special;
} input_t;


#define MAX_CONTROLLERS 4

typedef enum {
    CONTROLLER_XBOX,
    CONTROLLER_KEYPAD,
    CONTROLLER_DUMMY,
} controller_type_t;

typedef struct {

    controller_type_t type;
    input_t input;
    input_t last_input;
#ifdef __TICE__
    union {
        xbc_controller_t xbc;
    } controller;
#endif /* __TICE__ */

} controller_t;


typedef struct {
    
    controller_t controllers[4];
    int num_connected_controllers;

} controller_state_t;

#ifdef __TICE__
void input_scan_xbc(xbc_controller_t* controller, input_t* input);
void input_scan_kpad(input_t* input);
usb_error_t usb_event_handler(usb_event_t event, void* event_data, void* user_data);
#endif /* __TICE__ */

#endif