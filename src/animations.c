#include "player.h"

/* ----- animation definitions ----- */

/* luigi */

void luigi_missile_hit(void* p) {
    player_t* player = p;
    player->anim_frame += 4;
    player->rb.vel = (vec2_t){ 0 };
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
        .box = {.extent = {11, 5}, .pos = { 0 }}, .damage = 10, .kb = { 10000, 200 }}},
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
frame_data_t l_jab_kf1[] = {{ .type = FRAME_SET_SPRITE, .data.sprite = both_sprites(luigi_att) } };
frame_data_t l_jab_kf2[] = {{ .type = FRAME_HURTBOX, .data.hurtbox = { .on_hit = NULL,
        .box = {.extent = {8, 4}, .pos = { 9, 0 }}, .damage = 1, .kb = { 300, -50 }} } };


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


animation_t* luigi_animations[] = { NULL, &luigi_jab, NULL, NULL, NULL, NULL, NULL, NULL, &luigi_missile, NULL, NULL, NULL };




