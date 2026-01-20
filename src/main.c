#include <stdbool.h>
#include <time.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>
#include <math.h>

#include "physics.h"
#include "input.h"
#include "controller.h"
#include "player.h"
#include "gfx/gfx.h"

clock_t last_time;
#define FRAME_TIME CLOCKS_PER_SEC / 30.0

collider_t stage_col = {.pos = {160, 190}, .extent = {130, 20}, .layer = phy_layer_stage, .friction = 1.3f};
collider_t box_col = {.pos = {160, 110}, .extent = {20, 20}, .layer = phy_layer_stage};
#define MAX_PLAYERS 2
player_t players[MAX_PLAYERS];


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

static void begin() {
    usb_Init(usb_event_handler, &controller_state, NULL, USB_DEFAULT_INIT_FLAGS);
    // controller_state.controllers[0].type = CONTROLLER_KEYPAD;
    // controller_state.num_connected_controllers = 1;

    player_set_charac(&players[0], PLAYER_OIRAM);
    player_set_charac(&players[1], PLAYER_MARIO);
    phy_rbs[0] = &players[0].rb;
    phy_rbs[1] = &players[1].rb;
    phy_stage_colliders[0] = &stage_col;
    phy_stage_colliders[1] = &box_col;
}

static void end() {
    usb_Cleanup();
}

usb_error_t err;
static bool step() {
    last_time = clock();

    // remove maybe?
    kb_Scan();

    usb_HandleEvents();

    for (int i = 0; i < controller_state.num_connected_controllers; i++) {
        controller_t* controller = &controller_state.controllers[i];
        input_t* input = &controller->input;

        switch (controller_state.controllers[i].type) {
            case CONTROLLER_XBOX:
                input_scan_xbc(&controller->controller.xbc, input);
                break;

            case CONTROLLER_KEYPAD:
                input_scan_kpad(input);
                break;

            case CONTROLLER_DUMMY:
                break;
        }

        player_update(&players[i], input, &controller->last_input, 1.0f / 30.0f);

        controller->last_input = controller->input;
    }

    phy_step(1.0f / 30.0f);

    return !kb_IsDown(kb_KeyClear);
}

void draw() {
    /* Initialize graphics drawing */
    gfx_FillScreen(2);
    gfx_SetColor(3);

   
    gfx_Rectangle(phy_col_left(stage_col), phy_col_top(stage_col), stage_col.extent.x * 2, stage_col.extent.y * 2);
    // dbg_printf("phy_col_left: %f\nphy_col_top: %f\nextent x: %f\nextent y: %f\n\n", phy_col_left(stage_col), phy_col_top(stage_col), stage_col.extent.x * 2, stage_col.extent.y * 2);
    gfx_Rectangle(phy_col_left(box_col), phy_col_top(box_col), box_col.extent.x * 2, box_col.extent.y * 2);

    /* render players */
    player_draw(&players[0]);
    player_draw(&players[1]);

    gfx_SetTextXY(5, 5);
    gfx_SetTextBGColor(3);
    gfx_SetTextFGColor(2);
    gfx_PrintString("vel: (");
    gfx_PrintInt(players[0].rb.vel.x, 1);
    gfx_PrintString(", ");
    gfx_PrintInt(players[0].rb.vel.y, 1);
    gfx_PrintString(")");
}
