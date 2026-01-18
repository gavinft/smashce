#include <keypadc.h>
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
}
