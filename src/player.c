#include <graphx.h>
#include <debug.h>
#include <math.h>
#include "player.h"
#include "colors.h"
#include "gfx/gfx.h"

#ifndef NDEBUG
#define HURTBOXES_MAX (128)
box_t hurtboxes[HURTBOXES_MAX];
size_t hurtboxes_len;
#endif /* NDEBUG */

#define ATTACK_PERCENT_SCALE (100)

void player_set_charac(player_t *player, player_char_t charac) {
    switch (charac) {
        case PLAYER_OIRAM:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .box = {
                            .pos = {150, 30},
                            .extent = {16 / 2, 27.0f / 2}
                        },
                        .friction = 1.0f
                    },
                    .resistance = 0.1f,
                    .inv_mass = 1 / 2.0f,
                    .max_fall = 400
                },
                // TESTING
                .animations = luigi_animations,
                .current_animation = ANIM_NEUTRAL,

                .charac = PLAYER_OIRAM,
                .dir = DIR_RIGHT,
                .max_speed = 175,
                .ground_accel = 2500,
                .air_accel = 800,
                .jump_vel = -400,
                .can_grab_ledge = true,
            };
            break;
        
        case PLAYER_MARIO:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .box = {
                            .pos = {170, 30},
                            .extent = {16 / 2, 27.0f / 2}
                        },
                        .friction = 1.0f
                    },
                    .resistance = 0.1f,
                    .inv_mass = 1 / 2.0f,
                    .max_fall = 400
                },
                // TESTING
                .animations = luigi_animations,
                .current_animation = ANIM_NEUTRAL,

                .charac = PLAYER_MARIO,
                .dir = DIR_RIGHT,
                .max_speed = 175,
                .ground_accel = 2500,
                .air_accel = 800,
                .jump_vel = -400,
                .can_grab_ledge = true,
            };
            break;
        
        case PLAYER_LUIGI:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .box = {
                            .pos = {170, 30},
                            .extent = {luigi_neu_r_width / 2.0f, luigi_neu_r_height / 2.0f}
                        },
                        .friction = 0.3f
                    },
                    .resistance = 0.1f,
                    .inv_mass = 1 / 2.0f,
                    .max_fall = 400
                },
                .animations = luigi_animations,
                .current_animation = ANIM_NEUTRAL,

                .charac = PLAYER_LUIGI,
                // .sprite = luigi_neu_r,
                .dir = DIR_RIGHT,
                .max_speed = 115,
                .ground_accel = 2200,
                .air_accel = 800,
                .jump_vel = -400,
                .can_grab_ledge = true,
            };
            break;
    }
}

void player_jump(player_t *player) {
    // player->rb.vel.y = player->jump_vel;
    // animation handles this now
    player_set_anim(player, ANIM_JUMP, false);

    player->can_grab_ledge = true;
    if (player->rb.grounded || player->grabbed_ledge)
        return;
    player->jumps++;
}

void player_update(player_t *player, input_t *input, input_t* last_input, float dt) {
    float accel;
    float max_speed;

    if (player->iframes > 0)
        player->iframes--;

    if (!input)
        return;

    max_speed = player->max_speed;
    if (player->rb.grounded) {
        accel = player->ground_accel * dt;
        // dbg_printf("grounded\n");
    } else {
        accel = player->air_accel * dt;
        // dbg_printf("not grounded\n");
    }

    if (player->state != PLAYER_STATE_LOCKOUT) {
        if (input->move.x < 0) {
            if (player->rb.vel.x > -max_speed)
                player->rb.vel.x += accel * input->move.x;
        } else {
            if (player->rb.vel.x < max_speed)
                player->rb.vel.x += accel * input->move.x;
        }

        if (player->rb.grounded || player->grabbed_ledge) {
            player->jumps = 0;
        } else if (player->rb.grounded) {
            player->can_grab_ledge = true;
        } else if (player->can_grab_ledge && player->rb.vel.y > 0) { // if falling, check for ledge grab
            for (int i = 0; i < PHY_LEDGES_LEN; i++) {
                if (!phy_ledges[i])
                    continue;
                ledge_t* ledge = phy_ledges[i];
                if (player->dir != ledge->grab_dir)
                    continue;
                if (phy_box_overlap(player->rb.col.box, ledge->box) == false)
                    continue;
                // we want to have our head fallen past the top of the ledge:
                if (phy_box_top(player->rb.col.box) <= phy_box_top(ledge->box))
                    continue;
                
                // we can grab the ledge
                player->grabbed_ledge = ledge;
                player_set_anim(player, ANIM_LEDGE_GRAB, false);
                dbg_printf("grabbed ledge\n");
            }
        }

        if (input->jump && !last_input->jump && player->state == PLAYER_STATE_ACTIONABLE && (player->rb.grounded || player->jumps < 1)) {
            player_jump(player);
        }
    }
    
    if (player->rb.col.box.pos.y > 280)
        player_set_charac(player, player->charac);
}

void player_lateupdate(player_t *player, input_t *input, input_t* last_input, float dt) {
    rb_t* rb = &player->rb;
    ledge_t* ledge = player->grabbed_ledge;
    if (ledge) {

        rb->vel = (vec2_t) {0};
        // allign the right and top of boxes
        rb->col.box.pos.y = phy_box_top(ledge->box) + rb->col.box.extent.y + 1;
        if (ledge->grab_dir > 0) {
            rb->col.box.pos.x = phy_box_right(ledge->box) - rb->col.box.extent.x - 1;
        } else {
            rb->col.box.pos.x = phy_box_left(ledge->box) + rb->col.box.extent.x + 1;
        }
        player->dir = player->grabbed_ledge->grab_dir;
    }
}

bool player_hurtbox(player_t *player, box_t* box, vec2_t* kb, int damage, player_t* hitboxes, size_t hitboxes_len, int iframes) {
    #ifndef NDEBUG
    if (hurtboxes_len < HURTBOXES_MAX) {
        hurtboxes[hurtboxes_len++] = *box;
    }
    #endif /* NDEBUG */

    bool hit = false;
    
    for (size_t i = 0; i < hitboxes_len; i++) {
        collider_t *hitbox = &hitboxes[i].rb.col;
        if (phy_box_overlap(*box, hitbox->box)) {
            if (player == &hitboxes[i])
                continue;
            if (player->iframes > 0)
                continue;

            hitboxes[i].damage_percent += damage;

            float base_kb = ATTACK_PERCENT_SCALE * (hitboxes[i].damage_percent + 10);
            vec2_t total_kb = vec_AddMagnitude(*kb, base_kb);
            // this is not great, maybe option to not have it
            total_kb.x *= player->dir;

            phy_add_force(&hitboxes[i].rb, total_kb);
            player->iframes = iframes;
            
            hit = true;
        }
    }

    return hit;
}

static void anim_process_actions(player_t *player, keyframe_t *frame, input_t *input, input_t *last_input, player_t* hitboxes, int num_hitboxes) {
    // run all the things that happen in this keyframe
    for (int i = 0; i < frame->num_actions; i++) {
        frame_data_t* action = frame->frame_actions + i;

        switch (action->type) {
            case FRAME_CUSTOM_FUNC:
                if (action->data.custom_function(player, input, last_input, hitboxes, num_hitboxes))
                    return;
                break;
            case FRAME_HURTBOX:
                (void)0;
                vec2_t pos = {
                    player->rb.col.box.pos.x + action->data.hurtbox.box.pos.x * player->dir,
                    player->rb.col.box.pos.y + action->data.hurtbox.box.pos.y
                };
                if (player_hurtbox(player, &(box_t){.pos = pos, .extent = action->data.hurtbox.box.extent },
                    &action->data.hurtbox.kb, action->data.hurtbox.damage, hitboxes, num_hitboxes, 1)
                    && action->data.hurtbox.on_hit != NULL)
                    action->data.hurtbox.on_hit(player);
                break;
            case FRAME_SET_VELOCITY:
                // TODO: direction weird
                player->rb.vel.x = action->data.player_velocity.x * player->dir;
                player->rb.vel.y = action->data.player_velocity.y;
                break;
            case FRAME_SET_MAXFALL:
                player->rb.max_fall = action->data.max_fall;
                break;
            case FRAME_SET_ANIMATION:
                player->current_animation = action->data.next_anim;
                player->anim_frame = 0;
                player->anim_keyframe = 0;
                return; // return because animation has changed
            case FRAME_SET_SPRITE:
                player->sprite = player->dir > 0 ? action->data.sprite.right : action->data.sprite.left;
                player->sprite_offset = action->data.sprite.offset;
                break;
        }
    }
}

#ifndef NDEBUG
void player_anim_run_keyframe(player_t* player, input_t* input, input_t* last_input, animation_t* anim, player_t* hitboxes, int num_hitboxes, bool increment) {
#else
void player_anim_run_keyframe(player_t* player, input_t* input, input_t* last_input, animation_t* anim, player_t* hitboxes, int num_hitboxes) {
#endif

    // get the next keyframe
    // this could probably totally access out of bounds memory but ill only worry about that if it becomes a problem
    keyframe_t* frame = &anim->frames[player->anim_keyframe];

    // check if the keyframe is the current frame
    if (
    #ifndef NDEBUG
        // increment &&
    #endif 
        (player->anim_keyframe >= anim->num_keyframes ||
        (frame->duration != -1 && (player->anim_frame < frame->frame_number ||
        player->anim_frame >= frame->frame_number + frame->duration )))) {
        // frame is a regular frame, return

        #ifndef NDEBUG
        if (increment) {
        #endif
            player->anim_frame++;

            if (player->anim_frame >= anim->total_frames) { // reset if at end
                player->anim_frame = 0;
                player->anim_keyframe = 0;
                player->current_animation = ANIM_NEUTRAL;
            }
        #ifndef NDEBUG
        }
        #endif
        
        return;
        
    }

    #ifndef NDEBUG
    // if (player->anim_keyframe < anim->num_keyframes)
    #endif
        anim_process_actions(player, frame, input, last_input, hitboxes, num_hitboxes);

    #ifndef NDEBUG
    if (increment) {
    #endif
        // frame is a keyframe, increment both 
        
        if (frame->duration != -1) {
            player->anim_frame++;

            // add to the keyframe if the next frame is above frme number + duration
            if (player->anim_frame >= frame->frame_number + frame->duration && frame->duration)
                player->anim_keyframe++;

            // reset if at end
            if (player->anim_frame >= anim->total_frames) {
                player->anim_frame = 0;
                player->anim_keyframe = 0;
                player->current_animation = ANIM_NEUTRAL;
            }
        }
    #ifndef NDEBUG
    }
    #endif
}

void player_set_anim(player_t* player, animation_type_t anim, bool lockout) {
    player->current_animation = anim;

    if (lockout) {
        player->state = PLAYER_STATE_LOCKOUT;
        player->lockout_frames = player->animations[anim]->total_frames;
    } else 
        player->lockout_frames = 0;

    player->anim_frame = 0;
    player->anim_keyframe = 0;
}


void player_attackupdate(player_t *player, input_t *input, input_t* last_input, float dt, player_t* hitboxes, size_t hitboxes_len) {\

    if (player->state == PLAYER_STATE_LOCKOUT) {
        if (player->lockout_frames <= 0)
            player->state = PLAYER_STATE_ACTIONABLE;
        else
            player->lockout_frames -= 1;
    }

    #ifndef NDEBUG
    player_anim_run_keyframe(player, input, last_input, player->animations[player->current_animation], hitboxes, hitboxes_len, true);
    #else
    player_anim_run_keyframe(player, input, last_input, player->animations[player->current_animation], hitboxes, hitboxes_len);
    #endif
    
}

void player_draw_pos(player_t *player, vec2_t *pos) {
    gfx_TransparentSprite(player->sprite, pos->x - player->sprite->width / 2 + player->sprite_offset.x, pos->y - player->sprite->height / 2 + player->sprite_offset.y);
}

void player_draw(player_t *player) {
    rb_t *rb = &player->rb;
    vec2_t pos = {rb->col.box.pos.x, rb->col.box.pos.y};
    player_draw_pos(player, &pos);
}

#ifndef NDEBUG
void player_dbg_newframe() {
    hurtboxes_len = 0;
}

#define scaled_distance(value, middle, scale) (middle + (value - middle) * scale)

void player_dbg_drawboxes(player_t* hitboxes, size_t hitboxes_len, uint8_t scale) {
    gfx_SetColor(COLOR_DBG_HITBOX);
    for (size_t i = 0; i < hitboxes_len; i++) {
        collider_t* col = &hitboxes[i].rb.col;
        gfx_Rectangle(scaled_distance(col->box.pos.x, 160, scale) - col->box.extent.x * scale, scaled_distance(col->box.pos.y, 120, scale) - col->box.extent.y * scale, col->box.extent.x * 2 * scale, col->box.extent.y * 2 * scale);
    }
    gfx_SetColor(COLOR_DBG_HURTBOX);
    for (size_t i = 0; i < hurtboxes_len; i++) {
        gfx_Rectangle(scaled_distance(hurtboxes[i].pos.x, 160, scale) - hurtboxes[i].extent.x * scale, scaled_distance(hurtboxes[i].pos.y, 120, scale) - hurtboxes[i].extent.y * scale, hurtboxes[i].extent.x * 2 * scale, hurtboxes[i].extent.y * 2 * scale);
    }
}

void player_dbg_draw_scaled(player_t *player, vec2_t *pos, uint8_t scale) {
    gfx_ScaledTransparentSprite_NoClip(player->sprite, pos->x - (player->sprite->width / 2 + player->sprite_offset.x) * scale, pos->y - (player->sprite->height / 2 + player->sprite_offset.y) * scale, scale, scale);
}
#endif /* NDEBUG */
