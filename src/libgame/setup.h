/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* setup.h                                                  *
***********************************************************/

#ifndef SETUP_H
#define SETUP_H

#include "system.h"


/* values for setup file handling */
#define TYPE_BOOLEAN			(1 << 0)
#define TYPE_SWITCH			(1 << 1)
#define TYPE_YES_NO			(1 << 2)
#define TYPE_KEY			(1 << 3)
#define TYPE_KEY_X11			(1 << 4)
#define TYPE_INTEGER			(1 << 5)
#define TYPE_STRING			(1 << 6)

#define TYPE_BOOLEAN_STYLE		(TYPE_BOOLEAN | \
					 TYPE_SWITCH  | \
					 TYPE_YES_NO)

/* additional values for setup screen */
#define TYPE_ENTER_MENU			(1 << 7)
#define TYPE_LEAVE_MENU			(1 << 8)
#define TYPE_EMPTY			(1 << 9)
#define TYPE_KEYTEXT			(1 << 10)

#define TYPE_GHOSTED			(1 << 11)
#define TYPE_QUERY			(1 << 12)

#define TYPE_VALUE			(TYPE_BOOLEAN_STYLE | \
					 TYPE_KEY | \
					 TYPE_KEY_X11 | \
					 TYPE_INTEGER | \
					 TYPE_STRING)

#define TYPE_SKIP_ENTRY			(TYPE_EMPTY | \
					 TYPE_KEY | \
					 TYPE_STRING)

#define TYPE_ENTER_OR_LEAVE_MENU	(TYPE_ENTER_MENU | \
					 TYPE_LEAVE_MENU)

/* cookie token for file identifier and version number */
#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"

struct TokenInfo
{
  int type;
  void *value;
  char *text;
};

/* sort priorities of level series (also used as level series classes) */
#define LEVELCLASS_TUTORIAL_START	10
#define LEVELCLASS_TUTORIAL_END		99
#define LEVELCLASS_CLASSICS_START	100
#define LEVELCLASS_CLASSICS_END		199
#define LEVELCLASS_CONTRIBUTION_START	200
#define LEVELCLASS_CONTRIBUTION_END	299
#define LEVELCLASS_USER_START		300
#define LEVELCLASS_USER_END		399
#define LEVELCLASS_BD_START		400
#define LEVELCLASS_BD_END		499
#define LEVELCLASS_EM_START		500
#define LEVELCLASS_EM_END		599
#define LEVELCLASS_SP_START		600
#define LEVELCLASS_SP_END		699
#define LEVELCLASS_DX_START		700
#define LEVELCLASS_DX_END		799

#define LEVELCLASS_TUTORIAL		LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_CLASSICS		LEVELCLASS_CLASSICS_START
#define LEVELCLASS_CONTRIBUTION		LEVELCLASS_CONTRIBUTION_START
#define LEVELCLASS_USER			LEVELCLASS_USER_START
#define LEVELCLASS_BD			LEVELCLASS_BD_START
#define LEVELCLASS_EM			LEVELCLASS_EM_START
#define LEVELCLASS_SP			LEVELCLASS_SP_START
#define LEVELCLASS_DX			LEVELCLASS_DX_START

#define LEVELCLASS_UNDEFINED		999

#define IS_LEVELCLASS_TUTORIAL(p) \
	((p)->sort_priority >= LEVELCLASS_TUTORIAL_START && \
	 (p)->sort_priority <= LEVELCLASS_TUTORIAL_END)
#define IS_LEVELCLASS_CLASSICS(p) \
	((p)->sort_priority >= LEVELCLASS_CLASSICS_START && \
	 (p)->sort_priority <= LEVELCLASS_CLASSICS_END)
#define IS_LEVELCLASS_CONTRIBUTION(p) \
	((p)->sort_priority >= LEVELCLASS_CONTRIBUTION_START && \
	 (p)->sort_priority <= LEVELCLASS_CONTRIBUTION_END)
#define IS_LEVELCLASS_USER(p) \
	((p)->sort_priority >= LEVELCLASS_USER_START && \
	 (p)->sort_priority <= LEVELCLASS_USER_END)
#define IS_LEVELCLASS_BD(p) \
	((p)->sort_priority >= LEVELCLASS_BD_START && \
	 (p)->sort_priority <= LEVELCLASS_BD_END)
#define IS_LEVELCLASS_EM(p) \
	((p)->sort_priority >= LEVELCLASS_EM_START && \
	 (p)->sort_priority <= LEVELCLASS_EM_END)
#define IS_LEVELCLASS_SP(p) \
	((p)->sort_priority >= LEVELCLASS_SP_START && \
	 (p)->sort_priority <= LEVELCLASS_SP_END)
#define IS_LEVELCLASS_DX(p) \
	((p)->sort_priority >= LEVELCLASS_DX_START && \
	 (p)->sort_priority <= LEVELCLASS_DX_END)

#define LEVELCLASS(n)	(IS_LEVELCLASS_TUTORIAL(n) ? LEVELCLASS_TUTORIAL : \
			 IS_LEVELCLASS_CLASSICS(n) ? LEVELCLASS_CLASSICS : \
			 IS_LEVELCLASS_CONTRIBUTION(n) ? LEVELCLASS_CONTRIBUTION : \
			 IS_LEVELCLASS_USER(n) ? LEVELCLASS_USER : \
			 IS_LEVELCLASS_BD(n) ? LEVELCLASS_BD : \
			 IS_LEVELCLASS_EM(n) ? LEVELCLASS_EM : \
			 IS_LEVELCLASS_SP(n) ? LEVELCLASS_SP : \
			 IS_LEVELCLASS_DX(n) ? LEVELCLASS_DX : \
			 LEVELCLASS_UNDEFINED)

/* sort priorities of artwork */
#define ARTWORKCLASS_CLASSICS_START	100
#define ARTWORKCLASS_CLASSICS_END	199
#define ARTWORKCLASS_CONTRIBUTION_START	200
#define ARTWORKCLASS_CONTRIBUTION_END	299
#define ARTWORKCLASS_LEVEL_START	300
#define ARTWORKCLASS_LEVEL_END		399
#define ARTWORKCLASS_USER_START		400
#define ARTWORKCLASS_USER_END		499

#define ARTWORKCLASS_CLASSICS		ARTWORKCLASS_CLASSICS_START
#define ARTWORKCLASS_CONTRIBUTION	ARTWORKCLASS_CONTRIBUTION_START
#define ARTWORKCLASS_LEVEL		ARTWORKCLASS_LEVEL_START
#define ARTWORKCLASS_USER		ARTWORKCLASS_USER_START

#define ARTWORKCLASS_UNDEFINED		999

#define IS_ARTWORKCLASS_CLASSICS(p) \
	((p)->sort_priority >= ARTWORKCLASS_CLASSICS_START && \
	 (p)->sort_priority <= ARTWORKCLASS_CLASSICS_END)
#define IS_ARTWORKCLASS_CONTRIBUTION(p) \
	((p)->sort_priority >= ARTWORKCLASS_CONTRIBUTION_START && \
	 (p)->sort_priority <= ARTWORKCLASS_CONTRIBUTION_END)
#define IS_ARTWORKCLASS_LEVEL(p) \
	((p)->sort_priority >= ARTWORKCLASS_LEVEL_START && \
	 (p)->sort_priority <= ARTWORKCLASS_LEVEL_END)
#define IS_ARTWORKCLASS_USER(p) \
	((p)->sort_priority >= ARTWORKCLASS_USER_START && \
	 (p)->sort_priority <= ARTWORKCLASS_USER_END)

#define ARTWORKCLASS(n)	(IS_ARTWORKCLASS_CLASSICS(n) ? ARTWORKCLASS_CLASSICS :\
			 IS_ARTWORKCLASS_CONTRIBUTION(n) ? ARTWORKCLASS_CONTRIBUTION : \
			 IS_ARTWORKCLASS_LEVEL(n) ? ARTWORKCLASS_LEVEL : \
			 IS_ARTWORKCLASS_USER(n) ? ARTWORKCLASS_USER : \
			 ARTWORKCLASS_UNDEFINED)


void setLevelArtworkDir(TreeInfo *);
char *getLevelFilename(int);
char *getTapeFilename(int);
char *getScoreFilename(int);
char *getSetupFilename(void);
char *getImageFilename(char *);
char *getCustomImageFilename(char *);
char *getCustomSoundFilename(char *);
char *getCustomArtworkFilename(char *, int);
char *getCustomArtworkConfigFilename(int);
char *getCustomMusicDirectory(void);

void InitTapeDirectory(char *);
void InitScoreDirectory(char *);
void InitUserLevelDirectory(char *);
void InitLevelSetupDirectory(char *);

TreeInfo *newTreeInfo();
void pushTreeInfo(TreeInfo **, TreeInfo *);
int numTreeInfo(TreeInfo *);
boolean validLevelSeries(TreeInfo *);
TreeInfo *getFirstValidTreeInfoEntry(TreeInfo *);
TreeInfo *getTreeInfoFirstGroupEntry(TreeInfo *);
int numTreeInfoInGroup(TreeInfo *);
int posTreeInfo(TreeInfo *);
TreeInfo *getTreeInfoFromPos(TreeInfo *, int);
TreeInfo *getTreeInfoFromIdentifier(TreeInfo *, char *);
void dumpTreeInfo(TreeInfo *, int);
void sortTreeInfo(TreeInfo **,
		  int (*compare_function)(const void *, const void *));

char *getUserDataDir(void);
char *getSetupDir(void);
void createDirectory(char *, char *, int);
void InitUserDataDirectory(void);
void SetFilePermissions(char *, int);

char *getCookie(char *);
int getFileVersionFromCookieString(const char *);
boolean checkCookieString(const char *, const char *);

char *getFormattedSetupEntry(char *, char *);
void freeSetupFileList(struct SetupFileList *);
struct SetupFileList *newSetupFileList(char *, char *);
char *getTokenValue(struct SetupFileList *, char *);
void setTokenValue(struct SetupFileList *, char *, char *);
struct SetupFileList *loadSetupFileList(char *);
void checkSetupFileListIdentifier(struct SetupFileList *, char *);
void setSetupInfo(struct TokenInfo *, int, char *);
char *getSetupValue(int, void *);
char *getSetupLine(struct TokenInfo *, char *, int);

void LoadLevelInfo(void);
void LoadArtworkInfo(void);
void LoadLevelArtworkInfo(void);

void LoadLevelSetup_LastSeries(void);
void SaveLevelSetup_LastSeries(void);
void LoadLevelSetup_SeriesInfo(void);
void SaveLevelSetup_SeriesInfo(void);

#endif /* MISC_H */
