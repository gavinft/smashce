#include <graphx.h>
#include <debug.h>
#include "player.h"
#include "colors.h"
#include "gfx/gfx.h"

#ifndef NDEBUG
#define HURTBOXES_MAX (128)
collider_t hurtboxes[HURTBOXES_MAX];
size_t hurtboxes_len;
#endif /* NDEBUG */

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

void player_update(player_t *player, input_t *input, input_t* last_input, float dt) {
    float accel;
    float max_speed;

    if (!input)
        return;

    max_speed = player->max_speed;
    if (player->rb.grounded) {
        accel = player->ground_accel * dt;
        dbg_printf("grounded\n");
    } else {
        accel = player->air_accel * dt;
        dbg_printf("not grounded\n");
    }

    if (player->state != PLAYER_STATE_LOCKOUT) {
        if (input->move.x < 0) {
            if (player->rb.vel.x > -max_speed)
                player->rb.vel.x += accel * input->move.x;
        } else {
            if (player->rb.vel.x < max_speed)
                player->rb.vel.x += accel * input->move.x;
        }

        if (input->jump && !last_input->jump && player->rb.grounded )
            player->rb.vel.y = player->jump_vel;
    }
    
    if (player->rb.col.pos.y > 280)
        player_set_charac(player, player->charac);
}

void player_lateupdate(player_t *player, input_t *input, input_t* last_input, float dt) {
    
}

static void hurtbox(player_t *player, collider_t* box, vec2_t* kb, float dt, player_t* hitboxes, size_t hitboxes_len) {
    #ifndef NDEBUG
    if (hurtboxes_len < HURTBOXES_MAX) {
        hurtboxes[hurtboxes_len++] = *box;
    }
    #endif /* NDEBUG */

    for (size_t i = 0; i < hitboxes_len; i++) {
        collider_t *hitbox = &hitboxes[i].rb.col;
        if (phy_col_overlap(*box, *hitbox)) {
            if (player == &hitboxes[i])
                continue;
            vec2_t *vel = &hitboxes[i].rb.vel;
            vel->x += kb->x * dt;
            vel->y += kb->y * dt;
        }
    }
}

static void oiram_au(player_t *player, input_t *input, input_t *last_input, float dt, player_t* hitboxes, size_t hitboxes_len) {
    typedef enum {
        ANIM_DEFAULT,
        ANIM_JAB,
    } oiram_anim_t;

    if (player->state == PLAYER_STATE_LOCKOUT) {
        if (player->lockout_frames <= 0)
            player->state = PLAYER_STATE_ACTIONABLE;
        else
            player->lockout_frames -= 1;
    }

    switch (player->animation) {
        case ANIM_DEFAULT:
            if (input->attack && !last_input->attack) {
                player->animation = ANIM_JAB;
                player->state = PLAYER_STATE_LOCKOUT;
                player->lockout_frames = 10;
                player->anim_frame = -1;
            }
            break;
        case ANIM_JAB:
            player->anim_frame += 1;
            switch (player->anim_frame) {
                case 0:
                case 1:
                case 2:
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    hurtbox(player, &(collider_t){.pos = {player->rb.col.pos.x + 9, player->rb.col.pos.y}, .extent = {4, 4}}, &(vec2_t){10000, 300}, dt, hitboxes, hitboxes_len);
                    break;
                case 8:
                case 9:
                    break;
                case 10:
                    player->animation = ANIM_DEFAULT;
            }
            break;
    }
}

void player_attackupdate(player_t *player, input_t *input, input_t* last_input, float dt, player_t* hitboxes, size_t hitboxes_len) {
    oiram_au(player, input, last_input, dt, hitboxes, hitboxes_len);
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

#ifndef NDEBUG
void player_dbg_newframe() {
    hurtboxes_len = 0;
}

void player_dbg_drawboxes(player_t* hitboxes, size_t hitboxes_len) {
    gfx_SetColor(COLOR_DBG_HITBOX);
    for (size_t i = 0; i < hitboxes_len; i++) {
        collider_t* col = &hitboxes[i].rb.col;
        gfx_Rectangle(phy_col_left(*col), phy_col_top(*col), col->extent.x * 2, col->extent.y * 2);
    }
    gfx_SetColor(COLOR_DBG_HURTBOX);
    for (size_t i = 0; i < hurtboxes_len; i++) {
        gfx_Rectangle(phy_col_left(hurtboxes[i]), phy_col_top(hurtboxes[i]), hurtboxes[i].extent.x * 2, hurtboxes[i].extent.y * 2);
    }
}
#endif /* NDEBUG */
