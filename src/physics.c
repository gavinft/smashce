#include <debug.h>
#include <stdint.h>
#include <stdbool.h>
#include "physics.h"

collider_t* phy_colliders[PHY_COLLIDERS_LEN] = {0};
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
        predict->pos.x = phy_col_left(*that) - predict->extent.x;
    } else if (overlap_right) {
        predict->pos.x = phy_col_right(*that) + predict->extent.x;
    }
}

static void fix_pos_y(collider_t *predict, collider_t *that, bool overlap_top, bool overlap_bottom) {
    if (overlap_top) {
        predict->pos.y = phy_col_top(*that) - predict->extent.y;
    } else if (overlap_bottom) {
        predict->pos.y = phy_col_bot(*that) + predict->extent.y;
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
        
        rb->vel.y += PHY_GRAVITY * dt;
        predict.pos.x += rb->vel.x; /* predict based off velocity */
        predict.pos.y += rb->vel.y;

        // loop through colliders and resolve prediction based off them
        for (int i = 0; i < PHY_COLLIDERS_LEN; i++) {
            if (!phy_colliders[i])
                continue;
            if (phy_colliders[i] == &rb->col)
                continue;

            collider_t* that = phy_colliders[i];

            if (that->layer & phy_layer_player)
                continue;
            if (!phy_col_overlap(predict, *that))
                continue;
            dbg_printf("overlap!\n");
            
            // Overlap handling
            bool overlap_bottom = phy_col_bot(predict) > phy_col_bot(*that);
            bool overlap_top = phy_col_top(predict) < phy_col_top(*that);
            bool overlap_left = phy_col_left(predict) < phy_col_left(*that);
            bool overlap_right = phy_col_right(predict) > phy_col_right(*that);
            float xt, yt;

            // check when x intersect happens
            if (rb->vel.x > 0 && overlap_left) {
                xt = colpoint(phy_col_right(current), phy_col_right(predict), phy_col_left(*that));
            } else if (rb->vel.x < 0 && overlap_right) {
                xt = colpoint(phy_col_left(current), phy_col_left(predict), phy_col_right(*that));
            } else {
                xt = -1.0f;
            }

            // check when y intersect happens
            if (rb->vel.y > 0 && overlap_top) {
                yt = colpoint(phy_col_bot(current), phy_col_bot(predict), phy_col_top(*that));
            } else if (rb->vel.y < 0 && overlap_bottom) {
                yt = colpoint(phy_col_top(current), phy_col_top(predict), phy_col_bot(*that));
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
            }
        }

        /* Update rb's position to predicted and resolved position */
        rb->col = predict;
    }

    dbg_printf("\n");
}
