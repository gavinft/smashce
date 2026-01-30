#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <graphx.h>
#include "physics.h"
#include "vec.h"

struct player;
struct input;

typedef struct {
    box_t box; // pos stored as offset
    vec2_t kb;
    int damage;
    void (*on_hit)(struct player*); // void* is player again
} hurtbox_data_t;

typedef enum {
    ANIM_NEUTRAL,
    ANIM_LEDGE_GRAB,
    ANIM_ATTACK,
    ANIM_AIR_NEU,
    ANIM_AIR_FWD,
    ANIM_AIR_BCK,
    ANIM_AIR_UP,
    ANIM_AIR_DWN,
    ANIM_SP_NEU,
    ANIM_SP_SIDE,
    ANIM_SP_UP,
    ANIM_SP_DWN
} animation_type_t;

typedef enum {
    FRAME_CUSTOM_FUNC,
    FRAME_SET_VELOCITY,
    FRAME_SET_MAXFALL,
    FRAME_SET_SPRITE,
    FRAME_SET_ANIMATION,
    FRAME_HURTBOX,
} frame_data_type_t;

typedef struct {
    gfx_sprite_t* left;
    gfx_sprite_t* right;
    vec2_t offset;
} lr_sprite_t;

typedef struct {

    union {
        bool (*custom_function)(struct player*, struct input*, struct input*, struct player*, size_t); // returns whether or not animation ends early
        vec2_t player_velocity;
        float max_fall;
        lr_sprite_t sprite;
        hurtbox_data_t hurtbox;
        animation_type_t next_anim;
    } data;

    frame_data_type_t type;

} frame_data_t;

typedef struct {

    frame_data_t* frame_actions;
    int duration; // set to -1 for infinite
    int num_actions;
    int frame_number;

} keyframe_t;


typedef struct {
    keyframe_t* frames;
    int total_frames;
    int num_keyframes;
} animation_t;

#define both_sprites(name) {.left = (gfx_sprite_t*)name##_l_data, .right = name ## _r}
#define sprites_offset(name, x, y) {.left = (gfx_sprite_t*)name##_l_data, .right = name ## _r, .offset = {x, y}}

extern animation_t* luigi_animations[];

void animation_load_sprites();

#endif