#include <stdint.h>
#include <math.h>
#include "types.h"
#include "constants.h"

#if !defined(ELBEARBOY)
  #include <stdlib.h>
#endif

#ifndef min_
#define min_(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max_
#define max_(a, b) (((a) > (b)) ? (a) : (b))
#endif

template <class T>
inline T sq(T value) {
    return value * value;
}

Coords create_coords(float x, float y) {
  return { x, y };
}

uint8_t coords_distance(Coords* a, Coords* b) {
  return sqrt(sq(a->x - b->x) + sq(a->y - b->y)) * DISTANCE_MULTIPLIER;
}

uint8_t coords_distance_CM(Coords_CM* a, Coords_CM* b) {
  //return sqrt(sq(a->x_cm - b->x_cm) + sq(a->y_cm - b->y_cm)) * DISTANCE_MULTIPLIER;
    int16_t dx = abs(a->x - b->x);
    int16_t dy = abs(a->y - b->y);
    // Примерная оценка длины вектора без sqrt
    // Используем приближение: max(dx, dy) + 0.4 * min(dx, dy)
    int16_t max_val = max_(dx, dy);
    int16_t min_val = min_(dx, dy);
    int16_t approx_dist = max_val + (min_val >> 2); // ~ 0.25 * min_val
    return (approx_dist + 50) / 100; // масштабируем к единицам тайлов
}


UID create_uid(uint8_t type, uint8_t x, uint8_t y) {
  return ((y << LEVEL_WIDTH_BASE) | x) << 4 | type;
}
  
uint8_t uid_get_type(UID uid) {
  return uid & 0x0F;
}
