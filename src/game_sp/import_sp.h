// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// import_sp.h
// ============================================================================

#ifndef IMPORT_SP_H
#define IMPORT_SP_H

#include "../libgame/libgame.h"
#include "../conf_gfx.h"
#include "../game.h"

#include "export_sp.h"


// ============================================================================
// functions and definitions exported from main program to game_sp
// ============================================================================

void CheckSingleStepMode_SP(boolean, boolean);

void getGraphicSource_SP(struct GraphicInfo_SP *, int, int);
int getGraphicInfo_Delay(int);
boolean isNextAnimationFrame_SP(int, int);

#endif	// IMPORT_SP_H
