#include <stdbool.h>
#include <time.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>
#include <math.h>

#include "physics.h"
#include "input.h"
#include "controller.h"
#include "gfx/gfx.h"

clock_t last_time;
#define FRAME_TIME CLOCKS_PER_SEC / 30.0

#define MOVE_DEADZONE 0.01f

#define MOVE_DEADZONE 0.01f

collider_t stage_col = {.pos = {160, 190}, .extent = {130, 20}, .layer = phy_layer_stage, .friction = 1.3f};
collider_t box_col = {.pos = {160, 110}, .extent = {20, 20}, .layer = phy_layer_stage};
rb_t player;


controller_state_t controller_state;

static void begin();
static void end();
static bool step();
static void draw();

int main(void)
{
    begin(); // No rendering allowed!
    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);
    gfx_SetDrawBuffer(); // Draw to the buffer to avoid rendering artifacts

    while (step()) { // No rendering allowed in step!
        draw(); // As little non-rendering logic as possible
        gfx_SwapDraw(); // Queue the buffered frame to be displayed
        gfx_Wait();
        while (clock() < last_time + FRAME_TIME); // wait for end of frame
    }

    gfx_End();
    end();

    return 0;
}

static void reset_oiram() {
    player = (rb_t) {.col = {.pos = {160, 30}, .extent = {16 / 2, 27.0f / 2}, .layer = phy_layer_player, .friction = 1.0f}, .resistance = 0.1f, .max_fall = 400};
}

static void begin() {
    usb_Init(usb_event_handler, &controller_state, NULL, USB_DEFAULT_INIT_FLAGS);
    controller_state.controllers[0].type = CONTROLLER_KEYPAD;
    controller_state.num_connected_controllers = 1;

    reset_oiram();
    phy_rbs[0] = &player;
    phy_stage_colliders[0] = &stage_col;
    phy_stage_colliders[2] = &box_col;
}

static void end() {
    usb_Cleanup();
}
usb_error_t err;
static bool step() {
    last_time = clock();

    // remove maybe?
    kb_Scan();

    
    usb_error_t e = usb_HandleEvents();
    if (e != USB_SUCCESS) {
        err = e;
    }

    float accel;
    const float max_speed = 175;
    
    if (player.grounded) {
        accel = 2500 / 30.0;
        dbg_printf("grounded\n");
    } else {
        accel = 800 / 30.0;
        dbg_printf("not grounded\n");
    }

    for (int i = 0; i < controller_state.num_connected_controllers; i++) {
        input_t* input = &controller_state.controllers[i].input;

        switch (controller_state.controllers[i].type) {
            case CONTROLLER_XBOX:
                input_scan_xbc(&controller_state.controllers[i].controller.xbc, input);
                break;

            case CONTROLLER_KEYPAD:
                input_scan_kpad(input);
                break;
        }

        if (fabsf(input->move.x) > MOVE_DEADZONE) {
            if (input->move.x < 0) {
                if (player.vel.x > -max_speed)
                    player.vel.x += accel * input->move.x;
            } else {
                if (player.vel.x < max_speed)
                    player.vel.x += accel * input->move.x;
            }
        }

        if (input->jump && player.grounded)
            player.vel.y = -400;
    }

    phy_step(1.0 / 30.0);

    if (player.col.pos.y > 280)
        reset_oiram();

    // dbg_printf("player:\n\tx = %f\n\ty = %f\n", player.col.pos.x, player.col.pos.y);
    // dbg_printf("stage: lt (%.1f, %.1f) rb (%.1f, %.1f)\n", phy_col_left(stage_col), phy_col_top(stage_col), phy_col_right(stage_col), phy_col_bot(stage_col));
    // dbg_printf("box: kt (%.1f, %.1f) rb (%.1f, %.1f)\n", phy_col_left(box_col), phy_col_top(box_col), phy_col_right(box_col), phy_col_bot(box_col));

    return !kb_IsDown(kb_KeyClear);
}

void draw() {
    /* Initialize graphics drawing */
    gfx_FillScreen(2);
    gfx_SetColor(3);

    if (err != USB_SUCCESS) {
        gfx_SetTextXY(10, 10);
        gfx_PrintString("failed ");
        gfx_PrintInt(err, 1);
    }

   
    gfx_Rectangle(phy_col_left(stage_col), phy_col_top(stage_col), stage_col.extent.x * 2, stage_col.extent.y * 2);
    // dbg_printf("phy_col_left: %f\nphy_col_top: %f\nextent x: %f\nextent y: %f\n\n", phy_col_left(stage_col), phy_col_top(stage_col), stage_col.extent.x * 2, stage_col.extent.y * 2);
    gfx_Rectangle(phy_col_left(box_col), phy_col_top(box_col), box_col.extent.x * 2, box_col.extent.y * 2);

    /* A transparent sprite allows the background to show */
    gfx_TransparentSprite(oiram, player.col.pos.x - oiram_width / 2.0f, player.col.pos.y - oiram_height / 2.0f);

    gfx_SetTextXY(5, 5);
    gfx_SetTextBGColor(3);
    gfx_SetTextFGColor(2);
    gfx_PrintString("vel: (");
    gfx_PrintInt(player.vel.x, 1);
    gfx_PrintString(", ");
    gfx_PrintInt(player.vel.y, 1);
    gfx_PrintString(")");
}
