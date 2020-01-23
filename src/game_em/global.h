#ifndef GLOBAL_H
#define GLOBAL_H

#include "main_em.h"

extern int frame;

/* all global function prototypes */

int open_all(void);
void close_all(void);

void readjoy(byte, struct PLAYER *);

void game_initscreen(void);
void game_init_vars(void);

void play_sound(int, int, int);
void play_element_sound(int, int, int, int);

void logic_1(void);
void logic_2(void);
void logic_3(void);

int  cleanup_em_level(unsigned char *, int, char *);
void convert_em_level(unsigned char *, int);
void prepare_em_level(void);

#endif
