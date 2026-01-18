
#include <stdint.h>
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

            if (that->layer & phy_layer_player)
                continue;
            if (!phy_col_overlap(*this, *that))
                continue;
            
            // Overlap handling
            if (phy_col_bot(*this) > phy_col_top(*that)) {
                if (rb->vel.y <= 0)
                    rb->vel.y = 0;
                this->pos.y = phy_col_top(*that) + this->extent.y;
            }
            if (phy_col_top(*this) < phy_col_bot(*that)) {
                if (rb->vel.y >= 0)
                    rb->vel.y = 0;
                this->pos.y = phy_col_top(*that) - this->extent.y;
            }
            if (phy_col_left(*this) < phy_col_right(*that)) {
                if (rb->vel.x <= 0)
                    rb->vel.x = 0;
                this->pos.x = phy_col_top(*that) + this->extent.x;
            }
            if (phy_col_right(*this) > phy_col_right(*that)) {
                if (rb->vel.x >= 0)
                    rb->vel.x = 0;
                this->pos.x = phy_col_top(*that) - this->extent.x;
            }
        }
    }
}
