#include "player.h"

void player_set_charac(player_t *player, player_char_t charac) {
    switch (charac) {
        case PLAYER_OIRAM:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .pos = {150, 30},
                        .extent = {16 / 2, 27.0f / 2},
                        .layer = phy_layer_player,
                        .friction = 1.0f
                    },
                    .resistance = 0.1f,
                    .max_fall = 400
                },
                charac = PLAYER_OIRAM,
            };
            break;
        case PLAYER_MARIO:
            *player = (player_t) {
                .rb = {
                    .col = {
                        .pos = {170, 30},
                        .extent = {16 / 2, 27.0f / 2},
                        .layer = phy_layer_player,
                        .friction = 1.0f
                    },
                    .resistance = 0.1f,
                    .max_fall = 400
                },
                charac = PLAYER_OIRAM,
            };
            break;
    }
}

void player_update(player_t *player, input_t *input) {

}

void player_draw(player_t *player) {

}
