// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// setup.h
// ============================================================================

#ifndef SETUP_H
#define SETUP_H

#include "system.h"
#include "hash.h"


// values for setup file handling
#define TYPE_BOOLEAN			(1 << 0)
#define TYPE_SWITCH			(1 << 1)
#define TYPE_SWITCH_3_STATES		(1 << 2)
#define TYPE_YES_NO			(1 << 3)
#define TYPE_YES_NO_AUTO		(1 << 4)
#define TYPE_YES_NO_ASK			(1 << 5)
#define TYPE_YES_NO_ONCE		(1 << 6)
#define TYPE_ECS_AGA			(1 << 7)
#define TYPE_KEY			(1 << 8)
#define TYPE_KEY_X11			(1 << 9)
#define TYPE_INTEGER			(1 << 10)
#define TYPE_STRING			(1 << 11)
#define TYPE_PLAYER			(1 << 12)
#define TYPE_ELEMENT			(1 << 13)
#define TYPE_GRAPHIC			(1 << 14)

// additional values for setup screen
#define TYPE_ENTER_SCREEN		(1 << 15)
#define TYPE_LEAVE_SCREEN		(1 << 16)
#define TYPE_ENTER_MENU			(1 << 17)
#define TYPE_LEAVE_MENU			(1 << 18)
#define TYPE_ENTER_LIST			(1 << 19)
#define TYPE_LEAVE_LIST			(1 << 20)
#define TYPE_TEXT_INPUT			(1 << 21)
#define TYPE_EMPTY			(1 << 22)
#define TYPE_SKIPPABLE			(1 << 23)
#define TYPE_KEYTEXT			(1 << 24)
#define TYPE_HEADLINE			(1 << 25)

#define TYPE_GHOSTED			(1 << 26)
#define TYPE_QUERY			(1 << 27)

// additional values for internal purposes
#define TYPE_BITFIELD			(1 << 28)
#define TYPE_CONTENT			(1 << 29)
#define TYPE_ELEMENT_LIST		(1 << 30)
#define TYPE_CONTENT_LIST		(1 << 31)

// derived values for setup file handling
#define TYPE_BOOLEAN_STYLE		(TYPE_BOOLEAN | \
					 TYPE_SWITCH  | \
					 TYPE_YES_NO  | \
					 TYPE_ECS_AGA )

// derived values for setup screen
#define TYPE_VALUE			(TYPE_BOOLEAN_STYLE	| \
					 TYPE_SWITCH_3_STATES	| \
					 TYPE_YES_NO_AUTO	| \
					 TYPE_YES_NO_ASK	| \
					 TYPE_YES_NO_ONCE	| \
					 TYPE_KEY		| \
					 TYPE_KEY_X11		| \
					 TYPE_INTEGER		| \
					 TYPE_STRING		| \
					 TYPE_PLAYER		| \
					 TYPE_ELEMENT		| \
					 TYPE_GRAPHIC)

#define TYPE_SKIP_ENTRY			(TYPE_EMPTY		| \
					 TYPE_SKIPPABLE		| \
					 TYPE_KEY		| \
					 TYPE_STRING		| \
					 TYPE_HEADLINE		| \
					 TYPE_GHOSTED)

#define TYPE_ENTER			(TYPE_ENTER_SCREEN	| \
					 TYPE_ENTER_MENU	| \
					 TYPE_ENTER_LIST)

#define TYPE_LEAVE			(TYPE_LEAVE_SCREEN	| \
					 TYPE_LEAVE_MENU	| \
					 TYPE_LEAVE_LIST)

#define TYPE_ENTER_OR_LEAVE		(TYPE_ENTER | TYPE_LEAVE)

struct TokenInfo
{
  int type;
  void *value;
  char *text;
};

// some definitions for list and hash handling
typedef struct SetupFileList SetupFileList;
typedef struct hashtable     SetupFileHash;

#define BEGIN_HASH_ITERATION(hash, itr)				\
  if (hash != NULL && hashtable_count(hash) > 0)		\
  {								\
    struct hashtable_itr *itr = hashtable_iterator(hash);	\
    do {							\

#define HASH_ITERATION_TOKEN(itr)	((char *)hashtable_iterator_key(itr))
#define HASH_ITERATION_VALUE(itr)	((char *)hashtable_iterator_value(itr))

#define END_HASH_ITERATION(hash, itr)				\
    } while (hashtable_iterator_advance(itr));			\
    free(itr);							\
  }								\


// sort priorities of special tree entries
#define LEVELCLASS_TOP			0
#define LEVELCLASS_PARENT		1
#define LEVELCLASS_LAST_PLAYED_LEVEL	2

// sort priorities of level series (also used as level series classes)
#define LEVELCLASS_TUTORIAL_START	10
#define LEVELCLASS_TUTORIAL_END		99
#define LEVELCLASS_CLASSICS_START	100
#define LEVELCLASS_CLASSICS_END		199
#define LEVELCLASS_CONTRIB_START	200
#define LEVELCLASS_CONTRIB_END		299
#define LEVELCLASS_PRIVATE_START	300
#define LEVELCLASS_PRIVATE_END		399
#define LEVELCLASS_BD_START		400
#define LEVELCLASS_BD_END		499
#define LEVELCLASS_EM_START		500
#define LEVELCLASS_EM_END		599
#define LEVELCLASS_SP_START		600
#define LEVELCLASS_SP_END		699
#define LEVELCLASS_DX_START		700
#define LEVELCLASS_DX_END		799
#define LEVELCLASS_SB_START		800
#define LEVELCLASS_SB_END		899

#define LEVELCLASS_PREDEFINED_START	LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_PREDEFINED_END	LEVELCLASS_SB_END

#define LEVELCLASS_TUTORIAL		LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_CLASSICS		LEVELCLASS_CLASSICS_START
#define LEVELCLASS_CONTRIB		LEVELCLASS_CONTRIB_START
#define LEVELCLASS_PRIVATE		LEVELCLASS_PRIVATE_START
#define LEVELCLASS_BD			LEVELCLASS_BD_START
#define LEVELCLASS_EM			LEVELCLASS_EM_START
#define LEVELCLASS_SP			LEVELCLASS_SP_START
#define LEVELCLASS_DX			LEVELCLASS_DX_START
#define LEVELCLASS_SB			LEVELCLASS_SB_START

#define LEVELCLASS_UNDEFINED		999

#define IS_LEVELCLASS_TUTORIAL(p)					\
	((p)->sort_priority >= LEVELCLASS_TUTORIAL_START &&		\
	 (p)->sort_priority <= LEVELCLASS_TUTORIAL_END)
#define IS_LEVELCLASS_CLASSICS(p)					\
	((p)->sort_priority >= LEVELCLASS_CLASSICS_START &&		\
	 (p)->sort_priority <= LEVELCLASS_CLASSICS_END)
#define IS_LEVELCLASS_CONTRIB(p)					\
	((p)->sort_priority >= LEVELCLASS_CONTRIB_START &&		\
	 (p)->sort_priority <= LEVELCLASS_CONTRIB_END)
#define IS_LEVELCLASS_PRIVATE(p)					\
	((p)->sort_priority >= LEVELCLASS_PRIVATE_START &&		\
	 (p)->sort_priority <= LEVELCLASS_PRIVATE_END)
#define IS_LEVELCLASS_BD(p)						\
	((p)->sort_priority >= LEVELCLASS_BD_START &&			\
	 (p)->sort_priority <= LEVELCLASS_BD_END)
#define IS_LEVELCLASS_EM(p)						\
	((p)->sort_priority >= LEVELCLASS_EM_START &&			\
	 (p)->sort_priority <= LEVELCLASS_EM_END)
#define IS_LEVELCLASS_SP(p)						\
	((p)->sort_priority >= LEVELCLASS_SP_START &&			\
	 (p)->sort_priority <= LEVELCLASS_SP_END)
#define IS_LEVELCLASS_DX(p)						\
	((p)->sort_priority >= LEVELCLASS_DX_START &&			\
	 (p)->sort_priority <= LEVELCLASS_DX_END)
#define IS_LEVELCLASS_SB(p)						\
	((p)->sort_priority >= LEVELCLASS_SB_START &&			\
	 (p)->sort_priority <= LEVELCLASS_SB_END)
#define IS_LEVELCLASS_UNDEFINED(p)					\
	((p)->sort_priority < LEVELCLASS_PREDEFINED_START ||		\
	 (p)->sort_priority > LEVELCLASS_PREDEFINED_END)

#define LEVELCLASS(n)	(IS_LEVELCLASS_TUTORIAL(n) ? LEVELCLASS_TUTORIAL : \
			 IS_LEVELCLASS_CLASSICS(n) ? LEVELCLASS_CLASSICS : \
			 IS_LEVELCLASS_CONTRIB(n) ? LEVELCLASS_CONTRIB :   \
			 IS_LEVELCLASS_PRIVATE(n) ? LEVELCLASS_PRIVATE :   \
			 IS_LEVELCLASS_BD(n) ? LEVELCLASS_BD :		   \
			 IS_LEVELCLASS_EM(n) ? LEVELCLASS_EM :		   \
			 IS_LEVELCLASS_SP(n) ? LEVELCLASS_SP :		   \
			 IS_LEVELCLASS_DX(n) ? LEVELCLASS_DX :		   \
			 IS_LEVELCLASS_SB(n) ? LEVELCLASS_SB :		   \
			 LEVELCLASS_UNDEFINED)

// sort priorities of artwork
#define ARTWORKCLASS_CLASSICS_START	100
#define ARTWORKCLASS_CLASSICS_END	199
#define ARTWORKCLASS_CONTRIB_START	200
#define ARTWORKCLASS_CONTRIB_END	299
#define ARTWORKCLASS_PRIVATE_START	300
#define ARTWORKCLASS_PRIVATE_END	399
#define ARTWORKCLASS_LEVEL_START	400
#define ARTWORKCLASS_LEVEL_END		499

#define ARTWORKCLASS_CLASSICS		ARTWORKCLASS_CLASSICS_START
#define ARTWORKCLASS_CONTRIB		ARTWORKCLASS_CONTRIB_START
#define ARTWORKCLASS_PRIVATE		ARTWORKCLASS_PRIVATE_START
#define ARTWORKCLASS_LEVEL		ARTWORKCLASS_LEVEL_START

#define ARTWORKCLASS_UNDEFINED		999

#define IS_ARTWORKCLASS_CLASSICS(p)					\
	((p)->sort_priority >= ARTWORKCLASS_CLASSICS_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_CLASSICS_END)
#define IS_ARTWORKCLASS_CONTRIB(p)					\
	((p)->sort_priority >= ARTWORKCLASS_CONTRIB_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_CONTRIB_END)
#define IS_ARTWORKCLASS_PRIVATE(p)					\
	((p)->sort_priority >= ARTWORKCLASS_PRIVATE_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_PRIVATE_END)
#define IS_ARTWORKCLASS_LEVEL(p)					\
	((p)->sort_priority >= ARTWORKCLASS_LEVEL_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_LEVEL_END)

#define ARTWORKCLASS(n)	(IS_ARTWORKCLASS_CLASSICS(n) ? ARTWORKCLASS_CLASSICS :\
			 IS_ARTWORKCLASS_CONTRIB(n) ? ARTWORKCLASS_CONTRIB :  \
			 IS_ARTWORKCLASS_PRIVATE(n) ? ARTWORKCLASS_PRIVATE :  \
			 IS_ARTWORKCLASS_LEVEL(n) ? ARTWORKCLASS_LEVEL :      \
			 ARTWORKCLASS_UNDEFINED)

#define TREE_SORTING_DIR(ti)						\
	(((ti)->parent_link				   ? 0 :	\
	  (ti)->in_user_dir				   ? 4 * 200 :	\
	  (ti)->sort_priority >= LEVELCLASS_CLASSICS_START ? 3 * 200 +	\
	  (ti)->sort_priority % 100 :					\
	  (ti)->sort_priority >= LEVELCLASS_TUTORIAL_START ? 2 * 200 +	\
	  (ti)->sort_priority % 100 :					\
	  1 * 200) +							\
	 ((ti)->level_group ? 0 : 100))

#define TREE_COLOR_DIR(ti, active)					\
	((active)			? FC_YELLOW :			\
	 TREE_SORTING(ti) / 200 == 4	? FC_GREEN :			\
	 TREE_SORTING(ti) / 200 == 2	? FC_BLUE :			\
	 FC_RED)

#define TREE_SORTING(ti)						\
	(TREE_TYPE_IS_DIR((ti)->type) ? TREE_SORTING_DIR(ti) :		\
	 (ti)->sort_priority)

#define TREE_COLOR(ti, active)						\
	(TREE_TYPE_IS_DIR((ti)->type) ? TREE_COLOR_DIR(ti, active) :	\
	 (ti)->color)


char *getUserGraphicsDir(void);
char *getUserSoundsDir(void);
char *getUserMusicDir(void);
char *setLevelArtworkDir(TreeInfo *);
char *getProgramMainDataPath(char *, char *);
char *getProgramConfigFilename(char *);
char *getTapeFilename(int);
char *getTemporaryTapeFilename(void);
char *getDefaultSolutionTapeFilename(int);
char *getSokobanSolutionTapeFilename(int);
char *getSolutionTapeFilename(int);
char *getScoreFilename(int);
char *getScoreCacheFilename(int);
char *getScoreTapeBasename(char *);
char *getScoreTapeFilename(char *, int);
char *getScoreCacheTapeFilename(char *, int);
char *getSetupFilename(void);
char *getDefaultSetupFilename(void);
char *getPlatformSetupFilename(void);
char *getEditorSetupFilename(void);
char *getFilenameFromCurrentLevelDirUpward(char *);
char *getHelpAnimFilename(void);
char *getHelpTextFilename(void);
char *getLevelSetInfoFilename(int);
char *getLevelInfoFilename(int);
char *getLevelStoryFilename(int);
char *getLevelSetTitleMessageFilename(int, boolean);
char *getCreditsFilename(int, boolean);
char *getProgramInfoFilename(int);
char *getImageFilename(char *);
char *getCustomImageFilename(char *);
char *getCustomSoundFilename(char *);
char *getCustomMusicFilename(char *);
char *getCustomArtworkFilename(char *, int);
char *getCustomArtworkConfigFilename(int);
char *getCustomArtworkLevelConfigFilename(int);
char *getCustomMusicDirectory(void);
char *getCustomMusicDirectory_NoConf(void);

void MarkTapeDirectoryUploadsAsComplete(char *);
void MarkTapeDirectoryUploadsAsIncomplete(char *);
boolean CheckTapeDirectoryUploadsComplete(char *);

void InitMissingFileHash(void);
void InitTapeDirectory(char *);
void InitScoreDirectory(char *);
void InitScoreCacheDirectory(char *);
void InitScoreTapeDirectory(char *, int);
void InitScoreCacheTapeDirectory(char *, int);
void InitUserLevelDirectory(char *);
void InitNetworkLevelDirectory(char *);
void InitLevelSetupDirectory(char *);

TreeInfo *newTreeInfo(void);
TreeInfo *newTreeInfo_setDefaults(int);
void pushTreeInfo(TreeInfo **, TreeInfo *);
void removeTreeInfo(TreeInfo **);
int numTreeInfo(TreeInfo *);
boolean validLevelSeries(TreeInfo *);
TreeInfo *getValidLevelSeries(TreeInfo *, TreeInfo *);
TreeInfo *getFirstValidTreeInfoEntry(TreeInfo *);
TreeInfo *getNextValidTreeInfoEntry(TreeInfo *);
TreeInfo *getTreeInfoFirstGroupEntry(TreeInfo *);
int numTreeInfoInGroup(TreeInfo *);
int getPosFromTreeInfo(TreeInfo *);
TreeInfo *getTreeInfoFromPos(TreeInfo *, int);
TreeInfo *getTreeInfoFromIdentifier(TreeInfo *, char *);
int dumpTreeInfo(TreeInfo *, int);
void sortTreeInfoBySortFunction(TreeInfo **,
				int (*compare_function)(const void *,
							const void *));
void sortTreeInfo(TreeInfo **);
void freeTreeInfo(TreeInfo *);
TreeInfo *addTopTreeInfoNode(TreeInfo *);

char *getHomeDir(void);
char *getPersonalDataDir(void);
char *getMainUserGameDataDir(void);
char *getUserGameDataDir(void);
char *getSetupDir(void);
char *getLevelDirFromTreeInfo(TreeInfo *);
char *getUserLevelDir(char *);
char *getNetworkLevelDir(char *);
char *getCurrentLevelDir(void);
char *getNewUserLevelSubdir(void);
char *getTapeDir(char *);

void createDirectory(char *, char *);
void InitMainUserDataDirectory(void);
void InitUserDataDirectory(void);
void SetFilePermissions(char *, int);

void fprintFileHeader(FILE *, char *);
VersionType getFileVersionFromCookieString(const char *);
boolean checkCookieString(const char *, const char *);

char *getFormattedSetupEntry(char *, char *);

boolean getTokenValueFromSetupLine(char *, char **, char **);

SetupFileList *newSetupFileList(char *, char *);
void freeSetupFileList(SetupFileList *);
char *getListEntry(SetupFileList *, char *);
SetupFileList *setListEntry(SetupFileList *, char *, char *);
SetupFileList *addListEntry(SetupFileList *, char *, char *);
SetupFileList *loadSetupFileList(char *);

SetupFileHash *newSetupFileHash(void);
void freeSetupFileHash(SetupFileHash *);
char *getHashEntry(SetupFileHash *, char *);
void setHashEntry(SetupFileHash *, char *, char *);
void removeHashEntry(SetupFileHash *, char *);
SetupFileHash *loadSetupFileHash(char *);
void setSetupInfo(struct TokenInfo *, int, char *);
char *getSetupValue(int, void *);
char *getSetupLine(struct TokenInfo *, char *, int);

unsigned int get_hash_from_string(void *);
unsigned int get_hash_from_integer(void *);
int hash_key_strings_are_equal(void *, void *);
int hash_key_integers_are_equal(void *, void *);

int GetZipFileTreeType(char *);
boolean CheckZipFileForDirectory(char *, char *, int);
char *ExtractZipFileIntoDirectory(char *, char *, int);

boolean AdjustGraphicsForEMC(void);
boolean AdjustSoundsForEMC(void);

void SetCurrentArtwork(int);
void ChangeCurrentArtworkIfNeeded(int);

void LoadLevelInfo(void);
void LoadArtworkInfo(void);
void LoadLevelArtworkInfo(void);

char *getArtworkIdentifierForUserLevelSet(int);
TreeInfo *getArtworkTreeInfoForUserLevelSet(int);
boolean checkIfCustomArtworkExistsForCurrentLevelSet(void);
void AddUserLevelSetToLevelInfo(char *);
void AddTreeSetToTreeInfo(TreeInfo *, char *, char *, int);
boolean UpdateUserLevelSet(char *, char *, char *, int);
boolean CreateUserLevelSet(char *, char *, char *, int, boolean);

void UpdateLastPlayedLevels_TreeInfo(void);
void StoreLastPlayedLevels(TreeInfo *);
void ForcedStoreLastPlayedLevels(TreeInfo *);
void RestoreLastPlayedLevels(TreeInfo **);
boolean CheckLastPlayedLevels(void);

void LoadLevelSetup_LastSeries(void);
void SaveLevelSetup_LastSeries(void);
void SaveLevelSetup_LastSeries_Deactivate(void);
void LoadLevelSetup_SeriesInfo(void);
void SaveLevelSetup_SeriesInfo(void);

int LevelStats_getPlayed(int);
int LevelStats_getSolved(int);
void LevelStats_setPlayed(int, int);
void LevelStats_setSolved(int, int);
void LevelStats_incPlayed(int);
void LevelStats_incSolved(int);

void LoadUserSetup(void);
void SaveUserSetup(void);

#endif // MISC_H
