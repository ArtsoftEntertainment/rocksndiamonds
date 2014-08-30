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
*  tools.h                                                 *
***********************************************************/

#ifndef TOOLS_H
#define TOOLS_H

#include "main.h"

#include <sys/time.h>

/* for SetDrawtoField */
#define DRAW_DIRECT	0
#define DRAW_BUFFERED	1
#define DRAW_BACKBUFFER	2

/* for DrawElementShifted */
#define NO_CUTTING	0
#define CUT_ABOVE	1
#define CUT_BELOW	2
#define CUT_LEFT	4
#define CUT_RIGHT	8

/* for masking functions */
#define NO_MASKING	0
#define USE_MASKING	1
 
/* for MoveDoor */
#define DOOR_OPEN_1	1
#define DOOR_OPEN_2	2
#define DOOR_CLOSE_1	4
#define DOOR_CLOSE_2	8
#define DOOR_OPEN_BOTH	(DOOR_OPEN_1 | DOOR_OPEN_2)
#define DOOR_CLOSE_BOTH	(DOOR_CLOSE_1 | DOOR_CLOSE_2)
#define DOOR_ACTION_1	(DOOR_OPEN_1 | DOOR_CLOSE_1)
#define DOOR_ACTION_2	(DOOR_OPEN_2 | DOOR_CLOSE_2)
#define DOOR_ACTION	(DOOR_ACTION_1 | DOOR_ACTION_2)
#define DOOR_COPY_BACK	16
#define DOOR_NO_DELAY	32
#define DOOR_GET_STATE	64

/* for AreYouSure */
#define AYS_ASK		1
#define AYS_OPEN	2
#define AYS_CLOSE	4
#define AYS_CONFIRM	8
#define AYS_STAY_CLOSED	16
#define AYS_STAY_OPEN	32

void SetDrawtoField(int);
void BackToFront();
void FadeToFront();
void ClearWindow();
void DrawText(int, int, char *, int, int);
void DrawTextExt(Drawable, GC, int, int, char *, int, int);
void DrawPlayerField(void);
void DrawGraphicAnimation(int, int, int, int, int, int);
void DrawGraphic(int, int, int);
void DrawGraphicExt(Drawable, GC, int, int, int);
void DrawGraphicExtHiRes(Drawable, GC, int, int, int);
void DrawGraphicThruMask(int, int, int);
void DrawScreenElementThruMask(int, int, int);
void DrawLevelElementThruMask(int, int, int);
void DrawMiniGraphic(int, int, int);
void DrawMiniGraphicExt(Drawable, GC, int, int, int);
void DrawMiniGraphicExtHiRes(Drawable, GC, int, int, int);
void DrawGraphicShifted(int, int, int, int, int, int, int);
void DrawGraphicShiftedThruMask(int, int, int, int, int, int);
void DrawScreenElementShifted(int, int, int, int, int, int);
void DrawLevelElementShifted(int, int, int, int, int, int);
void ErdreichAnbroeckeln(int, int);
void DrawScreenElement(int, int, int);
void DrawLevelElement(int, int, int);
void DrawScreenField(int, int);
void DrawLevelField(int, int);
void DrawMiniElement(int, int, int);
void DrawMiniElementOrWall(int, int, int, int);
void DrawMicroElement(int, int, int);
void DrawLevel(void);
void DrawMiniLevel(int, int);
void DrawMicroLevel(int, int);
BOOL AreYouSure(char *, unsigned int);
unsigned int OpenDoor(unsigned int);
unsigned int CloseDoor(unsigned int);
unsigned int GetDoorState(void);
unsigned int MoveDoor(unsigned int);
int ReadPixel(Drawable, int, int);
int el2gfx(int);

#endif
