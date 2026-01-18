#include <stdbool.h>
#include <ti/getcsc.h>
#include <graphx.h>
#include <keypadc.h>

/* Include the converted graphics file */
#include "gfx/gfx.h"

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
    }

    gfx_End();
    end();

    return 0;
}

static void begin() {
    
}

static void end() {
    
}

static bool step() {
    kb_Scan();
    return !kb_IsDown(kb_KeyEnter);
}

void draw() {
    /* Initialize graphics drawing */
    gfx_FillScreen(1);

    /* Draw a normal sprite */
    gfx_Sprite_NoClip(oiram, 130, 110);

    /* A transparent sprite allows the background to show */
    gfx_TransparentSprite_NoClip(oiram, 190, 110);
}
