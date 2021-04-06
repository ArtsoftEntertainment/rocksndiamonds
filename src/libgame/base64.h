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

char* base64( const void* binaryData, int len, int *flen );
unsigned char* unbase64( const char* ascii, int len, int *flen );

#endif
