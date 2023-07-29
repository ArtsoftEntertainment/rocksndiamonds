// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// anim.h
// ============================================================================

#ifndef ANIM_H
#define ANIM_H


int getAnimationFrame(int, int, int, int, int);

void InitGlobalAnimEventsForCustomElements(void);
void InitGlobalAnimations(void);
void DrawGlobalAnimations(int, int);

void RestartGlobalAnimsByStatus(int);

boolean HandleGlobalAnimClicks(int, int, int, boolean);
void HandleGlobalAnimEventByElementChange(int);

int getGlobalAnimSyncFrame(void);

#endif
