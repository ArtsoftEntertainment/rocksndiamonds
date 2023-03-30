// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// engines.h
// ============================================================================

#ifndef ENGINES_H
#define ENGINES_H

#include "libgame/libgame.h"

#include "game_em/export.h"
#include "game_sp/export.h"
#include "game_mm/export.h"

#include "game.h"


// ============================================================================
// functions and definitions exported from main program to game_em
// ============================================================================

void UpdateEngineValues(int, int, int, int);

boolean swapTiles_EM(boolean);
boolean getTeamMode_EM(void);
boolean isActivePlayer_EM(int);

int getScreenFieldSizeX(void);
int getScreenFieldSizeY(void);

void PlayLevelSound_EM(int, int, int, int);
void InitGraphicInfo_EM(void);
boolean CheckSingleStepMode_EM(int, boolean, boolean, boolean);

void SetGfxAnimation_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourceObjectExt_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourcePlayerExt_EM(struct GraphicInfo_EM *, int, int, int);


// ============================================================================
// functions and definitions exported from main program to game_sp
// ============================================================================

void CheckSingleStepMode_SP(boolean, boolean);

void getGraphicSource_SP(struct GraphicInfo_SP *, int, int);
int getGraphicInfo_Delay(int);
boolean isNextAnimationFrame_SP(int, int);


// ============================================================================
// functions and definitions exported from main program to game_mm
// ============================================================================

void SetDrawtoField(int);
void BackToFront(void);

int el2img_mm(int);
int el_act2img_mm(int, int);

void CheckSingleStepMode_MM(boolean, boolean);
void ShowEnvelope(int);

int getGraphicAnimationFrame(int, int);
int getGraphicAnimationFrameXY(int, int, int);

void getGraphicSource(int, int, Bitmap **, int *, int *);
void getMiniGraphicSource(int, Bitmap **, int *, int *);
void getSizedGraphicSource(int, int, int, Bitmap **, int *, int *);
boolean getGraphicInfo_NewFrame(int, int, int);

void AdvanceFrameCounter(void);
void AdvanceGfxFrame(void);

int getAnimationFrame(int, int, int, int, int);

#endif	// ENGINES_H
