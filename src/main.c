#include <stdbool.h>
#include <time.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>

#include "physics.h"
#include "gfx/gfx.h"

clock_t last_time;
#define FRAME_TIME CLOCKS_PER_SEC / 30.0

collider_t stage_col = {.pos = {160, 190}, .extent = {130, 20}, .layer = phy_layer_stage};
rb_t player = {.col = {.pos = {160, 30}, .extent = {16 / 2, 27.0f / 2}, .layer = phy_layer_player}};

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
    phy_rbs[0] = &player;
    phy_colliders[0] = &stage_col;
    phy_colliders[1] = &player.col;
}

static void end() {
    
}

static bool step() {
    last_time = clock();
    kb_Scan();

    player.vel.x = 0;
    if (kb_IsDown(kb_KeyLeft))
        player.vel.x -= 5;
    if (kb_IsDown(kb_KeyRight))
        player.vel.x += 5;
    if (player.vel.y == 0 && kb_IsDown(kb_KeyUp))
        player.vel.y = -5;
    phy_step(1.0 / 30.0);

    return !kb_IsDown(kb_KeyClear);
}

void draw() {
    /* Initialize graphics drawing */
    gfx_FillScreen(1);

    gfx_SetColor(0);
    gfx_Rectangle(phy_col_left(stage_col), phy_col_top(stage_col), stage_col.extent.x * 2, stage_col.extent.y * 2);
    dbg_printf("phy_col_left: %f\nphy_col_top: %f\nextent x: %f\nextent y: %f\n\n", phy_col_left(stage_col), phy_col_top(stage_col), stage_col.extent.x * 2, stage_col.extent.y * 2);

    /* A transparent sprite allows the background to show */
    gfx_TransparentSprite(oiram, player.col.pos.x + oiram_width / 2, player.col.pos.y - oiram_height / 2);
}
