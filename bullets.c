#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bullets.h"
#include "objects.h"

int init_bullets_number = 128;

void add_bullet(struct magazine *mag, struct pixel_pos *space_craft_pos, int dir) {
    if (mag->num_of_bullets == mag->size) {
        struct bullet **temp_b = malloc(sizeof(bullet) * mag->num_of_bullets);
        memcpy(temp_b, mag->bullets, sizeof(bullet) * mag->num_of_bullets);
        mag->bullets = realloc(mag->bullets, sizeof(bullet) * mag->size * 2);
        memcpy(mag->bullets, temp_b, sizeof(bullet) * mag->num_of_bullets);
        mag->size = mag->size * 2;
    }
    mag->bullets[mag->num_of_bullets] = malloc(sizeof(bullet));
    if (dir == 1) { 
        mag->bullets[mag->num_of_bullets]->x = space_craft_pos->x;
        mag->bullets[mag->num_of_bullets]->y = space_craft_pos->y;
    } else {
        mag->bullets[mag->num_of_bullets]->x = space_craft_pos->x - 5;
        mag->bullets[mag->num_of_bullets]->y = space_craft_pos->y;
    }
    mag->bullets[mag->num_of_bullets]->power = 100;

    mag->num_of_bullets++;
}

void *init_magazine() {
    struct magazine *mag = malloc(sizeof(magazine) * init_bullets_number);
    mag->bullets = malloc(sizeof(bullet) * init_bullets_number);
    mag->num_of_bullets = 0;
    mag->size = init_bullets_number;

    return mag;
}
