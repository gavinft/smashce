#ifndef PHYSICS_H
#define PHYSICS_H

#include "vec.h"

typedef struct {
    vec2_t pos; /* center of collider */
    vec2_t size; /* x-width and y-height */
} collider_t;

extern collider_t* phy_colliders[16];

typedef struct {
    collider_t col;
    vec2_t vel;
} rb_t;

#endif /* PHYSICS_H */
