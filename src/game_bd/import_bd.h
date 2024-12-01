// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// import_bd.h
// ============================================================================

#ifndef IMPORT_BD_H
#define IMPORT_BD_H

// ============================================================================
// functions and definitions imported from main program to game_bd
// ============================================================================

#include "../libgame/libgame.h"
#include "../game.h"

#include "export_bd.h"


// ----------------------------------------------------------------------------
// imported functions
// ----------------------------------------------------------------------------

void InitGraphicInfo_BD(void);
boolean CheckSingleStepMode_BD(boolean, boolean, boolean);

void PlayLevelSound_BD(int, int, int, int);
void StopSound_BD(int, int);
boolean isSoundPlaying_BD(int, int);

void SetTimeLeft(int);
void BackToFront(void);

byte *TapePlayAction_BD(void);
byte *TapeCorrectAction_BD(byte *);
boolean TapeIsPlaying_ReplayBD(void);

boolean isLevelEditorFastStart(void);
boolean useOldEngine_BD(void);
boolean getTimePlayed_BD(void);

#endif	// IMPORT_BD_H
