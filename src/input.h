#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "vec.h"

typedef struct {
    vec2_t move;
    uint8_t packed_buttons;
} input_t;

#define input_jump (1 << 0)
#define input_attack (1 << 1)
#define input_special (1 << 2)

#endif