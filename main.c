#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <time.h>
#include "constants.h"
#include "objects.h"
#include "bullets.h"
#include "server.h"
#include "client.h"
#include "communication.h"
#include <pthread.h>
#include <math.h>

const int field_width = 640;
const int field_height = 480;
const int falling_stars_count = 40;
const int shooting_speed = 20000;
const int message_size = 14;

extern const int buf_size;

void refresh_field(void);
int init(void);
void waiting_screen(char *text, void (*res_f)());
void draw_score(void);
void reset_score(void);

void draw_stars(void);
void rand_set_stars(void);
void let_stars_fall(void);

void draw_space_craft(int dir);
void draw_oponent_space_craft(int dir);
void set_space_craft_starting_pos(void);
void move_space_craft(void);

void draw_player_bullets(void);
void draw_oponent_bullets(void);

char *send_pl_bullets(void);
char *spaceship_coordinates_to_sent(void);

void draw_rect(float x, float y, float x2, float y2, ALLEGRO_COLOR color);

void set_x_y_from_response(char *init_pos, int *x, int *y);
void proccess_request(char *init_pos);

struct pixel_pos **falling_stars_pos;
struct magazine *player_bullets_magazine;
struct magazine *oponent_bullets_magazine;

struct pixel_pos *space_craft_pos;
struct pixel_pos *oponent_space_craft_pos;
int player_health = 1000;

int bullet_dir;
int got_bullet;
int my_score = 0, oponent_score = 0;
int *my_socket;

ALLEGRO_KEYBOARD_STATE current_state;
ALLEGRO_FONT *font_50, *font_15, *bfont_15;

clock_t begin;

int main(int argc, char **argv) {
    ALLEGRO_DISPLAY *display = NULL;
    
    if (argc == 1) {
        printf("Please specify client/server typing s/c as arg for program \n");
        return -1;
    }

    if (argv[1][0] != 's' && argv[1][0] != 'c') {
        printf("Please type in only 'c' or 's' \n");
        return -1;
    }
    
    if (argv[1][0] == 's') {
        my_socket = init_server();
        bullet_dir = 1;
    } else if (argv[1][0] == 'c') {
        my_socket = init_client();
        bullet_dir = -1;
    }    

    if(!al_init()) {
       fprintf(stderr, "failed to initialize allegro!\n");
       return -1;
    }

    display = al_create_display(field_width, field_height);
    if(!display) {
       fprintf(stderr, "failed to create display!\n");
       return -1;
    }

    al_install_keyboard();
    al_init_font_addon();
    al_init_ttf_addon();
    if (!al_is_keyboard_installed()) return -1;

    init();
 
    waiting_screen("press 's' to start", NULL);

    while (1) {
        refresh_field();
        draw_stars();          
        draw_score();
        
        if (my_score > 1000) {
            waiting_screen("YOU WON! Press S for restart", &reset_score);
        } else if (oponent_score > 1000) {
            waiting_screen("YOU LOSE! Press s for restart", &reset_score);
        }
        
        char *init_pos = calloc(14, sizeof(char) * message_size);
        proccess_request(init_pos);
        int x = 0, y = 0;
        set_x_y_from_response(init_pos, &x, &y);

        if (init_pos[0] == 's') {
            oponent_space_craft_pos->x = x;
            oponent_space_craft_pos->y = y;
        } else if (init_pos[0] == 'b') {
            add_bullet(oponent_bullets_magazine, oponent_space_craft_pos, bullet_dir * (-1));
        }

        free(init_pos);
        
        draw_space_craft(bullet_dir);
        draw_oponent_space_craft(bullet_dir * (-1));
        draw_player_bullets();
        draw_oponent_bullets();
        let_stars_fall(); 

        al_get_keyboard_state(&current_state);
        if (al_key_down(&current_state, ALLEGRO_KEY_ESCAPE)) break;
        move_space_craft();
        
        al_flip_display();
    }

    free(falling_stars_pos);
    free(space_craft_pos);

    al_destroy_display(display);
    return 0;
}

int init(void) {
    begin = clock();

    srand(time(NULL));
    falling_stars_pos = malloc(sizeof(pixel_pos) * falling_stars_count);
    space_craft_pos = malloc(sizeof(pixel_pos));
    oponent_space_craft_pos = malloc(sizeof(pixel_pos));
   
    font_50 = al_load_ttf_font("font/Zyana-Regular.ttf", 50, 0);
    font_15 = al_load_ttf_font("font/Zyana-Regular.ttf", 15, 0);
    bfont_15 = al_load_ttf_font("font/Zyana-Regular.ttf", 25, 0);

    player_bullets_magazine = init_magazine();
    oponent_bullets_magazine = init_magazine();

    if (falling_stars_pos == NULL) {
        return -1;
    }
     
    set_space_craft_starting_pos();
    rand_set_stars();
    draw_stars();
    
    return 0;
}

void waiting_screen(char *text,void (*res_f)()) {
    for (int i = 0; i < 32; i++) {
        refresh_field();
        al_draw_text(font_50, al_map_rgb(0, i, i), field_width / 2 + 10, field_height / 2,
                    ALLEGRO_ALIGN_CENTRE, text);

        al_draw_text(font_50, al_map_rgb(0, i * 8, i * 8), field_width / 2, field_height / 2,
                    ALLEGRO_ALIGN_CENTRE, text);
        al_flip_display();
    }

    while (1) {
        refresh_field();

        al_draw_text(font_50, al_map_rgb(0, 32, 32), field_width / 2 + 10, field_height / 2,
                    ALLEGRO_ALIGN_CENTRE, text);

        al_draw_text(font_50, al_map_rgb(0, 255, 255), field_width / 2, field_height / 2,
                    ALLEGRO_ALIGN_CENTRE, text);

        al_flip_display();
        al_get_keyboard_state(&current_state);
        if (al_key_down(&current_state, ALLEGRO_KEY_S)) {
            if (res_f != NULL) (*res_f)();
            break;
        }
    }
}

void reset_score(void) {
    my_score = 0;
    oponent_score = 0;
}

void draw_score(void) { 
    al_draw_textf(bfont_15, al_map_rgb(255, 255, 255), 1, 0, 0, "score: %d", my_score);
    al_draw_textf(bfont_15, al_map_rgb(255, 255, 255), 
                 field_width, 0, ALLEGRO_ALIGN_RIGHT, "%d :oponent socre", oponent_score);
}

int char_to_int(char *c) {
    int num = 0;
    int index = 0;
    for (int i = strlen(c); i > 0; i--) {
        num += (c[index] - 48) * pow(10, i-1);
        index++;
    }
    return num;
}

void set_x_y_from_response(char *init_pos, int *x, int *y) {
    int index = 1;
    while (init_pos[index] != ',') {
        *x += init_pos[index] - 48;
        *x *= 10;
        index++;
    }
            
    index++;
            
    while (init_pos[index] != ',') {
        *y += init_pos[index] - 48;
        *y *= 10;
        index++;
    }
            
    *x /= 10;
    *y /= 10;
}

void proccess_request(char *init_pos) { 
  if (got_bullet) { 
        if (bullet_dir == -1) { 
            read_message(*my_socket, init_pos, message_size); 
            send_message(*my_socket, send_pl_bullets());
        } else {
            send_message(*my_socket, send_pl_bullets());
            read_message(*my_socket, init_pos, message_size);
        }
        got_bullet = 0;
    } else {
        if (bullet_dir == -1) { 
            read_message(*my_socket, init_pos, message_size);
            send_message(*my_socket, spaceship_coordinates_to_sent());
        } else {
            send_message(*my_socket, spaceship_coordinates_to_sent());
            read_message(*my_socket, init_pos, message_size);
        }
    }
}

void rand_set_stars(void) {
    for (int i = 0; i < falling_stars_count; i++) {
        falling_stars_pos[i] = NULL;
        falling_stars_pos[i] = malloc(sizeof(pixel_pos));

        falling_stars_pos[i]->x = rand() % field_width;
        falling_stars_pos[i]->y = rand() % field_height;
    }
}

void draw_stars(void) {
    for (int i = 0; i < falling_stars_count; i++) {
        al_draw_pixel(falling_stars_pos[i]->x, falling_stars_pos[i]->y,
                      al_map_rgb(255,255,255));
    }
}

void let_stars_fall(void) {
    for (int i = 0; i < falling_stars_count; i++) {
        falling_stars_pos[i]->x--;
        if (falling_stars_pos[i]->x < 0) {
            falling_stars_pos[i]->x = field_width;
            falling_stars_pos[i]->y = rand() % field_height;
        }
    }
}

void set_space_craft_starting_pos(void) {
    if (bullet_dir == 1) {
        space_craft_pos->x = field_width / 3;
        space_craft_pos->y = field_height / 2;
    } else {
        space_craft_pos->x = field_width - field_width / 3;
        space_craft_pos->y = field_height - field_height / 2;
    }
}

void draw_oponent_space_craft(int dir) {
    draw_rect(oponent_space_craft_pos->x, oponent_space_craft_pos->y,
              oponent_space_craft_pos->x + 10, oponent_space_craft_pos->y + 10,
              al_map_rgb(255,0,0));
    
    if (dir == 1) {
        draw_rect(oponent_space_craft_pos->x + 10, oponent_space_craft_pos->y + 3,
                  oponent_space_craft_pos->x + 12, oponent_space_craft_pos->y + 7,
                  al_map_rgb(0, 255, 0));
    } else {
        draw_rect(oponent_space_craft_pos->x - 2, oponent_space_craft_pos->y + 3,
                  oponent_space_craft_pos->x, oponent_space_craft_pos->y + 7,
                  al_map_rgb(0, 255, 0));
    }
}

void draw_space_craft(int dir) {
    draw_rect(space_craft_pos->x, space_craft_pos->y,
              space_craft_pos->x + 10, space_craft_pos->y + 10,
              al_map_rgb(255,0,0));
    
    if (dir == 1) {
        draw_rect(space_craft_pos->x + 10, space_craft_pos->y + 3,
                  space_craft_pos->x + 12, space_craft_pos->y + 7,
                  al_map_rgb(0, 255, 0));
    } else {
        draw_rect(space_craft_pos->x - 2, space_craft_pos->y + 3,
                  space_craft_pos->x, space_craft_pos->y + 7,
                  al_map_rgb(0, 255, 0));
    }
}

void draw_player_bullets(void) {
    for (int i = 0; i < player_bullets_magazine->num_of_bullets; i++) {
        player_bullets_magazine->bullets[i]->x += bullet_dir;

        draw_rect(player_bullets_magazine->bullets[i]->x, 
                  player_bullets_magazine->bullets[i]->y,
                  player_bullets_magazine->bullets[i]->x + 2,
                  player_bullets_magazine->bullets[i]->y + 2,
                  al_map_rgb(0, 255, 0));

        if (player_bullets_magazine->bullets[i]->x > oponent_space_craft_pos->x &&
            player_bullets_magazine->bullets[i]->x < oponent_space_craft_pos->x + 10 &&
            player_bullets_magazine->bullets[i]->y + 2 > oponent_space_craft_pos->y &&
            player_bullets_magazine->bullets[i]->y < oponent_space_craft_pos->y + 10) {
            my_score++;
        }
    }
}

void draw_oponent_bullets(void) {
    for (int i = 0; i < oponent_bullets_magazine->num_of_bullets; i++) {
        oponent_bullets_magazine->bullets[i]->x -= bullet_dir;
        
        draw_rect(oponent_bullets_magazine->bullets[i]->x, 
                  oponent_bullets_magazine->bullets[i]->y,
                  oponent_bullets_magazine->bullets[i]->x + 2,
                  oponent_bullets_magazine->bullets[i]->y + 2,
                  al_map_rgb(0, 255, 0));

        if (oponent_bullets_magazine->bullets[i]->x > space_craft_pos->x &&
            oponent_bullets_magazine->bullets[i]->x < space_craft_pos->x + 10 &&
            oponent_bullets_magazine->bullets[i]->y + 2 > space_craft_pos->y &&
            oponent_bullets_magazine->bullets[i]->y < space_craft_pos->y + 10) {
            oponent_score++;
        }
    }
}

void move_space_craft(void) { 
    if (al_key_down(&current_state, ALLEGRO_KEY_UP)) { 
        space_craft_pos->y--;
    }
    
    if (al_key_down(&current_state, ALLEGRO_KEY_DOWN)) {
        space_craft_pos->y++;
    }
    
    if (al_key_down(&current_state, ALLEGRO_KEY_LEFT)) {
        space_craft_pos->x--;
    }
    
    if (al_key_down(&current_state, ALLEGRO_KEY_RIGHT)) {
        space_craft_pos->x++;
    }

    if (al_key_down(&current_state, ALLEGRO_KEY_SPACE)) {
        clock_t end = clock();
        long long time_between_shots = (long long)(end - begin);
        if (time_between_shots > shooting_speed) {
            add_bullet(player_bullets_magazine, space_craft_pos, bullet_dir);
            got_bullet = 1;
            send_pl_bullets();
            begin = clock();
        }
    }
}

char *spaceship_coordinates_to_sent(void) {
    char *buf = calloc(14, sizeof(char) * 14);
    
    buf[0] = 's';

    char *temp = calloc(5, sizeof(char) * 5);
    snprintf(temp, 4, "%d", (int)space_craft_pos->x);
    strcat(buf, temp);
   
    strcat(buf, ",");

    snprintf(temp, 4, "%d", (int)space_craft_pos->y);
    strcat(buf, temp);
    strcat(buf, ",");

    return buf;
}

char *send_pl_bullets(void) {
    char *buf = calloc(14, sizeof(char) * 14);
    
    buf[0] = 'b';

    char *temp = calloc(5, sizeof(char) * 5);
    snprintf(temp, 4, "%d", (int)space_craft_pos->x);
    strcat(buf, temp);
   
    strcat(buf, ",");

    snprintf(temp, 4, "%d", (int)space_craft_pos->y);
    strcat(buf, temp);
    strcat(buf, ",");

    return buf;
}

void draw_rect(float x, float y, float x2, float y2, ALLEGRO_COLOR color) {
    for (int i = x; i < x2; i++) {
        for (int j = y; j < y2; j++) {
            al_draw_pixel(i, j, color);
        }
    }
}

void refresh_field(void) {
    al_clear_to_color(al_map_rgb(0,0,0));
}
