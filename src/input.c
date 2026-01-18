#include "input.h"
#include "controller.h"

#define XBC_STICK_INPUT_SCALE (1.0f / 32767) // idk

void get_xbc_input(xbc_controller_t* controller, input_t* input) {
    xbc_Scan(controller);

    input->move.x = controller->control_data.lx * XBC_STICK_INPUT_SCALE;
    input->move.y = controller->control_data.ly * XBC_STICK_INPUT_SCALE;

    uint16_t buttons = controller->control_data.digital_buttons;

    input->jump = buttons & (XBC_X | XBC_Y);
    input->attack = buttons & XBC_A;
    input->special = buttons & XBC_B;
}