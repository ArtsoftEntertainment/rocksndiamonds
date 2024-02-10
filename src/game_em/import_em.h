// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// import_em.h
// ============================================================================

#ifndef IMPORT_EM_H
#define IMPORT_EM_H

#include "../libgame/libgame.h"
#include "../game.h"

#include "export_em.h"


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

#endif	// IMPORT_EM_H
