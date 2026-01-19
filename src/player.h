#ifndef PLAYER_H
#define PLAYER_H

#include "physics.h"
#include "input.h"

typedef enum player_char {
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
} player_t;

void player_set_charac(player_t *player, player_char_t charac);
void player_update(player_t *player, input_t *input);
void player_draw(player_t *player);

#endif /* PLAYER_H */