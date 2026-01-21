#include <debug.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "physics.h"

collider_t* phy_stage_colliders[PHY_STAGE_COLLIDERS_LEN] = {0};
ledge_t* phy_ledges[PHY_LEDGES_LEN] = {0};
rb_t* phy_rbs[PHY_RBS_LEN] = {0};

/**
 * Returns the point of collision along an axis.
 * Will be negative if collision has already happened
 */
static inline float colpoint(float beg, float end, float wall) {
    if (end > beg) {
        return (wall - beg) / (end - beg);
    } else {
        return (beg - wall) / (beg - end);
    }
    // return fabsf((wall - beg) / (end - beg));
}

static void fix_pos_x(collider_t *predict, collider_t *that, bool overlap_left, bool overlap_right) {
    if (overlap_left) {
        predict->box.pos.x = phy_box_left(that->box) - predict->box.extent.x;
    } else if (overlap_right) {
        predict->box.pos.x = phy_box_right(that->box) + predict->box.extent.x;
    }
}

static void fix_pos_y(collider_t *predict, collider_t *that, bool overlap_top, bool overlap_bottom) {
    if (overlap_top) {
        predict->box.pos.y = phy_box_top(that->box) - predict->box.extent.y;
    } else if (overlap_bottom) {
        predict->box.pos.y = phy_box_bot(that->box) + predict->box.extent.y;
    }
}

static void fix_vel_x(rb_t *rb, bool overlap_left, bool overlap_right) {
    if (overlap_left) {
        if (rb->vel.x >= 0) {
            rb->vel.x = 0;
        }
    } else if (overlap_right) {
        if (rb->vel.x <= 0) {
            rb->vel.x = 0;
        }
    }
}

static void fix_vel_y(rb_t *rb, bool overlap_top, bool overlap_bottom) {
    if (overlap_top) {
        if (rb->vel.y >= 0) {
            rb->vel.y = 0;
            rb->grounded = true;
        }
    } else if (overlap_bottom) {
        if (rb->vel.y <= 0) {
            rb->vel.y = 0;
        }
    }
}

void phy_step(float dt) {
    for (int i = 0; i < PHY_RBS_LEN; i++) { // TODO: optimize loop (short circuit entire loop)
        if (!phy_rbs[i])
            continue;
        rb_t* rb = phy_rbs[i]; /* current rb */
        const collider_t current = rb->col; /* this is for comparing against */
        collider_t predict = current; /* copy current collider */
        
        // gravity
        if (rb->vel.y < rb->max_fall)
            rb->vel.y += PHY_GRAVITY * dt;

        // accumulated forces
        rb->vel.x += rb->total_force.x * rb->inv_mass * dt;
        rb->vel.y += rb->total_force.y * rb->inv_mass * dt;

        rb->total_force = (vec2_t) { 0 };

        // air resistance
        rb->vel.x += -1 * rb->resistance * rb->vel.x;
        // rb->vel.y += -1 * rb->floatness * rb->vel.y;

        predict.box.pos.x += rb->vel.x * dt; /* predict based off velocity */
        predict.box.pos.y += rb->vel.y * dt;

        // loop through colliders and resolve prediction based off them
        for (int i = 0; i < PHY_STAGE_COLLIDERS_LEN; i++) {
            if (!phy_stage_colliders[i])
                continue;
            if (phy_stage_colliders[i] == &rb->col)
                continue;

            collider_t* that = phy_stage_colliders[i];

            if (!phy_box_overlap(predict.box, that->box))
                continue;
            // dbg_printf("overlap!\n");
            
            // Overlap handling
            bool overlap_bottom = phy_box_bot(predict.box) > phy_box_bot(that->box);
            bool overlap_top = phy_box_top(predict.box) < phy_box_top(that->box);
            bool overlap_left = phy_box_left(predict.box) < phy_box_left(that->box);
            bool overlap_right = phy_box_right(predict.box) > phy_box_right(that->box);
            float xt, yt;

            // check when x intersect happens
            if (rb->vel.x > 0 && overlap_left) {
                xt = colpoint(phy_box_right(current.box), phy_box_right(predict.box), phy_box_left(that->box));
            } else if (rb->vel.x < 0 && overlap_right) {
                xt = colpoint(phy_box_left(current.box), phy_box_left(predict.box), phy_box_right(that->box));
            } else {
                xt = -1.0f;
            }

            // check when y intersect happens
            if (rb->vel.y > 0 && overlap_top) {
                yt = colpoint(phy_box_bot(current.box), phy_box_bot(predict.box), phy_box_top(that->box));
            } else if (rb->vel.y < 0 && overlap_bottom) {
                yt = colpoint(phy_box_top(current.box), phy_box_top(predict.box), phy_box_bot(that->box));
            } else {
                yt = -1.0f;
            }

            // the later intersect is when we actually intersect,
            // so the later intersect is the side that we enter from
            if (xt < 0 && yt < 0) {
                // TODO
                continue;
            } else if (xt > yt) {
                fix_pos_x(&predict, that, overlap_left, overlap_right);
                fix_vel_x(rb, overlap_left, overlap_right);
            } else { // yt >= xt
                fix_pos_y(&predict, that, overlap_top, overlap_bottom);
                fix_vel_y(rb, overlap_top, overlap_bottom);
                
                // apply friction
                float friction = fminf(PHY_GRAVITY * that->friction * predict.friction * dt, fabsf(rb->vel.x));
                if (rb->vel.x > 0) {
                    rb->vel.x -= friction;
                } else {
                    rb->vel.x += friction;
                }
            }
        }

        /* Update rb's position to predicted and resolved position */
        rb->col = predict;
        // check if grounded
        if (0.001f - fabsf(rb->vel.y) < 0)
            rb->grounded = false;
    }

    // dbg_printf("\n");
}

void phy_add_force(rb_t* rb, vec2_t force) {
    rb->total_force.x += force.x;
    rb->total_force.y += force.y;
}
