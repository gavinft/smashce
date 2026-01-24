#ifndef PLAYER_H
#define PLAYER_H

#include <graphx.h>
#include "physics.h"
#include "input.h"
#include "animations.h"
#include "gfx/gfx.h"

typedef enum {
    PLAYER_OIRAM,
    PLAYER_MARIO,
    PLAYER_LUIGI,
} player_char_t;

typedef enum {
    PLAYER_STATE_ACTIONABLE,
    PLAYER_STATE_LOCKOUT,
    PLAYER_STATE_FREEFALL,
} player_state_t;

typedef struct player {
    rb_t rb;
    gfx_sprite_t* sprite;
    vec2_t sprite_offset;
    player_char_t charac;
    player_state_t state;
    direction_t dir;
    ledge_t *grabbed_ledge;

    animation_t** animations; // array that has length of that enum in animations.h
    animation_type_t current_animation; 
    int anim_keyframe;
    
    float max_speed;
    float ground_accel;
    float air_accel;
    float jump_vel;
    int damage_percent;
    int jumps;
    int lockout_frames;
    int anim_frame;
    bool can_grab_ledge;
    int iframes;
} player_t;

typedef struct {
    collider_t box;
    player_t* attached;
} hitbox_t;

#define player_spr(name, dir) ((dir) > 0 ? name ## _r : name ## _l)

#define flippable_duplicate(name) gfx_UninitedSprite(name ## _l, name ## _r_width, name ## _r_height)
#define flip(name) gfx_FlipSpriteY(name ## _r, name ## _l)

// not a huge fan of these
#define extern_duplicate(name) \
    extern gfx_sprite_t* name ## _l;\
    extern uint8_t name##_l_data[]

extern_duplicate(oiram_neu);
extern_duplicate(mario_neu);

extern_duplicate(luigi_neu);
extern_duplicate(luigi_att);
extern_duplicate(luigi_ssp);
extern_duplicate(luigi_lg);
extern_duplicate(luigi_fair);
extern_duplicate(luigi_bair);
extern_duplicate(luigi_usp);
extern_duplicate(luigi_uair);
extern_duplicate(luigi_dair);
extern_duplicate(luigi_nair);
extern_duplicate(luigi_dsp);
extern_duplicate(luigi_dsp2);
extern_duplicate(luigi_dsp3);


#define TURN_DEADZONE (0.1f)
#define ATTACK_DIR_DEADZONE (0.1f)

void player_load_sprites();
void player_set_charac(player_t *player, player_char_t charac);
void player_update(player_t *player, input_t *input, input_t* last_input, float dt);
void player_lateupdate(player_t *player, input_t *input, input_t* last_input, float dt);
void player_attackupdate(player_t *player, input_t *input, input_t* last_input, float dt, player_t* hitboxes, size_t hitboxes_len);
#ifndef NDEBUG
void player_anim_run_keyframe(player_t* player, input_t* input, input_t* last_input, animation_t* anim, player_t* hitboxes, int num_hitboxes, bool increment);
#else
void player_anim_run_keyframe(player_t* player, input_t* input, input_t* last_input, animation_t* anim, player_t* hitboxes, int num_hitboxes);
#endif
void player_draw(player_t *player);
void player_draw_pos(player_t *player, vec2_t *pos);
bool hurtbox(player_t *player, box_t* box, vec2_t* kb, int damage, player_t* hitboxes, size_t hitboxes_len, int iframes);
void player_set_anim(player_t* player, animation_type_t anim, bool lockout);
void jump(player_t *player);

#ifndef NDEBUG
void player_dbg_newframe();
void player_dbg_drawboxes(player_t* hitboxes, size_t hitboxes_len, uint8_t scale);
void player_dbg_draw_scaled(player_t *player, vec2_t *pos, uint8_t scale);
#endif /* NDEBUG */

#endif /* PLAYER_H */
