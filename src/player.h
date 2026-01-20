#ifndef PLAYER_H
#define PLAYER_H

#include "physics.h"
#include "input.h"

typedef enum {
    PLAYER_OIRAM,
    PLAYER_MARIO,
} player_char_t;

typedef enum {
    PLAYER_STATE_ACTIONABLE,
    PLAYER_STATE_LOCKOUT,
    PLAYER_STATE_FREEFALL,
} player_state_t;

typedef struct {
    rb_t rb;
    player_char_t charac;
    player_state_t state;
    float max_speed;
    float ground_accel;
    float air_accel;
    float jump_vel;
    int jumps;
    int lockout_frames;
    int animation;
    int anim_frame;
} player_t;

typedef struct {
    collider_t box;
    player_t* attached;
} hitbox_t;

void player_set_charac(player_t *player, player_char_t charac);
void player_update(player_t *player, input_t *input, float dt);
void player_lateupdate(player_t *player, input_t *input, float dt);
void player_attackupdate(player_t *player, input_t *input, float dt, player_t* hitboxes, size_t hitboxes_len);
void player_draw(player_t *player);

#endif /* PLAYER_H */