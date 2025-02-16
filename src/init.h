// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// init.h
// ============================================================================

#ifndef INIT_H
#define INIT_H

#include "main.h"

#define setMoveIntoAcidProperty(l, e, v)	\
	(setBitfieldProperty(&(l)->can_move_into_acid_bits, EP_CAN_MOVE_INTO_ACID, e, v))
#define getMoveIntoAcidProperty(l, e)		\
	(getBitfieldProperty(&(l)->can_move_into_acid_bits, EP_CAN_MOVE_INTO_ACID, e))
#define setDontCollideWithProperty(l, e, v)	\
	(setBitfieldProperty(&(l)->dont_collide_with_bits,  EP_DONT_COLLIDE_WITH, e, v))
#define getDontCollideWithProperty(l, e)	\
	(getBitfieldProperty(&(l)->dont_collide_with_bits,  EP_DONT_COLLIDE_WITH, e))

void setBitfieldProperty(int *, int, int, boolean);
boolean getBitfieldProperty(int *, int, int);

void ResolveGroupElement(int);
void InitElementPropertiesStatic(void);
void InitElementPropertiesEngine(VersionType);
void InitElementPropertiesGfxElement(void);

void ReloadCustomArtwork(int);
void RedrawGlobalBorder(void);

void KeyboardAutoRepeatOffUnlessAutoplay(void);

void InitGfxBuffers(void);
void InitGadgets(void);
void InitImageTextures(void);
void FreeAllTextures(void);

boolean anyImagehasColorTemplate(void);
void InitColorTemplateImages(void);
void InitColorTemplateImagesIfNeeded(void);

void InitNetworkServer(void);

void DisplayExitMessage(char *, va_list);

void OpenAll(void);
void CloseAllAndExit(int);

#endif
