#include <graphx.h>
#include <debug.h>
#include <math.h>
#include "player.h"
#include "colors.h"
#include "gfx/gfx.h"

#ifndef NDEBUG
#define HURTBOXES_MAX (128)
collider_t hurtboxes[HURTBOXES_MAX];
size_t hurtboxes_len;
#endif /* NDEBUG */

#define TURN_DEADZONE (0.1f)
#define ATTACK_DIR_DEADZONE (0.1f)

#define flippable_duplicate(name) gfx_UninitedSprite(name ## _l, name ## _r_width, name ## _r_height)
#define flip(name) gfx_FlipSpriteY(name ## _r, name ## _l)

// reversed characters
// - eventually ill make some tool that auto makes these + other things
// gfx_UninitedSprite(oiram_neu_l, oiram_neu_r_width, oiram_neu_r_height);
flippable_duplicate(oiram_neu);
flippable_duplicate(mario_neu);

flippable_duplicate(luigi_neu);
flippable_duplicate(luigi_att);
flippable_duplicate(luigi_ssp);

void player_load_sprites() {
    flip(oiram_neu);
    flip(mario_neu);

    flip(luigi_neu);
    flip(luigi_att);
    flip(luigi_ssp);
}

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
                .dir = PLAYER_DIR_RIGHT,
                .sprite = oiram_neu_r,
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
                .sprite = mario_neu_r,
                .dir = PLAYER_DIR_RIGHT,
                .max_speed = 175,
                .ground_accel = 2500,
                .air_accel = 800,
                .jump_vel = -400,
            };
            break;
        
        case PLAYER_LUIGI:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .pos = {170, 30},
                        .extent = {luigi_neu_r_width / 2.0f, luigi_neu_r_height / 2.0f},
                        .layer = phy_layer_player,
                        .friction = 0.8f
                    },
                    .resistance = 0.1f,
                    .max_fall = 400
                },
                .charac = PLAYER_LUIGI,
                .sprite = luigi_neu_r,
                .dir = PLAYER_DIR_RIGHT,
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

        if (player->rb.grounded)
            player->jumps = 0;
        if (input->jump && !last_input->jump && (player->rb.grounded || player->jumps < 1)) {
            player->rb.vel.y = player->jump_vel;
            if (player->rb.grounded == false)
                player->jumps++;
        }
    }
    
    if (player->rb.col.pos.y > 280)
        player_set_charac(player, player->charac);
}

void player_lateupdate(player_t *player, input_t *input, input_t* last_input, float dt) {
    
}

static void side_special_attack_update_direction(player_t *player, input_t *input) {
    if (input->move.x > ATTACK_DIR_DEADZONE) {
        player->dir = PLAYER_DIR_RIGHT;
    } else if (input->move.x < ATTACK_DIR_DEADZONE) {
        player->dir = PLAYER_DIR_LEFT;
    }
}

static bool hurtbox(player_t *player, collider_t* box, vec2_t* kb, float dt, player_t* hitboxes, size_t hitboxes_len) {
    #ifndef NDEBUG
    if (hurtboxes_len < HURTBOXES_MAX) {
        hurtboxes[hurtboxes_len++] = *box;
    }
    #endif /* NDEBUG */

    bool hit = false;

    for (size_t i = 0; i < hitboxes_len; i++) {
        collider_t *hitbox = &hitboxes[i].rb.col;
        if (phy_col_overlap(*box, *hitbox)) {
            if (player == &hitboxes[i])
                continue;
            vec2_t *vel = &hitboxes[i].rb.vel;
            vel->x += kb->x * dt;
            vel->y += kb->y * dt;
            hit = true;
        }
    }

    return hit;
}

static void oiram_au(player_t *player, input_t *input, input_t *last_input, float dt, player_t* hitboxes, size_t hitboxes_len, bool is_mario) {
    enum {
        ANIM_DEFAULT,
        ANIM_JAB,
    };

    switch (player->animation) {
        case ANIM_DEFAULT:
            player->sprite = is_mario ? player_spr(mario_neu, player->dir) : player_spr(oiram_neu, player->dir);
            if (input->attack && !last_input->attack) {
                player->animation = ANIM_JAB;
                player->state = PLAYER_STATE_LOCKOUT;
                player->lockout_frames = 10;
                player->anim_frame = -1;
                break;
            }
            if (player->rb.grounded) {
                if (input->move.x < -TURN_DEADZONE)
                    player->dir = PLAYER_DIR_LEFT;
                else if (input->move.x > TURN_DEADZONE)
                    player->dir = PLAYER_DIR_RIGHT;
            }
            break;
        case ANIM_JAB:
            player->anim_frame += 1;
            switch (player->anim_frame) {
                case 0:
                    player->sprite = is_mario ? player_spr(mario_neu, player->dir) : player_spr(oiram_neu, player->dir);
                case 1: case 2:
                    break;
                case 3: case 4: case 5: case 6: case 7:
                    hurtbox(player, &(collider_t){.pos = {player->rb.col.pos.x + 9 * player->dir, player->rb.col.pos.y}, .extent = {4, 4}}, &(vec2_t){player->dir * 5000, -1000}, dt, hitboxes, hitboxes_len);
                    break;
                case 8: case 9:
                    break;
                case 10:
                    player->animation = ANIM_DEFAULT;
                    break;
            }
            break;
    }
}

static void luigi_au(player_t *player, input_t *input, input_t *last_input, float dt, player_t* hitboxes, size_t hitboxes_len) {
    enum {
        ANIM_DEFAULT,
        ANIM_JAB,
        ANIM_MISSILE,
    };

    switch (player->animation) {
        case ANIM_DEFAULT:
            player->sprite = player_spr(luigi_neu, player->dir);
            if (input->attack && !last_input->attack) {
                player->animation = ANIM_JAB;
                player->state = PLAYER_STATE_LOCKOUT;
                player->lockout_frames = 10;
                player->anim_frame = -1;
                break;
            }
            if (input->special && !last_input->special
                && fabsf(input->move.x) > ATTACK_DIR_DEADZONE) {
                player->animation = ANIM_MISSILE;
                player->state = PLAYER_STATE_LOCKOUT;
                player->lockout_frames = 10;
                player->anim_frame = -1;
                side_special_attack_update_direction(player, input);
            }

            if (player->rb.grounded) {
                if (input->move.x < -TURN_DEADZONE)
                    player->dir = PLAYER_DIR_LEFT;
                else if (input->move.x > TURN_DEADZONE)
                    player->dir = PLAYER_DIR_RIGHT;
            }
            break;
        case ANIM_JAB:
            player->anim_frame += 1;
            switch (player->anim_frame) {
                case 0:
                    player->sprite = player_spr(luigi_neu, player->dir);
                case 1: case 2:
                    break;
                case 3:
                    player->sprite = player_spr(luigi_att, player->dir);
                    break;
                case 4: case 5: case 6: case 7:
                    hurtbox(player, &(collider_t){.pos = {player->rb.col.pos.x + 9 * player->dir, player->rb.col.pos.y}, .extent = {4, 4}}, &(vec2_t){player->dir * 5000, -1000}, dt, hitboxes, hitboxes_len);
                    break;
                case 8: case 9:
                    break;
                case 10:
                    player->animation = ANIM_DEFAULT;
                    break;
            }
            break;

        case ANIM_MISSILE:
            player->anim_frame += 1;
            switch (player->anim_frame) {
                case 0:
                    player->rb.vel = (vec2_t){ 0 };
                    // TODO: change player collider
                    player->sprite = player_spr(luigi_ssp, player->dir);
                    player->rb.max_fall = 20;
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
                    break;
                case 9:
                    player->rb.vel.x = player->dir * player->max_speed * 5;
                    player->rb.max_fall = 80;
                    break;
                case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23: case 24: case 25:
                    (void)0;
                    bool hit = hurtbox(player, &(collider_t){.pos = {player->rb.col.pos.x, player->rb.col.pos.y}, .extent = {11, 5}}, &(vec2_t){player->dir * 5000, -1000}, dt, hitboxes, hitboxes_len);
                    
                    if (hit) {
                        player->anim_frame += 3;
                        // TODO: lockout framse
                        player->rb.vel = (vec2_t){ 0 };
                    }
                    break;
                case 26: case 27: case 28:
                    break;
                case 29:
                    player->animation = ANIM_DEFAULT;
                    player->rb.max_fall = 400;
                    break;
            }
            break;
    }
}

void player_attackupdate(player_t *player, input_t *input, input_t* last_input, float dt, player_t* hitboxes, size_t hitboxes_len) {\

    if (player->state == PLAYER_STATE_LOCKOUT) {
        if (player->lockout_frames <= 0)
            player->state = PLAYER_STATE_ACTIONABLE;
        else
            player->lockout_frames -= 1;
    }

    switch (player->charac) {
        case PLAYER_OIRAM:
            oiram_au(player, input, last_input, dt, hitboxes, hitboxes_len, false);
            break;
        case PLAYER_MARIO:
            oiram_au(player, input, last_input, dt, hitboxes, hitboxes_len, true);
            break;
        case PLAYER_LUIGI:
            luigi_au(player, input, last_input, dt, hitboxes, hitboxes_len);
            break;

    }
    
}

void player_draw(player_t *player) {
    rb_t *rb = &player->rb;
    gfx_TransparentSprite(player->sprite, rb->col.pos.x - player->sprite->width / 2, rb->col.pos.y - player->sprite->height / 2);
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
