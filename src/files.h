/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* files.h                                                  *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"


/* values for level file type identifier */
#define LEVEL_FILE_TYPE_UNKNOWN		0
#define LEVEL_FILE_TYPE_RND		1
#define LEVEL_FILE_TYPE_BD		2
#define LEVEL_FILE_TYPE_EM		3
#define LEVEL_FILE_TYPE_SP		4
#define LEVEL_FILE_TYPE_DX		5
#define LEVEL_FILE_TYPE_SB		6
#define LEVEL_FILE_TYPE_DC		7

#define NUM_LEVEL_FILE_TYPES		8

#define LEVEL_PACKED_START		100
#define PACKED_LEVELS(x)		(LEVEL_PACKED_START + x)

#define LEVEL_FILE_TYPE_RND_PACKED	PACKED_LEVELS(LEVEL_FILE_TYPE_RND)
#define LEVEL_FILE_TYPE_EM_PACKED	PACKED_LEVELS(LEVEL_FILE_TYPE_EM)

#define IS_SINGLE_LEVEL_FILE(x)		(x < LEVEL_PACKED_START)
#define IS_PACKED_LEVEL_FILE(x)		(x > LEVEL_PACKED_START)


void setElementChangePages(struct ElementInfo *, int);
void setElementChangeInfoToDefaults(struct ElementChangeInfo *);

char *getDefaultLevelFilename(int);

void LoadLevelFromFilename(struct LevelInfo *, char *);
void LoadLevel(int);
void LoadLevelTemplate(int);
void SaveLevel(int);
void SaveLevelTemplate();
void DumpLevel(struct LevelInfo *);

void LoadTapeFromFilename(char *);
void LoadTape(int);
void LoadSolutionTape(int);
void SaveTape(int);
void DumpTape(struct TapeInfo *);

void LoadScore(int);
void SaveScore(int);

void LoadSetup();
void SaveSetup();

void LoadCustomElementDescriptions();
void LoadSpecialMenuDesignSettings();
void LoadUserDefinedEditorElementList(int **, int *);
void LoadMusicInfo();
void LoadHelpAnimInfo();
void LoadHelpTextInfo();

void ConvertLevels(void);

#endif	/* FILES_H */
