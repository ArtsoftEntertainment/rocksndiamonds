#ifndef GLOBAL_H
#define GLOBAL_H

#include "main_em.h"


/* global variables */

extern int frame;
extern int screen_x, screen_y;


/* global function prototypes */

void game_initscreen(void);
void game_init_random(void);
void game_init_cave_buffers(void);

void play_sound(int, int, int);
void play_element_sound(int, int, int, int);

boolean logic_check_wrap(void);
void logic_move(void);
void logic_init(void);
void logic(void);

int  cleanup_em_level(unsigned char *, int, char *);
void convert_em_level(unsigned char *, int);
void prepare_em_level(void);

#endif
