#ifndef PLAYER_H
#define PLAYER_H

#include "physics.h"
#include "input.h"

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

typedef struct {
    rb_t rb;
    gfx_sprite_t* sprite;
    vec2_t sprite_offset;
    player_char_t charac;
    player_state_t state;
    direction_t dir;
    float max_speed;
    float ground_accel;
    float air_accel;
    float jump_vel;
    int damage_percent;
    int jumps;
    int lockout_frames;
    int animation;
    int anim_frame;
} player_t;

typedef struct {
    collider_t box;
    player_t* attached;
} hitbox_t;

#define player_spr(name, dir) ((dir) > 0 ? name ## _r : name ## _l)

void player_load_sprites();
void player_set_charac(player_t *player, player_char_t charac);
void player_update(player_t *player, input_t *input, input_t* last_input, float dt);
void player_lateupdate(player_t *player, input_t *input, input_t* last_input, float dt);
void player_attackupdate(player_t *player, input_t *input, input_t* last_input, float dt, player_t* hitboxes, size_t hitboxes_len);
void player_draw(player_t *player);

#ifndef NDEBUG
void player_dbg_newframe();
void player_dbg_drawboxes(player_t* hitboxes, size_t hitboxes_len);
#endif /* NDEBUG */

#endif /* PLAYER_H */