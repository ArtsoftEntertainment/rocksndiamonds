/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  �1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  screens.h                                               *
*                                                          *
*  Letzte Aenderung: 15.06.1995                            *
***********************************************************/

#ifndef SCREENS_H
#define SCREENS_H

#include "main.h"

/* Setup-Bits */
#define SETUP_TOONS			(1<<0)
#define SETUP_SOUND			(1<<1)
#define SETUP_SOUND_LOOPS		(1<<2)
#define SETUP_SOUND_MUSIC		(1<<3)
#define SETUP_DIRECT_DRAW		(1<<4)
#define SETUP_FADING			(1<<5)
#define SETUP_RECORD_EACH_GAME		(1<<6)
#define SETUP_2ND_JOYSTICK		(1<<7)

#define DEFAULT_SETUP			(SETUP_TOONS |		\
					 SETUP_SOUND |		\
					 SETUP_SOUND_LOOPS |	\
					 SETUP_SOUND_MUSIC)

/* Setup-Voreinstellungen */
#define SETUP_TOONS_ON(x)		(((x) & SETUP_TOONS) != 0)
#define SETUP_SOUND_ON(x)		(((x) & SETUP_SOUND) != 0)
#define SETUP_SOUND_LOOPS_ON(x)		(((x) & SETUP_SOUND_LOOPS) != 0)
#define SETUP_SOUND_MUSIC_ON(x)		(((x) & SETUP_SOUND_MUSIC) != 0)
#define SETUP_DIRECT_DRAW_ON(x)		(((x) & SETUP_DIRECT_DRAW) != 0)
#define SETUP_FADING_ON(x)		(((x) & SETUP_FADING) != 0)
#define SETUP_RECORD_EACH_GAME_ON(x)	(((x) & SETUP_RECORD_EACH_GAME) != 0)
#define SETUP_2ND_JOYSTICK_ON(x)	(((x) & SETUP_2ND_JOYSTICK) != 0)

void DrawMainMenu();
void HandleMainMenu(int, int, int, int, int);
void DrawHelpScreenElAction(int);
void DrawHelpScreenElText(int);
void DrawHelpScreenMusicText(int);
void DrawHelpScreenRegistrationText(void);
void DrawHelpScreen();
void HandleHelpScreen(int);
void HandleTypeName(int, KeySym);
void DrawChooseLevel(void);
void HandleChooseLevel(int, int, int, int, int);
void DrawHallOfFame(int);
void HandleHallOfFame(int);
void DrawSetupScreen();
void HandleSetupScreen(int, int, int, int, int);
void HandleVideoButtons(int, int, int);
void HandleSoundButtons(int, int, int);
void HandleGameButtons(int, int, int);
int CheckVideoButtons(int, int, int);
int CheckSoundButtons(int, int, int);
int CheckGameButtons(int, int, int);
int CheckChooseButtons(int, int, int);
int CheckConfirmButton(int, int, int);
void DrawCompleteVideoDisplay(void);

#endif
