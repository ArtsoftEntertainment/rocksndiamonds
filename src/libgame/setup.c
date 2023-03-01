// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// setup.c
// ============================================================================

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "platform.h"

#include "setup.h"
#include "sound.h"
#include "joystick.h"
#include "text.h"
#include "misc.h"
#include "hash.h"
#include "zip/miniunz.h"


#define ENABLE_UNUSED_CODE	FALSE	// for currently unused functions
#define DEBUG_NO_CONFIG_FILE	FALSE	// for extra-verbose debug output

#define NUM_LEVELCLASS_DESC	8

static char *levelclass_desc[NUM_LEVELCLASS_DESC] =
{
  "Tutorial Levels",
  "Classic Originals",
  "Contributions",
  "Private Levels",
  "Boulderdash",
  "Emerald Mine",
  "Supaplex",
  "DX Boulderdash"
};

#define TOKEN_VALUE_POSITION_SHORT		32
#define TOKEN_VALUE_POSITION_DEFAULT		40
#define TOKEN_COMMENT_POSITION_DEFAULT		60

#define TREE_NODE_TYPE_DEFAULT			0
#define TREE_NODE_TYPE_PARENT			1
#define TREE_NODE_TYPE_GROUP			2
#define TREE_NODE_TYPE_COPY			3

#define TREE_NODE_TYPE(ti)	(ti->node_group  ? TREE_NODE_TYPE_GROUP  : \
				 ti->parent_link ? TREE_NODE_TYPE_PARENT : \
				 ti->is_copy     ? TREE_NODE_TYPE_COPY   : \
				 TREE_NODE_TYPE_DEFAULT)


static void setTreeInfoToDefaults(TreeInfo *, int);
static TreeInfo *getTreeInfoCopy(TreeInfo *ti);
static int compareTreeInfoEntries(const void *, const void *);

static int token_value_position   = TOKEN_VALUE_POSITION_DEFAULT;
static int token_comment_position = TOKEN_COMMENT_POSITION_DEFAULT;

static SetupFileHash *artworkinfo_cache_old = NULL;
static SetupFileHash *artworkinfo_cache_new = NULL;
static SetupFileHash *optional_tokens_hash = NULL;
static SetupFileHash *missing_file_hash = NULL;
static boolean use_artworkinfo_cache = TRUE;
static boolean update_artworkinfo_cache = FALSE;


// ----------------------------------------------------------------------------
// file functions
// ----------------------------------------------------------------------------

static void WarnUsingFallback(char *filename)
{
  if (getHashEntry(missing_file_hash, filename) == NULL)
  {
    setHashEntry(missing_file_hash, filename, "");

    Debug("setup", "cannot find artwork file '%s' (using fallback)", filename);
  }
}

static char *getLevelClassDescription(TreeInfo *ti)
{
  int position = ti->sort_priority / 100;

  if (position >= 0 && position < NUM_LEVELCLASS_DESC)
    return levelclass_desc[position];
  else
    return "Unknown Level Class";
}

static char *getCacheDir(void)
{
  static char *cache_dir = NULL;

  if (cache_dir == NULL)
    cache_dir = getPath2(getMainUserGameDataDir(), CACHE_DIRECTORY);

  return cache_dir;
}

static char *getScoreDir(char *level_subdir)
{
  static char *score_dir = NULL;
  static char *score_level_dir = NULL;
  char *score_subdir = SCORES_DIRECTORY;

  if (score_dir == NULL)
    score_dir = getPath2(getMainUserGameDataDir(), score_subdir);

  if (level_subdir != NULL)
  {
    checked_free(score_level_dir);

    score_level_dir = getPath2(score_dir, level_subdir);

    return score_level_dir;
  }

  return score_dir;
}

static char *getScoreCacheDir(char *level_subdir)
{
  static char *score_dir = NULL;
  static char *score_level_dir = NULL;
  char *score_subdir = SCORES_DIRECTORY;

  if (score_dir == NULL)
    score_dir = getPath2(getCacheDir(), score_subdir);

  if (level_subdir != NULL)
  {
    checked_free(score_level_dir);

    score_level_dir = getPath2(score_dir, level_subdir);

    return score_level_dir;
  }

  return score_dir;
}

static char *getScoreTapeDir(char *level_subdir, int nr)
{
  static char *score_tape_dir = NULL;
  char tape_subdir[MAX_FILENAME_LEN];

  checked_free(score_tape_dir);

  sprintf(tape_subdir, "%03d", nr);
  score_tape_dir = getPath2(getScoreDir(level_subdir), tape_subdir);

  return score_tape_dir;
}

static char *getScoreCacheTapeDir(char *level_subdir, int nr)
{
  static char *score_cache_tape_dir = NULL;
  char tape_subdir[MAX_FILENAME_LEN];

  checked_free(score_cache_tape_dir);

  sprintf(tape_subdir, "%03d", nr);
  score_cache_tape_dir = getPath2(getScoreCacheDir(level_subdir), tape_subdir);

  return score_cache_tape_dir;
}

static char *getUserSubdir(int nr)
{
  static char user_subdir[16] = { 0 };

  sprintf(user_subdir, "%03d", nr);

  return user_subdir;
}

static char *getUserDir(int nr)
{
  static char *user_dir = NULL;
  char *main_data_dir = getMainUserGameDataDir();
  char *users_subdir = USERS_DIRECTORY;
  char *user_subdir = getUserSubdir(nr);

  checked_free(user_dir);

  if (nr != -1)
    user_dir = getPath3(main_data_dir, users_subdir, user_subdir);
  else
    user_dir = getPath2(main_data_dir, users_subdir);

  return user_dir;
}

static char *getLevelSetupDir(char *level_subdir)
{
  static char *levelsetup_dir = NULL;
  char *data_dir = getUserGameDataDir();
  char *levelsetup_subdir = LEVELSETUP_DIRECTORY;

  checked_free(levelsetup_dir);

  if (level_subdir != NULL)
    levelsetup_dir = getPath3(data_dir, levelsetup_subdir, level_subdir);
  else
    levelsetup_dir = getPath2(data_dir, levelsetup_subdir);

  return levelsetup_dir;
}

static char *getNetworkDir(void)
{
  static char *network_dir = NULL;

  if (network_dir == NULL)
    network_dir = getPath2(getMainUserGameDataDir(), NETWORK_DIRECTORY);

  return network_dir;
}

char *getLevelDirFromTreeInfo(TreeInfo *node)
{
  static char *level_dir = NULL;

  if (node == NULL)
    return options.level_directory;

  checked_free(level_dir);

  level_dir = getPath2((node->in_user_dir ? getUserLevelDir(NULL) :
			options.level_directory), node->fullpath);

  return level_dir;
}

char *getUserLevelDir(char *level_subdir)
{
  static char *userlevel_dir = NULL;
  char *data_dir = getMainUserGameDataDir();
  char *userlevel_subdir = LEVELS_DIRECTORY;

  checked_free(userlevel_dir);

  if (level_subdir != NULL)
    userlevel_dir = getPath3(data_dir, userlevel_subdir, level_subdir);
  else
    userlevel_dir = getPath2(data_dir, userlevel_subdir);

  return userlevel_dir;
}

char *getNetworkLevelDir(char *level_subdir)
{
  static char *network_level_dir = NULL;
  char *data_dir = getNetworkDir();
  char *networklevel_subdir = LEVELS_DIRECTORY;

  checked_free(network_level_dir);

  if (level_subdir != NULL)
    network_level_dir = getPath3(data_dir, networklevel_subdir, level_subdir);
  else
    network_level_dir = getPath2(data_dir, networklevel_subdir);

  return network_level_dir;
}

char *getCurrentLevelDir(void)
{
  return getLevelDirFromTreeInfo(leveldir_current);
}

char *getNewUserLevelSubdir(void)
{
  static char *new_level_subdir = NULL;
  char *subdir_prefix = getLoginName();
  char subdir_suffix[10];
  int max_suffix_number = 1000;
  int i = 0;

  while (++i < max_suffix_number)
  {
    sprintf(subdir_suffix, "_%d", i);

    checked_free(new_level_subdir);
    new_level_subdir = getStringCat2(subdir_prefix, subdir_suffix);

    if (!directoryExists(getUserLevelDir(new_level_subdir)))
      break;
  }

  return new_level_subdir;
}

char *getTapeDir(char *level_subdir)
{
  static char *tape_dir = NULL;
  char *data_dir = getUserGameDataDir();
  char *tape_subdir = TAPES_DIRECTORY;

  checked_free(tape_dir);

  if (level_subdir != NULL)
    tape_dir = getPath3(data_dir, tape_subdir, level_subdir);
  else
    tape_dir = getPath2(data_dir, tape_subdir);

  return tape_dir;
}

static char *getSolutionTapeDir(void)
{
  static char *tape_dir = NULL;
  char *data_dir = getCurrentLevelDir();
  char *tape_subdir = TAPES_DIRECTORY;

  checked_free(tape_dir);

  tape_dir = getPath2(data_dir, tape_subdir);

  return tape_dir;
}

static char *getDefaultGraphicsDir(char *graphics_subdir)
{
  static char *graphics_dir = NULL;

  if (graphics_subdir == NULL)
    return options.graphics_directory;

  checked_free(graphics_dir);

  graphics_dir = getPath2(options.graphics_directory, graphics_subdir);

  return graphics_dir;
}

static char *getDefaultSoundsDir(char *sounds_subdir)
{
  static char *sounds_dir = NULL;

  if (sounds_subdir == NULL)
    return options.sounds_directory;

  checked_free(sounds_dir);

  sounds_dir = getPath2(options.sounds_directory, sounds_subdir);

  return sounds_dir;
}

static char *getDefaultMusicDir(char *music_subdir)
{
  static char *music_dir = NULL;

  if (music_subdir == NULL)
    return options.music_directory;

  checked_free(music_dir);

  music_dir = getPath2(options.music_directory, music_subdir);

  return music_dir;
}

static char *getClassicArtworkSet(int type)
{
  return (type == TREE_TYPE_GRAPHICS_DIR ? GFX_CLASSIC_SUBDIR :
	  type == TREE_TYPE_SOUNDS_DIR   ? SND_CLASSIC_SUBDIR :
	  type == TREE_TYPE_MUSIC_DIR    ? MUS_CLASSIC_SUBDIR : "");
}

static char *getClassicArtworkDir(int type)
{
  return (type == TREE_TYPE_GRAPHICS_DIR ?
	  getDefaultGraphicsDir(GFX_CLASSIC_SUBDIR) :
	  type == TREE_TYPE_SOUNDS_DIR ?
	  getDefaultSoundsDir(SND_CLASSIC_SUBDIR) :
	  type == TREE_TYPE_MUSIC_DIR ?
	  getDefaultMusicDir(MUS_CLASSIC_SUBDIR) : "");
}

char *getUserGraphicsDir(void)
{
  static char *usergraphics_dir = NULL;

  if (usergraphics_dir == NULL)
    usergraphics_dir = getPath2(getMainUserGameDataDir(), GRAPHICS_DIRECTORY);

  return usergraphics_dir;
}

char *getUserSoundsDir(void)
{
  static char *usersounds_dir = NULL;

  if (usersounds_dir == NULL)
    usersounds_dir = getPath2(getMainUserGameDataDir(), SOUNDS_DIRECTORY);

  return usersounds_dir;
}

char *getUserMusicDir(void)
{
  static char *usermusic_dir = NULL;

  if (usermusic_dir == NULL)
    usermusic_dir = getPath2(getMainUserGameDataDir(), MUSIC_DIRECTORY);

  return usermusic_dir;
}

static char *getSetupArtworkDir(TreeInfo *ti)
{
  static char *artwork_dir = NULL;

  if (ti == NULL)
    return NULL;

  checked_free(artwork_dir);

  artwork_dir = getPath2(ti->basepath, ti->fullpath);

  return artwork_dir;
}

char *setLevelArtworkDir(TreeInfo *ti)
{
  char **artwork_path_ptr, **artwork_set_ptr;
  TreeInfo *level_artwork;

  if (ti == NULL || leveldir_current == NULL)
    return NULL;

  artwork_path_ptr = LEVELDIR_ARTWORK_PATH_PTR(leveldir_current, ti->type);
  artwork_set_ptr  = LEVELDIR_ARTWORK_SET_PTR( leveldir_current, ti->type);

  checked_free(*artwork_path_ptr);

  if ((level_artwork = getTreeInfoFromIdentifier(ti, *artwork_set_ptr)))
  {
    *artwork_path_ptr = getStringCopy(getSetupArtworkDir(level_artwork));
  }
  else
  {
    /*
      No (or non-existing) artwork configured in "levelinfo.conf". This would
      normally result in using the artwork configured in the setup menu. But
      if an artwork subdirectory exists (which might contain custom artwork
      or an artwork configuration file), this level artwork must be treated
      as relative to the default "classic" artwork, not to the artwork that
      is currently configured in the setup menu.

      Update: For "special" versions of R'n'D (like "R'n'D jue"), do not use
      the "default" artwork (which would be "jue0" for "R'n'D jue"), but use
      the real "classic" artwork from the original R'n'D (like "gfx_classic").
    */

    char *dir = getPath2(getCurrentLevelDir(), ARTWORK_DIRECTORY(ti->type));

    checked_free(*artwork_set_ptr);

    if (directoryExists(dir))
    {
      *artwork_path_ptr = getStringCopy(getClassicArtworkDir(ti->type));
      *artwork_set_ptr = getStringCopy(getClassicArtworkSet(ti->type));
    }
    else
    {
      *artwork_path_ptr = getStringCopy(UNDEFINED_FILENAME);
      *artwork_set_ptr = NULL;
    }

    free(dir);
  }

  return *artwork_set_ptr;
}

static char *getLevelArtworkSet(int type)
{
  if (leveldir_current == NULL)
    return NULL;

  return LEVELDIR_ARTWORK_SET(leveldir_current, type);
}

static char *getLevelArtworkDir(int type)
{
  if (leveldir_current == NULL)
    return UNDEFINED_FILENAME;

  return LEVELDIR_ARTWORK_PATH(leveldir_current, type);
}

char *getProgramMainDataPath(char *command_filename, char *base_path)
{
  // check if the program's main data base directory is configured
  if (!strEqual(base_path, "."))
    return getStringCopy(base_path);

  /* if the program is configured to start from current directory (default),
     determine program package directory from program binary (some versions
     of KDE/Konqueror and Mac OS X (especially "Mavericks") apparently do not
     set the current working directory to the program package directory) */
  char *main_data_path = getBasePath(command_filename);

#if defined(PLATFORM_MAC)
  if (strSuffix(main_data_path, MAC_APP_BINARY_SUBDIR))
  {
    char *main_data_path_old = main_data_path;

    // cut relative path to Mac OS X application binary directory from path
    main_data_path[strlen(main_data_path) -
		   strlen(MAC_APP_BINARY_SUBDIR)] = '\0';

    // cut trailing path separator from path (but not if path is root directory)
    if (strSuffix(main_data_path, "/") && !strEqual(main_data_path, "/"))
      main_data_path[strlen(main_data_path) - 1] = '\0';

    // replace empty path with current directory
    if (strEqual(main_data_path, ""))
      main_data_path = ".";

    // add relative path to Mac OS X application resources directory to path
    main_data_path = getPath2(main_data_path, MAC_APP_FILES_SUBDIR);

    free(main_data_path_old);
  }
#endif

  return main_data_path;
}

char *getProgramConfigFilename(char *command_filename)
{
  static char *config_filename_1 = NULL;
  static char *config_filename_2 = NULL;
  static char *config_filename_3 = NULL;
  static boolean initialized = FALSE;

  if (!initialized)
  {
    char *command_filename_1 = getStringCopy(command_filename);

    // strip trailing executable suffix from command filename
    if (strSuffix(command_filename_1, ".exe"))
      command_filename_1[strlen(command_filename_1) - 4] = '\0';

    char *base_path = getProgramMainDataPath(command_filename, BASE_PATH);
    char *conf_directory = getPath2(base_path, CONF_DIRECTORY);

    char *command_basepath = getBasePath(command_filename);
    char *command_basename = getBaseNameNoSuffix(command_filename);
    char *command_filename_2 = getPath2(command_basepath, command_basename);

    config_filename_1 = getStringCat2(command_filename_1, ".conf");
    config_filename_2 = getStringCat2(command_filename_2, ".conf");
    config_filename_3 = getPath2(conf_directory, SETUP_FILENAME);

    checked_free(base_path);
    checked_free(conf_directory);

    checked_free(command_basepath);
    checked_free(command_basename);

    checked_free(command_filename_1);
    checked_free(command_filename_2);

    initialized = TRUE;
  }

  // 1st try: look for config file that exactly matches the binary filename
  if (fileExists(config_filename_1))
    return config_filename_1;

  // 2nd try: look for config file that matches binary filename without suffix
  if (fileExists(config_filename_2))
    return config_filename_2;

  // 3rd try: return setup config filename in global program config directory
  return config_filename_3;
}

static char *getPlatformConfigFilename(char *config_filename)
{
  static char *platform_config_filename = NULL;
  static boolean initialized = FALSE;

  if (!initialized)
  {
    char *config_basepath = getBasePath(config_filename);
    char *config_basename = getBaseNameNoSuffix(config_filename);
    char *config_filename_prefix = getPath2(config_basepath, config_basename);
    char *platform_string_lower = getStringToLower(PLATFORM_STRING);
    char *platform_suffix = getStringCat2("-", platform_string_lower);

    platform_config_filename = getStringCat3(config_filename_prefix,
					     platform_suffix, ".conf");

    checked_free(config_basepath);
    checked_free(config_basename);
    checked_free(config_filename_prefix);
    checked_free(platform_string_lower);
    checked_free(platform_suffix);

    initialized = TRUE;
  }

  return platform_config_filename;
}

char *getTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, TAPEFILE_EXTENSION);
  filename = getPath2(getTapeDir(leveldir_current->subdir), basename);

  return filename;
}

char *getTemporaryTapeFilename(void)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "tmp.%s", TAPEFILE_EXTENSION);
  filename = getPath2(getTapeDir(NULL), basename);

  return filename;
}

char *getDefaultSolutionTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, TAPEFILE_EXTENSION);
  filename = getPath2(getSolutionTapeDir(), basename);

  return filename;
}

char *getSokobanSolutionTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.sln", nr);
  filename = getPath2(getSolutionTapeDir(), basename);

  return filename;
}

char *getSolutionTapeFilename(int nr)
{
  char *filename = getDefaultSolutionTapeFilename(nr);

  if (!fileExists(filename))
  {
    char *filename2 = getSokobanSolutionTapeFilename(nr);

    if (fileExists(filename2))
      return filename2;
  }

  return filename;
}

char *getScoreFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, SCOREFILE_EXTENSION);

  // used instead of "leveldir_current->subdir" (for network games)
  filename = getPath2(getScoreDir(levelset.identifier), basename);

  return filename;
}

char *getScoreCacheFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, SCOREFILE_EXTENSION);

  // used instead of "leveldir_current->subdir" (for network games)
  filename = getPath2(getScoreCacheDir(levelset.identifier), basename);

  return filename;
}

char *getScoreTapeBasename(char *name)
{
  static char basename[MAX_FILENAME_LEN];
  char basename_raw[MAX_FILENAME_LEN];
  char timestamp[20];

  sprintf(timestamp, "%s", getCurrentTimestamp());
  sprintf(basename_raw, "%s-%s", timestamp, name);
  sprintf(basename, "%s-%08x", timestamp, get_hash_from_key(basename_raw));

  return basename;
}

char *getScoreTapeFilename(char *basename_no_ext, int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%s.%s", basename_no_ext, TAPEFILE_EXTENSION);

  // used instead of "leveldir_current->subdir" (for network games)
  filename = getPath2(getScoreTapeDir(levelset.identifier, nr), basename);

  return filename;
}

char *getScoreCacheTapeFilename(char *basename_no_ext, int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%s.%s", basename_no_ext, TAPEFILE_EXTENSION);

  // used instead of "leveldir_current->subdir" (for network games)
  filename = getPath2(getScoreCacheTapeDir(levelset.identifier, nr), basename);

  return filename;
}

char *getSetupFilename(void)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getSetupDir(), SETUP_FILENAME);

  return filename;
}

char *getDefaultSetupFilename(void)
{
  return program.config_filename;
}

char *getPlatformSetupFilename(void)
{
  return getPlatformConfigFilename(program.config_filename);
}

char *getEditorSetupFilename(void)
{
  static char *filename = NULL;

  checked_free(filename);
  filename = getPath2(getCurrentLevelDir(), EDITORSETUP_FILENAME);

  if (fileExists(filename))
    return filename;

  checked_free(filename);
  filename = getPath2(getSetupDir(), EDITORSETUP_FILENAME);

  return filename;
}

char *getHelpAnimFilename(void)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getCurrentLevelDir(), HELPANIM_FILENAME);

  return filename;
}

char *getHelpTextFilename(void)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getCurrentLevelDir(), HELPTEXT_FILENAME);

  return filename;
}

static char *getLevelSetInfoBasename(int nr)
{
  static char basename[32];

  sprintf(basename, "levelset_%d.txt", nr + 1);

  return basename;
}

char *getLevelSetInfoFilename(int nr)
{
  char *basename = getLevelSetInfoBasename(nr);
  static char *info_subdir = NULL;
  static char *filename = NULL;

  if (info_subdir == NULL)
    info_subdir = getPath2(DOCS_DIRECTORY, LEVELSET_INFO_DIRECTORY);

  checked_free(filename);

  // look for level set info file the current level set directory
  filename = getPath3(getCurrentLevelDir(), info_subdir, basename);
  if (fileExists(filename))
    return filename;

  if (nr > 0)
    return NULL;

  char *basenames[] =
  {
    "README",
    "README.TXT",
    "README.txt",
    "Readme",
    "Readme.txt",
    "readme",
    "readme.txt",

    NULL
  };
  int i;

  for (i = 0; basenames[i] != NULL; i++)
  {
    checked_free(filename);
    filename = getPath2(getCurrentLevelDir(), basenames[i]);

    if (fileExists(filename))
      return filename;
  }

  return NULL;
}

static char *getLevelSetTitleMessageBasename(int nr, boolean initial)
{
  static char basename[32];

  sprintf(basename, "%s_%d.txt",
	  (initial ? "titlemessage_initial" : "titlemessage"), nr + 1);

  return basename;
}

char *getLevelSetTitleMessageFilename(int nr, boolean initial)
{
  static char *filename = NULL;
  char *basename;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getLevelSetTitleMessageBasename(nr, initial);

  if (!gfx.override_level_graphics)
  {
    // 1st try: look for special artwork in current level series directory
    filename = getPath3(getCurrentLevelDir(), GRAPHICS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    // 2nd try: look for message file in current level set directory
    filename = getPath2(getCurrentLevelDir(), basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    // check if there is special artwork configured in level series config
    if (getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) != NULL)
    {
      // 3rd try: look for special artwork configured in level series config
      filename = getPath2(getLevelArtworkDir(ARTWORK_TYPE_GRAPHICS), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      // take missing artwork configured in level set config from default
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    // 4th try: look for special artwork in configured artwork directory
    filename = getPath2(getSetupArtworkDir(artwork.gfx_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  // 5th try: look for default artwork in new default artwork directory
  filename = getPath2(getDefaultGraphicsDir(GFX_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  // 6th try: look for default artwork in old default artwork directory
  filename = getPath2(options.graphics_directory, basename);
  if (fileExists(filename))
    return filename;

  return NULL;		// cannot find specified artwork file anywhere
}

static char *getCreditsBasename(int nr)
{
  static char basename[32];

  sprintf(basename, "credits_%d.txt", nr + 1);

  return basename;
}

char *getCreditsFilename(int nr, boolean global)
{
  char *basename = getCreditsBasename(nr);
  char *basepath = NULL;
  static char *credits_subdir = NULL;
  static char *filename = NULL;

  if (credits_subdir == NULL)
    credits_subdir = getPath2(DOCS_DIRECTORY, CREDITS_DIRECTORY);

  checked_free(filename);

  // look for credits file in the game's base or current level set directory
  basepath = (global ? options.base_directory : getCurrentLevelDir());

  filename = getPath3(basepath, credits_subdir, basename);
  if (fileExists(filename))
    return filename;

  return NULL;		// cannot find credits file
}

static char *getProgramInfoBasename(int nr)
{
  static char basename[32];

  sprintf(basename, "program_%d.txt", nr + 1);

  return basename;
}

char *getProgramInfoFilename(int nr)
{
  char *basename = getProgramInfoBasename(nr);
  static char *info_subdir = NULL;
  static char *filename = NULL;

  if (info_subdir == NULL)
    info_subdir = getPath2(DOCS_DIRECTORY, PROGRAM_INFO_DIRECTORY);

  checked_free(filename);

  // look for program info file in the game's base directory
  filename = getPath3(options.base_directory, info_subdir, basename);
  if (fileExists(filename))
    return filename;

  return NULL;		// cannot find program info file
}

static char *getCorrectedArtworkBasename(char *basename)
{
  return basename;
}

char *getCustomImageFilename(char *basename)
{
  static char *filename = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getCorrectedArtworkBasename(basename);

  if (!gfx.override_level_graphics)
  {
    // 1st try: look for special artwork in current level series directory
    filename = getImg3(getCurrentLevelDir(), GRAPHICS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    // check if there is special artwork configured in level series config
    if (getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) != NULL)
    {
      // 2nd try: look for special artwork configured in level series config
      filename = getImg2(getLevelArtworkDir(ARTWORK_TYPE_GRAPHICS), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      // take missing artwork configured in level set config from default
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    // 3rd try: look for special artwork in configured artwork directory
    filename = getImg2(getSetupArtworkDir(artwork.gfx_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  // 4th try: look for default artwork in new default artwork directory
  filename = getImg2(getDefaultGraphicsDir(GFX_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  // 5th try: look for default artwork in old default artwork directory
  filename = getImg2(options.graphics_directory, basename);
  if (fileExists(filename))
    return filename;

  if (!strEqual(GFX_FALLBACK_FILENAME, UNDEFINED_FILENAME))
  {
    free(filename);

    WarnUsingFallback(basename);

    // 6th try: look for fallback artwork in old default artwork directory
    // (needed to prevent errors when trying to access unused artwork files)
    filename = getImg2(options.graphics_directory, GFX_FALLBACK_FILENAME);
    if (fileExists(filename))
      return filename;
  }

  return NULL;		// cannot find specified artwork file anywhere
}

char *getCustomSoundFilename(char *basename)
{
  static char *filename = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getCorrectedArtworkBasename(basename);

  if (!gfx.override_level_sounds)
  {
    // 1st try: look for special artwork in current level series directory
    filename = getPath3(getCurrentLevelDir(), SOUNDS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    // check if there is special artwork configured in level series config
    if (getLevelArtworkSet(ARTWORK_TYPE_SOUNDS) != NULL)
    {
      // 2nd try: look for special artwork configured in level series config
      filename = getPath2(getLevelArtworkDir(TREE_TYPE_SOUNDS_DIR), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      // take missing artwork configured in level set config from default
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    // 3rd try: look for special artwork in configured artwork directory
    filename = getPath2(getSetupArtworkDir(artwork.snd_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  // 4th try: look for default artwork in new default artwork directory
  filename = getPath2(getDefaultSoundsDir(SND_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  // 5th try: look for default artwork in old default artwork directory
  filename = getPath2(options.sounds_directory, basename);
  if (fileExists(filename))
    return filename;

  if (!strEqual(SND_FALLBACK_FILENAME, UNDEFINED_FILENAME))
  {
    free(filename);

    WarnUsingFallback(basename);

    // 6th try: look for fallback artwork in old default artwork directory
    // (needed to prevent errors when trying to access unused artwork files)
    filename = getPath2(options.sounds_directory, SND_FALLBACK_FILENAME);
    if (fileExists(filename))
      return filename;
  }

  return NULL;		// cannot find specified artwork file anywhere
}

char *getCustomMusicFilename(char *basename)
{
  static char *filename = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getCorrectedArtworkBasename(basename);

  if (!gfx.override_level_music)
  {
    // 1st try: look for special artwork in current level series directory
    filename = getPath3(getCurrentLevelDir(), MUSIC_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    // check if there is special artwork configured in level series config
    if (getLevelArtworkSet(ARTWORK_TYPE_MUSIC) != NULL)
    {
      // 2nd try: look for special artwork configured in level series config
      filename = getPath2(getLevelArtworkDir(TREE_TYPE_MUSIC_DIR), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      // take missing artwork configured in level set config from default
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    // 3rd try: look for special artwork in configured artwork directory
    filename = getPath2(getSetupArtworkDir(artwork.mus_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  // 4th try: look for default artwork in new default artwork directory
  filename = getPath2(getDefaultMusicDir(MUS_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  // 5th try: look for default artwork in old default artwork directory
  filename = getPath2(options.music_directory, basename);
  if (fileExists(filename))
    return filename;

  if (!strEqual(MUS_FALLBACK_FILENAME, UNDEFINED_FILENAME))
  {
    free(filename);

    WarnUsingFallback(basename);

    // 6th try: look for fallback artwork in old default artwork directory
    // (needed to prevent errors when trying to access unused artwork files)
    filename = getPath2(options.music_directory, MUS_FALLBACK_FILENAME);
    if (fileExists(filename))
      return filename;
  }

  return NULL;		// cannot find specified artwork file anywhere
}

char *getCustomArtworkFilename(char *basename, int type)
{
  if (type == ARTWORK_TYPE_GRAPHICS)
    return getCustomImageFilename(basename);
  else if (type == ARTWORK_TYPE_SOUNDS)
    return getCustomSoundFilename(basename);
  else if (type == ARTWORK_TYPE_MUSIC)
    return getCustomMusicFilename(basename);
  else
    return UNDEFINED_FILENAME;
}

char *getCustomArtworkConfigFilename(int type)
{
  return getCustomArtworkFilename(ARTWORKINFO_FILENAME(type), type);
}

char *getCustomArtworkLevelConfigFilename(int type)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getLevelArtworkDir(type), ARTWORKINFO_FILENAME(type));

  return filename;
}

static boolean directoryExists_CheckMusic(char *directory, boolean check_music)
{
  if (!directoryExists(directory))
    return FALSE;

  if (!check_music)
    return TRUE;

  Directory *dir;
  DirectoryEntry *dir_entry;
  int num_music = getMusicListSize();
  boolean music_found = FALSE;

  if ((dir = openDirectory(directory)) == NULL)
    return FALSE;

  while ((dir_entry = readDirectory(dir)) != NULL)	// loop all entries
  {
    char *basename = dir_entry->basename;
    boolean music_already_used = FALSE;
    int i;

    // skip all music files that are configured in music config file
    for (i = 0; i < num_music; i++)
    {
      struct FileInfo *music = getMusicListEntry(i);

      if (strEqual(basename, music->filename))
      {
	music_already_used = TRUE;

	break;
      }
    }

    if (music_already_used)
      continue;

    if (FileIsMusic(dir_entry->filename))
    {
      music_found = TRUE;

      break;
    }
  }

  closeDirectory(dir);

  return music_found;
}

static char *getCustomMusicDirectoryExt(boolean check_music)
{
  static char *directory = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(directory);

  if (!gfx.override_level_music)
  {
    // 1st try: look for special artwork in current level series directory
    directory = getPath2(getCurrentLevelDir(), MUSIC_DIRECTORY);
    if (directoryExists_CheckMusic(directory, check_music))
      return directory;

    free(directory);

    // check if there is special artwork configured in level series config
    if (getLevelArtworkSet(ARTWORK_TYPE_MUSIC) != NULL)
    {
      // 2nd try: look for special artwork configured in level series config
      directory = getStringCopy(getLevelArtworkDir(TREE_TYPE_MUSIC_DIR));
      if (directoryExists_CheckMusic(directory, check_music))
	return directory;

      free(directory);

      // take missing artwork configured in level set config from default
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    // 3rd try: look for special artwork in configured artwork directory
    directory = getStringCopy(getSetupArtworkDir(artwork.mus_current));
    if (directoryExists_CheckMusic(directory, check_music))
      return directory;

    free(directory);
  }

  // 4th try: look for default artwork in new default artwork directory
  directory = getStringCopy(getDefaultMusicDir(MUS_DEFAULT_SUBDIR));
  if (directoryExists_CheckMusic(directory, check_music))
    return directory;

  free(directory);

  // 5th try: look for default artwork in old default artwork directory
  directory = getStringCopy(options.music_directory);
  if (directoryExists_CheckMusic(directory, check_music))
    return directory;

  return NULL;		// cannot find specified artwork file anywhere
}

char *getCustomMusicDirectory(void)
{
  return getCustomMusicDirectoryExt(FALSE);
}

char *getCustomMusicDirectory_NoConf(void)
{
  return getCustomMusicDirectoryExt(TRUE);
}

void MarkTapeDirectoryUploadsAsComplete(char *level_subdir)
{
  char *filename = getPath2(getTapeDir(level_subdir), UPLOADED_FILENAME);

  touchFile(filename);

  checked_free(filename);
}

void MarkTapeDirectoryUploadsAsIncomplete(char *level_subdir)
{
  char *filename = getPath2(getTapeDir(level_subdir), UPLOADED_FILENAME);

  unlink(filename);

  checked_free(filename);
}

boolean CheckTapeDirectoryUploadsComplete(char *level_subdir)
{
  char *filename = getPath2(getTapeDir(level_subdir), UPLOADED_FILENAME);
  boolean success = fileExists(filename);

  checked_free(filename);

  return success;
}

void InitMissingFileHash(void)
{
  if (missing_file_hash == NULL)
    freeSetupFileHash(missing_file_hash);

  missing_file_hash = newSetupFileHash();
}

void InitTapeDirectory(char *level_subdir)
{
  boolean new_tape_dir = !directoryExists(getTapeDir(level_subdir));

  createDirectory(getUserGameDataDir(), "user data");
  createDirectory(getTapeDir(NULL), "main tape");
  createDirectory(getTapeDir(level_subdir), "level tape");

  if (new_tape_dir)
    MarkTapeDirectoryUploadsAsComplete(level_subdir);
}

void InitScoreDirectory(char *level_subdir)
{
  createDirectory(getMainUserGameDataDir(), "main user data");
  createDirectory(getScoreDir(NULL), "main score");
  createDirectory(getScoreDir(level_subdir), "level score");
}

void InitScoreCacheDirectory(char *level_subdir)
{
  createDirectory(getMainUserGameDataDir(), "main user data");
  createDirectory(getCacheDir(), "cache data");
  createDirectory(getScoreCacheDir(NULL), "main score");
  createDirectory(getScoreCacheDir(level_subdir), "level score");
}

void InitScoreTapeDirectory(char *level_subdir, int nr)
{
  InitScoreDirectory(level_subdir);

  createDirectory(getScoreTapeDir(level_subdir, nr), "score tape");
}

void InitScoreCacheTapeDirectory(char *level_subdir, int nr)
{
  InitScoreCacheDirectory(level_subdir);

  createDirectory(getScoreCacheTapeDir(level_subdir, nr), "score tape");
}

static void SaveUserLevelInfo(void);

void InitUserLevelDirectory(char *level_subdir)
{
  if (!directoryExists(getUserLevelDir(level_subdir)))
  {
    createDirectory(getMainUserGameDataDir(), "main user data");
    createDirectory(getUserLevelDir(NULL), "main user level");

    if (setup.internal.create_user_levelset)
    {
      createDirectory(getUserLevelDir(level_subdir), "user level");

      SaveUserLevelInfo();
    }
  }
}

void InitNetworkLevelDirectory(char *level_subdir)
{
  if (!directoryExists(getNetworkLevelDir(level_subdir)))
  {
    createDirectory(getMainUserGameDataDir(), "main user data");
    createDirectory(getNetworkDir(), "network data");
    createDirectory(getNetworkLevelDir(NULL), "main network level");
    createDirectory(getNetworkLevelDir(level_subdir), "network level");
  }
}

void InitLevelSetupDirectory(char *level_subdir)
{
  createDirectory(getUserGameDataDir(), "user data");
  createDirectory(getLevelSetupDir(NULL), "main level setup");
  createDirectory(getLevelSetupDir(level_subdir), "level setup");
}

static void InitCacheDirectory(void)
{
  createDirectory(getMainUserGameDataDir(), "main user data");
  createDirectory(getCacheDir(), "cache data");
}


// ----------------------------------------------------------------------------
// some functions to handle lists of level and artwork directories
// ----------------------------------------------------------------------------

TreeInfo *newTreeInfo(void)
{
  return checked_calloc(sizeof(TreeInfo));
}

TreeInfo *newTreeInfo_setDefaults(int type)
{
  TreeInfo *ti = newTreeInfo();

  setTreeInfoToDefaults(ti, type);

  return ti;
}

void pushTreeInfo(TreeInfo **node_first, TreeInfo *node_new)
{
  node_new->next = *node_first;
  *node_first = node_new;
}

void removeTreeInfo(TreeInfo **node_first)
{
  TreeInfo *node_old = *node_first;

  *node_first = node_old->next;
  node_old->next = NULL;

  freeTreeInfo(node_old);
}

int numTreeInfo(TreeInfo *node)
{
  int num = 0;

  while (node)
  {
    num++;
    node = node->next;
  }

  return num;
}

boolean validLevelSeries(TreeInfo *node)
{
  // in a number of cases, tree node is no valid level set
  if (node == NULL || node->node_group || node->parent_link || node->is_copy)
    return FALSE;

  return TRUE;
}

TreeInfo *getValidLevelSeries(TreeInfo *node, TreeInfo *default_node)
{
  if (validLevelSeries(node))
    return node;
  else if (node->is_copy)
    return getTreeInfoFromIdentifier(leveldir_first, node->identifier);
  else
    return getFirstValidTreeInfoEntry(default_node);
}

static TreeInfo *getValidTreeInfoEntryExt(TreeInfo *node, boolean get_next_node)
{
  if (node == NULL)
    return NULL;

  if (node->node_group)		// enter node group (step down into tree)
    return getFirstValidTreeInfoEntry(node->node_group);

  if (node->parent_link)	// skip first node (back link) of node group
    get_next_node = TRUE;

  if (!get_next_node)		// get current regular tree node
    return node;

  // get next regular tree node, or step up until one is found
  while (node->next == NULL && node->node_parent != NULL)
    node = node->node_parent;

  return getFirstValidTreeInfoEntry(node->next);
}

TreeInfo *getFirstValidTreeInfoEntry(TreeInfo *node)
{
  return getValidTreeInfoEntryExt(node, FALSE);
}

TreeInfo *getNextValidTreeInfoEntry(TreeInfo *node)
{
  return getValidTreeInfoEntryExt(node, TRUE);
}

TreeInfo *getTreeInfoFirstGroupEntry(TreeInfo *node)
{
  if (node == NULL)
    return NULL;

  if (node->node_parent == NULL)		// top level group
    return *node->node_top;
  else						// sub level group
    return node->node_parent->node_group;
}

int numTreeInfoInGroup(TreeInfo *node)
{
  return numTreeInfo(getTreeInfoFirstGroupEntry(node));
}

int getPosFromTreeInfo(TreeInfo *node)
{
  TreeInfo *node_cmp = getTreeInfoFirstGroupEntry(node);
  int pos = 0;

  while (node_cmp)
  {
    if (node_cmp == node)
      return pos;

    pos++;
    node_cmp = node_cmp->next;
  }

  return 0;
}

TreeInfo *getTreeInfoFromPos(TreeInfo *node, int pos)
{
  TreeInfo *node_default = node;
  int pos_cmp = 0;

  while (node)
  {
    if (pos_cmp == pos)
      return node;

    pos_cmp++;
    node = node->next;
  }

  return node_default;
}

static TreeInfo *getTreeInfoFromIdentifierExt(TreeInfo *node, char *identifier,
					      int node_type_wanted)
{
  if (identifier == NULL)
    return NULL;

  while (node)
  {
    if (TREE_NODE_TYPE(node) == node_type_wanted &&
	strEqual(identifier, node->identifier))
      return node;

    if (node->node_group)
    {
      TreeInfo *node_group = getTreeInfoFromIdentifierExt(node->node_group,
							  identifier,
							  node_type_wanted);
      if (node_group)
	return node_group;
    }

    node = node->next;
  }

  return NULL;
}

TreeInfo *getTreeInfoFromIdentifier(TreeInfo *node, char *identifier)
{
  return getTreeInfoFromIdentifierExt(node, identifier, TREE_NODE_TYPE_DEFAULT);
}

static TreeInfo *cloneTreeNode(TreeInfo **node_top, TreeInfo *node_parent,
			       TreeInfo *node, boolean skip_sets_without_levels)
{
  TreeInfo *node_new;

  if (node == NULL)
    return NULL;

  if (!node->parent_link && !node->level_group &&
      skip_sets_without_levels && node->levels == 0)
    return cloneTreeNode(node_top, node_parent, node->next,
			 skip_sets_without_levels);

  node_new = getTreeInfoCopy(node);		// copy complete node

  node_new->node_top = node_top;		// correct top node link
  node_new->node_parent = node_parent;		// correct parent node link

  if (node->level_group)
    node_new->node_group = cloneTreeNode(node_top, node_new, node->node_group,
					 skip_sets_without_levels);

  node_new->next = cloneTreeNode(node_top, node_parent, node->next,
				 skip_sets_without_levels);
  
  return node_new;
}

static void cloneTree(TreeInfo **ti_new, TreeInfo *ti, boolean skip_empty_sets)
{
  TreeInfo *ti_cloned = cloneTreeNode(ti_new, NULL, ti, skip_empty_sets);

  *ti_new = ti_cloned;
}

static boolean adjustTreeGraphicsForEMC(TreeInfo *node)
{
  boolean settings_changed = FALSE;

  while (node)
  {
    boolean want_ecs = (setup.prefer_aga_graphics == FALSE);
    boolean want_aga = (setup.prefer_aga_graphics == TRUE);
    boolean has_only_ecs = (!node->graphics_set && !node->graphics_set_aga);
    boolean has_only_aga = (!node->graphics_set && !node->graphics_set_ecs);
    char *graphics_set = NULL;

    if (node->graphics_set_ecs && (want_ecs || has_only_ecs))
      graphics_set = node->graphics_set_ecs;

    if (node->graphics_set_aga && (want_aga || has_only_aga))
      graphics_set = node->graphics_set_aga;

    if (graphics_set && !strEqual(node->graphics_set, graphics_set))
    {
      setString(&node->graphics_set, graphics_set);
      settings_changed = TRUE;
    }

    if (node->node_group != NULL)
      settings_changed |= adjustTreeGraphicsForEMC(node->node_group);

    node = node->next;
  }

  return settings_changed;
}

static boolean adjustTreeSoundsForEMC(TreeInfo *node)
{
  boolean settings_changed = FALSE;

  while (node)
  {
    boolean want_default = (setup.prefer_lowpass_sounds == FALSE);
    boolean want_lowpass = (setup.prefer_lowpass_sounds == TRUE);
    boolean has_only_default = (!node->sounds_set && !node->sounds_set_lowpass);
    boolean has_only_lowpass = (!node->sounds_set && !node->sounds_set_default);
    char *sounds_set = NULL;

    if (node->sounds_set_default && (want_default || has_only_default))
      sounds_set = node->sounds_set_default;

    if (node->sounds_set_lowpass && (want_lowpass || has_only_lowpass))
      sounds_set = node->sounds_set_lowpass;

    if (sounds_set && !strEqual(node->sounds_set, sounds_set))
    {
      setString(&node->sounds_set, sounds_set);
      settings_changed = TRUE;
    }

    if (node->node_group != NULL)
      settings_changed |= adjustTreeSoundsForEMC(node->node_group);

    node = node->next;
  }

  return settings_changed;
}

int dumpTreeInfo(TreeInfo *node, int depth)
{
  char bullet_list[] = { '-', '*', 'o' };
  int num_leaf_nodes = 0;
  int i;

  if (depth == 0)
    Debug("tree", "Dumping TreeInfo:");

  while (node)
  {
    char bullet = bullet_list[depth % ARRAY_SIZE(bullet_list)];

    for (i = 0; i < depth * 2; i++)
      DebugContinued("", " ");

    DebugContinued("tree", "%c '%s' ['%s] [PARENT: '%s'] %s\n",
		   bullet, node->name, node->identifier,
		   (node->node_parent ? node->node_parent->identifier : "-"),
		   (node->node_group ? "[GROUP]" :
		    node->is_copy ? "[COPY]" : ""));

    if (!node->node_group && !node->parent_link)
      num_leaf_nodes++;

    /*
    // use for dumping artwork info tree
    Debug("tree", "subdir == '%s' ['%s', '%s'] [%d])",
	  node->subdir, node->fullpath, node->basepath, node->in_user_dir);
    */

    if (node->node_group != NULL)
      num_leaf_nodes += dumpTreeInfo(node->node_group, depth + 1);

    node = node->next;
  }

  if (depth == 0)
    Debug("tree", "Summary: %d leaf nodes found", num_leaf_nodes);

  return num_leaf_nodes;
}

void sortTreeInfoBySortFunction(TreeInfo **node_first,
				int (*compare_function)(const void *,
							const void *))
{
  int num_nodes = numTreeInfo(*node_first);
  TreeInfo **sort_array;
  TreeInfo *node = *node_first;
  int i = 0;

  if (num_nodes == 0)
    return;

  // allocate array for sorting structure pointers
  sort_array = checked_calloc(num_nodes * sizeof(TreeInfo *));

  // writing structure pointers to sorting array
  while (i < num_nodes && node)		// double boundary check...
  {
    sort_array[i] = node;

    i++;
    node = node->next;
  }

  // sorting the structure pointers in the sorting array
  qsort(sort_array, num_nodes, sizeof(TreeInfo *),
	compare_function);

  // update the linkage of list elements with the sorted node array
  for (i = 0; i < num_nodes - 1; i++)
    sort_array[i]->next = sort_array[i + 1];
  sort_array[num_nodes - 1]->next = NULL;

  // update the linkage of the main list anchor pointer
  *node_first = sort_array[0];

  free(sort_array);

  // now recursively sort the level group structures
  node = *node_first;
  while (node)
  {
    if (node->node_group != NULL)
      sortTreeInfoBySortFunction(&node->node_group, compare_function);

    node = node->next;
  }
}

void sortTreeInfo(TreeInfo **node_first)
{
  sortTreeInfoBySortFunction(node_first, compareTreeInfoEntries);
}


// ============================================================================
// some stuff from "files.c"
// ============================================================================

#if defined(PLATFORM_WINDOWS)
#ifndef S_IRGRP
#define S_IRGRP S_IRUSR
#endif
#ifndef S_IROTH
#define S_IROTH S_IRUSR
#endif
#ifndef S_IWGRP
#define S_IWGRP S_IWUSR
#endif
#ifndef S_IWOTH
#define S_IWOTH S_IWUSR
#endif
#ifndef S_IXGRP
#define S_IXGRP S_IXUSR
#endif
#ifndef S_IXOTH
#define S_IXOTH S_IXUSR
#endif
#ifndef S_IRWXG
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif
#ifndef S_ISGID
#define S_ISGID 0
#endif
#endif	// PLATFORM_WINDOWS

// file permissions for newly written files
#define MODE_R_ALL		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_W_ALL		(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_X_ALL		(S_IXUSR | S_IXGRP | S_IXOTH)

#define MODE_W_PRIVATE		(S_IWUSR)
#define MODE_W_PUBLIC_FILE	(S_IWUSR | S_IWGRP)
#define MODE_W_PUBLIC_DIR	(S_IWUSR | S_IWGRP | S_ISGID)

#define DIR_PERMS_PRIVATE	(MODE_R_ALL | MODE_X_ALL | MODE_W_PRIVATE)
#define DIR_PERMS_PUBLIC	(MODE_R_ALL | MODE_X_ALL | MODE_W_PUBLIC_DIR)
#define DIR_PERMS_PUBLIC_ALL	(MODE_R_ALL | MODE_X_ALL | MODE_W_ALL)

#define FILE_PERMS_PRIVATE	(MODE_R_ALL | MODE_W_PRIVATE)
#define FILE_PERMS_PUBLIC	(MODE_R_ALL | MODE_W_PUBLIC_FILE)
#define FILE_PERMS_PUBLIC_ALL	(MODE_R_ALL | MODE_W_ALL)


char *getHomeDir(void)
{
  static char *dir = NULL;

#if defined(PLATFORM_WINDOWS)
  if (dir == NULL)
  {
    dir = checked_malloc(MAX_PATH + 1);

    if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, dir)))
      strcpy(dir, ".");
  }
#elif defined(PLATFORM_EMSCRIPTEN)
  dir = PERSISTENT_DIRECTORY;
#elif defined(PLATFORM_UNIX)
  if (dir == NULL)
  {
    if ((dir = getenv("HOME")) == NULL)
    {
      dir = getUnixHomeDir();

      if (dir != NULL)
	dir = getStringCopy(dir);
      else
	dir = ".";
    }
  }
#else
  dir = ".";
#endif

  return dir;
}

char *getPersonalDataDir(void)
{
  static char *personal_data_dir = NULL;

#if defined(PLATFORM_MAC)
  if (personal_data_dir == NULL)
    personal_data_dir = getPath2(getHomeDir(), "Documents");
#else
  if (personal_data_dir == NULL)
    personal_data_dir = getHomeDir();
#endif

  return personal_data_dir;
}

char *getMainUserGameDataDir(void)
{
  static char *main_user_data_dir = NULL;

#if defined(PLATFORM_ANDROID)
  if (main_user_data_dir == NULL)
    main_user_data_dir = (char *)(SDL_AndroidGetExternalStorageState() &
				  SDL_ANDROID_EXTERNAL_STORAGE_WRITE ?
				  SDL_AndroidGetExternalStoragePath() :
				  SDL_AndroidGetInternalStoragePath());
#else
  if (main_user_data_dir == NULL)
    main_user_data_dir = getPath2(getPersonalDataDir(),
				  program.userdata_subdir);
#endif

  return main_user_data_dir;
}

char *getUserGameDataDir(void)
{
  if (user.nr == 0)
    return getMainUserGameDataDir();
  else
    return getUserDir(user.nr);
}

char *getSetupDir(void)
{
  return getUserGameDataDir();
}

static mode_t posix_umask(mode_t mask)
{
#if defined(PLATFORM_UNIX)
  return umask(mask);
#else
  return 0;
#endif
}

static int posix_mkdir(const char *pathname, mode_t mode)
{
#if defined(PLATFORM_WINDOWS)
  return mkdir(pathname);
#else
  return mkdir(pathname, mode);
#endif
}

static boolean posix_process_running_setgid(void)
{
#if defined(PLATFORM_UNIX)
  return (getgid() != getegid());
#else
  return FALSE;
#endif
}

void createDirectory(char *dir, char *text)
{
  if (directoryExists(dir))
    return;

  // leave "other" permissions in umask untouched, but ensure group parts
  // of USERDATA_DIR_MODE are not masked
  int permission_class = PERMS_PRIVATE;
  mode_t dir_mode = (permission_class == PERMS_PRIVATE ?
		     DIR_PERMS_PRIVATE : DIR_PERMS_PUBLIC);
  mode_t last_umask = posix_umask(0);
  mode_t group_umask = ~(dir_mode & S_IRWXG);
  int running_setgid = posix_process_running_setgid();

  if (permission_class == PERMS_PUBLIC)
  {
    // if we're setgid, protect files against "other"
    // else keep umask(0) to make the dir world-writable

    if (running_setgid)
      posix_umask(last_umask & group_umask);
    else
      dir_mode = DIR_PERMS_PUBLIC_ALL;
  }

  if (posix_mkdir(dir, dir_mode) != 0)
    Warn("cannot create %s directory '%s': %s", text, dir, strerror(errno));

  if (permission_class == PERMS_PUBLIC && !running_setgid)
    chmod(dir, dir_mode);

  posix_umask(last_umask);		// restore previous umask
}

void InitMainUserDataDirectory(void)
{
  createDirectory(getMainUserGameDataDir(), "main user data");
}

void InitUserDataDirectory(void)
{
  createDirectory(getMainUserGameDataDir(), "main user data");

  if (user.nr != 0)
  {
    createDirectory(getUserDir(-1), "users");
    createDirectory(getUserDir(user.nr), "user data");
  }
}

void SetFilePermissions(char *filename, int permission_class)
{
  int running_setgid = posix_process_running_setgid();
  int perms = (permission_class == PERMS_PRIVATE ?
	       FILE_PERMS_PRIVATE : FILE_PERMS_PUBLIC);

  if (permission_class == PERMS_PUBLIC && !running_setgid)
    perms = FILE_PERMS_PUBLIC_ALL;

  chmod(filename, perms);
}

void fprintFileHeader(FILE *file, char *basename)
{
  char *prefix = "# ";
  char *sep1 = "=";

  fprintf_line_with_prefix(file, prefix, sep1, 77);
  fprintf(file, "%s%s\n", prefix, basename);
  fprintf_line_with_prefix(file, prefix, sep1, 77);
  fprintf(file, "\n");
}

int getFileVersionFromCookieString(const char *cookie)
{
  const char *ptr_cookie1, *ptr_cookie2;
  const char *pattern1 = "_FILE_VERSION_";
  const char *pattern2 = "?.?";
  const int len_cookie = strlen(cookie);
  const int len_pattern1 = strlen(pattern1);
  const int len_pattern2 = strlen(pattern2);
  const int len_pattern = len_pattern1 + len_pattern2;
  int version_super, version_major;

  if (len_cookie <= len_pattern)
    return -1;

  ptr_cookie1 = &cookie[len_cookie - len_pattern];
  ptr_cookie2 = &cookie[len_cookie - len_pattern2];

  if (strncmp(ptr_cookie1, pattern1, len_pattern1) != 0)
    return -1;

  if (ptr_cookie2[0] < '0' || ptr_cookie2[0] > '9' ||
      ptr_cookie2[1] != '.' ||
      ptr_cookie2[2] < '0' || ptr_cookie2[2] > '9')
    return -1;

  version_super = ptr_cookie2[0] - '0';
  version_major = ptr_cookie2[2] - '0';

  return VERSION_IDENT(version_super, version_major, 0, 0);
}

boolean checkCookieString(const char *cookie, const char *template)
{
  const char *pattern = "_FILE_VERSION_?.?";
  const int len_cookie = strlen(cookie);
  const int len_template = strlen(template);
  const int len_pattern = strlen(pattern);

  if (len_cookie != len_template)
    return FALSE;

  if (strncmp(cookie, template, len_cookie - len_pattern) != 0)
    return FALSE;

  return TRUE;
}


// ----------------------------------------------------------------------------
// setup file list and hash handling functions
// ----------------------------------------------------------------------------

char *getFormattedSetupEntry(char *token, char *value)
{
  int i;
  static char entry[MAX_LINE_LEN];

  // if value is an empty string, just return token without value
  if (*value == '\0')
    return token;

  // start with the token and some spaces to format output line
  sprintf(entry, "%s:", token);
  for (i = strlen(entry); i < token_value_position; i++)
    strcat(entry, " ");

  // continue with the token's value
  strcat(entry, value);

  return entry;
}

SetupFileList *newSetupFileList(char *token, char *value)
{
  SetupFileList *new = checked_malloc(sizeof(SetupFileList));

  new->token = getStringCopy(token);
  new->value = getStringCopy(value);

  new->next = NULL;

  return new;
}

void freeSetupFileList(SetupFileList *list)
{
  if (list == NULL)
    return;

  checked_free(list->token);
  checked_free(list->value);

  if (list->next)
    freeSetupFileList(list->next);

  free(list);
}

char *getListEntry(SetupFileList *list, char *token)
{
  if (list == NULL)
    return NULL;

  if (strEqual(list->token, token))
    return list->value;
  else
    return getListEntry(list->next, token);
}

SetupFileList *setListEntry(SetupFileList *list, char *token, char *value)
{
  if (list == NULL)
    return NULL;

  if (strEqual(list->token, token))
  {
    checked_free(list->value);

    list->value = getStringCopy(value);

    return list;
  }
  else if (list->next == NULL)
    return (list->next = newSetupFileList(token, value));
  else
    return setListEntry(list->next, token, value);
}

SetupFileList *addListEntry(SetupFileList *list, char *token, char *value)
{
  if (list == NULL)
    return NULL;

  if (list->next == NULL)
    return (list->next = newSetupFileList(token, value));
  else
    return addListEntry(list->next, token, value);
}

#if ENABLE_UNUSED_CODE
#ifdef DEBUG
static void printSetupFileList(SetupFileList *list)
{
  if (!list)
    return;

  Debug("setup:printSetupFileList", "token: '%s'", list->token);
  Debug("setup:printSetupFileList", "value: '%s'", list->value);

  printSetupFileList(list->next);
}
#endif
#endif

#ifdef DEBUG
DEFINE_HASHTABLE_INSERT(insert_hash_entry, char, char);
DEFINE_HASHTABLE_SEARCH(search_hash_entry, char, char);
DEFINE_HASHTABLE_CHANGE(change_hash_entry, char, char);
DEFINE_HASHTABLE_REMOVE(remove_hash_entry, char, char);
#else
#define insert_hash_entry hashtable_insert
#define search_hash_entry hashtable_search
#define change_hash_entry hashtable_change
#define remove_hash_entry hashtable_remove
#endif

unsigned int get_hash_from_key(void *key)
{
  /*
    djb2

    This algorithm (k=33) was first reported by Dan Bernstein many years ago in
    'comp.lang.c'. Another version of this algorithm (now favored by Bernstein)
    uses XOR: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33 (why
    it works better than many other constants, prime or not) has never been
    adequately explained.

    If you just want to have a good hash function, and cannot wait, djb2
    is one of the best string hash functions i know. It has excellent
    distribution and speed on many different sets of keys and table sizes.
    You are not likely to do better with one of the "well known" functions
    such as PJW, K&R, etc.

    Ozan (oz) Yigit [http://www.cs.yorku.ca/~oz/hash.html]
  */

  char *str = (char *)key;
  unsigned int hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;	// hash * 33 + c

  return hash;
}

int hash_keys_are_equal(void *key1, void *key2)
{
  return (strEqual((char *)key1, (char *)key2));
}

SetupFileHash *newSetupFileHash(void)
{
  SetupFileHash *new_hash =
    create_hashtable(16, 0.75, get_hash_from_key, hash_keys_are_equal);

  if (new_hash == NULL)
    Fail("create_hashtable() failed -- out of memory");

  return new_hash;
}

void freeSetupFileHash(SetupFileHash *hash)
{
  if (hash == NULL)
    return;

  hashtable_destroy(hash, 1);	// 1 == also free values stored in hash
}

char *getHashEntry(SetupFileHash *hash, char *token)
{
  if (hash == NULL)
    return NULL;

  return search_hash_entry(hash, token);
}

void setHashEntry(SetupFileHash *hash, char *token, char *value)
{
  char *value_copy;

  if (hash == NULL)
    return;

  value_copy = getStringCopy(value);

  // change value; if it does not exist, insert it as new
  if (!change_hash_entry(hash, token, value_copy))
    if (!insert_hash_entry(hash, getStringCopy(token), value_copy))
      Fail("cannot insert into hash -- aborting");
}

char *removeHashEntry(SetupFileHash *hash, char *token)
{
  if (hash == NULL)
    return NULL;

  return remove_hash_entry(hash, token);
}

#if ENABLE_UNUSED_CODE
#if DEBUG
static void printSetupFileHash(SetupFileHash *hash)
{
  BEGIN_HASH_ITERATION(hash, itr)
  {
    Debug("setup:printSetupFileHash", "token: '%s'", HASH_ITERATION_TOKEN(itr));
    Debug("setup:printSetupFileHash", "value: '%s'", HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(hash, itr)
}
#endif
#endif

#define ALLOW_TOKEN_VALUE_SEPARATOR_BEING_WHITESPACE		1
#define CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING		0
#define CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH		0

static boolean token_value_separator_found = FALSE;
#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
static boolean token_value_separator_warning = FALSE;
#endif
#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
static boolean token_already_exists_warning = FALSE;
#endif

static boolean getTokenValueFromSetupLineExt(char *line,
					     char **token_ptr, char **value_ptr,
					     char *filename, char *line_raw,
					     int line_nr,
					     boolean separator_required)
{
  static char line_copy[MAX_LINE_LEN + 1], line_raw_copy[MAX_LINE_LEN + 1];
  char *token, *value, *line_ptr;

  // when externally invoked via ReadTokenValueFromLine(), copy line buffers
  if (line_raw == NULL)
  {
    strncpy(line_copy, line, MAX_LINE_LEN);
    line_copy[MAX_LINE_LEN] = '\0';
    line = line_copy;

    strcpy(line_raw_copy, line_copy);
    line_raw = line_raw_copy;
  }

  // cut trailing comment from input line
  for (line_ptr = line; *line_ptr; line_ptr++)
  {
    if (*line_ptr == '#')
    {
      *line_ptr = '\0';
      break;
    }
  }

  // cut trailing whitespaces from input line
  for (line_ptr = &line[strlen(line)]; line_ptr >= line; line_ptr--)
    if ((*line_ptr == ' ' || *line_ptr == '\t') && *(line_ptr + 1) == '\0')
      *line_ptr = '\0';

  // ignore empty lines
  if (*line == '\0')
    return FALSE;

  // cut leading whitespaces from token
  for (token = line; *token; token++)
    if (*token != ' ' && *token != '\t')
      break;

  // start with empty value as reliable default
  value = "";

  token_value_separator_found = FALSE;

  // find end of token to determine start of value
  for (line_ptr = token; *line_ptr; line_ptr++)
  {
    // first look for an explicit token/value separator, like ':' or '='
    if (*line_ptr == ':' || *line_ptr == '=')
    {
      *line_ptr = '\0';			// terminate token string
      value = line_ptr + 1;		// set beginning of value

      token_value_separator_found = TRUE;

      break;
    }
  }

#if ALLOW_TOKEN_VALUE_SEPARATOR_BEING_WHITESPACE
  // fallback: if no token/value separator found, also allow whitespaces
  if (!token_value_separator_found && !separator_required)
  {
    for (line_ptr = token; *line_ptr; line_ptr++)
    {
      if (*line_ptr == ' ' || *line_ptr == '\t')
      {
	*line_ptr = '\0';		// terminate token string
	value = line_ptr + 1;		// set beginning of value

	token_value_separator_found = TRUE;

	break;
      }
    }

#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
    if (token_value_separator_found)
    {
      if (!token_value_separator_warning)
      {
	Debug("setup", "---");

	if (filename != NULL)
	{
	  Debug("setup", "missing token/value separator(s) in config file:");
	  Debug("setup", "- config file: '%s'", filename);
	}
	else
	{
	  Debug("setup", "missing token/value separator(s):");
	}

	token_value_separator_warning = TRUE;
      }

      if (filename != NULL)
	Debug("setup", "- line %d: '%s'", line_nr, line_raw);
      else
	Debug("setup", "- line: '%s'", line_raw);
    }
#endif
  }
#endif

  // cut trailing whitespaces from token
  for (line_ptr = &token[strlen(token)]; line_ptr >= token; line_ptr--)
    if ((*line_ptr == ' ' || *line_ptr == '\t') && *(line_ptr + 1) == '\0')
      *line_ptr = '\0';

  // cut leading whitespaces from value
  for (; *value; value++)
    if (*value != ' ' && *value != '\t')
      break;

  *token_ptr = token;
  *value_ptr = value;

  return TRUE;
}

boolean getTokenValueFromSetupLine(char *line, char **token, char **value)
{
  // while the internal (old) interface does not require a token/value
  // separator (for downwards compatibility with existing files which
  // don't use them), it is mandatory for the external (new) interface

  return getTokenValueFromSetupLineExt(line, token, value, NULL, NULL, 0, TRUE);
}

static boolean loadSetupFileData(void *setup_file_data, char *filename,
				 boolean top_recursion_level, boolean is_hash)
{
  static SetupFileHash *include_filename_hash = NULL;
  char line[MAX_LINE_LEN], line_raw[MAX_LINE_LEN], previous_line[MAX_LINE_LEN];
  char *token, *value, *line_ptr;
  void *insert_ptr = NULL;
  boolean read_continued_line = FALSE;
  File *file;
  int line_nr = 0, token_count = 0, include_count = 0;

#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
  token_value_separator_warning = FALSE;
#endif

#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
  token_already_exists_warning = FALSE;
#endif

  if (!(file = openFile(filename, MODE_READ)))
  {
#if DEBUG_NO_CONFIG_FILE
    Debug("setup", "cannot open configuration file '%s'", filename);
#endif

    return FALSE;
  }

  // use "insert pointer" to store list end for constant insertion complexity
  if (!is_hash)
    insert_ptr = setup_file_data;

  // on top invocation, create hash to mark included files (to prevent loops)
  if (top_recursion_level)
    include_filename_hash = newSetupFileHash();

  // mark this file as already included (to prevent including it again)
  setHashEntry(include_filename_hash, getBaseNamePtr(filename), "true");

  while (!checkEndOfFile(file))
  {
    // read next line of input file
    if (!getStringFromFile(file, line, MAX_LINE_LEN))
      break;

    // check if line was completely read and is terminated by line break
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
      line_nr++;

    // cut trailing line break (this can be newline and/or carriage return)
    for (line_ptr = &line[strlen(line)]; line_ptr >= line; line_ptr--)
      if ((*line_ptr == '\n' || *line_ptr == '\r') && *(line_ptr + 1) == '\0')
	*line_ptr = '\0';

    // copy raw input line for later use (mainly debugging output)
    strcpy(line_raw, line);

    if (read_continued_line)
    {
      // append new line to existing line, if there is enough space
      if (strlen(previous_line) + strlen(line_ptr) < MAX_LINE_LEN)
	strcat(previous_line, line_ptr);

      strcpy(line, previous_line);	// copy storage buffer to line

      read_continued_line = FALSE;
    }

    // if the last character is '\', continue at next line
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\\')
    {
      line[strlen(line) - 1] = '\0';	// cut off trailing backslash
      strcpy(previous_line, line);	// copy line to storage buffer

      read_continued_line = TRUE;

      continue;
    }

    if (!getTokenValueFromSetupLineExt(line, &token, &value, filename,
				       line_raw, line_nr, FALSE))
      continue;

    if (*token)
    {
      if (strEqual(token, "include"))
      {
	if (getHashEntry(include_filename_hash, value) == NULL)
	{
	  char *basepath = getBasePath(filename);
	  char *basename = getBaseName(value);
	  char *filename_include = getPath2(basepath, basename);

	  loadSetupFileData(setup_file_data, filename_include, FALSE, is_hash);

	  free(basepath);
	  free(basename);
	  free(filename_include);

	  include_count++;
	}
	else
	{
	  Warn("ignoring already processed file '%s'", value);
	}
      }
      else
      {
	if (is_hash)
	{
#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
	  char *old_value =
	    getHashEntry((SetupFileHash *)setup_file_data, token);

	  if (old_value != NULL)
	  {
	    if (!token_already_exists_warning)
	    {
	      Debug("setup", "---");
	      Debug("setup", "duplicate token(s) found in config file:");
	      Debug("setup", "- config file: '%s'", filename);

	      token_already_exists_warning = TRUE;
	    }

	    Debug("setup", "- token: '%s' (in line %d)", token, line_nr);
	    Debug("setup", "  old value: '%s'", old_value);
	    Debug("setup", "  new value: '%s'", value);
	  }
#endif

	  setHashEntry((SetupFileHash *)setup_file_data, token, value);
	}
	else
	{
	  insert_ptr = addListEntry((SetupFileList *)insert_ptr, token, value);
	}

	token_count++;
      }
    }
  }

  closeFile(file);

#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
  if (token_value_separator_warning)
    Debug("setup", "---");
#endif

#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
  if (token_already_exists_warning)
    Debug("setup", "---");
#endif

  if (token_count == 0 && include_count == 0)
    Warn("configuration file '%s' is empty", filename);

  if (top_recursion_level)
    freeSetupFileHash(include_filename_hash);

  return TRUE;
}

static int compareSetupFileData(const void *object1, const void *object2)
{
  const struct ConfigInfo *entry1 = (struct ConfigInfo *)object1;
  const struct ConfigInfo *entry2 = (struct ConfigInfo *)object2;

  return strcmp(entry1->token, entry2->token);
}

static void saveSetupFileHash(SetupFileHash *hash, char *filename)
{
  int item_count = hashtable_count(hash);
  int item_size = sizeof(struct ConfigInfo);
  struct ConfigInfo *sort_array = checked_malloc(item_count * item_size);
  FILE *file;
  int i = 0;

  // copy string pointers from hash to array
  BEGIN_HASH_ITERATION(hash, itr)
  {
    sort_array[i].token = HASH_ITERATION_TOKEN(itr);
    sort_array[i].value = HASH_ITERATION_VALUE(itr);

    i++;

    if (i > item_count)		// should never happen
      break;
  }
  END_HASH_ITERATION(hash, itr)

  // sort string pointers from hash in array
  qsort(sort_array, item_count, item_size, compareSetupFileData);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write configuration file '%s'", filename);

    return;
  }

  fprintf(file, "%s\n\n", getFormattedSetupEntry("program.version",
						 program.version_string));
  for (i = 0; i < item_count; i++)
    fprintf(file, "%s\n", getFormattedSetupEntry(sort_array[i].token,
						 sort_array[i].value));
  fclose(file);

  checked_free(sort_array);
}

SetupFileList *loadSetupFileList(char *filename)
{
  SetupFileList *setup_file_list = newSetupFileList("", "");
  SetupFileList *first_valid_list_entry;

  if (!loadSetupFileData(setup_file_list, filename, TRUE, FALSE))
  {
    freeSetupFileList(setup_file_list);

    return NULL;
  }

  first_valid_list_entry = setup_file_list->next;

  // free empty list header
  setup_file_list->next = NULL;
  freeSetupFileList(setup_file_list);

  return first_valid_list_entry;
}

SetupFileHash *loadSetupFileHash(char *filename)
{
  SetupFileHash *setup_file_hash = newSetupFileHash();

  if (!loadSetupFileData(setup_file_hash, filename, TRUE, TRUE))
  {
    freeSetupFileHash(setup_file_hash);

    return NULL;
  }

  return setup_file_hash;
}


// ============================================================================
// setup file stuff
// ============================================================================

#define TOKEN_STR_LAST_LEVEL_SERIES		"last_level_series"
#define TOKEN_STR_LAST_PLAYED_MENU_USED		"last_played_menu_used"
#define TOKEN_STR_LAST_PLAYED_LEVEL		"last_played_level"
#define TOKEN_STR_HANDICAP_LEVEL		"handicap_level"
#define TOKEN_STR_LAST_USER			"last_user"

// level directory info
#define LEVELINFO_TOKEN_IDENTIFIER		0
#define LEVELINFO_TOKEN_NAME			1
#define LEVELINFO_TOKEN_NAME_SORTING		2
#define LEVELINFO_TOKEN_AUTHOR			3
#define LEVELINFO_TOKEN_YEAR			4
#define LEVELINFO_TOKEN_PROGRAM_TITLE		5
#define LEVELINFO_TOKEN_PROGRAM_COPYRIGHT	6
#define LEVELINFO_TOKEN_PROGRAM_COMPANY		7
#define LEVELINFO_TOKEN_IMPORTED_FROM		8
#define LEVELINFO_TOKEN_IMPORTED_BY		9
#define LEVELINFO_TOKEN_TESTED_BY		10
#define LEVELINFO_TOKEN_LEVELS			11
#define LEVELINFO_TOKEN_FIRST_LEVEL		12
#define LEVELINFO_TOKEN_SORT_PRIORITY		13
#define LEVELINFO_TOKEN_LATEST_ENGINE		14
#define LEVELINFO_TOKEN_LEVEL_GROUP		15
#define LEVELINFO_TOKEN_READONLY		16
#define LEVELINFO_TOKEN_GRAPHICS_SET_ECS	17
#define LEVELINFO_TOKEN_GRAPHICS_SET_AGA	18
#define LEVELINFO_TOKEN_GRAPHICS_SET		19
#define LEVELINFO_TOKEN_SOUNDS_SET_DEFAULT	20
#define LEVELINFO_TOKEN_SOUNDS_SET_LOWPASS	21
#define LEVELINFO_TOKEN_SOUNDS_SET		22
#define LEVELINFO_TOKEN_MUSIC_SET		23
#define LEVELINFO_TOKEN_FILENAME		24
#define LEVELINFO_TOKEN_FILETYPE		25
#define LEVELINFO_TOKEN_SPECIAL_FLAGS		26
#define LEVELINFO_TOKEN_EMPTY_LEVEL_NAME	27
#define LEVELINFO_TOKEN_FORCE_LEVEL_NAME	28
#define LEVELINFO_TOKEN_HANDICAP		29
#define LEVELINFO_TOKEN_TIME_LIMIT		30
#define LEVELINFO_TOKEN_SKIP_LEVELS		31
#define LEVELINFO_TOKEN_USE_EMC_TILES		32
#define LEVELINFO_TOKEN_INFO_SCREENS_FROM_MAIN	33

#define NUM_LEVELINFO_TOKENS			34

static LevelDirTree ldi;

static struct TokenInfo levelinfo_tokens[] =
{
  // level directory info
  { TYPE_STRING,	&ldi.identifier,	"identifier"		},
  { TYPE_STRING,	&ldi.name,		"name"			},
  { TYPE_STRING,	&ldi.name_sorting,	"name_sorting"		},
  { TYPE_STRING,	&ldi.author,		"author"		},
  { TYPE_STRING,	&ldi.year,		"year"			},
  { TYPE_STRING,	&ldi.program_title,	"program_title"		},
  { TYPE_STRING,	&ldi.program_copyright,	"program_copyright"	},
  { TYPE_STRING,	&ldi.program_company,	"program_company"	},
  { TYPE_STRING,	&ldi.imported_from,	"imported_from"		},
  { TYPE_STRING,	&ldi.imported_by,	"imported_by"		},
  { TYPE_STRING,	&ldi.tested_by,		"tested_by"		},
  { TYPE_INTEGER,	&ldi.levels,		"levels"		},
  { TYPE_INTEGER,	&ldi.first_level,	"first_level"		},
  { TYPE_INTEGER,	&ldi.sort_priority,	"sort_priority"		},
  { TYPE_BOOLEAN,	&ldi.latest_engine,	"latest_engine"		},
  { TYPE_BOOLEAN,	&ldi.level_group,	"level_group"		},
  { TYPE_BOOLEAN,	&ldi.readonly,		"readonly"		},
  { TYPE_STRING,	&ldi.graphics_set_ecs,	"graphics_set.ecs"	},
  { TYPE_STRING,	&ldi.graphics_set_aga,	"graphics_set.aga"	},
  { TYPE_STRING,	&ldi.graphics_set,	"graphics_set"		},
  { TYPE_STRING,	&ldi.sounds_set_default,"sounds_set.default"	},
  { TYPE_STRING,	&ldi.sounds_set_lowpass,"sounds_set.lowpass"	},
  { TYPE_STRING,	&ldi.sounds_set,	"sounds_set"		},
  { TYPE_STRING,	&ldi.music_set,		"music_set"		},
  { TYPE_STRING,	&ldi.level_filename,	"filename"		},
  { TYPE_STRING,	&ldi.level_filetype,	"filetype"		},
  { TYPE_STRING,	&ldi.special_flags,	"special_flags"		},
  { TYPE_STRING,	&ldi.empty_level_name,	"empty_level_name"	},
  { TYPE_BOOLEAN,	&ldi.force_level_name,	"force_level_name"	},
  { TYPE_BOOLEAN,	&ldi.handicap,		"handicap"		},
  { TYPE_BOOLEAN,	&ldi.time_limit,	"time_limit"		},
  { TYPE_BOOLEAN,	&ldi.skip_levels,	"skip_levels"		},
  { TYPE_BOOLEAN,	&ldi.use_emc_tiles,	"use_emc_tiles"		},
  { TYPE_BOOLEAN,	&ldi.info_screens_from_main, "info_screens_from_main" }
};

static struct TokenInfo artworkinfo_tokens[] =
{
  // artwork directory info
  { TYPE_STRING,	&ldi.identifier,	"identifier"		},
  { TYPE_STRING,	&ldi.subdir,		"subdir"		},
  { TYPE_STRING,	&ldi.name,		"name"			},
  { TYPE_STRING,	&ldi.name_sorting,	"name_sorting"		},
  { TYPE_STRING,	&ldi.author,		"author"		},
  { TYPE_STRING,	&ldi.program_title,	"program_title"		},
  { TYPE_STRING,	&ldi.program_copyright,	"program_copyright"	},
  { TYPE_STRING,	&ldi.program_company,	"program_company"	},
  { TYPE_INTEGER,	&ldi.sort_priority,	"sort_priority"		},
  { TYPE_STRING,	&ldi.basepath,		"basepath"		},
  { TYPE_STRING,	&ldi.fullpath,		"fullpath"		},
  { TYPE_BOOLEAN,	&ldi.in_user_dir,	"in_user_dir"		},
  { TYPE_STRING,	&ldi.class_desc,	"class_desc"		},

  { -1,			NULL,			NULL			},
};

static char *optional_tokens[] =
{
  "program_title",
  "program_copyright",
  "program_company",

  NULL
};

static void setTreeInfoToDefaults(TreeInfo *ti, int type)
{
  ti->type = type;

  ti->node_top = (ti->type == TREE_TYPE_LEVEL_DIR    ? &leveldir_first :
		  ti->type == TREE_TYPE_GRAPHICS_DIR ? &artwork.gfx_first :
		  ti->type == TREE_TYPE_SOUNDS_DIR   ? &artwork.snd_first :
		  ti->type == TREE_TYPE_MUSIC_DIR    ? &artwork.mus_first :
		  NULL);

  ti->node_parent = NULL;
  ti->node_group = NULL;
  ti->next = NULL;

  ti->cl_first = -1;
  ti->cl_cursor = -1;

  ti->subdir = NULL;
  ti->fullpath = NULL;
  ti->basepath = NULL;
  ti->identifier = NULL;
  ti->name = getStringCopy(ANONYMOUS_NAME);
  ti->name_sorting = NULL;
  ti->author = getStringCopy(ANONYMOUS_NAME);
  ti->year = NULL;

  ti->program_title = NULL;
  ti->program_copyright = NULL;
  ti->program_company = NULL;

  ti->sort_priority = LEVELCLASS_UNDEFINED;	// default: least priority
  ti->latest_engine = FALSE;			// default: get from level
  ti->parent_link = FALSE;
  ti->is_copy = FALSE;
  ti->in_user_dir = FALSE;
  ti->user_defined = FALSE;
  ti->color = 0;
  ti->class_desc = NULL;

  ti->infotext = getStringCopy(TREE_INFOTEXT(ti->type));

  if (ti->type == TREE_TYPE_LEVEL_DIR)
  {
    ti->imported_from = NULL;
    ti->imported_by = NULL;
    ti->tested_by = NULL;

    ti->graphics_set_ecs = NULL;
    ti->graphics_set_aga = NULL;
    ti->graphics_set = NULL;
    ti->sounds_set_default = NULL;
    ti->sounds_set_lowpass = NULL;
    ti->sounds_set = NULL;
    ti->music_set = NULL;
    ti->graphics_path = getStringCopy(UNDEFINED_FILENAME);
    ti->sounds_path = getStringCopy(UNDEFINED_FILENAME);
    ti->music_path = getStringCopy(UNDEFINED_FILENAME);

    ti->level_filename = NULL;
    ti->level_filetype = NULL;

    ti->special_flags = NULL;

    ti->empty_level_name = NULL;
    ti->force_level_name = FALSE;

    ti->levels = 0;
    ti->first_level = 0;
    ti->last_level = 0;
    ti->level_group = FALSE;
    ti->handicap_level = 0;
    ti->readonly = TRUE;
    ti->handicap = TRUE;
    ti->time_limit = TRUE;
    ti->skip_levels = FALSE;

    ti->use_emc_tiles = FALSE;
    ti->info_screens_from_main = FALSE;
  }
}

static void setTreeInfoToDefaultsFromParent(TreeInfo *ti, TreeInfo *parent)
{
  if (parent == NULL)
  {
    Warn("setTreeInfoToDefaultsFromParent(): parent == NULL");

    setTreeInfoToDefaults(ti, TREE_TYPE_UNDEFINED);

    return;
  }

  // copy all values from the parent structure

  ti->type = parent->type;

  ti->node_top = parent->node_top;
  ti->node_parent = parent;
  ti->node_group = NULL;
  ti->next = NULL;

  ti->cl_first = -1;
  ti->cl_cursor = -1;

  ti->subdir = NULL;
  ti->fullpath = NULL;
  ti->basepath = NULL;
  ti->identifier = NULL;
  ti->name = getStringCopy(ANONYMOUS_NAME);
  ti->name_sorting = NULL;
  ti->author = getStringCopy(parent->author);
  ti->year = getStringCopy(parent->year);

  ti->program_title = getStringCopy(parent->program_title);
  ti->program_copyright = getStringCopy(parent->program_copyright);
  ti->program_company = getStringCopy(parent->program_company);

  ti->sort_priority = parent->sort_priority;
  ti->latest_engine = parent->latest_engine;
  ti->parent_link = FALSE;
  ti->is_copy = FALSE;
  ti->in_user_dir = parent->in_user_dir;
  ti->user_defined = parent->user_defined;
  ti->color = parent->color;
  ti->class_desc = getStringCopy(parent->class_desc);

  ti->infotext = getStringCopy(parent->infotext);

  if (ti->type == TREE_TYPE_LEVEL_DIR)
  {
    ti->imported_from = getStringCopy(parent->imported_from);
    ti->imported_by = getStringCopy(parent->imported_by);
    ti->tested_by = getStringCopy(parent->tested_by);

    ti->graphics_set_ecs = getStringCopy(parent->graphics_set_ecs);
    ti->graphics_set_aga = getStringCopy(parent->graphics_set_aga);
    ti->graphics_set = getStringCopy(parent->graphics_set);
    ti->sounds_set_default = getStringCopy(parent->sounds_set_default);
    ti->sounds_set_lowpass = getStringCopy(parent->sounds_set_lowpass);
    ti->sounds_set = getStringCopy(parent->sounds_set);
    ti->music_set = getStringCopy(parent->music_set);
    ti->graphics_path = getStringCopy(UNDEFINED_FILENAME);
    ti->sounds_path = getStringCopy(UNDEFINED_FILENAME);
    ti->music_path = getStringCopy(UNDEFINED_FILENAME);

    ti->level_filename = getStringCopy(parent->level_filename);
    ti->level_filetype = getStringCopy(parent->level_filetype);

    ti->special_flags = getStringCopy(parent->special_flags);

    ti->empty_level_name = getStringCopy(parent->empty_level_name);
    ti->force_level_name = parent->force_level_name;

    ti->levels = parent->levels;
    ti->first_level = parent->first_level;
    ti->last_level = parent->last_level;
    ti->level_group = FALSE;
    ti->handicap_level = parent->handicap_level;
    ti->readonly = parent->readonly;
    ti->handicap = parent->handicap;
    ti->time_limit = parent->time_limit;
    ti->skip_levels = parent->skip_levels;

    ti->use_emc_tiles = parent->use_emc_tiles;
    ti->info_screens_from_main = parent->info_screens_from_main;
  }
}

static TreeInfo *getTreeInfoCopy(TreeInfo *ti)
{
  TreeInfo *ti_copy = newTreeInfo();

  // copy all values from the original structure

  ti_copy->type			= ti->type;

  ti_copy->node_top		= ti->node_top;
  ti_copy->node_parent		= ti->node_parent;
  ti_copy->node_group		= ti->node_group;
  ti_copy->next			= ti->next;

  ti_copy->cl_first		= ti->cl_first;
  ti_copy->cl_cursor		= ti->cl_cursor;

  ti_copy->subdir		= getStringCopy(ti->subdir);
  ti_copy->fullpath		= getStringCopy(ti->fullpath);
  ti_copy->basepath		= getStringCopy(ti->basepath);
  ti_copy->identifier		= getStringCopy(ti->identifier);
  ti_copy->name			= getStringCopy(ti->name);
  ti_copy->name_sorting		= getStringCopy(ti->name_sorting);
  ti_copy->author		= getStringCopy(ti->author);
  ti_copy->year			= getStringCopy(ti->year);

  ti_copy->program_title	= getStringCopy(ti->program_title);
  ti_copy->program_copyright	= getStringCopy(ti->program_copyright);
  ti_copy->program_company	= getStringCopy(ti->program_company);

  ti_copy->imported_from	= getStringCopy(ti->imported_from);
  ti_copy->imported_by		= getStringCopy(ti->imported_by);
  ti_copy->tested_by		= getStringCopy(ti->tested_by);

  ti_copy->graphics_set_ecs	= getStringCopy(ti->graphics_set_ecs);
  ti_copy->graphics_set_aga	= getStringCopy(ti->graphics_set_aga);
  ti_copy->graphics_set		= getStringCopy(ti->graphics_set);
  ti_copy->sounds_set_default	= getStringCopy(ti->sounds_set_default);
  ti_copy->sounds_set_lowpass	= getStringCopy(ti->sounds_set_lowpass);
  ti_copy->sounds_set		= getStringCopy(ti->sounds_set);
  ti_copy->music_set		= getStringCopy(ti->music_set);
  ti_copy->graphics_path	= getStringCopy(ti->graphics_path);
  ti_copy->sounds_path		= getStringCopy(ti->sounds_path);
  ti_copy->music_path		= getStringCopy(ti->music_path);

  ti_copy->level_filename	= getStringCopy(ti->level_filename);
  ti_copy->level_filetype	= getStringCopy(ti->level_filetype);

  ti_copy->special_flags	= getStringCopy(ti->special_flags);

  ti_copy->empty_level_name	= getStringCopy(ti->empty_level_name);
  ti_copy->force_level_name	= ti->force_level_name;

  ti_copy->levels		= ti->levels;
  ti_copy->first_level		= ti->first_level;
  ti_copy->last_level		= ti->last_level;
  ti_copy->sort_priority	= ti->sort_priority;

  ti_copy->latest_engine	= ti->latest_engine;

  ti_copy->level_group		= ti->level_group;
  ti_copy->parent_link		= ti->parent_link;
  ti_copy->is_copy		= ti->is_copy;
  ti_copy->in_user_dir		= ti->in_user_dir;
  ti_copy->user_defined		= ti->user_defined;
  ti_copy->readonly		= ti->readonly;
  ti_copy->handicap		= ti->handicap;
  ti_copy->time_limit		= ti->time_limit;
  ti_copy->skip_levels		= ti->skip_levels;

  ti_copy->use_emc_tiles	= ti->use_emc_tiles;
  ti_copy->info_screens_from_main = ti->info_screens_from_main;

  ti_copy->color		= ti->color;
  ti_copy->class_desc		= getStringCopy(ti->class_desc);
  ti_copy->handicap_level	= ti->handicap_level;

  ti_copy->infotext		= getStringCopy(ti->infotext);

  return ti_copy;
}

void freeTreeInfo(TreeInfo *ti)
{
  if (ti == NULL)
    return;

  checked_free(ti->subdir);
  checked_free(ti->fullpath);
  checked_free(ti->basepath);
  checked_free(ti->identifier);

  checked_free(ti->name);
  checked_free(ti->name_sorting);
  checked_free(ti->author);
  checked_free(ti->year);

  checked_free(ti->program_title);
  checked_free(ti->program_copyright);
  checked_free(ti->program_company);

  checked_free(ti->class_desc);

  checked_free(ti->infotext);

  if (ti->type == TREE_TYPE_LEVEL_DIR)
  {
    checked_free(ti->imported_from);
    checked_free(ti->imported_by);
    checked_free(ti->tested_by);

    checked_free(ti->graphics_set_ecs);
    checked_free(ti->graphics_set_aga);
    checked_free(ti->graphics_set);
    checked_free(ti->sounds_set_default);
    checked_free(ti->sounds_set_lowpass);
    checked_free(ti->sounds_set);
    checked_free(ti->music_set);

    checked_free(ti->graphics_path);
    checked_free(ti->sounds_path);
    checked_free(ti->music_path);

    checked_free(ti->level_filename);
    checked_free(ti->level_filetype);

    checked_free(ti->special_flags);
  }

  // recursively free child node
  if (ti->node_group)
    freeTreeInfo(ti->node_group);

  // recursively free next node
  if (ti->next)
    freeTreeInfo(ti->next);

  checked_free(ti);
}

void setSetupInfo(struct TokenInfo *token_info,
		  int token_nr, char *token_value)
{
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;

  if (token_value == NULL)
    return;

  // set setup field to corresponding token value
  switch (token_type)
  {
    case TYPE_BOOLEAN:
    case TYPE_SWITCH:
      *(boolean *)setup_value = get_boolean_from_string(token_value);
      break;

    case TYPE_SWITCH3:
      *(int *)setup_value = get_switch3_from_string(token_value);
      break;

    case TYPE_KEY:
      *(Key *)setup_value = getKeyFromKeyName(token_value);
      break;

    case TYPE_KEY_X11:
      *(Key *)setup_value = getKeyFromX11KeyName(token_value);
      break;

    case TYPE_INTEGER:
      *(int *)setup_value = get_integer_from_string(token_value);
      break;

    case TYPE_STRING:
      checked_free(*(char **)setup_value);
      *(char **)setup_value = getStringCopy(token_value);
      break;

    case TYPE_PLAYER:
      *(int *)setup_value = get_player_nr_from_string(token_value);
      break;

    default:
      break;
  }
}

static int compareTreeInfoEntries(const void *object1, const void *object2)
{
  const TreeInfo *entry1 = *((TreeInfo **)object1);
  const TreeInfo *entry2 = *((TreeInfo **)object2);
  int tree_sorting1 = TREE_SORTING(entry1);
  int tree_sorting2 = TREE_SORTING(entry2);

  if (tree_sorting1 != tree_sorting2)
    return (tree_sorting1 - tree_sorting2);
  else
    return strcasecmp(entry1->name_sorting, entry2->name_sorting);
}

static TreeInfo *createParentTreeInfoNode(TreeInfo *node_parent)
{
  TreeInfo *ti_new;

  if (node_parent == NULL)
    return NULL;

  ti_new = newTreeInfo();
  setTreeInfoToDefaults(ti_new, node_parent->type);

  ti_new->node_parent = node_parent;
  ti_new->parent_link = TRUE;

  setString(&ti_new->identifier, node_parent->identifier);
  setString(&ti_new->name, BACKLINK_TEXT_PARENT);
  setString(&ti_new->name_sorting, ti_new->name);

  setString(&ti_new->subdir, STRING_PARENT_DIRECTORY);
  setString(&ti_new->fullpath, node_parent->fullpath);

  ti_new->sort_priority = LEVELCLASS_PARENT;
  ti_new->latest_engine = node_parent->latest_engine;

  setString(&ti_new->class_desc, getLevelClassDescription(ti_new));

  pushTreeInfo(&node_parent->node_group, ti_new);

  return ti_new;
}

static TreeInfo *createTopTreeInfoNode(TreeInfo *node_first)
{
  if (node_first == NULL)
    return NULL;

  TreeInfo *ti_new = newTreeInfo();
  int type = node_first->type;

  setTreeInfoToDefaults(ti_new, type);

  ti_new->node_parent = NULL;
  ti_new->parent_link = FALSE;

  setString(&ti_new->identifier, "top_tree_node");
  setString(&ti_new->name, TREE_INFOTEXT(type));
  setString(&ti_new->name_sorting, ti_new->name);

  setString(&ti_new->subdir, STRING_TOP_DIRECTORY);
  setString(&ti_new->fullpath, ".");

  ti_new->sort_priority = LEVELCLASS_TOP;
  ti_new->latest_engine = node_first->latest_engine;

  setString(&ti_new->class_desc, TREE_INFOTEXT(type));

  ti_new->node_group = node_first;
  ti_new->level_group = TRUE;

  TreeInfo *ti_new2 = createParentTreeInfoNode(ti_new);

  setString(&ti_new2->name, TREE_BACKLINK_TEXT(type));
  setString(&ti_new2->name_sorting, ti_new2->name);

  return ti_new;
}

static void setTreeInfoParentNodes(TreeInfo *node, TreeInfo *node_parent)
{
  while (node)
  {
    if (node->node_group)
      setTreeInfoParentNodes(node->node_group, node);

    node->node_parent = node_parent;

    node = node->next;
  }
}

TreeInfo *addTopTreeInfoNode(TreeInfo *node_first)
{
  // add top tree node with back link node in previous tree
  node_first = createTopTreeInfoNode(node_first);

  // set all parent links (back links) in complete tree
  setTreeInfoParentNodes(node_first, NULL);

  return node_first;
}


// ----------------------------------------------------------------------------
// functions for handling level and custom artwork info cache
// ----------------------------------------------------------------------------

static void LoadArtworkInfoCache(void)
{
  InitCacheDirectory();

  if (artworkinfo_cache_old == NULL)
  {
    char *filename = getPath2(getCacheDir(), ARTWORKINFO_CACHE_FILE);

    // try to load artwork info hash from already existing cache file
    artworkinfo_cache_old = loadSetupFileHash(filename);

    // try to get program version that artwork info cache was written with
    char *version = getHashEntry(artworkinfo_cache_old, "program.version");

    // check program version of artwork info cache against current version
    if (!strEqual(version, program.version_string))
    {
      freeSetupFileHash(artworkinfo_cache_old);

      artworkinfo_cache_old = NULL;
    }

    // if no artwork info cache file was found, start with empty hash
    if (artworkinfo_cache_old == NULL)
      artworkinfo_cache_old = newSetupFileHash();

    free(filename);
  }

  if (artworkinfo_cache_new == NULL)
    artworkinfo_cache_new = newSetupFileHash();

  update_artworkinfo_cache = FALSE;
}

static void SaveArtworkInfoCache(void)
{
  if (!update_artworkinfo_cache)
    return;

  char *filename = getPath2(getCacheDir(), ARTWORKINFO_CACHE_FILE);

  InitCacheDirectory();

  saveSetupFileHash(artworkinfo_cache_new, filename);

  free(filename);
}

static char *getCacheTokenPrefix(char *prefix1, char *prefix2)
{
  static char *prefix = NULL;

  checked_free(prefix);

  prefix = getStringCat2WithSeparator(prefix1, prefix2, ".");

  return prefix;
}

// (identical to above function, but separate string buffer needed -- nasty)
static char *getCacheToken(char *prefix, char *suffix)
{
  static char *token = NULL;

  checked_free(token);

  token = getStringCat2WithSeparator(prefix, suffix, ".");

  return token;
}

static char *getFileTimestampString(char *filename)
{
  return getStringCopy(i_to_a(getFileTimestampEpochSeconds(filename)));
}

static boolean modifiedFileTimestamp(char *filename, char *timestamp_string)
{
  struct stat file_status;

  if (timestamp_string == NULL)
    return TRUE;

  if (!fileExists(filename))			// file does not exist
    return (atoi(timestamp_string) != 0);

  if (stat(filename, &file_status) != 0)	// cannot stat file
    return TRUE;

  return (file_status.st_mtime != atoi(timestamp_string));
}

static TreeInfo *getArtworkInfoCacheEntry(LevelDirTree *level_node, int type)
{
  char *identifier = level_node->subdir;
  char *type_string = ARTWORK_DIRECTORY(type);
  char *token_prefix = getCacheTokenPrefix(type_string, identifier);
  char *token_main = getCacheToken(token_prefix, "CACHED");
  char *cache_entry = getHashEntry(artworkinfo_cache_old, token_main);
  boolean cached = (cache_entry != NULL && strEqual(cache_entry, "true"));
  TreeInfo *artwork_info = NULL;

  if (!use_artworkinfo_cache)
    return NULL;

  if (optional_tokens_hash == NULL)
  {
    int i;

    // create hash from list of optional tokens (for quick access)
    optional_tokens_hash = newSetupFileHash();
    for (i = 0; optional_tokens[i] != NULL; i++)
      setHashEntry(optional_tokens_hash, optional_tokens[i], "");
  }

  if (cached)
  {
    int i;

    artwork_info = newTreeInfo();
    setTreeInfoToDefaults(artwork_info, type);

    // set all structure fields according to the token/value pairs
    ldi = *artwork_info;
    for (i = 0; artworkinfo_tokens[i].type != -1; i++)
    {
      char *token_suffix = artworkinfo_tokens[i].text;
      char *token = getCacheToken(token_prefix, token_suffix);
      char *value = getHashEntry(artworkinfo_cache_old, token);
      boolean optional =
	(getHashEntry(optional_tokens_hash, token_suffix) != NULL);

      setSetupInfo(artworkinfo_tokens, i, value);

      // check if cache entry for this item is mandatory, but missing
      if (value == NULL && !optional)
      {
	Warn("missing cache entry '%s'", token);

	cached = FALSE;
      }
    }

    *artwork_info = ldi;
  }

  if (cached)
  {
    char *filename_levelinfo = getPath2(getLevelDirFromTreeInfo(level_node),
					LEVELINFO_FILENAME);
    char *filename_artworkinfo = getPath2(getSetupArtworkDir(artwork_info),
					  ARTWORKINFO_FILENAME(type));

    // check if corresponding "levelinfo.conf" file has changed
    token_main = getCacheToken(token_prefix, "TIMESTAMP_LEVELINFO");
    cache_entry = getHashEntry(artworkinfo_cache_old, token_main);

    if (modifiedFileTimestamp(filename_levelinfo, cache_entry))
      cached = FALSE;

    // check if corresponding "<artworkinfo>.conf" file has changed
    token_main = getCacheToken(token_prefix, "TIMESTAMP_ARTWORKINFO");
    cache_entry = getHashEntry(artworkinfo_cache_old, token_main);

    if (modifiedFileTimestamp(filename_artworkinfo, cache_entry))
      cached = FALSE;

    checked_free(filename_levelinfo);
    checked_free(filename_artworkinfo);
  }

  if (!cached && artwork_info != NULL)
  {
    freeTreeInfo(artwork_info);

    return NULL;
  }

  return artwork_info;
}

static void setArtworkInfoCacheEntry(TreeInfo *artwork_info,
				     LevelDirTree *level_node, int type)
{
  char *identifier = level_node->subdir;
  char *type_string = ARTWORK_DIRECTORY(type);
  char *token_prefix = getCacheTokenPrefix(type_string, identifier);
  char *token_main = getCacheToken(token_prefix, "CACHED");
  boolean set_cache_timestamps = TRUE;
  int i;

  setHashEntry(artworkinfo_cache_new, token_main, "true");

  if (set_cache_timestamps)
  {
    char *filename_levelinfo = getPath2(getLevelDirFromTreeInfo(level_node),
					LEVELINFO_FILENAME);
    char *filename_artworkinfo = getPath2(getSetupArtworkDir(artwork_info),
					  ARTWORKINFO_FILENAME(type));
    char *timestamp_levelinfo = getFileTimestampString(filename_levelinfo);
    char *timestamp_artworkinfo = getFileTimestampString(filename_artworkinfo);

    token_main = getCacheToken(token_prefix, "TIMESTAMP_LEVELINFO");
    setHashEntry(artworkinfo_cache_new, token_main, timestamp_levelinfo);

    token_main = getCacheToken(token_prefix, "TIMESTAMP_ARTWORKINFO");
    setHashEntry(artworkinfo_cache_new, token_main, timestamp_artworkinfo);

    checked_free(filename_levelinfo);
    checked_free(filename_artworkinfo);
    checked_free(timestamp_levelinfo);
    checked_free(timestamp_artworkinfo);
  }

  ldi = *artwork_info;
  for (i = 0; artworkinfo_tokens[i].type != -1; i++)
  {
    char *token = getCacheToken(token_prefix, artworkinfo_tokens[i].text);
    char *value = getSetupValue(artworkinfo_tokens[i].type,
				artworkinfo_tokens[i].value);
    if (value != NULL)
      setHashEntry(artworkinfo_cache_new, token, value);
  }
}


// ----------------------------------------------------------------------------
// functions for loading level info and custom artwork info
// ----------------------------------------------------------------------------

int GetZipFileTreeType(char *zip_filename)
{
  static char *top_dir_path = NULL;
  static char *top_dir_conf_filename[NUM_BASE_TREE_TYPES] = { NULL };
  static char *conf_basename[NUM_BASE_TREE_TYPES] =
  {
    GRAPHICSINFO_FILENAME,
    SOUNDSINFO_FILENAME,
    MUSICINFO_FILENAME,
    LEVELINFO_FILENAME
  };
  int j;

  checked_free(top_dir_path);
  top_dir_path = NULL;

  for (j = 0; j < NUM_BASE_TREE_TYPES; j++)
  {
    checked_free(top_dir_conf_filename[j]);
    top_dir_conf_filename[j] = NULL;
  }

  char **zip_entries = zip_list(zip_filename);

  // check if zip file successfully opened
  if (zip_entries == NULL || zip_entries[0] == NULL)
    return TREE_TYPE_UNDEFINED;

  // first zip file entry is expected to be top level directory
  char *top_dir = zip_entries[0];

  // check if valid top level directory found in zip file
  if (!strSuffix(top_dir, "/"))
    return TREE_TYPE_UNDEFINED;

  // get filenames of valid configuration files in top level directory
  for (j = 0; j < NUM_BASE_TREE_TYPES; j++)
    top_dir_conf_filename[j] = getStringCat2(top_dir, conf_basename[j]);

  int tree_type = TREE_TYPE_UNDEFINED;
  int e = 0;

  while (zip_entries[e] != NULL)
  {
    // check if every zip file entry is below top level directory
    if (!strPrefix(zip_entries[e], top_dir))
      return TREE_TYPE_UNDEFINED;

    // check if this zip file entry is a valid configuration filename
    for (j = 0; j < NUM_BASE_TREE_TYPES; j++)
    {
      if (strEqual(zip_entries[e], top_dir_conf_filename[j]))
      {
	// only exactly one valid configuration file allowed
	if (tree_type != TREE_TYPE_UNDEFINED)
	  return TREE_TYPE_UNDEFINED;

	tree_type = j;
      }
    }

    e++;
  }

  return tree_type;
}

static boolean CheckZipFileForDirectory(char *zip_filename, char *directory,
					int tree_type)
{
  static char *top_dir_path = NULL;
  static char *top_dir_conf_filename = NULL;

  checked_free(top_dir_path);
  checked_free(top_dir_conf_filename);

  top_dir_path = NULL;
  top_dir_conf_filename = NULL;

  char *conf_basename = (tree_type == TREE_TYPE_LEVEL_DIR ? LEVELINFO_FILENAME :
			 ARTWORKINFO_FILENAME(tree_type));

  // check if valid configuration filename determined
  if (conf_basename == NULL || strEqual(conf_basename, ""))
    return FALSE;

  char **zip_entries = zip_list(zip_filename);

  // check if zip file successfully opened
  if (zip_entries == NULL || zip_entries[0] == NULL)
    return FALSE;

  // first zip file entry is expected to be top level directory
  char *top_dir = zip_entries[0];

  // check if valid top level directory found in zip file
  if (!strSuffix(top_dir, "/"))
    return FALSE;

  // get path of extracted top level directory
  top_dir_path = getPath2(directory, top_dir);

  // remove trailing directory separator from top level directory path
  // (required to be able to check for file and directory in next step)
  top_dir_path[strlen(top_dir_path) - 1] = '\0';

  // check if zip file's top level directory already exists in target directory
  if (fileExists(top_dir_path))		// (checks for file and directory)
    return FALSE;

  // get filename of configuration file in top level directory
  top_dir_conf_filename = getStringCat2(top_dir, conf_basename);

  boolean found_top_dir_conf_filename = FALSE;
  int i = 0;

  while (zip_entries[i] != NULL)
  {
    // check if every zip file entry is below top level directory
    if (!strPrefix(zip_entries[i], top_dir))
      return FALSE;

    // check if this zip file entry is the configuration filename
    if (strEqual(zip_entries[i], top_dir_conf_filename))
      found_top_dir_conf_filename = TRUE;

    i++;
  }

  // check if valid configuration filename was found in zip file
  if (!found_top_dir_conf_filename)
    return FALSE;

  return TRUE;
}

char *ExtractZipFileIntoDirectory(char *zip_filename, char *directory,
				  int tree_type)
{
  boolean zip_file_valid = CheckZipFileForDirectory(zip_filename, directory,
						    tree_type);

  if (!zip_file_valid)
  {
    Warn("zip file '%s' rejected!", zip_filename);

    return NULL;
  }

  char **zip_entries = zip_extract(zip_filename, directory);

  if (zip_entries == NULL)
  {
    Warn("zip file '%s' could not be extracted!", zip_filename);

    return NULL;
  }

  Info("zip file '%s' successfully extracted!", zip_filename);

  // first zip file entry contains top level directory
  char *top_dir = zip_entries[0];

  // remove trailing directory separator from top level directory
  top_dir[strlen(top_dir) - 1] = '\0';

  return top_dir;
}

static void ProcessZipFilesInDirectory(char *directory, int tree_type)
{
  Directory *dir;
  DirectoryEntry *dir_entry;

  if ((dir = openDirectory(directory)) == NULL)
  {
    // display error if directory is main "options.graphics_directory" etc.
    if (tree_type == TREE_TYPE_LEVEL_DIR ||
	directory == OPTIONS_ARTWORK_DIRECTORY(tree_type))
      Warn("cannot read directory '%s'", directory);

    return;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	// loop all entries
  {
    // skip non-zip files (and also directories with zip extension)
    if (!strSuffixLower(dir_entry->basename, ".zip") || dir_entry->is_directory)
      continue;

    char *zip_filename = getPath2(directory, dir_entry->basename);
    char *zip_filename_extracted = getStringCat2(zip_filename, ".extracted");
    char *zip_filename_rejected  = getStringCat2(zip_filename, ".rejected");

    // check if zip file hasn't already been extracted or rejected
    if (!fileExists(zip_filename_extracted) &&
	!fileExists(zip_filename_rejected))
    {
      char *top_dir = ExtractZipFileIntoDirectory(zip_filename, directory,
						  tree_type);
      char *marker_filename = (top_dir != NULL ? zip_filename_extracted :
			       zip_filename_rejected);
      FILE *marker_file;

      // create empty file to mark zip file as extracted or rejected
      if ((marker_file = fopen(marker_filename, MODE_WRITE)))
	fclose(marker_file);

      free(zip_filename);
      free(zip_filename_extracted);
      free(zip_filename_rejected);
    }
  }

  closeDirectory(dir);
}

// forward declaration for recursive call by "LoadLevelInfoFromLevelDir()"
static void LoadLevelInfoFromLevelDir(TreeInfo **, TreeInfo *, char *);

static boolean LoadLevelInfoFromLevelConf(TreeInfo **node_first,
					  TreeInfo *node_parent,
					  char *level_directory,
					  char *directory_name)
{
  char *directory_path = getPath2(level_directory, directory_name);
  char *filename = getPath2(directory_path, LEVELINFO_FILENAME);
  SetupFileHash *setup_file_hash;
  LevelDirTree *leveldir_new = NULL;
  int i;

  // unless debugging, silently ignore directories without "levelinfo.conf"
  if (!options.debug && !fileExists(filename))
  {
    free(directory_path);
    free(filename);

    return FALSE;
  }

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash == NULL)
  {
#if DEBUG_NO_CONFIG_FILE
    Debug("setup", "ignoring level directory '%s'", directory_path);
#endif

    free(directory_path);
    free(filename);

    return FALSE;
  }

  leveldir_new = newTreeInfo();

  if (node_parent)
    setTreeInfoToDefaultsFromParent(leveldir_new, node_parent);
  else
    setTreeInfoToDefaults(leveldir_new, TREE_TYPE_LEVEL_DIR);

  leveldir_new->subdir = getStringCopy(directory_name);

  // set all structure fields according to the token/value pairs
  ldi = *leveldir_new;
  for (i = 0; i < NUM_LEVELINFO_TOKENS; i++)
    setSetupInfo(levelinfo_tokens, i,
		 getHashEntry(setup_file_hash, levelinfo_tokens[i].text));
  *leveldir_new = ldi;

  if (strEqual(leveldir_new->name, ANONYMOUS_NAME))
    setString(&leveldir_new->name, leveldir_new->subdir);

  if (leveldir_new->identifier == NULL)
    leveldir_new->identifier = getStringCopy(leveldir_new->subdir);

  if (leveldir_new->name_sorting == NULL)
    leveldir_new->name_sorting = getStringCopy(leveldir_new->name);

  if (node_parent == NULL)		// top level group
  {
    leveldir_new->basepath = getStringCopy(level_directory);
    leveldir_new->fullpath = getStringCopy(leveldir_new->subdir);
  }
  else					// sub level group
  {
    leveldir_new->basepath = getStringCopy(node_parent->basepath);
    leveldir_new->fullpath = getPath2(node_parent->fullpath, directory_name);
  }

  leveldir_new->last_level =
    leveldir_new->first_level + leveldir_new->levels - 1;

  leveldir_new->in_user_dir =
    (!strEqual(leveldir_new->basepath, options.level_directory));

  // adjust some settings if user's private level directory was detected
  if (leveldir_new->sort_priority == LEVELCLASS_UNDEFINED &&
      leveldir_new->in_user_dir &&
      (strEqual(leveldir_new->subdir, getLoginName()) ||
       strEqual(leveldir_new->name,   getLoginName()) ||
       strEqual(leveldir_new->author, getRealName())))
  {
    leveldir_new->sort_priority = LEVELCLASS_PRIVATE_START;
    leveldir_new->readonly = FALSE;
  }

  leveldir_new->user_defined =
    (leveldir_new->in_user_dir && IS_LEVELCLASS_PRIVATE(leveldir_new));

  setString(&leveldir_new->class_desc, getLevelClassDescription(leveldir_new));

  leveldir_new->handicap_level =	// set handicap to default value
    (leveldir_new->user_defined || !leveldir_new->handicap ?
     leveldir_new->last_level : leveldir_new->first_level);

  DrawInitTextItem(leveldir_new->name);

  pushTreeInfo(node_first, leveldir_new);

  freeSetupFileHash(setup_file_hash);

  if (leveldir_new->level_group)
  {
    // create node to link back to current level directory
    createParentTreeInfoNode(leveldir_new);

    // recursively step into sub-directory and look for more level series
    LoadLevelInfoFromLevelDir(&leveldir_new->node_group,
			      leveldir_new, directory_path);
  }

  free(directory_path);
  free(filename);

  return TRUE;
}

static void LoadLevelInfoFromLevelDir(TreeInfo **node_first,
				      TreeInfo *node_parent,
				      char *level_directory)
{
  // ---------- 1st stage: process any level set zip files ----------

  ProcessZipFilesInDirectory(level_directory, TREE_TYPE_LEVEL_DIR);

  // ---------- 2nd stage: check for level set directories ----------

  Directory *dir;
  DirectoryEntry *dir_entry;
  boolean valid_entry_found = FALSE;

  if ((dir = openDirectory(level_directory)) == NULL)
  {
    Warn("cannot read level directory '%s'", level_directory);

    return;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	// loop all entries
  {
    char *directory_name = dir_entry->basename;
    char *directory_path = getPath2(level_directory, directory_name);

    // skip entries for current and parent directory
    if (strEqual(directory_name, ".") ||
	strEqual(directory_name, ".."))
    {
      free(directory_path);

      continue;
    }

    // find out if directory entry is itself a directory
    if (!dir_entry->is_directory)			// not a directory
    {
      free(directory_path);

      continue;
    }

    free(directory_path);

    if (strEqual(directory_name, GRAPHICS_DIRECTORY) ||
	strEqual(directory_name, SOUNDS_DIRECTORY) ||
	strEqual(directory_name, MUSIC_DIRECTORY))
      continue;

    valid_entry_found |= LoadLevelInfoFromLevelConf(node_first, node_parent,
						    level_directory,
						    directory_name);
  }

  closeDirectory(dir);

  // special case: top level directory may directly contain "levelinfo.conf"
  if (node_parent == NULL && !valid_entry_found)
  {
    // check if this directory directly contains a file "levelinfo.conf"
    valid_entry_found |= LoadLevelInfoFromLevelConf(node_first, node_parent,
						    level_directory, ".");
  }

  boolean valid_entry_expected =
    (strEqual(level_directory, options.level_directory) ||
     setup.internal.create_user_levelset);

  if (valid_entry_expected && !valid_entry_found)
    Warn("cannot find any valid level series in directory '%s'",
	 level_directory);
}

boolean AdjustGraphicsForEMC(void)
{
  boolean settings_changed = FALSE;

  settings_changed |= adjustTreeGraphicsForEMC(leveldir_first_all);
  settings_changed |= adjustTreeGraphicsForEMC(leveldir_first);

  return settings_changed;
}

boolean AdjustSoundsForEMC(void)
{
  boolean settings_changed = FALSE;

  settings_changed |= adjustTreeSoundsForEMC(leveldir_first_all);
  settings_changed |= adjustTreeSoundsForEMC(leveldir_first);

  return settings_changed;
}

void LoadLevelInfo(void)
{
  InitUserLevelDirectory(getLoginName());

  DrawInitTextHead("Loading level series");

  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, options.level_directory);
  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, getUserLevelDir(NULL));

  leveldir_first = createTopTreeInfoNode(leveldir_first);

  /* after loading all level set information, clone the level directory tree
     and remove all level sets without levels (these may still contain artwork
     to be offered in the setup menu as "custom artwork", and are therefore
     checked for existing artwork in the function "LoadLevelArtworkInfo()") */
  leveldir_first_all = leveldir_first;
  cloneTree(&leveldir_first, leveldir_first_all, TRUE);

  AdjustGraphicsForEMC();
  AdjustSoundsForEMC();

  // before sorting, the first entries will be from the user directory
  leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

  if (leveldir_first == NULL)
    Fail("cannot find any valid level series in any directory");

  sortTreeInfo(&leveldir_first);

#if ENABLE_UNUSED_CODE
  dumpTreeInfo(leveldir_first, 0);
#endif
}

static boolean LoadArtworkInfoFromArtworkConf(TreeInfo **node_first,
					      TreeInfo *node_parent,
					      char *base_directory,
					      char *directory_name, int type)
{
  char *directory_path = getPath2(base_directory, directory_name);
  char *filename = getPath2(directory_path, ARTWORKINFO_FILENAME(type));
  SetupFileHash *setup_file_hash = NULL;
  TreeInfo *artwork_new = NULL;
  int i;

  if (fileExists(filename))
    setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash == NULL)	// no config file -- look for artwork files
  {
    Directory *dir;
    DirectoryEntry *dir_entry;
    boolean valid_file_found = FALSE;

    if ((dir = openDirectory(directory_path)) != NULL)
    {
      while ((dir_entry = readDirectory(dir)) != NULL)
      {
	if (FileIsArtworkType(dir_entry->filename, type))
	{
	  valid_file_found = TRUE;

	  break;
	}
      }

      closeDirectory(dir);
    }

    if (!valid_file_found)
    {
#if DEBUG_NO_CONFIG_FILE
      if (!strEqual(directory_name, "."))
	Debug("setup", "ignoring artwork directory '%s'", directory_path);
#endif

      free(directory_path);
      free(filename);

      return FALSE;
    }
  }

  artwork_new = newTreeInfo();

  if (node_parent)
    setTreeInfoToDefaultsFromParent(artwork_new, node_parent);
  else
    setTreeInfoToDefaults(artwork_new, type);

  artwork_new->subdir = getStringCopy(directory_name);

  if (setup_file_hash)	// (before defining ".color" and ".class_desc")
  {
    // set all structure fields according to the token/value pairs
    ldi = *artwork_new;
    for (i = 0; i < NUM_LEVELINFO_TOKENS; i++)
      setSetupInfo(levelinfo_tokens, i,
		   getHashEntry(setup_file_hash, levelinfo_tokens[i].text));
    *artwork_new = ldi;

    if (strEqual(artwork_new->name, ANONYMOUS_NAME))
      setString(&artwork_new->name, artwork_new->subdir);

    if (artwork_new->identifier == NULL)
      artwork_new->identifier = getStringCopy(artwork_new->subdir);

    if (artwork_new->name_sorting == NULL)
      artwork_new->name_sorting = getStringCopy(artwork_new->name);
  }

  if (node_parent == NULL)		// top level group
  {
    artwork_new->basepath = getStringCopy(base_directory);
    artwork_new->fullpath = getStringCopy(artwork_new->subdir);
  }
  else					// sub level group
  {
    artwork_new->basepath = getStringCopy(node_parent->basepath);
    artwork_new->fullpath = getPath2(node_parent->fullpath, directory_name);
  }

  artwork_new->in_user_dir =
    (!strEqual(artwork_new->basepath, OPTIONS_ARTWORK_DIRECTORY(type)));

  setString(&artwork_new->class_desc, getLevelClassDescription(artwork_new));

  if (setup_file_hash == NULL)	// (after determining ".user_defined")
  {
    if (strEqual(artwork_new->subdir, "."))
    {
      if (artwork_new->user_defined)
      {
	setString(&artwork_new->identifier, "private");
	artwork_new->sort_priority = ARTWORKCLASS_PRIVATE;
      }
      else
      {
	setString(&artwork_new->identifier, "classic");
	artwork_new->sort_priority = ARTWORKCLASS_CLASSICS;
      }

      setString(&artwork_new->class_desc,
		getLevelClassDescription(artwork_new));
    }
    else
    {
      setString(&artwork_new->identifier, artwork_new->subdir);
    }

    setString(&artwork_new->name, artwork_new->identifier);
    setString(&artwork_new->name_sorting, artwork_new->name);
  }

  pushTreeInfo(node_first, artwork_new);

  freeSetupFileHash(setup_file_hash);

  free(directory_path);
  free(filename);

  return TRUE;
}

static void LoadArtworkInfoFromArtworkDir(TreeInfo **node_first,
					  TreeInfo *node_parent,
					  char *base_directory, int type)
{
  // ---------- 1st stage: process any artwork set zip files ----------

  ProcessZipFilesInDirectory(base_directory, type);

  // ---------- 2nd stage: check for artwork set directories ----------

  Directory *dir;
  DirectoryEntry *dir_entry;
  boolean valid_entry_found = FALSE;

  if ((dir = openDirectory(base_directory)) == NULL)
  {
    // display error if directory is main "options.graphics_directory" etc.
    if (base_directory == OPTIONS_ARTWORK_DIRECTORY(type))
      Warn("cannot read directory '%s'", base_directory);

    return;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	// loop all entries
  {
    char *directory_name = dir_entry->basename;
    char *directory_path = getPath2(base_directory, directory_name);

    // skip directory entries for current and parent directory
    if (strEqual(directory_name, ".") ||
	strEqual(directory_name, ".."))
    {
      free(directory_path);

      continue;
    }

    // skip directory entries which are not a directory
    if (!dir_entry->is_directory)			// not a directory
    {
      free(directory_path);

      continue;
    }

    free(directory_path);

    // check if this directory contains artwork with or without config file
    valid_entry_found |= LoadArtworkInfoFromArtworkConf(node_first, node_parent,
							base_directory,
							directory_name, type);
  }

  closeDirectory(dir);

  // check if this directory directly contains artwork itself
  valid_entry_found |= LoadArtworkInfoFromArtworkConf(node_first, node_parent,
						      base_directory, ".",
						      type);
  if (!valid_entry_found)
    Warn("cannot find any valid artwork in directory '%s'", base_directory);
}

static TreeInfo *getDummyArtworkInfo(int type)
{
  // this is only needed when there is completely no artwork available
  TreeInfo *artwork_new = newTreeInfo();

  setTreeInfoToDefaults(artwork_new, type);

  setString(&artwork_new->subdir,   UNDEFINED_FILENAME);
  setString(&artwork_new->fullpath, UNDEFINED_FILENAME);
  setString(&artwork_new->basepath, UNDEFINED_FILENAME);

  setString(&artwork_new->identifier,   UNDEFINED_FILENAME);
  setString(&artwork_new->name,         UNDEFINED_FILENAME);
  setString(&artwork_new->name_sorting, UNDEFINED_FILENAME);

  return artwork_new;
}

void SetCurrentArtwork(int type)
{
  ArtworkDirTree **current_ptr = ARTWORK_CURRENT_PTR(artwork, type);
  ArtworkDirTree *first_node = ARTWORK_FIRST_NODE(artwork, type);
  char *setup_set = SETUP_ARTWORK_SET(setup, type);
  char *default_subdir = ARTWORK_DEFAULT_SUBDIR(type);

  // set current artwork to artwork configured in setup menu
  *current_ptr = getTreeInfoFromIdentifier(first_node, setup_set);

  // if not found, set current artwork to default artwork
  if (*current_ptr == NULL)
    *current_ptr = getTreeInfoFromIdentifier(first_node, default_subdir);

  // if not found, set current artwork to first artwork in tree
  if (*current_ptr == NULL)
    *current_ptr = getFirstValidTreeInfoEntry(first_node);
}

void ChangeCurrentArtworkIfNeeded(int type)
{
  char *current_identifier = ARTWORK_CURRENT_IDENTIFIER(artwork, type);
  char *setup_set = SETUP_ARTWORK_SET(setup, type);

  if (!strEqual(current_identifier, setup_set))
    SetCurrentArtwork(type);
}

void LoadArtworkInfo(void)
{
  LoadArtworkInfoCache();

  DrawInitTextHead("Looking for custom artwork");

  LoadArtworkInfoFromArtworkDir(&artwork.gfx_first, NULL,
				options.graphics_directory,
				TREE_TYPE_GRAPHICS_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.gfx_first, NULL,
				getUserGraphicsDir(),
				TREE_TYPE_GRAPHICS_DIR);

  LoadArtworkInfoFromArtworkDir(&artwork.snd_first, NULL,
				options.sounds_directory,
				TREE_TYPE_SOUNDS_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.snd_first, NULL,
				getUserSoundsDir(),
				TREE_TYPE_SOUNDS_DIR);

  LoadArtworkInfoFromArtworkDir(&artwork.mus_first, NULL,
				options.music_directory,
				TREE_TYPE_MUSIC_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.mus_first, NULL,
				getUserMusicDir(),
				TREE_TYPE_MUSIC_DIR);

  if (artwork.gfx_first == NULL)
    artwork.gfx_first = getDummyArtworkInfo(TREE_TYPE_GRAPHICS_DIR);
  if (artwork.snd_first == NULL)
    artwork.snd_first = getDummyArtworkInfo(TREE_TYPE_SOUNDS_DIR);
  if (artwork.mus_first == NULL)
    artwork.mus_first = getDummyArtworkInfo(TREE_TYPE_MUSIC_DIR);

  // before sorting, the first entries will be from the user directory
  SetCurrentArtwork(ARTWORK_TYPE_GRAPHICS);
  SetCurrentArtwork(ARTWORK_TYPE_SOUNDS);
  SetCurrentArtwork(ARTWORK_TYPE_MUSIC);

  artwork.gfx_current_identifier = artwork.gfx_current->identifier;
  artwork.snd_current_identifier = artwork.snd_current->identifier;
  artwork.mus_current_identifier = artwork.mus_current->identifier;

#if ENABLE_UNUSED_CODE
  Debug("setup:LoadArtworkInfo", "graphics set == %s",
	artwork.gfx_current_identifier);
  Debug("setup:LoadArtworkInfo", "sounds set == %s",
	artwork.snd_current_identifier);
  Debug("setup:LoadArtworkInfo", "music set == %s",
	artwork.mus_current_identifier);
#endif

  sortTreeInfo(&artwork.gfx_first);
  sortTreeInfo(&artwork.snd_first);
  sortTreeInfo(&artwork.mus_first);

#if ENABLE_UNUSED_CODE
  dumpTreeInfo(artwork.gfx_first, 0);
  dumpTreeInfo(artwork.snd_first, 0);
  dumpTreeInfo(artwork.mus_first, 0);
#endif
}

static void MoveArtworkInfoIntoSubTree(ArtworkDirTree **artwork_node)
{
  ArtworkDirTree *artwork_new = newTreeInfo();
  char *top_node_name = "standalone artwork";

  setTreeInfoToDefaults(artwork_new, (*artwork_node)->type);

  artwork_new->level_group = TRUE;

  setString(&artwork_new->identifier,   top_node_name);
  setString(&artwork_new->name,         top_node_name);
  setString(&artwork_new->name_sorting, top_node_name);

  // create node to link back to current custom artwork directory
  createParentTreeInfoNode(artwork_new);

  // move existing custom artwork tree into newly created sub-tree
  artwork_new->node_group->next = *artwork_node;

  // change custom artwork tree to contain only newly created node
  *artwork_node = artwork_new;
}

static void LoadArtworkInfoFromLevelInfoExt(ArtworkDirTree **artwork_node,
					    ArtworkDirTree *node_parent,
					    LevelDirTree *level_node,
					    boolean empty_level_set_mode)
{
  int type = (*artwork_node)->type;

  // recursively check all level directories for artwork sub-directories

  while (level_node)
  {
    boolean empty_level_set = (level_node->levels == 0);

    // check all tree entries for artwork, but skip parent link entries
    if (!level_node->parent_link && empty_level_set == empty_level_set_mode)
    {
      TreeInfo *artwork_new = getArtworkInfoCacheEntry(level_node, type);
      boolean cached = (artwork_new != NULL);

      if (cached)
      {
	pushTreeInfo(artwork_node, artwork_new);
      }
      else
      {
	TreeInfo *topnode_last = *artwork_node;
	char *path = getPath2(getLevelDirFromTreeInfo(level_node),
			      ARTWORK_DIRECTORY(type));

	LoadArtworkInfoFromArtworkDir(artwork_node, NULL, path, type);

	if (topnode_last != *artwork_node)	// check for newly added node
	{
	  artwork_new = *artwork_node;

	  setString(&artwork_new->identifier,   level_node->subdir);
	  setString(&artwork_new->name,         level_node->name);
	  setString(&artwork_new->name_sorting, level_node->name_sorting);

	  artwork_new->sort_priority = level_node->sort_priority;
	  artwork_new->in_user_dir = level_node->in_user_dir;

	  update_artworkinfo_cache = TRUE;
	}

	free(path);
      }

      // insert artwork info (from old cache or filesystem) into new cache
      if (artwork_new != NULL)
	setArtworkInfoCacheEntry(artwork_new, level_node, type);
    }

    DrawInitTextItem(level_node->name);

    if (level_node->node_group != NULL)
    {
      TreeInfo *artwork_new = newTreeInfo();

      if (node_parent)
	setTreeInfoToDefaultsFromParent(artwork_new, node_parent);
      else
	setTreeInfoToDefaults(artwork_new, type);

      artwork_new->level_group = TRUE;

      setString(&artwork_new->identifier,   level_node->subdir);

      if (node_parent == NULL)		// check for top tree node
      {
	char *top_node_name = (empty_level_set_mode ?
			       "artwork for certain level sets" :
			       "artwork included in level sets");

	setString(&artwork_new->name,         top_node_name);
	setString(&artwork_new->name_sorting, top_node_name);
      }
      else
      {
	setString(&artwork_new->name,         level_node->name);
	setString(&artwork_new->name_sorting, level_node->name_sorting);
      }

      pushTreeInfo(artwork_node, artwork_new);

      // create node to link back to current custom artwork directory
      createParentTreeInfoNode(artwork_new);

      // recursively step into sub-directory and look for more custom artwork
      LoadArtworkInfoFromLevelInfoExt(&artwork_new->node_group, artwork_new,
				      level_node->node_group,
				      empty_level_set_mode);

      // if sub-tree has no custom artwork at all, remove it
      if (artwork_new->node_group->next == NULL)
	removeTreeInfo(artwork_node);
    }

    level_node = level_node->next;
  }
}

static void LoadArtworkInfoFromLevelInfo(ArtworkDirTree **artwork_node)
{
  // move peviously loaded artwork tree into separate sub-tree
  MoveArtworkInfoIntoSubTree(artwork_node);

  // load artwork from level sets into separate sub-trees
  LoadArtworkInfoFromLevelInfoExt(artwork_node, NULL, leveldir_first_all, TRUE);
  LoadArtworkInfoFromLevelInfoExt(artwork_node, NULL, leveldir_first_all, FALSE);

  // add top tree node over all sub-trees and set parent links
  *artwork_node = addTopTreeInfoNode(*artwork_node);
}

void LoadLevelArtworkInfo(void)
{
  print_timestamp_init("LoadLevelArtworkInfo");

  DrawInitTextHead("Looking for custom level artwork");

  print_timestamp_time("DrawTimeText");

  LoadArtworkInfoFromLevelInfo(&artwork.gfx_first);
  print_timestamp_time("LoadArtworkInfoFromLevelInfo (gfx)");
  LoadArtworkInfoFromLevelInfo(&artwork.snd_first);
  print_timestamp_time("LoadArtworkInfoFromLevelInfo (snd)");
  LoadArtworkInfoFromLevelInfo(&artwork.mus_first);
  print_timestamp_time("LoadArtworkInfoFromLevelInfo (mus)");

  SaveArtworkInfoCache();

  print_timestamp_time("SaveArtworkInfoCache");

  // needed for reloading level artwork not known at ealier stage
  ChangeCurrentArtworkIfNeeded(ARTWORK_TYPE_GRAPHICS);
  ChangeCurrentArtworkIfNeeded(ARTWORK_TYPE_SOUNDS);
  ChangeCurrentArtworkIfNeeded(ARTWORK_TYPE_MUSIC);

  print_timestamp_time("getTreeInfoFromIdentifier");

  sortTreeInfo(&artwork.gfx_first);
  sortTreeInfo(&artwork.snd_first);
  sortTreeInfo(&artwork.mus_first);

  print_timestamp_time("sortTreeInfo");

#if ENABLE_UNUSED_CODE
  dumpTreeInfo(artwork.gfx_first, 0);
  dumpTreeInfo(artwork.snd_first, 0);
  dumpTreeInfo(artwork.mus_first, 0);
#endif

  print_timestamp_done("LoadLevelArtworkInfo");
}

static boolean AddTreeSetToTreeInfoExt(TreeInfo *tree_node_old, char *tree_dir,
				       char *tree_subdir_new, int type)
{
  if (tree_node_old == NULL)
  {
    if (type == TREE_TYPE_LEVEL_DIR)
    {
      // get level info tree node of personal user level set
      tree_node_old = getTreeInfoFromIdentifier(leveldir_first, getLoginName());

      // this may happen if "setup.internal.create_user_levelset" is FALSE
      // or if file "levelinfo.conf" is missing in personal user level set
      if (tree_node_old == NULL)
	tree_node_old = leveldir_first->node_group;
    }
    else
    {
      // get artwork info tree node of first artwork set
      tree_node_old = ARTWORK_FIRST_NODE(artwork, type);
    }
  }

  if (tree_dir == NULL)
    tree_dir = TREE_USERDIR(type);

  if (tree_node_old   == NULL ||
      tree_dir        == NULL ||
      tree_subdir_new == NULL)		// should not happen
    return FALSE;

  int draw_deactivation_mask = GetDrawDeactivationMask();

  // override draw deactivation mask (temporarily disable drawing)
  SetDrawDeactivationMask(REDRAW_ALL);

  if (type == TREE_TYPE_LEVEL_DIR)
  {
    // load new level set config and add it next to first user level set
    LoadLevelInfoFromLevelConf(&tree_node_old->next,
			       tree_node_old->node_parent,
			       tree_dir, tree_subdir_new);
  }
  else
  {
    // load new artwork set config and add it next to first artwork set
    LoadArtworkInfoFromArtworkConf(&tree_node_old->next,
				   tree_node_old->node_parent,
				   tree_dir, tree_subdir_new, type);
  }

  // set draw deactivation mask to previous value
  SetDrawDeactivationMask(draw_deactivation_mask);

  // get first node of level or artwork info tree
  TreeInfo **tree_node_first = TREE_FIRST_NODE_PTR(type);

  // get tree info node of newly added level or artwork set
  TreeInfo *tree_node_new = getTreeInfoFromIdentifier(*tree_node_first,
						      tree_subdir_new);

  if (tree_node_new == NULL)		// should not happen
    return FALSE;

  // correct top link and parent node link of newly created tree node
  tree_node_new->node_top    = tree_node_old->node_top;
  tree_node_new->node_parent = tree_node_old->node_parent;

  // sort tree info to adjust position of newly added tree set
  sortTreeInfo(tree_node_first);

  return TRUE;
}

void AddTreeSetToTreeInfo(TreeInfo *tree_node, char *tree_dir,
			  char *tree_subdir_new, int type)
{
  if (!AddTreeSetToTreeInfoExt(tree_node, tree_dir, tree_subdir_new, type))
    Fail("internal tree info structure corrupted -- aborting");
}

void AddUserLevelSetToLevelInfo(char *level_subdir_new)
{
  AddTreeSetToTreeInfo(NULL, NULL, level_subdir_new, TREE_TYPE_LEVEL_DIR);
}

char *getArtworkIdentifierForUserLevelSet(int type)
{
  char *classic_artwork_set = getClassicArtworkSet(type);

  // check for custom artwork configured in "levelinfo.conf"
  char *leveldir_artwork_set =
    *LEVELDIR_ARTWORK_SET_PTR(leveldir_current, type);
  boolean has_leveldir_artwork_set =
    (leveldir_artwork_set != NULL && !strEqual(leveldir_artwork_set,
					       classic_artwork_set));

  // check for custom artwork in sub-directory "graphics" etc.
  TreeInfo *artwork_first_node = ARTWORK_FIRST_NODE(artwork, type);
  char *leveldir_identifier = leveldir_current->identifier;
  boolean has_artwork_subdir =
    (getTreeInfoFromIdentifier(artwork_first_node,
			       leveldir_identifier) != NULL);

  return (has_leveldir_artwork_set ? leveldir_artwork_set :
	  has_artwork_subdir       ? leveldir_identifier :
	  classic_artwork_set);
}

TreeInfo *getArtworkTreeInfoForUserLevelSet(int type)
{
  char *artwork_set = getArtworkIdentifierForUserLevelSet(type);
  TreeInfo *artwork_first_node = ARTWORK_FIRST_NODE(artwork, type);
  TreeInfo *ti = getTreeInfoFromIdentifier(artwork_first_node, artwork_set);

  if (ti == NULL)
  {
    ti = getTreeInfoFromIdentifier(artwork_first_node,
				   ARTWORK_DEFAULT_SUBDIR(type));
    if (ti == NULL)
      Fail("cannot find default graphics -- should not happen");
  }

  return ti;
}

boolean checkIfCustomArtworkExistsForCurrentLevelSet(void)
{
  char *graphics_set =
    getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_GRAPHICS);
  char *sounds_set =
    getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_SOUNDS);
  char *music_set =
    getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_MUSIC);

  return (!strEqual(graphics_set, GFX_CLASSIC_SUBDIR) ||
	  !strEqual(sounds_set,   SND_CLASSIC_SUBDIR) ||
	  !strEqual(music_set,    MUS_CLASSIC_SUBDIR));
}

boolean UpdateUserLevelSet(char *level_subdir, char *level_name,
			   char *level_author, int num_levels)
{
  char *filename = getPath2(getUserLevelDir(level_subdir), LEVELINFO_FILENAME);
  char *filename_tmp = getStringCat2(filename, ".tmp");
  FILE *file = NULL;
  FILE *file_tmp = NULL;
  char line[MAX_LINE_LEN];
  boolean success = FALSE;
  LevelDirTree *leveldir = getTreeInfoFromIdentifier(leveldir_first,
						     level_subdir);
  // update values in level directory tree

  if (level_name != NULL)
    setString(&leveldir->name, level_name);

  if (level_author != NULL)
    setString(&leveldir->author, level_author);

  if (num_levels != -1)
    leveldir->levels = num_levels;

  // update values that depend on other values

  setString(&leveldir->name_sorting, leveldir->name);

  leveldir->last_level = leveldir->first_level + leveldir->levels - 1;

  // sort order of level sets may have changed
  sortTreeInfo(&leveldir_first);

  if ((file     = fopen(filename,     MODE_READ)) &&
      (file_tmp = fopen(filename_tmp, MODE_WRITE)))
  {
    while (fgets(line, MAX_LINE_LEN, file))
    {
      if (strPrefix(line, "name:") && level_name != NULL)
	fprintf(file_tmp, "%-32s%s\n", "name:", level_name);
      else if (strPrefix(line, "author:") && level_author != NULL)
	fprintf(file_tmp, "%-32s%s\n", "author:", level_author);
      else if (strPrefix(line, "levels:") && num_levels != -1)
	fprintf(file_tmp, "%-32s%d\n", "levels:", num_levels);
      else
	fputs(line, file_tmp);
    }

    success = TRUE;
  }

  if (file)
    fclose(file);

  if (file_tmp)
    fclose(file_tmp);

  if (success)
    success = (rename(filename_tmp, filename) == 0);

  free(filename);
  free(filename_tmp);

  return success;
}

boolean CreateUserLevelSet(char *level_subdir, char *level_name,
			   char *level_author, int num_levels,
			   boolean use_artwork_set)
{
  LevelDirTree *level_info;
  char *filename;
  FILE *file;
  int i;

  // create user level sub-directory, if needed
  createDirectory(getUserLevelDir(level_subdir), "user level");

  filename = getPath2(getUserLevelDir(level_subdir), LEVELINFO_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write level info file '%s'", filename);

    free(filename);

    return FALSE;
  }

  level_info = newTreeInfo();

  // always start with reliable default values
  setTreeInfoToDefaults(level_info, TREE_TYPE_LEVEL_DIR);

  setString(&level_info->name, level_name);
  setString(&level_info->author, level_author);
  level_info->levels = num_levels;
  level_info->first_level = 1;
  level_info->sort_priority = LEVELCLASS_PRIVATE_START;
  level_info->readonly = FALSE;

  if (use_artwork_set)
  {
    level_info->graphics_set =
      getStringCopy(getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_GRAPHICS));
    level_info->sounds_set =
      getStringCopy(getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_SOUNDS));
    level_info->music_set =
      getStringCopy(getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_MUSIC));
  }

  token_value_position = TOKEN_VALUE_POSITION_SHORT;

  fprintFileHeader(file, LEVELINFO_FILENAME);

  ldi = *level_info;
  for (i = 0; i < NUM_LEVELINFO_TOKENS; i++)
  {
    if (i == LEVELINFO_TOKEN_NAME ||
	i == LEVELINFO_TOKEN_AUTHOR ||
	i == LEVELINFO_TOKEN_LEVELS ||
	i == LEVELINFO_TOKEN_FIRST_LEVEL ||
	i == LEVELINFO_TOKEN_SORT_PRIORITY ||
	i == LEVELINFO_TOKEN_READONLY ||
	(use_artwork_set && (i == LEVELINFO_TOKEN_GRAPHICS_SET ||
			     i == LEVELINFO_TOKEN_SOUNDS_SET ||
			     i == LEVELINFO_TOKEN_MUSIC_SET)))
      fprintf(file, "%s\n", getSetupLine(levelinfo_tokens, "", i));

    // just to make things nicer :)
    if (i == LEVELINFO_TOKEN_AUTHOR ||
	i == LEVELINFO_TOKEN_FIRST_LEVEL ||
	(use_artwork_set && i == LEVELINFO_TOKEN_READONLY))
      fprintf(file, "\n");	
  }

  token_value_position = TOKEN_VALUE_POSITION_DEFAULT;

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  freeTreeInfo(level_info);
  free(filename);

  return TRUE;
}

static void SaveUserLevelInfo(void)
{
  CreateUserLevelSet(getLoginName(), getLoginName(), getRealName(), 100, FALSE);
}

char *getSetupValue(int type, void *value)
{
  static char value_string[MAX_LINE_LEN];

  if (value == NULL)
    return NULL;

  switch (type)
  {
    case TYPE_BOOLEAN:
      strcpy(value_string, (*(boolean *)value ? "true" : "false"));
      break;

    case TYPE_SWITCH:
      strcpy(value_string, (*(boolean *)value ? "on" : "off"));
      break;

    case TYPE_SWITCH3:
      strcpy(value_string, (*(int *)value == AUTO  ? "auto" :
			    *(int *)value == FALSE ? "off" : "on"));
      break;

    case TYPE_YES_NO:
      strcpy(value_string, (*(boolean *)value ? "yes" : "no"));
      break;

    case TYPE_YES_NO_AUTO:
      strcpy(value_string, (*(int *)value == AUTO  ? "auto" :
			    *(int *)value == FALSE ? "no" : "yes"));
      break;

    case TYPE_ECS_AGA:
      strcpy(value_string, (*(boolean *)value ? "AGA" : "ECS"));
      break;

    case TYPE_KEY:
      strcpy(value_string, getKeyNameFromKey(*(Key *)value));
      break;

    case TYPE_KEY_X11:
      strcpy(value_string, getX11KeyNameFromKey(*(Key *)value));
      break;

    case TYPE_INTEGER:
      sprintf(value_string, "%d", *(int *)value);
      break;

    case TYPE_STRING:
      if (*(char **)value == NULL)
	return NULL;

      strcpy(value_string, *(char **)value);
      break;

    case TYPE_PLAYER:
      sprintf(value_string, "player_%d", *(int *)value + 1);
      break;

    default:
      value_string[0] = '\0';
      break;
  }

  if (type & TYPE_GHOSTED)
    strcpy(value_string, "n/a");

  return value_string;
}

char *getSetupLine(struct TokenInfo *token_info, char *prefix, int token_nr)
{
  int i;
  char *line;
  static char token_string[MAX_LINE_LEN];
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;
  char *token_text = token_info[token_nr].text;
  char *value_string = getSetupValue(token_type, setup_value);

  // build complete token string
  sprintf(token_string, "%s%s", prefix, token_text);

  // build setup entry line
  line = getFormattedSetupEntry(token_string, value_string);

  if (token_type == TYPE_KEY_X11)
  {
    Key key = *(Key *)setup_value;
    char *keyname = getKeyNameFromKey(key);

    // add comment, if useful
    if (!strEqual(keyname, "(undefined)") &&
	!strEqual(keyname, "(unknown)"))
    {
      // add at least one whitespace
      strcat(line, " ");
      for (i = strlen(line); i < token_comment_position; i++)
	strcat(line, " ");

      strcat(line, "# ");
      strcat(line, keyname);
    }
  }

  return line;
}

static void InitLastPlayedLevels_ParentNode(void)
{
  LevelDirTree **leveldir_top = &leveldir_first->node_group->next;
  LevelDirTree *leveldir_new = NULL;

  // check if parent node for last played levels already exists
  if (strEqual((*leveldir_top)->identifier, TOKEN_STR_LAST_LEVEL_SERIES))
    return;

  leveldir_new = newTreeInfo();

  setTreeInfoToDefaultsFromParent(leveldir_new, leveldir_first);

  leveldir_new->level_group = TRUE;
  leveldir_new->sort_priority = LEVELCLASS_LAST_PLAYED_LEVEL;

  setString(&leveldir_new->identifier, TOKEN_STR_LAST_LEVEL_SERIES);
  setString(&leveldir_new->name, "<< (last played level sets)");
  setString(&leveldir_new->name_sorting, leveldir_new->name);

  pushTreeInfo(leveldir_top, leveldir_new);

  // create node to link back to current level directory
  createParentTreeInfoNode(leveldir_new);
}

void UpdateLastPlayedLevels_TreeInfo(void)
{
  char **last_level_series = setup.level_setup.last_level_series;
  LevelDirTree *leveldir_last;
  TreeInfo **node_new = NULL;
  int i;

  if (last_level_series[0] == NULL)
    return;

  InitLastPlayedLevels_ParentNode();

  leveldir_last = getTreeInfoFromIdentifierExt(leveldir_first,
					       TOKEN_STR_LAST_LEVEL_SERIES,
					       TREE_NODE_TYPE_GROUP);
  if (leveldir_last == NULL)
    return;

  node_new = &leveldir_last->node_group->next;

  freeTreeInfo(*node_new);

  *node_new = NULL;

  for (i = 0; last_level_series[i] != NULL; i++)
  {
    LevelDirTree *node_last = getTreeInfoFromIdentifier(leveldir_first,
							last_level_series[i]);
    if (node_last == NULL)
      continue;

    *node_new = getTreeInfoCopy(node_last);	// copy complete node

    (*node_new)->node_top = &leveldir_first;	// correct top node link
    (*node_new)->node_parent = leveldir_last;	// correct parent node link

    (*node_new)->is_copy = TRUE;		// mark entry as node copy

    (*node_new)->node_group = NULL;
    (*node_new)->next = NULL;

    (*node_new)->cl_first = -1;			// force setting tree cursor

    node_new = &((*node_new)->next);
  }
}

static void UpdateLastPlayedLevels_List(void)
{
  char **last_level_series = setup.level_setup.last_level_series;
  int pos = MAX_LEVELDIR_HISTORY - 1;
  int i;

  // search for potentially already existing entry in list of level sets
  for (i = 0; last_level_series[i] != NULL; i++)
    if (strEqual(last_level_series[i], leveldir_current->identifier))
      pos = i;

  // move list of level sets one entry down (using potentially free entry)
  for (i = pos; i > 0; i--)
    setString(&last_level_series[i], last_level_series[i - 1]);

  // put last played level set at top position
  setString(&last_level_series[0], leveldir_current->identifier);
}

#define LAST_PLAYED_MODE_SET			1
#define LAST_PLAYED_MODE_SET_FORCED		2
#define LAST_PLAYED_MODE_GET			3

static TreeInfo *StoreOrRestoreLastPlayedLevels(TreeInfo *node, int mode)
{
  static char *identifier = NULL;

  if (mode == LAST_PLAYED_MODE_SET)
  {
    setString(&identifier, (node && node->is_copy ? node->identifier : NULL));
  }
  else if (mode == LAST_PLAYED_MODE_SET_FORCED)
  {
    setString(&identifier, (node ? node->identifier : NULL));
  }
  else if (mode == LAST_PLAYED_MODE_GET)
  {
    TreeInfo *node_new = getTreeInfoFromIdentifierExt(leveldir_first,
						      identifier,
						      TREE_NODE_TYPE_COPY);
    return (node_new != NULL ? node_new : node);
  }

  return NULL;		// not used
}

void StoreLastPlayedLevels(TreeInfo *node)
{
  StoreOrRestoreLastPlayedLevels(node, LAST_PLAYED_MODE_SET);
}

void ForcedStoreLastPlayedLevels(TreeInfo *node)
{
  StoreOrRestoreLastPlayedLevels(node, LAST_PLAYED_MODE_SET_FORCED);
}

void RestoreLastPlayedLevels(TreeInfo **node)
{
  *node = StoreOrRestoreLastPlayedLevels(*node, LAST_PLAYED_MODE_GET);
}

boolean CheckLastPlayedLevels(void)
{
  return (StoreOrRestoreLastPlayedLevels(NULL, LAST_PLAYED_MODE_GET) != NULL);
}

void LoadLevelSetup_LastSeries(void)
{
  // --------------------------------------------------------------------------
  // ~/.<program>/levelsetup.conf
  // --------------------------------------------------------------------------

  char *filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);
  SetupFileHash *level_setup_hash = NULL;
  int pos = 0;
  int i;

  // always start with reliable default values
  leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

  // start with empty history of last played level sets
  setString(&setup.level_setup.last_level_series[0], NULL);

  if (!strEqual(DEFAULT_LEVELSET, UNDEFINED_LEVELSET))
  {
    leveldir_current = getTreeInfoFromIdentifier(leveldir_first,
						 DEFAULT_LEVELSET);
    if (leveldir_current == NULL)
      leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);
  }

  if ((level_setup_hash = loadSetupFileHash(filename)))
  {
    char *last_level_series =
      getHashEntry(level_setup_hash, TOKEN_STR_LAST_LEVEL_SERIES);

    leveldir_current = getTreeInfoFromIdentifier(leveldir_first,
						 last_level_series);
    if (leveldir_current == NULL)
      leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

    char *last_played_menu_used =
      getHashEntry(level_setup_hash, TOKEN_STR_LAST_PLAYED_MENU_USED);

    // store if last level set was selected from "last played" menu
    if (strEqual(last_played_menu_used, "true"))
      ForcedStoreLastPlayedLevels(leveldir_current);

    for (i = 0; i < MAX_LEVELDIR_HISTORY; i++)
    {
      char token[strlen(TOKEN_STR_LAST_LEVEL_SERIES) + 10];
      LevelDirTree *leveldir_last;

      sprintf(token, "%s.%03d", TOKEN_STR_LAST_LEVEL_SERIES, i);

      last_level_series = getHashEntry(level_setup_hash, token);

      leveldir_last = getTreeInfoFromIdentifier(leveldir_first,
						last_level_series);
      if (leveldir_last != NULL)
	setString(&setup.level_setup.last_level_series[pos++],
		  last_level_series);
    }

    setString(&setup.level_setup.last_level_series[pos], NULL);

    freeSetupFileHash(level_setup_hash);
  }
  else
  {
    Debug("setup", "using default setup values");
  }

  free(filename);
}

static void SaveLevelSetup_LastSeries_Ext(boolean deactivate_last_level_series)
{
  // --------------------------------------------------------------------------
  // ~/.<program>/levelsetup.conf
  // --------------------------------------------------------------------------

  // check if the current level directory structure is available at this point
  if (leveldir_current == NULL)
    return;

  char **last_level_series = setup.level_setup.last_level_series;
  char *filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);
  FILE *file;
  int i;

  InitUserDataDirectory();

  UpdateLastPlayedLevels_List();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write setup file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, LEVELSETUP_FILENAME);

  if (deactivate_last_level_series)
    fprintf(file, "# %s\n# ", "the following level set may have caused a problem and was deactivated");

  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_LAST_LEVEL_SERIES,
						 leveldir_current->identifier));

  // store if last level set was selected from "last played" menu
  boolean last_played_menu_used = CheckLastPlayedLevels();
  char *setup_value = getSetupValue(TYPE_BOOLEAN, &last_played_menu_used);

  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_LAST_PLAYED_MENU_USED,
						 setup_value));

  for (i = 0; last_level_series[i] != NULL; i++)
  {
    char token[strlen(TOKEN_STR_LAST_LEVEL_SERIES) + 1 + 10 + 1];

    sprintf(token, "%s.%03d", TOKEN_STR_LAST_LEVEL_SERIES, i);

    fprintf(file, "%s\n", getFormattedSetupEntry(token, last_level_series[i]));
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

void SaveLevelSetup_LastSeries(void)
{
  SaveLevelSetup_LastSeries_Ext(FALSE);
}

void SaveLevelSetup_LastSeries_Deactivate(void)
{
  SaveLevelSetup_LastSeries_Ext(TRUE);
}

static void checkSeriesInfo(void)
{
  static char *level_directory = NULL;
  Directory *dir;
#if 0
  DirectoryEntry *dir_entry;
#endif

  checked_free(level_directory);

  // check for more levels besides the 'levels' field of 'levelinfo.conf'

  level_directory = getPath2((leveldir_current->in_user_dir ?
			      getUserLevelDir(NULL) :
			      options.level_directory),
			     leveldir_current->fullpath);

  if ((dir = openDirectory(level_directory)) == NULL)
  {
    Warn("cannot read level directory '%s'", level_directory);

    return;
  }

#if 0
  while ((dir_entry = readDirectory(dir)) != NULL)	// loop all entries
  {
    if (strlen(dir_entry->basename) > 4 &&
	dir_entry->basename[3] == '.' &&
	strEqual(&dir_entry->basename[4], LEVELFILE_EXTENSION))
    {
      char levelnum_str[4];
      int levelnum_value;

      strncpy(levelnum_str, dir_entry->basename, 3);
      levelnum_str[3] = '\0';

      levelnum_value = atoi(levelnum_str);

      if (levelnum_value < leveldir_current->first_level)
      {
	Warn("additional level %d found", levelnum_value);

	leveldir_current->first_level = levelnum_value;
      }
      else if (levelnum_value > leveldir_current->last_level)
      {
	Warn("additional level %d found", levelnum_value);

	leveldir_current->last_level = levelnum_value;
      }
    }
  }
#endif

  closeDirectory(dir);
}

void LoadLevelSetup_SeriesInfo(void)
{
  char *filename;
  SetupFileHash *level_setup_hash = NULL;
  char *level_subdir = leveldir_current->subdir;
  int i;

  // always start with reliable default values
  level_nr = leveldir_current->first_level;

  for (i = 0; i < MAX_LEVELS; i++)
  {
    LevelStats_setPlayed(i, 0);
    LevelStats_setSolved(i, 0);
  }

  checkSeriesInfo();

  // --------------------------------------------------------------------------
  // ~/.<program>/levelsetup/<level series>/levelsetup.conf
  // --------------------------------------------------------------------------

  level_subdir = leveldir_current->subdir;

  filename = getPath2(getLevelSetupDir(level_subdir), LEVELSETUP_FILENAME);

  if ((level_setup_hash = loadSetupFileHash(filename)))
  {
    char *token_value;

    // get last played level in this level set

    token_value = getHashEntry(level_setup_hash, TOKEN_STR_LAST_PLAYED_LEVEL);

    if (token_value)
    {
      level_nr = atoi(token_value);

      if (level_nr < leveldir_current->first_level)
	level_nr = leveldir_current->first_level;
      if (level_nr > leveldir_current->last_level)
	level_nr = leveldir_current->last_level;
    }

    // get handicap level in this level set

    token_value = getHashEntry(level_setup_hash, TOKEN_STR_HANDICAP_LEVEL);

    if (token_value)
    {
      int level_nr = atoi(token_value);

      if (level_nr < leveldir_current->first_level)
	level_nr = leveldir_current->first_level;
      if (level_nr > leveldir_current->last_level + 1)
	level_nr = leveldir_current->last_level;

      if (leveldir_current->user_defined || !leveldir_current->handicap)
	level_nr = leveldir_current->last_level;

      leveldir_current->handicap_level = level_nr;
    }

    // get number of played and solved levels in this level set

    BEGIN_HASH_ITERATION(level_setup_hash, itr)
    {
      char *token = HASH_ITERATION_TOKEN(itr);
      char *value = HASH_ITERATION_VALUE(itr);

      if (strlen(token) == 3 &&
	  token[0] >= '0' && token[0] <= '9' &&
	  token[1] >= '0' && token[1] <= '9' &&
	  token[2] >= '0' && token[2] <= '9')
      {
	int level_nr = atoi(token);

	if (value != NULL)
	  LevelStats_setPlayed(level_nr, atoi(value));	// read 1st column

	value = strchr(value, ' ');

	if (value != NULL)
	  LevelStats_setSolved(level_nr, atoi(value));	// read 2nd column
      }
    }
    END_HASH_ITERATION(hash, itr)

    freeSetupFileHash(level_setup_hash);
  }
  else
  {
    Debug("setup", "using default setup values");
  }

  free(filename);
}

void SaveLevelSetup_SeriesInfo(void)
{
  char *filename;
  char *level_subdir = leveldir_current->subdir;
  char *level_nr_str = int2str(level_nr, 0);
  char *handicap_level_str = int2str(leveldir_current->handicap_level, 0);
  FILE *file;
  int i;

  // --------------------------------------------------------------------------
  // ~/.<program>/levelsetup/<level series>/levelsetup.conf
  // --------------------------------------------------------------------------

  InitLevelSetupDirectory(level_subdir);

  filename = getPath2(getLevelSetupDir(level_subdir), LEVELSETUP_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write setup file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, LEVELSETUP_FILENAME);

  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_PLAYED_LEVEL,
					       level_nr_str));
  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_HANDICAP_LEVEL,
						 handicap_level_str));

  for (i = leveldir_current->first_level; i <= leveldir_current->last_level;
       i++)
  {
    if (LevelStats_getPlayed(i) > 0 ||
	LevelStats_getSolved(i) > 0)
    {
      char token[16];
      char value[16];

      sprintf(token, "%03d", i);
      sprintf(value, "%d %d", LevelStats_getPlayed(i), LevelStats_getSolved(i));

      fprintf(file, "%s\n", getFormattedSetupEntry(token, value));
    }
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

int LevelStats_getPlayed(int nr)
{
  return (nr >= 0 && nr < MAX_LEVELS ? level_stats[nr].played : 0);
}

int LevelStats_getSolved(int nr)
{
  return (nr >= 0 && nr < MAX_LEVELS ? level_stats[nr].solved : 0);
}

void LevelStats_setPlayed(int nr, int value)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].played = value;
}

void LevelStats_setSolved(int nr, int value)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].solved = value;
}

void LevelStats_incPlayed(int nr)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].played++;
}

void LevelStats_incSolved(int nr)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].solved++;
}

void LoadUserSetup(void)
{
  // --------------------------------------------------------------------------
  // ~/.<program>/usersetup.conf
  // --------------------------------------------------------------------------

  char *filename = getPath2(getMainUserGameDataDir(), USERSETUP_FILENAME);
  SetupFileHash *user_setup_hash = NULL;

  // always start with reliable default values
  user.nr = 0;

  if ((user_setup_hash = loadSetupFileHash(filename)))
  {
    char *token_value;

    // get last selected user number
    token_value = getHashEntry(user_setup_hash, TOKEN_STR_LAST_USER);

    if (token_value)
      user.nr = MIN(MAX(0, atoi(token_value)), MAX_PLAYER_NAMES - 1);

    freeSetupFileHash(user_setup_hash);
  }
  else
  {
    Debug("setup", "using default setup values");
  }

  free(filename);
}

void SaveUserSetup(void)
{
  // --------------------------------------------------------------------------
  // ~/.<program>/usersetup.conf
  // --------------------------------------------------------------------------

  char *filename = getPath2(getMainUserGameDataDir(), USERSETUP_FILENAME);
  FILE *file;

  InitMainUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write setup file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, USERSETUP_FILENAME);

  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_USER,
					       i_to_a(user.nr)));
  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}
