/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tools.h                                                  *
***********************************************************/

#ifndef TOOLS_H
#define TOOLS_H

#include "main.h"

/* for SetDrawtoField */
#define DRAW_DIRECT		0
#define DRAW_BUFFERED		1
#define DRAW_BACKBUFFER		2

/* for DrawElementShifted */
#define NO_CUTTING		0
#define CUT_ABOVE		(1 << 0)
#define CUT_BELOW		(1 << 1)
#define CUT_LEFT		(1 << 2)
#define CUT_RIGHT		(1 << 3)

/* for masking functions */
#define NO_MASKING		0
#define USE_MASKING		1
 
/* for MoveDoor */
#define DOOR_OPEN_1		(1 << 0)
#define DOOR_OPEN_2		(1 << 1)
#define DOOR_CLOSE_1		(1 << 2)
#define DOOR_CLOSE_2		(1 << 3)
#define DOOR_OPEN_ALL		(DOOR_OPEN_1 | DOOR_OPEN_2)
#define DOOR_CLOSE_ALL		(DOOR_CLOSE_1 | DOOR_CLOSE_2)
#define DOOR_ACTION_1		(DOOR_OPEN_1 | DOOR_CLOSE_1)
#define DOOR_ACTION_2		(DOOR_OPEN_2 | DOOR_CLOSE_2)
#define DOOR_ACTION		(DOOR_ACTION_1 | DOOR_ACTION_2)
#define DOOR_COPY_BACK		(1 << 4)
#define DOOR_NO_DELAY		(1 << 5)
#define DOOR_GET_STATE		(1 << 6)
#define DOOR_SET_STATE		(1 << 7)

/* for Request */
#define REQ_ASK			(1 << 0)
#define REQ_OPEN		(1 << 1)
#define REQ_CLOSE		(1 << 2)
#define REQ_CONFIRM		(1 << 3)
#define REQ_STAY_CLOSED		(1 << 4)
#define REQ_STAY_OPEN		(1 << 5)
#define REQ_PLAYER		(1 << 6)

#define REQUEST_WAIT_FOR	(REQ_ASK | REQ_CONFIRM | REQ_PLAYER)

void SetDrawtoField(int);
void RedrawPlayfield(boolean, int, int, int, int);
void BackToFront();
void FadeToFront();
void ClearWindow();
void SetMainBackgroundImage(int);
void SetDoorBackgroundImage(int);
void DrawBackground(int, int, int, int);

void MarkTileDirty(int, int);
void SetBorderElement();

int getGraphicAnimationFrame(int, int);
void DrawGraphicAnimationExt(DrawBuffer *, int, int, int, int, int);
boolean checkDrawGraphicAnimation(int, int, int);
boolean checkDrawLevelGraphicAnimation(int, int, int);
boolean DrawGraphicAnimation(int, int, int);
boolean DrawLevelGraphicAnimation(int, int, int);
boolean DrawLevelElementAnimation(int, int, int);

void DrawAllPlayers(void);
void DrawPlayerField(int, int);
void DrawPlayer(struct PlayerInfo *);

void getGraphicSource(int, int, Bitmap **, int *, int *);
void DrawGraphic(int, int, int, int);
void DrawGraphicExt(DrawBuffer *, int, int, int, int);
void DrawGraphicThruMask(int, int, int, int);
void DrawGraphicThruMaskExt(DrawBuffer *, int, int, int, int);

void DrawMiniGraphic(int, int, int);
void getMiniGraphicSource(int, Bitmap **, int *, int *);
void DrawMiniGraphicExt(DrawBuffer *, int, int, int);

void DrawGraphicShifted(int, int, int, int, int, int, int, int);
void DrawGraphicShiftedThruMask(int, int, int, int, int, int, int);
void DrawScreenElementExt(int, int, int, int, int, int, int);
void DrawLevelElementExt(int, int, int, int, int, int, int);
void DrawScreenElementShifted(int, int, int, int, int, int);
void DrawLevelElementShifted(int, int, int, int, int, int);
void DrawScreenElementThruMask(int, int, int);
void DrawLevelElementThruMask(int, int, int);
void DrawLevelFieldThruMask(int, int);
void DrawCrumbledSand(int, int);
void DrawScreenElement(int, int, int);
void DrawLevelElement(int, int, int);
void DrawScreenField(int, int);
void DrawLevelField(int, int);

void DrawMiniElement(int, int, int);
void DrawMiniElementOrWall(int, int, int, int);

void getMicroGraphicSource(int, Bitmap **, int *, int *);
void DrawMicroElement(int, int, int);
void DrawLevel(void);
void DrawMiniLevel(int, int, int, int);
void DrawMicroLevel(int, int, boolean);

boolean Request(char *, unsigned int);
unsigned int OpenDoor(unsigned int);
unsigned int CloseDoor(unsigned int);
unsigned int GetDoorState(void);
unsigned int SetDoorState(unsigned int);
unsigned int MoveDoor(unsigned int);

void DrawSpecialEditorDoor();
void UndrawSpecialEditorDoor();

int ReadPixel(DrawBuffer *, int, int);

void CreateToolButtons();
void FreeToolButtons();

int get_next_element(int);
int el2gfx(int);
int el2img(int);
int el_dir2img(int, int);
int el_dir_act2img(int, int, int);

#endif	/* TOOLS_H */
