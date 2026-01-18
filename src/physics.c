#include <debug.h>
#include <stdint.h>
#include <stdbool.h>
#include "physics.h"

collider_t* phy_colliders[PHY_COLLIDERS_LEN] = {0};
rb_t* phy_rbs[PHY_RBS_LEN] = {0};

void phy_step(float dt) {
    for (int i = 0; i < PHY_RBS_LEN; i++) { // TODO: optimize loop (short circuit entire loop)
        if (!phy_rbs[i])
            continue;
        rb_t* rb = phy_rbs[i];
        collider_t predict = rb->col; /* copy current collider */
        
        rb->vel.y += PHY_GRAVITY * dt;
        predict.pos.x += rb->vel.x;
        predict.pos.y += rb->vel.y;

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
            if (overlap_left != overlap_right) {
                if (overlap_left) {
                    dbg_printf("overlap left\n");
                    if (rb->vel.x <= 0)
                        rb->vel.x = 0;
                    predict.pos.x = phy_col_left(*that) - predict.extent.x;
                }
                else if (overlap_right) {
                    dbg_printf("overlap right\n");
                    if (rb->vel.x >= 0)
                        rb->vel.x = 0;
                    predict.pos.x = phy_col_right(*that) + predict.extent.x;
                }
            } else { // if overlap_top != overlap_bottom, but we want to fallback
                if (overlap_bottom) {
                    dbg_printf("overlap bottom\n");
                    if (rb->vel.y >= 0)
                        rb->vel.y = 0;
                    predict.pos.y = phy_col_bot(*that) + predict.extent.y;
                }
                else if (overlap_top) {
                    dbg_printf("overlap top\n");
                    if (rb->vel.y >= 0)
                        rb->vel.y = 0;
                    predict.pos.y = phy_col_top(*that) - predict.extent.y;
                }
            }
            
        }

        /* Update position to predicted and fixed position */
        rb->col = predict;
    }

    dbg_printf("\n");
}
