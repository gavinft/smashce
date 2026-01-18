#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "vec.h"

typedef struct {
    vec2_t move;
    bool jump;
    bool attack;
    bool special;
} input_t;


#endif