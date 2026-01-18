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
        collider_t* this = &rb->col;
        
        rb->vel.y += PHY_GRAVITY * dt;
        this->pos.x += rb->vel.x;
        this->pos.y += rb->vel.y;

        for (int i = 0; i < PHY_COLLIDERS_LEN; i++) {
            if (!phy_colliders[i])
                continue;
            if (phy_colliders[i] == &rb->col)
                continue;

            collider_t* that = phy_colliders[i];
            dbg_printf("collider: %d\n", i);

            if (that->layer & phy_layer_player)
                continue;
            dbg_printf("not player\n");
            if (!phy_col_overlap(*this, *that))
                continue;
            dbg_printf("overlap!\n");
            // Overlap handling
            bool overlap_bottom = phy_col_bot(*this) > phy_col_bot(*that);
            bool overlap_top = phy_col_top(*this) < phy_col_top(*that);
            bool overlap_left = phy_col_left(*this) > phy_col_left(*that);
            bool overlap_right = phy_col_right(*this) < phy_col_right(*that);
            if (overlap_left != overlap_right) {
                if (overlap_left) {
                    dbg_printf("overlap left\n");
                    if (rb->vel.x <= 0)
                        rb->vel.x = 0;
                    this->pos.x = phy_col_top(*that) + this->extent.x;
                }
                else if (overlap_right) {
                    dbg_printf("overlap right\n");
                    if (rb->vel.x >= 0)
                        rb->vel.x = 0;
                    this->pos.x = phy_col_top(*that) - this->extent.x;
                }
            } else { // if overlap_top != overlap_bottom, but we want to fallback
                if (overlap_bottom) {
                    dbg_printf("overlap bottom\n");
                    if (rb->vel.y <= 0)
                        rb->vel.y = 0;
                    this->pos.y = phy_col_top(*that) + this->extent.y;
                }
                else if (overlap_top) {
                    dbg_printf("overlap top\n");
                    if (rb->vel.y >= 0)
                        rb->vel.y = 0;
                    this->pos.y = phy_col_top(*that) - this->extent.y;
                }
            }
            
        }
    }

    dbg_printf("\n");
}
