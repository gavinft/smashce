#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <usbdrvce.h>
#include <stdbool.h>

typedef enum {

    XBC_DPAD_U       = 1 << 0,
    XBC_DPAD_D       = 1 << 1,
    XBC_DPAD_L       = 1 << 2,
    XBC_DPAD_R       = 1 << 3,
    XBC_START        = 1 << 4,
    XBC_BACK         = 1 << 5,
    XBC_LSTICK_PRESS = 1 << 6,
    XBC_RSTICK_PRESS = 1 << 7,

    XBC_LB           = 1 << 8,
    XBC_RB           = 1 << 9,
    XBC_LOGO         = 1 << 10,
    // bit 3 of byte 2 unused
    XBC_A            = 1 << 12,
    XBC_B            = 1 << 13,
    XBC_X            = 1 << 14,
    XBC_Y            = 1 << 15
 
} xbc_digital_button;

typedef struct {

    uint8_t message_type;     // should always be 0x00
    uint8_t length;           // should always be 0x14 (20)
    uint16_t digital_buttons; // packed bits
    uint8_t lt;               // analog triggers (0 = not pressed, 244 = fully pressed)
    uint8_t rt;
    int16_t lx;               // left analog stick (signed)
    int16_t ly;
    int16_t rx;               // right analog stick
    int16_t ry;
    uint8_t extra[10];        // idk bro

} xbc_control_data_t;

typedef struct {

    xbc_control_data_t control_data;
    usb_endpoint_t control_endpoint;
    usb_endpoint_t rumble_led_endpoint;
    bool transfer_active;

} xbc_controller_t;

enum {
    XBC_SET_RUMBLE = 0,
    XBC_SET_LED = 1
}; // rumble or led mode

typedef enum {
    XBC_LED_OFF,
    XBC_LED_BLINK,

    XBC_LED_READY_1, // upper left blink then on
    XBC_LED_READY_2, // upper right
    XBC_LED_READY_4, // bottom right
    XBC_LED_READY_3, // bottom left

    XBC_LED_ON_1,
    XBC_LED_ON_2,
    XBC_LED_ON_4,
    XBC_LED_ON_3,

    XBC_LED_ROTATE,
    XBC_LED_BLINK_CURRENT, // make the current setting start blinking
    XBC_LED_SLOW_BLINK_CURRENT, // same but with a slow blink
    XBC_LED_BLINK_ALTERNATE // alternate between ul + br and ur + bl

} xbc_led_mode;

#define XBC_RUMBLE_PACKET_SIZE 8
#define XBC_LED_PACKET_SIZE 3

typedef struct {
    uint8_t mode; // rumble or led mode
    uint8_t size;
    uint8_t led_mode;
    uint8_t heavy_rumble; // left side
    uint8_t light_rumble; // right side
    uint8_t extra[3];
} xbc_rumble_led_data_t;

void xbc_Scan(xbc_controller_t* controller);
void xbc_SetRumble(xbc_controller_t* controller, uint8_t light_rumble, uint8_t heavy_rumble);
void xbc_SetLED(xbc_controller_t* controller, xbc_led_mode led_mode);
void xbc_Init(xbc_controller_t* controller, usb_device_t device) ;


#endif
