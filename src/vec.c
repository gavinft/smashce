#include "vec.h"

vec2_t vec_AddMagnitude(vec2_t v, float a) {
    float length = vec_Magnitude(v);
    float ratio = (length + a) / length;
    return (vec2_t) {v.x * ratio, v.y * ratio};
}