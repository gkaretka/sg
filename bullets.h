#ifndef __BULLETS_H__
#define __BULLETS_H__

#include "objects.h"

struct bullet {
    float x;
    float y;
    float power;
} bullet;

struct magazine { 
    int size;
    int num_of_bullets;
    struct bullet **bullets;
} magazine;

void add_bullet(struct magazine *mag, struct pixel_pos *space_craft_pos, int dir);
void* init_magazine();

#endif
