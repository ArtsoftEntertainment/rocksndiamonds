// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2021 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// base64.h
// ============================================================================

#ifndef BASE64_H
#define BASE64_H

#include "system.h"


int base64_encoded_size(int);
int base64_decoded_size(const char *);

void base64_encode(char *, const void *, int);
void base64_decode(byte *, const char *);

#endif
