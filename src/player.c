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

flippable_duplicate(oiram_neu);
flippable_duplicate(mario_neu);

flippable_duplicate(luigi_neu);
flippable_duplicate(luigi_att);
flippable_duplicate(luigi_ssp);
flippable_duplicate(luigi_lg);
flippable_duplicate(luigi_fair);


void player_load_sprites() {
    flip(oiram_neu);
    flip(mario_neu);

    flip(luigi_neu);

    flip(luigi_att);
    flip(luigi_ssp);
    flip(luigi_lg);
    flip(luigi_fair);
}

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
                .charac = PLAYER_OIRAM,
                .dir = DIR_RIGHT,
                .sprite = oiram_neu_r,
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
                .charac = PLAYER_MARIO,
                .sprite = mario_neu_r,
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
                        .friction = 0.8f
                    },
                    .resistance = 0.1f,
                    .inv_mass = 1 / 2.0f,
                    .max_fall = 400
                },
                .animations = luigi_animations,
                .current_animation = ANIM_NEUTRAL,

                .charac = PLAYER_LUIGI,
                .sprite = luigi_neu_r,
                .dir = DIR_RIGHT,
                .max_speed = 175,
                .ground_accel = 2500,
                .air_accel = 800,
                .jump_vel = -400,
                .can_grab_ledge = true,
            };
            break;
    }
}

static void jump(player_t *player) {
    player->rb.vel.y = player->jump_vel;
    player->can_grab_ledge = true;
    if (player->rb.grounded || player->grabbed_ledge)
        return;
    player->jumps++;
}

void player_update(player_t *player, input_t *input, input_t* last_input, float dt) {
    float accel;
    float max_speed;

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
                player->animation_tmp = -1; // ledgegrab animation_tmp
                player->anim_frame = -1;
                dbg_printf("grabbed ledge\n");
            }
        }

        if (input->jump && !last_input->jump && player->state == PLAYER_STATE_ACTIONABLE && (player->rb.grounded || player->jumps < 1)) {
            jump(player);
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

static void side_special_attack_update_direction(player_t *player, input_t *input) {
    if (input->move.x > ATTACK_DIR_DEADZONE) {
        player->dir = DIR_RIGHT;
    } else if (input->move.x < ATTACK_DIR_DEADZONE) {
        player->dir = DIR_LEFT;
    }
}

bool hurtbox(player_t *player, box_t* box, vec2_t* kb, int damage, player_t* hitboxes, size_t hitboxes_len) {
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

            hitboxes[i].damage_percent += damage;

            float base_kb = ATTACK_PERCENT_SCALE * (hitboxes[i].damage_percent + 10);
            vec2_t total_kb = vec_AddMagnitude(*kb, base_kb);
            // this is not great, maybe option to not have it
            total_kb.x *= player->dir;

            phy_add_force(&hitboxes[i].rb, total_kb);
            
            hit = true;
        }
    }

    return hit;
}

static bool try_leave_ledge(player_t *player, input_t *input, input_t *last_input) {
    if (input->jump && !last_input->jump) {
        jump(player);
        player->grabbed_ledge = NULL;
        dbg_printf("jumped out of ledge\n");
        return true;
    } else if (input->move.y < -ATTACK_DIR_DEADZONE && last_input->move.y >= -ATTACK_DIR_DEADZONE) {
        player->grabbed_ledge = NULL;
        player->can_grab_ledge = false;
        dbg_printf("dropped ledge\n");
        return true;
    }
    return false;
}

void anim_run_keyframe(player_t* player, animation_t* anim, player_t* hitboxes, int num_hitboxes) {

    // get the next keyframe
    keyframe_t* frame = &anim->frames[player->anim_keyframe];

    // run the kf if the anim frame is between kf and kf+dur

    // if (player->anim_frame >= frame->frame_number &&
    //     player->anim_frame < frame->frame_number + frame->duration)


    // check if the keyframe is the current frame
    if (player->anim_frame < frame->frame_number ||
        player->anim_frame >= frame->frame_number + frame->duration ||
        player->anim_keyframe >= anim->num_keyframes) {
        // frame is a regular frame, return
        dbg_printf("no keyframe on frame %d, keyframe %d, kffn %d\n", player->anim_frame, player->anim_keyframe, frame->frame_number);

        player->anim_frame++;

        if (player->anim_frame >= anim->total_frames) { // reset if at end
            player->anim_frame = 0;
            player->anim_keyframe = 0;
            player->current_animation = ANIM_NEUTRAL;
        }

        return;
    }

    // run all the things that happen in this keyframe
    for (int i = 0; i < frame->num_actions; i++) {
        frame_data_t* action = frame->frame_actions + i;

        dbg_printf("new action (%d) on frame: %d (%d) kf: %d\n\n",
            action->type, player->anim_frame, frame->frame_number, player->anim_keyframe);

        switch (action->type) {
            case FRAME_CUSTOM_FUNC:
                action->data.custom_function(player);
                break;
            case FRAME_HURTBOX:
                (void)0;
                vec2_t pos = {
                    player->rb.col.box.pos.x + action->data.hurtbox.box.pos.x * player->dir,
                    player->rb.col.box.pos.y + action->data.hurtbox.box.pos.y
                };
                if (hurtbox(player, &(box_t){.pos = pos, .extent = action->data.hurtbox.box.extent },
                    &action->data.hurtbox.kb, action->data.hurtbox.damage, hitboxes, num_hitboxes)
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
            case FRAME_SET_ANIMATION: // maybe this is not a thing and is instead always set to neutral at end
                player->current_animation = action->data.next_anim;
                break;
            case FRAME_SET_SPRITE:
                player->sprite = player->dir > 0 ? action->data.sprite.right : action->data.sprite.left;
                break;
        }

    }

    // frame is a keyframe, increment both 
    player->anim_frame++;

    // add to the keyframe if the next frame is above frme number + duration
    if (player->anim_frame >= frame->frame_number + frame->duration)
        player->anim_keyframe++;

    // reset if at end
    if (player->anim_frame >= anim->total_frames) {
        player->anim_frame = 0;
        player->anim_keyframe = 0;
        player->current_animation = ANIM_NEUTRAL;
    }
    

}

void player_anim_ready(player_t* player, animation_type_t anim, int lockout_frames) {
    player->current_animation = anim;

    if (lockout_frames > 0) {
        player->state = PLAYER_STATE_LOCKOUT;
        player->lockout_frames = 10;
    } else 
        player->lockout_frames = 0;

    player->anim_frame = 0;
    player->anim_keyframe = 0;
}

#define same_dir(a, b) ((a < 0 && b < 0) || (a > 0 && b > 0))

void player_step(player_t* player, input_t* input, input_t* last_input, player_t* hitboxes, int num_hitboxes) {
    if (player->current_animation != ANIM_NEUTRAL && player->current_animation != ANIM_LEDGE_GRAB)
        dbg_printf("stepping on animation %d\n", player->current_animation);

    switch (player->current_animation) {
        
        case ANIM_NEUTRAL:
            // TODO: perhaps treat this as a normal animation with an action for custom function which does most of this and actions that do player specific stuff
            // TODO: account for different normal fall speeds
            player->rb.max_fall = 400;
            // TODO: deal with this (different players exist) (maybe dont)
            player->sprite = player_spr(luigi_neu, player->dir);

            // NORMAL ATTACK
            if (input->attack && !last_input->attack && player->rb.grounded) {
                player_anim_ready(player, ANIM_ATTACK, 10);
                break;
            }

            // SIDE AERIAL
            if (input->attack && !last_input->attack &&
                fabsf(input->move.x) > ATTACK_DIR_DEADZONE && !player->rb.grounded) {

                if (same_dir(input->move.x, player->dir))
                    player_anim_ready(player, ANIM_AIR_FWD, 10);
                // else 
                //     player_anim_ready(player, ANIM_AIR_BCK, 10);

                break;
            }

            // SIDE SPECIAL
            if (input->special && !last_input->special &&
                fabsf(input->move.x) > ATTACK_DIR_DEADZONE) {
                player_anim_ready(player, ANIM_SP_SIDE, 10);
                side_special_attack_update_direction(player, input);
                break;
            }

            if (player->rb.grounded) {
                if (input->move.x < -TURN_DEADZONE)
                    player->dir = DIR_LEFT;
                else if (input->move.x > TURN_DEADZONE)
                    player->dir = DIR_RIGHT;
            }
            break;


        case ANIM_LEDGE_GRAB:

            // TODO: Not sure this works
            // TODO: Deal with different sprites (do as standard animation with custom function to decrement and try leave, as well as change sprite action)
            player->sprite = player_spr(luigi_lg, player->dir);
            // leaving ledge
            if (try_leave_ledge(player, input, last_input)) {
                player->animation_tmp = ANIM_NEUTRAL;
                player->anim_frame = 0;
            }
            break;

        default:
            anim_run_keyframe(player, player->animations[player->current_animation], hitboxes, num_hitboxes);
            return;
    }

}

void player_attackupdate(player_t *player, input_t *input, input_t* last_input, float dt, player_t* hitboxes, size_t hitboxes_len) {\

    if (player->state == PLAYER_STATE_LOCKOUT) {
        if (player->lockout_frames <= 0)
            player->state = PLAYER_STATE_ACTIONABLE;
        else
            player->lockout_frames -= 1;
    }

    player_step(player, input, last_input, hitboxes, hitboxes_len);
    
}

void player_draw(player_t *player) {
    rb_t *rb = &player->rb;
    gfx_TransparentSprite(player->sprite, rb->col.box.pos.x - player->sprite->width / 2 + player->sprite_offset.x, rb->col.box.pos.y - player->sprite->height / 2 + player->sprite_offset.x);
}

#ifndef NDEBUG
void player_dbg_newframe() {
    hurtboxes_len = 0;
}

void player_dbg_drawboxes(player_t* hitboxes, size_t hitboxes_len) {
    gfx_SetColor(COLOR_DBG_HITBOX);
    for (size_t i = 0; i < hitboxes_len; i++) {
        collider_t* col = &hitboxes[i].rb.col;
        gfx_Rectangle(phy_box_left(col->box), phy_box_top(col->box), col->box.extent.x * 2, col->box.extent.y * 2);
    }
    gfx_SetColor(COLOR_DBG_HURTBOX);
    for (size_t i = 0; i < hurtboxes_len; i++) {
        gfx_Rectangle(phy_box_left(hurtboxes[i]), phy_box_top(hurtboxes[i]), hurtboxes[i].extent.x * 2, hurtboxes[i].extent.y * 2);
    }
}
#endif /* NDEBUG */
