// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// main_bd.h
// ============================================================================

#ifndef MAIN_BD_H
#define MAIN_BD_H

// ============================================================================
// external functions and definitions imported from main program to game_bd
// ============================================================================

#include "import_bd.h"


// ============================================================================
// functions and definitions that are exported from game_bd to main program
// ============================================================================

#include "export_bd.h"


// ============================================================================
// internal functions and definitions that are not exported to main program
// ============================================================================

#include "bd_bdcff.h"
#include "bd_cave.h"
#include "bd_cavedb.h"
#include "bd_caveset.h"
#include "bd_caveobject.h"
#include "bd_caveengine.h"
#include "bd_gameplay.h"
#include "bd_c64import.h"
#include "bd_graphics.h"
#include "bd_random.h"
#include "bd_sound.h"


// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

/* screen sizes and positions for BD engine */

#define TILESIZE		32

extern int			TILESIZE_VAR;

#define TILEX			TILESIZE_VAR
#define TILEY			TILESIZE_VAR

extern int			SCR_FIELDX, SCR_FIELDY;

/* often used screen positions */

extern int			SX, SY;

#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)


// ----------------------------------------------------------------------------
// data structure definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// exported variables
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

#endif	// MAIN_BD_H
