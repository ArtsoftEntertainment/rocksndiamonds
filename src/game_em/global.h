#ifndef GLOBAL_H
#define GLOBAL_H

#include "main_em.h"

extern int frame;

/* all global function prototypes */

void readjoy(byte, struct PLAYER *);

void game_initscreen(void);
void game_init_random(void);
void game_init_cave_buffers(void);

void play_sound(int, int, int);
void play_element_sound(int, int, int, int);

void logic_players(void);
void logic_objects(void);
void logic_globals(void);

int  cleanup_em_level(unsigned char *, int, char *);
void convert_em_level(unsigned char *, int);
void prepare_em_level(void);

#endif
