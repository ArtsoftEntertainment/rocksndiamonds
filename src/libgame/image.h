// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// image.h
// ============================================================================

#ifndef IMAGE_H
#define IMAGE_H

#include "system.h"


// bitmap array positions for various element sizes, if available
//
// for any loaded image, the "standard" size (which represents the 32x32 pixel
// size for game elements) is always defined; other bitmap sizes may be NULL
//
// formats from 32x32 down to 1x1 are standard bitmap sizes for game elements
// (used in the game, in the level editor, in the level preview etc.)
//
// "CUSTOM" sizes for game elements (like 64x64) may be additionally created;
// all "OTHER" image sizes are stored if different from all other bitmap sizes,
// which may be used "as is" by global animations (as the "standard" size used
// normally may be wrong due to being scaled up or down to a different size if
// the same image contains game elements in a non-standard size)

#define IMG_BITMAP_32x32	0
#define IMG_BITMAP_16x16	1
#define IMG_BITMAP_8x8		2
#define IMG_BITMAP_4x4		3
#define IMG_BITMAP_2x2		4
#define IMG_BITMAP_1x1		5
#define IMG_BITMAP_CUSTOM	6
#define IMG_BITMAP_OTHER	7

#define NUM_IMG_BITMAPS		8

// these bitmap pointers point to one of the above bitmaps (do not free them)
#define IMG_BITMAP_PTR_GAME	8
#define IMG_BITMAP_PTR_ORIGINAL	9

#define NUM_IMG_BITMAP_POINTERS	10

// this bitmap pointer points to the bitmap with default image size
#define IMG_BITMAP_STANDARD	IMG_BITMAP_32x32

// maximum number of statically and dynamically defined image files
#define MAX_IMAGE_FILES		1000000


#define GET_BITMAP_ID_FROM_TILESIZE(x)	((x) ==  1 ? IMG_BITMAP_1x1   :	\
					 (x) ==  2 ? IMG_BITMAP_2x2   :	\
					 (x) ==  4 ? IMG_BITMAP_4x4   :	\
					 (x) ==  8 ? IMG_BITMAP_8x8   :	\
					 (x) == 16 ? IMG_BITMAP_16x16 :	\
					 (x) == 32 ? IMG_BITMAP_32x32 : \
					 IMG_BITMAP_CUSTOM)

#define GET_TILESIZE_FROM_BITMAP_ID(x)	((x) == IMG_BITMAP_1x1   ? 1  :	\
					 (x) == IMG_BITMAP_2x2   ? 2  :	\
					 (x) == IMG_BITMAP_4x4   ? 4  :	\
					 (x) == IMG_BITMAP_8x8   ? 8  :	\
					 (x) == IMG_BITMAP_16x16 ? 16 :	\
					 (x) == IMG_BITMAP_32x32 ? 32 :	\
					 0)


int getImageListSize(void);
struct FileInfo *getImageListEntryFromImageID(int);
Bitmap **getBitmapsFromImageID(int);
int getOriginalImageWidthFromImageID(int);
int getOriginalImageHeightFromImageID(int);
char *getTokenFromImageID(int);
int getImageIDFromToken(char *);
char *getImageConfigFilename(void);
int getImageListPropertyMappingSize(void);
struct PropertyMapping *getImageListPropertyMapping(void);
void InitImageList(struct ConfigInfo *, int, struct ConfigTypeInfo *,
		   char **, char **, char **, char **, char **);

void ReloadCustomImages(void);
void CreateImageWithSmallImages(int, int, int);
void CreateImageTextures(int);
void FreeAllImageTextures(void);
void ScaleImage(int, int);

void FreeAllImages(void);

#endif	// IMAGE_H
