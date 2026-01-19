#include <graphx.h>
#include <debug.h>
#include "player.h"
#include "gfx/gfx.h"

void player_set_charac(player_t *player, player_char_t charac) {
    switch (charac) {
        case PLAYER_OIRAM:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .pos = {150, 30},
                        .extent = {16 / 2, 27.0f / 2},
                        .layer = phy_layer_player,
                        .friction = 1.0f
                    },
                    .resistance = 0.1f,
                    .max_fall = 400
                },
                .charac = PLAYER_OIRAM,
                .max_speed = 175,
                .ground_accel = 2500,
                .air_accel = 800,
                .jump_vel = -400,
            };
            break;
        case PLAYER_MARIO:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .pos = {170, 30},
                        .extent = {16 / 2, 27.0f / 2},
                        .layer = phy_layer_player,
                        .friction = 1.0f
                    },
                    .resistance = 0.1f,
                    .max_fall = 400
                },
                .charac = PLAYER_MARIO,
                .max_speed = 175,
                .ground_accel = 2500,
                .air_accel = 800,
                .jump_vel = -400,
            };
            break;
    }
}

void player_update(player_t *player, input_t *input, float dt) {
    float accel;
    float max_speed = player->max_speed;

    if (player->rb.grounded) {
        accel = player->ground_accel * dt;
        dbg_printf("grounded\n");
    } else {
        accel = player->air_accel * dt;
        dbg_printf("not grounded\n");
    }

    if (input->move.x < 0) {
        if (player->rb.vel.x > -max_speed)
            player->rb.vel.x += accel * input->move.x;
    } else {
        if (player->rb.vel.x < max_speed)
            player->rb.vel.x += accel * input->move.x;
    }

    if (input->jump && player->rb.grounded)
        player->rb.vel.y = player->jump_vel;
    
    if (player->rb.col.pos.y > 280)
        player_set_charac(player, player->charac);
}

void player_draw(player_t *player) {
    rb_t *rb = &player->rb;

    switch (player->charac) {
        case PLAYER_OIRAM:
            gfx_TransparentSprite(oiram, rb->col.pos.x - oiram_width / 2.0f, rb->col.pos.y - oiram_height / 2.0f);
            break;
        case PLAYER_MARIO:
            gfx_TransparentSprite(mario, rb->col.pos.x - mario_width / 2.0f, rb->col.pos.y - mario_height / 2.0f);
            break;
    }
}
