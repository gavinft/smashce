#include "player.h"
#include <debug.h>
/* ----- animation definitions ----- */

#define same_dir(a, b) ((a < 0 && b < 0) || (a > 0 && b > 0))


static void side_special_attack_update_direction(player_t *player, input_t *input) {
    if (input->move.x > ATTACK_DIR_DEADZONE)
        player->dir = DIR_RIGHT;
    else if (input->move.x < ATTACK_DIR_DEADZONE)
        player->dir = DIR_LEFT;
    
}

static bool neutral_scan_attacks(player_t* player, input_t* input, input_t* last_input) {

    // NORMAL ATTACK
    if (input->attack && !last_input->attack && player->rb.grounded) {
        player_set_anim(player, ANIM_ATTACK, true);
        return true;
    }

    // AERIAL
    if (input->attack && !last_input->attack && !player->rb.grounded) {

        float abs_x = fabsf(input->move.x);

        // SIDE
        if (abs_x > ATTACK_DIR_DEADZONE && abs_x > fabsf(input->move.y)) {
            if (same_dir(input->move.x, player->dir))
                player_set_anim(player, ANIM_AIR_FWD, false);
            else 
                player_set_anim(player, ANIM_AIR_BCK, false);
        }

        // UP
        else if (input->move.y > ATTACK_DIR_DEADZONE)
            player_set_anim(player, ANIM_AIR_UP, false);

        // DOWN
        else if (input->move.y < -ATTACK_DIR_DEADZONE)
            player_set_anim(player, ANIM_AIR_DWN, false);

        // NEUTRAL
        else if (abs_x < ATTACK_DIR_DEADZONE)
            player_set_anim(player, ANIM_AIR_NEU, false);


        return true;
    }


    // SPECIAL
    if (input->special && !last_input->special) {

        float abs_x = fabsf(input->move.x);

        // SIDE
        if (abs_x > ATTACK_DIR_DEADZONE && abs_x > fabsf(input->move.y)) {
            player_set_anim(player, ANIM_SP_SIDE, true);
            side_special_attack_update_direction(player, input);
        }

        // UP
        else if (input->move.y > ATTACK_DIR_DEADZONE)
            player_set_anim(player, ANIM_SP_UP, true);

        // DOWN
        else if (input->move.y < -ATTACK_DIR_DEADZONE)
            player_set_anim(player, ANIM_SP_DWN, true);
        
        return true;
    }


    if (player->rb.grounded) {
        if (input->move.x < -TURN_DEADZONE)
            player->dir = DIR_LEFT;
        else if (input->move.x > TURN_DEADZONE)
            player->dir = DIR_RIGHT;
    }

    return false;
}

// for use in every neutral anim
#define ACTION_SCAN_INPUT (frame_data_t){ .type = FRAME_CUSTOM_FUNC, .data.custom_function = neutral_scan_attacks }


// leaving ledge
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

static bool ledge_check_leave(player_t* player, input_t* input, input_t* last_input) {
    if (try_leave_ledge(player, input, last_input)) {
        player_set_anim(player, ANIM_NEUTRAL, false);
        return true;
    }

    return false;
}

// for use in every ledge grab anim
#define ACTION_LEDGE_LEAVE (frame_data_t){ .type = FRAME_CUSTOM_FUNC, .data.custom_function = ledge_check_leave }


/* luigi */

frame_data_t l_neutral_kf0[] = {
    ACTION_SCAN_INPUT, 
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_neu) },
    { .type = FRAME_SET_MAXFALL, .data.max_fall = 400 }
};

keyframe_t l_neutral_keyframes[] = { { .frame_number = 0, .duration = -1, .num_actions = 3, .frame_actions = l_neutral_kf0 } };

animation_t luigi_neutral = {
    .total_frames = 1,
    .num_keyframes = 1,
    .frames = l_neutral_keyframes
};

// //
frame_data_t l_ledge_kf0[] = {
    ACTION_LEDGE_LEAVE, 
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_lg) },
};

keyframe_t l_ledge_keyframes[] = { { .frame_number = 0, .duration = -1, .num_actions = 2, .frame_actions = l_ledge_kf0 } };

animation_t luigi_ledge_grab = {
    .total_frames = 1,
    .num_keyframes = 1,
    .frames = l_ledge_keyframes
};

// //
static void luigi_missile_hit(player_t* p) {
    p->anim_frame += 4;
    p->rb.vel = (vec2_t){ 0 };
}

frame_data_t l_missile_kf0[] = {
    { .type = FRAME_SET_VELOCITY, .data.player_velocity = { 0 } },
    { .type = FRAME_SET_MAXFALL, .data.max_fall = 20 },
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_ssp) }
};
frame_data_t l_missile_kf1[] = {
    { .type = FRAME_SET_VELOCITY, .data.player_velocity = { 400, 0 } },
    { .type = FRAME_SET_MAXFALL, .data.max_fall = 80 }
};
frame_data_t l_missile_kf2[] = {
    { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = luigi_missile_hit,
        .box = {.extent = {11, 5}, .pos = { 0 }}, .damage = 12, .kb = { 10000, 200 }}},
};

keyframe_t l_missile_keyframes[] = {
    { .frame_number = 0, .duration = 1, .num_actions = 3, .frame_actions = l_missile_kf0 },
    { .frame_number = 9, .duration = 1, .num_actions = 2, .frame_actions = l_missile_kf1 },
    { .frame_number = 10, .duration = 15, .num_actions = 1, .frame_actions = l_missile_kf2 },
};

animation_t luigi_missile = {
    .total_frames = 30,
    .num_keyframes = 3,
    .frames = l_missile_keyframes
};

// //
// frame_data_t l_jab_kf0[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_att) }; }
frame_data_t l_jab_kf1[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_att) } };
frame_data_t l_jab_kf2[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {8, 4}, .pos = { 9, 0 }}, .damage = 2, .kb = { 300, -50 }} } };


keyframe_t l_jab_keyframes[] = {
    { .frame_number = 3, .duration = 1, .num_actions = 1, .frame_actions = l_jab_kf1 },
    { .frame_number = 4, .duration = 4, .num_actions = 1, .frame_actions = l_jab_kf2 }
};

animation_t luigi_jab = {
    .total_frames = 10,
    .num_keyframes = 2,
    .frames = l_jab_keyframes
};

// //
frame_data_t l_fair_kf0[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_fair) } };
frame_data_t l_fair_kf1[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {6, 6}, .pos = { 6, -6 }}, .damage = 9, .kb = { 300, 400 }} } };

keyframe_t l_fair_keyframes[] = {
    { .frame_number = 4, .duration = 1, .num_actions = 1, .frame_actions = l_fair_kf0 },
    { .frame_number = 5, .duration = 2, .num_actions = 1, .frame_actions = l_fair_kf1 }
};

animation_t luigi_forward_air = {
    .total_frames = 16, 
    .num_keyframes = 2,
    .frames = l_fair_keyframes
};

// //
frame_data_t l_bair_kf0[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_bair) } };
frame_data_t l_bair_kf1[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {8, 6}, .pos = { -6, 3 }}, .damage = 8, .kb = { -2000, 100 }} } };


keyframe_t l_bair_keyframes[] = {
    { .frame_number = 3, .duration = 1, .num_actions = 1, .frame_actions = l_bair_kf0 },
    { .frame_number = 4, .duration = 5, .num_actions = 1, .frame_actions = l_bair_kf1 }
};

animation_t luigi_back_air = {
    .total_frames = 14,
    .num_keyframes = 2,
    .frames = l_bair_keyframes
};

// //
frame_data_t l_upsp_kf0[] = {
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_usp) },
    { .type = FRAME_SET_VELOCITY, .data.player_velocity = (vec2_t){ 0, -400 } }
};
frame_data_t l_upsp_kf1[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {6, 10}, .pos = { 6, -3 }}, .damage = 12, .kb = { 200, -3000 }} } };
frame_data_t l_upsp_kf2[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_neu) } };

keyframe_t l_upsp_keyframes[] = {
    { .frame_number = 3, .duration = 1, .num_actions = 2, .frame_actions = l_upsp_kf0 },
    { .frame_number = 4, .duration = 8, .num_actions = 1, .frame_actions = l_upsp_kf1 },
    { .frame_number = 15, .duration = 1, .num_actions = 1, .frame_actions = l_upsp_kf2 }
};

animation_t luigi_up_special = {
    .total_frames = 25,
    .num_keyframes = 3,
    .frames = l_upsp_keyframes
};

// //
frame_data_t l_uair_kf0[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_uair) } };
frame_data_t l_uair_kf1[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {10, 5}, .pos = { 0, -10 }}, .damage = 9, .kb = { 1000, -1000 }} } };
frame_data_t l_uair_kf2[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_neu) } };

keyframe_t l_uair_keyframes[] = {
    { .frame_number = 2, .duration = 1, .num_actions = 1, .frame_actions = l_uair_kf0 },
    { .frame_number = 3, .duration = 2, .num_actions = 1, .frame_actions = l_uair_kf1 },
    { .frame_number = 8, .duration = 1, .num_actions = 1, .frame_actions = l_uair_kf2 }
};

animation_t luigi_up_air = {
    .total_frames = 16,
    .num_keyframes = 3,
    .frames = l_uair_keyframes
};

// //
frame_data_t l_dair_kf0[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_dair) } };
frame_data_t l_dair_kf1[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {5, 8}, .pos = { 4, 16 }}, .damage = 10, .kb = { 300, 2000 }} } };


keyframe_t l_dair_keyframes[] = {
    { .frame_number = 4, .duration = 1, .num_actions = 1, .frame_actions = l_dair_kf0 },
    { .frame_number = 5, .duration = 3, .num_actions = 1, .frame_actions = l_dair_kf1 }
};

animation_t luigi_down_air = {
    .total_frames = 16,
    .num_keyframes = 2,
    .frames = l_dair_keyframes
};

// //
frame_data_t l_nair_kf0[] = { { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_nair) } };
frame_data_t l_nair_kf1[] = { { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {8, 4}, .pos = { 4, 16 }}, .damage = 10, .kb = { 1200, 600 }} } };


keyframe_t l_nair_keyframes[] = {
    { .frame_number = 1, .duration = 1, .num_actions = 1, .frame_actions = l_nair_kf0 },
    { .frame_number = 2, .duration = 14, .num_actions = 1, .frame_actions = l_nair_kf1 }
};

animation_t luigi_neu_air = {
    .total_frames = 23,
    .num_keyframes = 2,
    .frames = l_nair_keyframes
};

// //

static bool down_b_mash(player_t* player, input_t* input, input_t* last_input) {

    if (input->special && !last_input->special)
        // phy_add_force(&player->rb, (vec2_t) {0, -100});
        player->rb.col.box.pos.y -= 4;

    return false;

}

frame_data_t l_dsp_kf0[] = {
    { .type = FRAME_SET_SPRITE, .data.sprite = sprites_offset(luigi_dsp2, 0, -1) },
    { .type = FRAME_SET_MAXFALL, .data.max_fall = 10 },
    { .type = FRAME_SET_VELOCITY, .data.player_velocity = {0}}
};
frame_data_t l_dsp_kf1[] = {
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_dsp) },
    { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL, .box = {.extent = {8, 5}, .pos = { 8, -3 }}, .damage = 10, .kb = { 0, -800 }} },
    { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL, .box = {.extent = {8, 5}, .pos = { -8, -3 }}, .damage = 10, .kb = { 0, -800 }} },
    { .type = FRAME_CUSTOM_FUNC, .data.custom_function = down_b_mash }
};
frame_data_t l_dsp_kf2[] = { { .type = FRAME_SET_SPRITE, .data.sprite = sprites_offset(luigi_dsp2, 0, -1) } };
frame_data_t l_dsp_kf3[] = {
    { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL, .box = {.extent = {8, 5}, .pos = { 8, -5 }}, .damage = 10, .kb = { 0, -800 }} },
    { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL, .box = {.extent = {8, 5}, .pos = { -8, -5 }}, .damage = 10, .kb = { 0, -800 }} },
    { .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL, .box = {.extent = {8, 4}, .pos = { 0, 16 }}, .damage = 10, .kb = { 0, 1200 }} },
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_dsp3) }
};
frame_data_t l_dsp_kf4[] = { 
    { .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_neu) },
    { .type = FRAME_SET_MAXFALL, .data.max_fall = 20 },
};

keyframe_t l_dsp_keyframes[] = {
    { .frame_number = 0, .duration = 1, .num_actions = 3, .frame_actions = l_dsp_kf0 },
    { .frame_number = 4, .duration = 20, .num_actions = 4, .frame_actions = l_dsp_kf1 },
    { .frame_number = 24, .duration = 1, .num_actions = 1, .frame_actions = l_dsp_kf2 },
    { .frame_number = 30, .duration = 2, .num_actions = 4, .frame_actions = l_dsp_kf3 },
    { .frame_number = 35, .duration = 1, .num_actions = 2, .frame_actions = l_dsp_kf4 },
    // { .frame_number = 34, .duration = 1, .num_actions = 1, .frame_actions = l_dsp_kf4 },
};

animation_t luigi_down_special = {
    .total_frames = 42,
    .num_keyframes = 5,
    .frames = l_dsp_keyframes
};

animation_t* luigi_animations[] = {
    &luigi_neutral, &luigi_ledge_grab, &luigi_jab,
    &luigi_neu_air, &luigi_forward_air, &luigi_back_air,
    &luigi_up_air, &luigi_down_air, NULL, 
    &luigi_missile, &luigi_up_special, &luigi_down_special
};

