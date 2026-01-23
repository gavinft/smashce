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
#include "colors.h"
#include "gfx/gfx.h"

clock_t last_time;
#define FRAME_TIME CLOCKS_PER_SEC / 30.0

collider_t stage_col = {.box = {.pos = {160, 190}, .extent = {130, 20}}, .friction = 1.3f};
collider_t box_col = {.box = {.pos = {160, 110}, .extent = {20, 20}}, .friction = 0.3f};
ledge_t left_ledge = {.box = {.pos = {24, 175}, .extent = {7, 10}}, .grab_dir = DIR_RIGHT};
ledge_t right_ledge = {.box = {.pos = {296, 175}, .extent = {7, 10}}, .grab_dir = DIR_LEFT};
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
    controller_state.controllers[0].type = CONTROLLER_KEYPAD;
    controller_state.controllers[1].type = CONTROLLER_DUMMY;

    controller_state.num_connected_controllers = 2;

    player_set_charac(&players[0], PLAYER_LUIGI);
    player_set_charac(&players[1], PLAYER_MARIO);
    phy_rbs[0] = &players[0].rb;
    phy_rbs[1] = &players[1].rb;
    phy_stage_colliders[0] = &stage_col;
    phy_stage_colliders[1] = &box_col;
    phy_ledges[0] = &left_ledge;
    phy_ledges[1] = &right_ledge;

    player_load_sprites();

    dbg_printf("begun\n");
}

static void end() {
    usb_Cleanup();
}

static int min(int first, int second) {
    return first < second ? first : second;
}

usb_error_t err;
static bool step() {
    last_time = clock();

    #ifndef NDEBUG
    player_dbg_newframe();
    #endif /* NDEBUG */

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
    }

    for (int i = 0; i < controller_state.num_connected_controllers; i++) {
        controller_t* controller = &controller_state.controllers[i];
        player_lateupdate(&players[i], &controller->input, &controller->last_input, 1.0f / 30.0f);
    }

    for (int i = 0; i < controller_state.num_connected_controllers; i++) {
        controller_t* controller = &controller_state.controllers[i];
        player_attackupdate(&players[i], &controller->input, &controller->last_input, 1.0f / 30.0f,
            players, min(controller_state.num_connected_controllers, MAX_PLAYERS));

        controller->last_input = controller->input; // has to happen at some point b4 next frame
    }

    phy_step(1.0f / 30.0f);

    return !kb_IsDown(kb_KeyClear);
}

#define draw_box(box) (gfx_Rectangle(phy_box_left(box), phy_box_top(box), box.extent.x * 2, box.extent.y * 2))
#define fill_box(box) (gfx_FillRectangle(phy_box_left(box), phy_box_top(box), box.extent.x * 2, box.extent.y * 2))

void draw() {
    /* Initialize graphics drawing */
    gfx_FillScreen(COLOR_BG);
    
    gfx_SetColor(COLOR_STAGE);
    draw_box(stage_col.box);
    draw_box(box_col.box);

    #ifndef NDEBUG
    /* show ledges */
    gfx_SetColor(COLOR_DBG_HITBOX);
    draw_box(left_ledge.box);
    draw_box(right_ledge.box);
    #endif /* NDEBUG */

    /* render players */
    box_t screen = {.pos = {160, 120}, .extent = {160, 120}}; // 320x240
    player_t* oob_players[2];
    size_t oob_players_len = 0;
    if (phy_box_overlap(players[0].rb.col.box, screen)) {
    player_draw(&players[0]);
    } else {
        oob_players[oob_players_len++] = &players[0];
    }
    if (phy_box_overlap(players[1].rb.col.box, screen)) {
    player_draw(&players[1]);
    } else {
        oob_players[oob_players_len++] = &players[1];
    }
    gfx_SetTextXY(50, GFX_LCD_HEIGHT - 20);
    gfx_PrintInt(players[0].damage_percent, 3);
    gfx_SetTextXY(100, GFX_LCD_HEIGHT - 20);
    gfx_PrintInt(players[1].damage_percent, 3);

    gfx_SetTextXY(5, 5);
    gfx_SetTextBGColor(COLOR_STAGE);
    gfx_SetTextFGColor(COLOR_BG);
    gfx_PrintString("vel: (");
    gfx_PrintInt(players[0].rb.vel.x, 1);
    gfx_PrintString(", ");
    gfx_PrintInt(players[0].rb.vel.y, 1);
    gfx_PrintString(")");

    // draw out of bounds players
    for (size_t i = 0; i < oob_players_len; i++) {
        #define OOB_WINDOW_OFFSET (3)
        box_t window = {oob_players[i]->rb.col.box.pos, {30, 25}};
        
        if (phy_box_left(oob_players[i]->rb.col.box) > phy_box_right(screen)) {
            float distance = phy_box_left(oob_players[i]->rb.col.box) - phy_box_right(screen);
            window.extent.x -= fminf(distance / 4, 10);
            window.extent.y -= fminf(distance / 4, 10);
            window.pos.x = phy_box_right(screen) - OOB_WINDOW_OFFSET - window.extent.x;
        } else if (phy_box_right(oob_players[i]->rb.col.box) < phy_box_left(screen)) {
            float distance = phy_box_left(screen) - phy_box_right(oob_players[i]->rb.col.box);
            window.extent.x -= fminf(distance / 4, 10);
            window.extent.y -= fminf(distance / 4, 10);
            window.pos.x = phy_box_left(screen) + OOB_WINDOW_OFFSET + window.extent.x;
        }

        if (phy_box_top(oob_players[i]->rb.col.box) > phy_box_bot(screen)) {
            window.pos.y = phy_box_bot(screen) - OOB_WINDOW_OFFSET - window.extent.y;
        } else if (phy_box_bot(oob_players[i]->rb.col.box) < phy_box_top(screen)) {
            window.pos.y = phy_box_top(screen) + OOB_WINDOW_OFFSET + window.extent.y;
        }

        gfx_SetColor(COLOR_BG);
        fill_box(window);
        gfx_SetColor(COLOR_DBG_HITBOX);
        draw_box(window);
        player_draw_pos(oob_players[i], &window.pos);
    }
  
    gfx_SetTextXY(230, 5);
    gfx_PrintString("Raw FPS: ");
    gfx_PrintInt(CLOCKS_PER_SEC / (float)(clock() - last_time), 2);

    #ifndef NDEBUG
    /* show hitboxes and hurtboxes */
    player_dbg_drawboxes(players, min(controller_state.num_connected_controllers, MAX_PLAYERS));
    #endif /* NDEBUG */
}
