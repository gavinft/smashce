#ifndef VEC_H
#define VEC_H

#include <math.h>

typedef struct {
    float x;
    float y;
} vec2_t;

inline float vec_Magnitude(vec2_t v);

vec2_t vec_AddMagnitude(vec2_t v, float a);

#endif /* VEC_H */