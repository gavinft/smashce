#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include "vec.h"

#define PHY_GRAVITY 900

typedef struct {
    vec2_t pos; /* center of collider */
    vec2_t extent; /* x-width and y-height */
    uint8_t layer;
    float friction;
} collider_t;

typedef struct {
    collider_t col;
    vec2_t vel;
    float resistance;
    float max_fall;
    bool grounded;
} rb_t;

#define phy_layer_player (1 << 0)
#define phy_layer_stage (1 << 1)

#define phy_col_bot(col) ((col).pos.y + (col).extent.y)
#define phy_col_top(col) ((col).pos.y - (col).extent.y)
#define phy_col_left(col) ((col).pos.x - (col).extent.x)
#define phy_col_right(col) ((col).pos.x + (col).extent.x)
#define phy_col_overlap(col1, col2) (phy_col_bot(col1) > phy_col_top(col2) && \
                                     phy_col_top(col1) < phy_col_bot(col2) && \
                                     phy_col_left(col1) < phy_col_right(col2) && \
                                     phy_col_right(col1) > phy_col_left(col2))

#define PHY_COLLIDERS_LEN (4)
extern collider_t* phy_stage_colliders[PHY_COLLIDERS_LEN];

#define PHY_RBS_LEN (4)
extern rb_t* phy_rbs[PHY_RBS_LEN];

void phy_step(float dt);

#endif /* PHYSICS_H */
