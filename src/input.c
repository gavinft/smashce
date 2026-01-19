#include <keypadc.h>
#include <stdint.h>
#include "input.h"
#include "controller.h"

#define XBC_STICK_INPUT_SCALE (1.0f / 32767) // idk

void input_scan_xbc(xbc_controller_t* controller, input_t* input) {
    xbc_Scan(controller);

    input->move.x = controller->control_data.lx * XBC_STICK_INPUT_SCALE;
    input->move.y = controller->control_data.ly * XBC_STICK_INPUT_SCALE;

    uint16_t buttons = controller->control_data.digital_buttons;

    input->jump = buttons & (XBC_X | XBC_Y);
    input->attack = buttons & XBC_A;
    input->special = buttons & XBC_B;
}

void input_scan_kpad(input_t* input) {
    kb_Scan();

    // move: arrow keys
    // - modify (half strength): clear
    // jump: alpha
    // attack: 2nd
    // special: mode

    float mod = kb_Data[6] & kb_Clear ? 0.5f : 1.0f;
    if (kb_Data[7] & kb_Left)
        input->move.x -= 1.0f * mod;
    if (kb_Data[7] & kb_Right)
        input->move.x += 1.0f * mod;
    
    input->jump = kb_Data[2] & kb_Alpha;
    input->attack = kb_Data[1] & kb_2nd;
    input->special = kb_Data[1] & kb_Mode;
}

usb_error_t usb_event_handler(usb_event_t event, void* event_data, void* user_data) {

    controller_state_t* state = user_data;
    xbc_controller_t* controller;
    uint24_t id;

    switch (event) {
        case USB_DEVICE_CONNECTED_EVENT:

            if (usb_GetRole() & USB_ROLE_DEVICE) // connected device is a host, not a controller or hub
                break;

            // connected device is hub
            usb_device_flags_t flags = usb_GetDeviceFlags(event_data);

            if (flags & USB_IS_HUB) { // not sure if hubs have to be reset
                usb_SetDeviceData(event_data, (void*)(uint24_t)UINT24_MAX);
                break;
            }
            
            // is a controller
            usb_ResetDevice(event_data);

            break;

        case USB_DEVICE_ENABLED_EVENT:

            // controller setup
            
            controller = &state->controllers[state->num_connected_controllers++];

            // keep track of the player number in device data
            usb_SetDeviceData(event_data, (void*)(uint24_t)(state->num_connected_controllers - 1));

            xbc_Init(controller, event_data);
            xbc_SetLED(controller, state->num_connected_controllers + 1);

            break;

        case USB_DEVICE_DISCONNECTED_EVENT:
        case USB_DEVICE_DISABLED_EVENT:

            id = (uint24_t)usb_GetDeviceData(event_data);

            if (id == UINT24_MAX) { // hub
                // should the controllers list be cleared?
                break;
            }

            state->num_connected_controllers--;
            for (uint24_t i = id; i < state->num_connected_controllers; i++) {
                state->controllers[i] = state->controllers[i + 1]; // memcpy?
                usb_SetDeviceData(usb_GetEndpointDevice(state->controllers[i].control_endpoint), (void*)i);
                xbc_SetLED(&state->controllers[i], i + 2);
            }

            break;
    }

    return USB_SUCCESS;
}