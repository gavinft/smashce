#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include "vec.h"

#define PHY_GRAVITY 900

typedef enum {
    DIR_RIGHT = 1,
    DIR_LEFT = -1,
} direction_t;

typedef struct {
    vec2_t pos; /* center of box/collider */
    vec2_t extent; /* x-width/2 and y-height/2 */
} box_t;

typedef struct {
    box_t box;
    float friction;
} collider_t;

typedef struct {
    collider_t col;
    vec2_t vel;
    vec2_t total_force;
    float inv_mass;
    float resistance;
    float max_fall;
    bool grounded;
} rb_t;

#define PHY_STAGE_COLLIDERS_LEN (4)
extern collider_t* phy_stage_colliders[PHY_STAGE_COLLIDERS_LEN];

#define PHY_RBS_LEN (4)
extern rb_t* phy_rbs[PHY_RBS_LEN];

#define phy_box_bot(box) ((box).pos.y + (box).extent.y)
#define phy_box_top(box) ((box).pos.y - (box).extent.y)
#define phy_box_left(box) ((box).pos.x - (box).extent.x)
#define phy_box_right(box) ((box).pos.x + (box).extent.x)
#define phy_box_overlap(box1, box2) (phy_box_bot(box1) > phy_box_top(box2) && \
                                     phy_box_top(box1) < phy_box_bot(box2) && \
                                     phy_box_left(box1) < phy_box_right(box2) && \
                                     phy_box_right(box1) > phy_box_left(box2))

void phy_step(float dt);
void phy_add_force(rb_t* rb, vec2_t force);

#endif /* PHYSICS_H */
