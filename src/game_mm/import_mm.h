// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// import_mm.h
// ============================================================================

#ifndef IMPORT_MM_H
#define IMPORT_MM_H

#include "../libgame/libgame.h"
#include "../conf_gfx.h"
#include "../game.h"

#include "export_mm.h"


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

#endif	// IMPORT_MM_H
