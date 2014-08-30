/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  files.h                                                 *
***********************************************************/

#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "files.h"
#include "tools.h"
#include "misc.h"
#include "tape.h"
#include "joystick.h"

#define MAX_FILENAME_LEN	256	/* maximal filename length */
#define MAX_LINE_LEN		1000	/* maximal input line length */
#define CHUNK_ID_LEN		4	/* IFF style chunk id length */
#define LEVEL_HEADER_SIZE	80	/* size of level file header */
#define LEVEL_HEADER_UNUSED	18	/* unused level header bytes */
#define TAPE_HEADER_SIZE	20	/* size of tape file header */
#define TAPE_HEADER_UNUSED	7	/* unused tape header bytes */
#define FILE_VERSION_1_0	10	/* old 1.0 file version */
#define FILE_VERSION_1_2	12	/* actual file version */

/* file identifier strings */
#define LEVEL_COOKIE		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_1.2"
#define SCORE_COOKIE		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"
#define TAPE_COOKIE		"ROCKSNDIAMONDS_TAPE_FILE_VERSION_1.2"
#define SETUP_COOKIE		"ROCKSNDIAMONDS_SETUP_FILE_VERSION_1.2"
#define LEVELSETUP_COOKIE	"ROCKSNDIAMONDS_LEVELSETUP_FILE_VERSION_1.2"
#define LEVELINFO_COOKIE	"ROCKSNDIAMONDS_LEVELINFO_FILE_VERSION_1.2"
/* old file identifiers for backward compatibility */
#define LEVEL_COOKIE_10		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_1.0"
#define TAPE_COOKIE_10		"ROCKSNDIAMONDS_LEVELREC_FILE_VERSION_1.0"

/* file names and filename extensions */
#ifndef MSDOS
#define USERDATA_DIRECTORY	".rocksndiamonds"
#define SETUP_FILENAME		"setup.conf"
#define LEVELSETUP_FILENAME	"levelsetup.conf"
#define LEVELINFO_FILENAME	"levelinfo.conf"
#define TAPEFILE_EXTENSION	"tape"
#define SCOREFILE_EXTENSION	"score"
#else
#define USERDATA_DIRECTORY	"userdata"
#define SETUP_FILENAME		"setup.cnf"
#define LEVELSETUP_FILENAME	"lvlsetup.cnf"
#define LEVELINFO_FILENAME	"lvlinfo.cnf"
#define TAPEFILE_EXTENSION	"rec"
#define SCOREFILE_EXTENSION	"sco"
#endif

/* file permissions for newly written files */
#define MODE_R_ALL		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_W_ALL		(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_X_ALL		(S_IXUSR | S_IXGRP | S_IXOTH)
#define USERDATA_DIR_MODE	(MODE_R_ALL | MODE_X_ALL | S_IWUSR)
#define LEVEL_PERMS		(MODE_R_ALL | MODE_W_ALL)
#define SCORE_PERMS		LEVEL_PERMS
#define TAPE_PERMS		LEVEL_PERMS
#define SETUP_PERMS		LEVEL_PERMS

static void SaveUserLevelInfo();		/* for 'InitUserLevelDir()' */
static char *getSetupLine(char *, int);		/* for 'SaveUserLevelInfo()' */

static char *getGlobalDataDir()
{
  return GAME_DIR;
}

static char *getUserDataDir()
{
  static char *userdata_dir = NULL;

  if (!userdata_dir)
  {
    char *home_dir = getHomeDir();
    char *data_dir = USERDATA_DIRECTORY;

    userdata_dir = checked_malloc(strlen(home_dir) + strlen(data_dir) + 2);
    sprintf(userdata_dir, "%s/%s", home_dir, data_dir);
  }

  return userdata_dir;
}

static char *getSetupDir()
{
  return getUserDataDir();
}

static char *getUserLevelDir(char *level_subdir)
{
  static char *userlevel_dir = NULL;
  char *data_dir = getUserDataDir();
  char *userlevel_subdir = LEVELS_DIRECTORY;

  if (userlevel_dir)
    free(userlevel_dir);

  userlevel_dir = checked_malloc(strlen(data_dir) + strlen(userlevel_subdir) +
				 strlen(level_subdir) + 3);
  sprintf(userlevel_dir, "%s/%s%s%s", data_dir, userlevel_subdir,
	  (strlen(level_subdir) > 0 ? "/" : ""), level_subdir);

  return userlevel_dir;
}

static char *getTapeDir(char *level_subdir)
{
  static char *tape_dir = NULL;
  char *data_dir = getUserDataDir();
  char *tape_subdir = TAPES_DIRECTORY;

  if (tape_dir)
    free(tape_dir);

  tape_dir = checked_malloc(strlen(data_dir) + strlen(tape_subdir) +
			    strlen(level_subdir) + 3);
  sprintf(tape_dir, "%s/%s%s%s", data_dir, tape_subdir,
	  (strlen(level_subdir) > 0 ? "/" : ""), level_subdir);

  return tape_dir;
}

static char *getScoreDir(char *level_subdir)
{
  static char *score_dir = NULL;
  char *data_dir = getGlobalDataDir();
  char *score_subdir = SCORES_DIRECTORY;

  if (score_dir)
    free(score_dir);

  score_dir = checked_malloc(strlen(data_dir) + strlen(score_subdir) +
			     strlen(level_subdir) + 3);
  sprintf(score_dir, "%s/%s%s%s", data_dir, score_subdir,
	  (strlen(level_subdir) > 0 ? "/" : ""), level_subdir);

  return score_dir;
}

static void createDirectory(char *dir, char *text)
{
  if (access(dir, F_OK) != 0)
    if (mkdir(dir, USERDATA_DIR_MODE) != 0)
      Error(ERR_WARN, "cannot create %s directory '%s'", text, dir);
}

static void InitUserDataDirectory()
{
  createDirectory(getUserDataDir(), "user data");
}

static void InitTapeDirectory(char *level_subdir)
{
  createDirectory(getTapeDir(""), "main tape");
  createDirectory(getTapeDir(level_subdir), "level tape");
}

static void InitScoreDirectory(char *level_subdir)
{
  createDirectory(getScoreDir(""), "main score");
  createDirectory(getScoreDir(level_subdir), "level score");
}

static void InitUserLevelDirectory(char *level_subdir)
{
  if (access(getUserLevelDir(level_subdir), F_OK) != 0)
  {
    createDirectory(getUserLevelDir(""), "main user level");
    createDirectory(getUserLevelDir(level_subdir), "user level");

    SaveUserLevelInfo();
  }
}

static void setLevelInfoToDefaults()
{
  int i, x, y;

  lev_fieldx = level.fieldx = STD_LEV_FIELDX;
  lev_fieldy = level.fieldy = STD_LEV_FIELDY;

  for(x=0; x<MAX_LEV_FIELDX; x++) 
    for(y=0; y<MAX_LEV_FIELDY; y++) 
      Feld[x][y] = Ur[x][y] = EL_ERDREICH;

  level.time = 100;
  level.edelsteine = 0;
  level.tempo_amoebe = 10;
  level.dauer_sieb = 10;
  level.dauer_ablenk = 10;
  level.amoebe_inhalt = EL_DIAMANT;

  strcpy(level.name, "Nameless Level");

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level.score[i] = 10;

  for(i=0; i<4; i++)
    for(x=0; x<3; x++)
      for(y=0; y<3; y++)
	level.mampfer_inhalt[i][x][y] = EL_FELSBROCKEN;

  Feld[0][0] = Ur[0][0] = EL_SPIELFIGUR;
  Feld[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] =
    Ur[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] = EL_AUSGANG_ZU;
}

void LoadLevel(int level_nr)
{
  int i, x, y;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_LINE_LEN];
  char chunk[CHUNK_ID_LEN + 1];
  int file_version = FILE_VERSION_1_2;	/* last version of level files */
  int chunk_length;
  FILE *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults();

  if (leveldir[leveldir_nr].user_defined)
    sprintf(filename, "%s/%s/%d",
	    getUserLevelDir(""), leveldir[leveldir_nr].filename, level_nr);
  else
    sprintf(filename, "%s/%s/%d",
	    options.level_directory, leveldir[leveldir_nr].filename, level_nr);

  if (!(file = fopen(filename, "r")))
  {
    Error(ERR_WARN, "cannot read level '%s' - creating new level", filename);
    return;
  }

  /* check file identifier */
  fgets(cookie, MAX_LINE_LEN, file);
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (strcmp(cookie, LEVEL_COOKIE_10) == 0)	/* old 1.0 level format */
    file_version = FILE_VERSION_1_0;
  else if (strcmp(cookie, LEVEL_COOKIE) != 0)	/* unknown level format */
  {
    Error(ERR_WARN, "wrong file identifier of level file '%s'", filename);
    fclose(file);
    return;
  }

  /* read chunk "HEAD" */
  if (file_version >= FILE_VERSION_1_2)
  {
    /* first check header chunk identifier and chunk length */
    fgets(chunk, CHUNK_ID_LEN + 1, file);
    chunk_length =
      (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
    if (strcmp(chunk, "HEAD") || chunk_length != LEVEL_HEADER_SIZE)
    {
      Error(ERR_WARN, "wrong 'HEAD' chunk of level file '%s'", filename);
      fclose(file);
      return;
    }
  }

  lev_fieldx = level.fieldx = fgetc(file);
  lev_fieldy = level.fieldy = fgetc(file);

  level.time		= (fgetc(file)<<8) | fgetc(file);
  level.edelsteine	= (fgetc(file)<<8) | fgetc(file);

  for(i=0; i<MAX_LEVNAMLEN; i++)
    level.name[i]	= fgetc(file);
  level.name[MAX_LEVNAMLEN - 1] = 0;

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level.score[i]	= fgetc(file);

  for(i=0; i<4; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	level.mampfer_inhalt[i][x][y] = fgetc(file);

  level.tempo_amoebe	= fgetc(file);
  level.dauer_sieb	= fgetc(file);
  level.dauer_ablenk	= fgetc(file);
  level.amoebe_inhalt = fgetc(file);

  for(i=0; i<LEVEL_HEADER_UNUSED; i++)	/* skip unused header bytes */
    fgetc(file);

  /* read chunk "BODY" */
  if (file_version >= FILE_VERSION_1_2)
  {
    /* next check body chunk identifier and chunk length */
    fgets(chunk, CHUNK_ID_LEN + 1, file);
    chunk_length =
      (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
    if (strcmp(chunk, "BODY") || chunk_length != lev_fieldx * lev_fieldy)
    {
      Error(ERR_WARN, "wrong 'BODY' chunk of level file '%s'", filename);
      fclose(file);
      return;
    }
  }

  for(y=0; y<lev_fieldy; y++) 
    for(x=0; x<lev_fieldx; x++) 
      Feld[x][y] = Ur[x][y] = fgetc(file);

  fclose(file);

  if (level.time <= 10)		/* minimum playing time of each level */
    level.time = 10;
}

void SaveLevel(int level_nr)
{
  int i, x, y;
  char filename[MAX_FILENAME_LEN];
  FILE *file;
  int chunk_length;

  if (leveldir[leveldir_nr].user_defined)
    sprintf(filename, "%s/%s/%d",
	    getUserLevelDir(""), leveldir[leveldir_nr].filename, level_nr);
  else
    sprintf(filename, "%s/%s/%d",
	    options.level_directory, leveldir[leveldir_nr].filename, level_nr);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);
    return;
  }

  fputs(LEVEL_COOKIE, file);		/* file identifier */
  fputc('\n', file);

  fputs("HEAD", file);			/* chunk identifier for file header */

  chunk_length = LEVEL_HEADER_SIZE;

  fputc((chunk_length >>  24) & 0xff, file);
  fputc((chunk_length >>  16) & 0xff, file);
  fputc((chunk_length >>   8) & 0xff, file);
  fputc((chunk_length >>   0) & 0xff, file);

  fputc(level.fieldx, file);
  fputc(level.fieldy, file);
  fputc(level.time / 256, file);
  fputc(level.time % 256, file);
  fputc(level.edelsteine / 256, file);
  fputc(level.edelsteine % 256, file);

  for(i=0; i<MAX_LEVNAMLEN; i++)
    fputc(level.name[i], file);
  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    fputc(level.score[i], file);
  for(i=0; i<4; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	fputc(level.mampfer_inhalt[i][x][y], file);
  fputc(level.tempo_amoebe, file);
  fputc(level.dauer_sieb, file);
  fputc(level.dauer_ablenk, file);
  fputc(level.amoebe_inhalt, file);

  for(i=0; i<LEVEL_HEADER_UNUSED; i++)	/* set unused header bytes to zero */
    fputc(0, file);

  fputs("BODY", file);			/* chunk identifier for file body */
  chunk_length = lev_fieldx * lev_fieldy;

  fputc((chunk_length >>  24) & 0xff, file);
  fputc((chunk_length >>  16) & 0xff, file);
  fputc((chunk_length >>   8) & 0xff, file);
  fputc((chunk_length >>   0) & 0xff, file);

  for(y=0; y<lev_fieldy; y++) 
    for(x=0; x<lev_fieldx; x++) 
      fputc(Ur[x][y], file);

  fclose(file);

  chmod(filename, LEVEL_PERMS);
}

void LoadTape(int level_nr)
{
  int i, j;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_LINE_LEN];
  char chunk[CHUNK_ID_LEN + 1];
  FILE *file;
  boolean player_participates[MAX_PLAYERS];
  int num_participating_players;
  int file_version = FILE_VERSION_1_2;	/* last version of tape files */
  int chunk_length;

  sprintf(filename, "%s/%d.%s",
	  getTapeDir(leveldir[leveldir_nr].filename),
	  level_nr, TAPEFILE_EXTENSION);

  if (!(file = fopen(filename, "r")))
    return;

  /* check file identifier */
  fgets(cookie, MAX_LINE_LEN, file);
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (strcmp(cookie, TAPE_COOKIE_10) == 0)	/* old 1.0 tape format */
    file_version = FILE_VERSION_1_0;
  else if (strcmp(cookie, TAPE_COOKIE) != 0)	/* unknown tape format */
  {
    Error(ERR_WARN, "wrong file identifier of tape file '%s'", filename);
    fclose(file);
    return;
  }

  /* read chunk "HEAD" */
  if (file_version >= FILE_VERSION_1_2)
  {
    /* first check header chunk identifier and chunk length */
    fgets(chunk, CHUNK_ID_LEN + 1, file);
    chunk_length =
      (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);

    if (strcmp(chunk, "HEAD") || chunk_length != TAPE_HEADER_SIZE)
    {
      Error(ERR_WARN, "wrong 'HEAD' chunk of tape file '%s'", filename);
      fclose(file);
      return;
    }
  }

  tape.random_seed =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
  tape.date =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
  tape.length =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);

  /* read header fields that are new since version 1.2 */
  if (file_version >= FILE_VERSION_1_2)
  {
    byte store_participating_players = fgetc(file);

    for(i=0; i<TAPE_HEADER_UNUSED; i++)		/* skip unused header bytes */
      fgetc(file);

    /* check which players participate in this tape recording */
    num_participating_players = 0;
    for(i=0; i<MAX_PLAYERS; i++)
    {
      player_participates[i] = FALSE;

      if (store_participating_players & (1 << i))
      {
	player_participates[i] = TRUE;
	num_participating_players++;
      }
    }
  }

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  /* read chunk "BODY" */
  if (file_version >= FILE_VERSION_1_2)
  {
    /* next check body chunk identifier and chunk length */
    fgets(chunk, CHUNK_ID_LEN + 1, file);
    chunk_length =
      (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
    if (strcmp(chunk, "BODY") ||
	chunk_length != (num_participating_players + 1) * tape.length)
    {
      Error(ERR_WARN, "wrong 'BODY' chunk of tape file '%s'", filename);
      fclose(file);
      return;
    }
  }

  for(i=0; i<tape.length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
    {
      tape.pos[i].action[j] = MV_NO_MOVING;

      /* pre-1.2 tapes store data for only one player */
      if (file_version == FILE_VERSION_1_0 && j > 0)
	continue;

      if (player_participates[j])
	tape.pos[i].action[j] = fgetc(file);
    }

    tape.pos[i].delay = fgetc(file);

    if (file_version == FILE_VERSION_1_0)
    {
      /* eliminate possible diagonal moves in old tapes */
      /* this is only for backward compatibility */

      byte joy_dir[4] = { JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
      byte action = tape.pos[i].action[0];
      int k, num_moves = 0;

      for (k=0; k<4; k++)
      {
	if (action & joy_dir[k])
	{
	  tape.pos[i + num_moves].action[0] = joy_dir[k];
	  if (num_moves > 0)
	    tape.pos[i + num_moves].delay = 0;
	  num_moves++;
	}
      }

      if (num_moves > 1)
      {
	num_moves--;
	i += num_moves;
	tape.length += num_moves;
      }
    }

    if (feof(file))
      break;
  }

  fclose(file);

  if (i != tape.length)
    Error(ERR_WARN, "level recording file '%s' corrupted", filename);

  tape.length_seconds = GetTapeLength();
}

void SaveTape(int level_nr)
{
  int i, j;
  char filename[MAX_FILENAME_LEN];
  FILE *file;
  boolean new_tape = TRUE;
  boolean player_participates[MAX_PLAYERS];
  byte store_participating_players;
  int num_participating_players;
  int chunk_length;

  InitTapeDirectory(leveldir[leveldir_nr].filename);

  sprintf(filename, "%s/%d.%s",
	  getTapeDir(leveldir[leveldir_nr].filename),
	  level_nr, TAPEFILE_EXTENSION);

  /* if a tape still exists, ask to overwrite it */
  if ((file = fopen(filename, "r")))
  {
    new_tape = FALSE;
    fclose(file);

    if (!Request("Replace old tape ?", REQ_ASK))
      return;
  }

  for(i=0; i<MAX_PLAYERS; i++)
    player_participates[i] = FALSE;

  /* check which players participate in this tape recording */
  for(i=0; i<tape.length; i++)
    for(j=0; j<MAX_PLAYERS; j++)
      if (tape.pos[i].action[j] != 0)
	player_participates[j] = TRUE;

  /* count number of players and set corresponding bits for compact storage */
  store_participating_players = 0;
  num_participating_players = 0;
  for(i=0; i<MAX_PLAYERS; i++)
  {
    if (player_participates[i])
    {
      num_participating_players++;
      store_participating_players |= (1 << i);
    }
  }

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot save level recording file '%s'", filename);
    return;
  }

  fputs(TAPE_COOKIE, file);		/* file identifier */
  fputc('\n', file);

  fputs("HEAD", file);			/* chunk identifier for file header */

  chunk_length = TAPE_HEADER_SIZE;

  fputc((chunk_length >>  24) & 0xff, file);
  fputc((chunk_length >>  16) & 0xff, file);
  fputc((chunk_length >>   8) & 0xff, file);
  fputc((chunk_length >>   0) & 0xff, file);

  fputc((tape.random_seed >> 24) & 0xff, file);
  fputc((tape.random_seed >> 16) & 0xff, file);
  fputc((tape.random_seed >>  8) & 0xff, file);
  fputc((tape.random_seed >>  0) & 0xff, file);

  fputc((tape.date >>  24) & 0xff, file);
  fputc((tape.date >>  16) & 0xff, file);
  fputc((tape.date >>   8) & 0xff, file);
  fputc((tape.date >>   0) & 0xff, file);

  fputc((tape.length >>  24) & 0xff, file);
  fputc((tape.length >>  16) & 0xff, file);
  fputc((tape.length >>   8) & 0xff, file);
  fputc((tape.length >>   0) & 0xff, file);

  fputc(store_participating_players, file);

  for(i=0; i<TAPE_HEADER_UNUSED; i++)	/* set unused header bytes to zero */
    fputc(0, file);

  fputs("BODY", file);			/* chunk identifier for file body */
  chunk_length = (num_participating_players + 1) * tape.length;

  fputc((chunk_length >>  24) & 0xff, file);
  fputc((chunk_length >>  16) & 0xff, file);
  fputc((chunk_length >>   8) & 0xff, file);
  fputc((chunk_length >>   0) & 0xff, file);

  for(i=0; i<tape.length; i++)
  {
    int j;

    for(j=0; j<MAX_PLAYERS; j++)
      if (player_participates[j])
	fputc(tape.pos[i].action[j], file);

    fputc(tape.pos[i].delay, file);
  }

  fclose(file);

  chmod(filename, TAPE_PERMS);

  tape.changed = FALSE;

  if (new_tape)
    Request("tape saved !", REQ_CONFIRM);
}

void LoadScore(int level_nr)
{
  int i;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_LINE_LEN];
  char line[MAX_LINE_LEN];
  char *line_ptr;
  FILE *file;

  /* start with empty score table */
  for(i=0; i<MAX_SCORE_ENTRIES; i++)
  {
    strcpy(highscore[i].Name, EMPTY_ALIAS);
    highscore[i].Score = 0;
  }

  sprintf(filename, "%s/%d.%s",
	  getScoreDir(leveldir[leveldir_nr].filename),
	  level_nr, SCOREFILE_EXTENSION);

  if (!(file = fopen(filename, "r")))
    return;

  /* check file identifier */
  fgets(cookie, MAX_LINE_LEN, file);

  if (strcmp(cookie, SCORE_COOKIE) != 0)
  {
    Error(ERR_WARN, "wrong file identifier of score file '%s'", filename);
    fclose(file);
    return;
  }

  for(i=0; i<MAX_SCORE_ENTRIES; i++)
  {
    fscanf(file, "%d", &highscore[i].Score);
    fgets(line, MAX_LINE_LEN, file);

    if (line[strlen(line)-1] == '\n')
      line[strlen(line)-1] = '\0';

    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr != ' ' && *line_ptr != '\t' && *line_ptr != '\0')
      {
	strncpy(highscore[i].Name, line_ptr, MAX_NAMELEN - 1);
	highscore[i].Name[MAX_NAMELEN - 1] = '\0';
	break;
      }
    }
  }

  fclose(file);
}

void SaveScore(int level_nr)
{
  int i;
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  InitScoreDirectory(leveldir[leveldir_nr].filename);

  sprintf(filename, "%s/%d.%s",
	  getScoreDir(leveldir[leveldir_nr].filename),
	  level_nr, SCOREFILE_EXTENSION);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot save score for level %d", level_nr);
    return;
  }

  fprintf(file, "%s\n\n", SCORE_COOKIE);

  for(i=0; i<MAX_SCORE_ENTRIES; i++)
    fprintf(file, "%d %s\n", highscore[i].Score, highscore[i].Name);

  fclose(file);

  chmod(filename, SCORE_PERMS);
}

#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"
#define TOKEN_STR_LAST_LEVEL_SERIES	"last_level_series"
#define TOKEN_STR_PLAYER_PREFIX		"player_"

#define TOKEN_VALUE_POSITION		30

/* global setup */
#define SETUP_TOKEN_PLAYER_NAME		0
#define SETUP_TOKEN_SOUND		1
#define SETUP_TOKEN_SOUND_LOOPS		2
#define SETUP_TOKEN_SOUND_MUSIC		3
#define SETUP_TOKEN_SOUND_SIMPLE	4
#define SETUP_TOKEN_TOONS		5
#define SETUP_TOKEN_DOUBLE_BUFFERING	6
#define SETUP_TOKEN_SCROLL_DELAY	7
#define SETUP_TOKEN_SOFT_SCROLLING	8
#define SETUP_TOKEN_FADING		9
#define SETUP_TOKEN_AUTORECORD		10
#define SETUP_TOKEN_QUICK_DOORS		11
#define SETUP_TOKEN_TEAM_MODE		12

/* player setup */
#define SETUP_TOKEN_USE_JOYSTICK	13
#define SETUP_TOKEN_JOY_DEVICE_NAME	14
#define SETUP_TOKEN_JOY_XLEFT		15
#define SETUP_TOKEN_JOY_XMIDDLE		16
#define SETUP_TOKEN_JOY_XRIGHT		17
#define SETUP_TOKEN_JOY_YUPPER		18
#define SETUP_TOKEN_JOY_YMIDDLE		19
#define SETUP_TOKEN_JOY_YLOWER		20
#define SETUP_TOKEN_JOY_SNAP		21
#define SETUP_TOKEN_JOY_BOMB		22
#define SETUP_TOKEN_KEY_LEFT		23
#define SETUP_TOKEN_KEY_RIGHT		24
#define SETUP_TOKEN_KEY_UP		25
#define SETUP_TOKEN_KEY_DOWN		26
#define SETUP_TOKEN_KEY_SNAP		27
#define SETUP_TOKEN_KEY_BOMB		28

/* level directory info */
#define LEVELINFO_TOKEN_NAME		29
#define LEVELINFO_TOKEN_LEVELS		30
#define LEVELINFO_TOKEN_SORT_PRIORITY	31
#define LEVELINFO_TOKEN_READONLY	32

#define FIRST_GLOBAL_SETUP_TOKEN	SETUP_TOKEN_PLAYER_NAME
#define LAST_GLOBAL_SETUP_TOKEN		SETUP_TOKEN_TEAM_MODE

#define FIRST_PLAYER_SETUP_TOKEN	SETUP_TOKEN_USE_JOYSTICK
#define LAST_PLAYER_SETUP_TOKEN		SETUP_TOKEN_KEY_BOMB

#define FIRST_LEVELINFO_TOKEN		LEVELINFO_TOKEN_NAME
#define LAST_LEVELINFO_TOKEN		LEVELINFO_TOKEN_READONLY

#define TYPE_BOOLEAN			1
#define TYPE_SWITCH			2
#define TYPE_KEYSYM			3
#define TYPE_INTEGER			4
#define TYPE_STRING			5

static struct SetupInfo si;
static struct SetupInputInfo sii;
static struct LevelDirInfo ldi;
static struct
{
  int type;
  void *value;
  char *text;
} token_info[] =
{
  /* global setup */
  { TYPE_STRING,  &si.player_name,	"player_name"			},
  { TYPE_SWITCH,  &si.sound,		"sound"				},
  { TYPE_SWITCH,  &si.sound_loops,	"repeating_sound_loops"		},
  { TYPE_SWITCH,  &si.sound_music,	"background_music"		},
  { TYPE_SWITCH,  &si.sound_simple,	"simple_sound_effects"		},
  { TYPE_SWITCH,  &si.toons,		"toons"				},
  { TYPE_SWITCH,  &si.double_buffering,	"double_buffering"		},
  { TYPE_SWITCH,  &si.scroll_delay,	"scroll_delay"			},
  { TYPE_SWITCH,  &si.soft_scrolling,	"soft_scrolling"		},
  { TYPE_SWITCH,  &si.fading,		"screen_fading"			},
  { TYPE_SWITCH,  &si.autorecord,	"automatic_tape_recording"	},
  { TYPE_SWITCH,  &si.quick_doors,	"quick_doors"			},
  { TYPE_SWITCH,  &si.team_mode,	"team_mode"			},

  /* player setup */
  { TYPE_BOOLEAN, &sii.use_joystick,	".use_joystick"			},
  { TYPE_STRING,  &sii.joy.device_name,	".joy.device_name"		},
  { TYPE_INTEGER, &sii.joy.xleft,	".joy.xleft"			},
  { TYPE_INTEGER, &sii.joy.xmiddle,	".joy.xmiddle"			},
  { TYPE_INTEGER, &sii.joy.xright,	".joy.xright"			},
  { TYPE_INTEGER, &sii.joy.yupper,	".joy.yupper"			},
  { TYPE_INTEGER, &sii.joy.ymiddle,	".joy.ymiddle"			},
  { TYPE_INTEGER, &sii.joy.ylower,	".joy.ylower"			},
  { TYPE_INTEGER, &sii.joy.snap,	".joy.snap_field"		},
  { TYPE_INTEGER, &sii.joy.bomb,	".joy.place_bomb"		},
  { TYPE_KEYSYM,  &sii.key.left,	".key.move_left"		},
  { TYPE_KEYSYM,  &sii.key.right,	".key.move_right"		},
  { TYPE_KEYSYM,  &sii.key.up,		".key.move_up"			},
  { TYPE_KEYSYM,  &sii.key.down,	".key.move_down"		},
  { TYPE_KEYSYM,  &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEYSYM,  &sii.key.bomb,	".key.place_bomb"		},

  /* level directory info */
  { TYPE_STRING,  &ldi.name,		"name"				},
  { TYPE_INTEGER, &ldi.levels,		"levels"			},
  { TYPE_INTEGER, &ldi.sort_priority,	"sort_priority"			},
  { TYPE_BOOLEAN, &ldi.readonly,	"readonly"			}
};

static char *string_tolower(char *s)
{
  static char s_lower[100];
  int i;

  if (strlen(s) >= 100)
    return s;

  strcpy(s_lower, s);

  for (i=0; i<strlen(s_lower); i++)
    s_lower[i] = tolower(s_lower[i]);

  return s_lower;
}

static int get_string_integer_value(char *s)
{
  static char *number_text[][3] =
  {
    { "0", "zero", "null", },
    { "1", "one", "first" },
    { "2", "two", "second" },
    { "3", "three", "third" },
    { "4", "four", "fourth" },
    { "5", "five", "fifth" },
    { "6", "six", "sixth" },
    { "7", "seven", "seventh" },
    { "8", "eight", "eighth" },
    { "9", "nine", "ninth" },
    { "10", "ten", "tenth" },
    { "11", "eleven", "eleventh" },
    { "12", "twelve", "twelfth" },
  };

  int i, j;

  for (i=0; i<13; i++)
    for (j=0; j<3; j++)
      if (strcmp(string_tolower(s), number_text[i][j]) == 0)
	return i;

  return atoi(s);
}

static boolean get_string_boolean_value(char *s)
{
  if (strcmp(string_tolower(s), "true") == 0 ||
      strcmp(string_tolower(s), "yes") == 0 ||
      strcmp(string_tolower(s), "on") == 0 ||
      get_string_integer_value(s) == 1)
    return TRUE;
  else
    return FALSE;
}

static char *getFormattedSetupEntry(char *token, char *value)
{
  int i;
  static char entry[MAX_LINE_LEN];

  sprintf(entry, "%s:", token);
  for (i=strlen(entry); i<TOKEN_VALUE_POSITION; i++)
    entry[i] = ' ';
  entry[i] = '\0';

  strcat(entry, value);

  return entry;
}

static void freeSetupFileList(struct SetupFileList *setup_file_list)
{
  if (!setup_file_list)
    return;

  if (setup_file_list->token)
    free(setup_file_list->token);
  if (setup_file_list->value)
    free(setup_file_list->value);
  if (setup_file_list->next)
    freeSetupFileList(setup_file_list->next);
  free(setup_file_list);
}

static struct SetupFileList *newSetupFileList(char *token, char *value)
{
  struct SetupFileList *new = checked_malloc(sizeof(struct SetupFileList));

  new->token = checked_malloc(strlen(token) + 1);
  strcpy(new->token, token);

  new->value = checked_malloc(strlen(value) + 1);
  strcpy(new->value, value);

  new->next = NULL;

  return new;
}

static char *getTokenValue(struct SetupFileList *setup_file_list,
			   char *token)
{
  if (!setup_file_list)
    return NULL;

  if (strcmp(setup_file_list->token, token) == 0)
    return setup_file_list->value;
  else
    return getTokenValue(setup_file_list->next, token);
}

static void setTokenValue(struct SetupFileList *setup_file_list,
			  char *token, char *value)
{
  if (!setup_file_list)
    return;

  if (strcmp(setup_file_list->token, token) == 0)
  {
    free(setup_file_list->value);
    setup_file_list->value = checked_malloc(strlen(value) + 1);
    strcpy(setup_file_list->value, value);
  }
  else if (setup_file_list->next == NULL)
    setup_file_list->next = newSetupFileList(token, value);
  else
    setTokenValue(setup_file_list->next, token, value);
}

#ifdef DEBUG
static void printSetupFileList(struct SetupFileList *setup_file_list)
{
  if (!setup_file_list)
    return;

  printf("token: '%s'\n", setup_file_list->token);
  printf("value: '%s'\n", setup_file_list->value);

  printSetupFileList(setup_file_list->next);
}
#endif

static struct SetupFileList *loadSetupFileList(char *filename)
{
  int line_len;
  char line[MAX_LINE_LEN];
  char *token, *value, *line_ptr;
  struct SetupFileList *setup_file_list = newSetupFileList("", "");
  struct SetupFileList *first_valid_list_entry;

  FILE *file;

  if (!(file = fopen(filename, "r")))
  {
    Error(ERR_WARN, "cannot open setup/info file '%s'", filename);
    return NULL;
  }

  while(!feof(file))
  {
    /* read next line of input file */
    if (!fgets(line, MAX_LINE_LEN, file))
      break;

    /* cut trailing comment or whitespace from input line */
    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr == '#' || *line_ptr == '\n')
      {
	*line_ptr = '\0';
	break;
      }
    }

    /* cut trailing whitespaces from input line */
    for (line_ptr = &line[strlen(line)]; line_ptr > line; line_ptr--)
      if ((*line_ptr == ' ' || *line_ptr == '\t') && line_ptr[1] == '\0')
	*line_ptr = '\0';

    /* ignore empty lines */
    if (*line == '\0')
      continue;

    line_len = strlen(line);

    /* cut leading whitespaces from token */
    for (token = line; *token; token++)
      if (*token != ' ' && *token != '\t')
	break;

    /* find end of token */
    for (line_ptr = token; *line_ptr; line_ptr++)
    {
      if (*line_ptr == ' ' || *line_ptr == '\t' || *line_ptr == ':')
      {
	*line_ptr = '\0';
	break;
      }
    }

    if (line_ptr < line + line_len)
      value = line_ptr + 1;
    else
      value = "\0";

    /* cut leading whitespaces from value */
    for (; *value; value++)
      if (*value != ' ' && *value != '\t')
	break;

    if (*token && *value)
      setTokenValue(setup_file_list, token, value);
  }

  fclose(file);

  first_valid_list_entry = setup_file_list->next;

  /* free empty list header */
  setup_file_list->next = NULL;
  freeSetupFileList(setup_file_list);

  if (first_valid_list_entry == NULL)
    Error(ERR_WARN, "setup/info file '%s' is empty", filename);

  return first_valid_list_entry;
}

static void checkSetupFileListIdentifier(struct SetupFileList *setup_file_list,
					 char *identifier)
{
  if (!setup_file_list)
    return;

  if (strcmp(setup_file_list->token, TOKEN_STR_FILE_IDENTIFIER) == 0)
  {
    if (strcmp(setup_file_list->value, identifier) != 0)
    {
      Error(ERR_WARN, "setup/info file has wrong version");
      return;
    }
    else
      return;
  }

  if (setup_file_list->next)
    checkSetupFileListIdentifier(setup_file_list->next, identifier);
  else
  {
    Error(ERR_WARN, "setup/info file has no version information");
    return;
  }
}

static void setLevelDirInfoToDefaults(struct LevelDirInfo *ldi)
{
  ldi->name = getStringCopy("non-existing");
  ldi->levels = 0;
  ldi->sort_priority = 999;	/* default: least priority */
  ldi->readonly = TRUE;
}

static void setSetupInfoToDefaults(struct SetupInfo *si)
{
  int i;

  si->player_name = getStringCopy(getLoginName());

  si->sound = TRUE;
  si->sound_loops = FALSE;
  si->sound_music = FALSE;
  si->sound_simple = FALSE;
  si->toons = TRUE;
  si->double_buffering = TRUE;
  si->direct_draw = !si->double_buffering;
  si->scroll_delay = FALSE;
  si->soft_scrolling = TRUE;
  si->fading = FALSE;
  si->autorecord = FALSE;
  si->quick_doors = FALSE;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    si->input[i].use_joystick = FALSE;
    si->input[i].joy.device_name = getStringCopy(joystick_device_name[i]);
    si->input[i].joy.xleft   = JOYSTICK_XLEFT;
    si->input[i].joy.xmiddle = JOYSTICK_XMIDDLE;
    si->input[i].joy.xright  = JOYSTICK_XRIGHT;
    si->input[i].joy.yupper  = JOYSTICK_YUPPER;
    si->input[i].joy.ymiddle = JOYSTICK_YMIDDLE;
    si->input[i].joy.ylower  = JOYSTICK_YLOWER;
    si->input[i].joy.snap  = (i == 0 ? JOY_BUTTON_1 : 0);
    si->input[i].joy.bomb  = (i == 0 ? JOY_BUTTON_2 : 0);
    si->input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KEY_UNDEFINDED);
    si->input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KEY_UNDEFINDED);
    si->input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KEY_UNDEFINDED);
    si->input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KEY_UNDEFINDED);
    si->input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KEY_UNDEFINDED);
    si->input[i].key.bomb  = (i == 0 ? DEFAULT_KEY_BOMB  : KEY_UNDEFINDED);
  }
}

static void setSetupInfo(int token_nr, char *token_value)
{
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;

  if (token_value == NULL)
    return;

  /* set setup field to corresponding token value */
  switch (token_type)
  {
    case TYPE_BOOLEAN:
    case TYPE_SWITCH:
      *(boolean *)setup_value = get_string_boolean_value(token_value);
      break;

    case TYPE_KEYSYM:
      *(KeySym *)setup_value = getKeySymFromX11KeyName(token_value);
      break;

    case TYPE_INTEGER:
      *(int *)setup_value = get_string_integer_value(token_value);
      break;

    case TYPE_STRING:
      if (*(char **)setup_value != NULL)
	free(*(char **)setup_value);
      *(char **)setup_value = getStringCopy(token_value);
      break;

    default:
      break;
  }
}

static void decodeSetupFileList(struct SetupFileList *setup_file_list)
{
  int i, pnr;

  if (!setup_file_list)
    return;

  /* handle global setup values */
  si = setup;
  for (i=FIRST_GLOBAL_SETUP_TOKEN; i<=LAST_GLOBAL_SETUP_TOKEN; i++)
    setSetupInfo(i, getTokenValue(setup_file_list, token_info[i].text));
  setup = si;

  /* handle player specific setup values */
  for (pnr=0; pnr<MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);

    sii = setup.input[pnr];
    for (i=FIRST_PLAYER_SETUP_TOKEN; i<=LAST_PLAYER_SETUP_TOKEN; i++)
    {
      char full_token[100];

      sprintf(full_token, "%s%s", prefix, token_info[i].text);
      setSetupInfo(i, getTokenValue(setup_file_list, full_token));
    }
    setup.input[pnr] = sii;
  }
}

int getLevelSeriesNrFromLevelSeriesName(char *level_series_name)
{
  int i;

  if (!level_series_name)
    return 0;

  for (i=0; i<num_leveldirs; i++)
    if (strcmp(level_series_name, leveldir[i].filename) == 0)
      return i;

  return 0;
}

int getLastPlayedLevelOfLevelSeries(char *level_series_name)
{
  char *token_value;
  int level_series_nr = getLevelSeriesNrFromLevelSeriesName(level_series_name);
  int last_level_nr = 0;

  if (!level_series_name)
    return 0;

  token_value = getTokenValue(level_setup_list, level_series_name);

  if (token_value)
  {
    int highest_level_nr = leveldir[level_series_nr].levels - 1;

    last_level_nr = atoi(token_value);

    if (last_level_nr < 0)
      last_level_nr = 0;
    if (last_level_nr > highest_level_nr)
      last_level_nr = highest_level_nr;
  }

  return last_level_nr;
}

static int compareLevelDirInfoEntries(const void *object1, const void *object2)
{
  const struct LevelDirInfo *entry1 = object1;
  const struct LevelDirInfo *entry2 = object2;
  int compare_result;

  if (entry1->sort_priority != entry2->sort_priority)
    compare_result = entry1->sort_priority - entry2->sort_priority;
  else
  {
    char *name1 = getStringToLower(entry1->name);
    char *name2 = getStringToLower(entry2->name);

    compare_result = strcmp(name1, name2);

    free(name1);
    free(name2);
  }

  return compare_result;
}

static int LoadLevelInfoFromLevelDir(char *level_directory, int start_entry)
{
  DIR *dir;
  struct stat file_status;
  char *directory = NULL;
  char *filename = NULL;
  struct SetupFileList *setup_file_list = NULL;
  struct dirent *dir_entry;
  int i, current_entry = start_entry;

  if ((dir = opendir(level_directory)) == NULL)
    Error(ERR_EXIT, "cannot read level directory '%s'", level_directory);

  while (current_entry < MAX_LEVDIR_ENTRIES)
  {
    if ((dir_entry = readdir(dir)) == NULL)	/* last directory entry */
      break;

    /* skip entries for current and parent directory */
    if (strcmp(dir_entry->d_name, ".")  == 0 ||
	strcmp(dir_entry->d_name, "..") == 0)
      continue;

    /* find out if directory entry is itself a directory */
    directory = getPath2(level_directory, dir_entry->d_name);
    if (stat(directory, &file_status) != 0 ||		/* cannot stat file */
	(file_status.st_mode & S_IFMT) != S_IFDIR)	/* not a directory */
    {
      free(directory);
      continue;
    }

    filename = getPath2(directory, LEVELINFO_FILENAME);
    setup_file_list = loadSetupFileList(filename);

    if (setup_file_list)
    {
      checkSetupFileListIdentifier(setup_file_list, LEVELINFO_COOKIE);
      setLevelDirInfoToDefaults(&leveldir[current_entry]);

      ldi = leveldir[current_entry];
      for (i=FIRST_LEVELINFO_TOKEN; i<=LAST_LEVELINFO_TOKEN; i++)
	setSetupInfo(i, getTokenValue(setup_file_list, token_info[i].text));
      leveldir[current_entry] = ldi;

      leveldir[current_entry].filename = getStringCopy(dir_entry->d_name);
      leveldir[current_entry].user_defined =
	(level_directory == options.level_directory ? FALSE : TRUE);

      freeSetupFileList(setup_file_list);
      current_entry++;
    }
    else
      Error(ERR_WARN, "ignoring level directory '%s'", directory);

    free(directory);
    free(filename);
  }

  if (current_entry == MAX_LEVDIR_ENTRIES)
    Error(ERR_WARN, "using %d level directories -- ignoring the rest",
	  current_entry);

  closedir(dir);

  if (current_entry == start_entry && start_entry != -1)
    Error(ERR_EXIT, "cannot find any valid level series in directory '%s'",
	  level_directory);

  return current_entry;
}

void LoadLevelInfo()
{
  InitUserLevelDirectory(getLoginName());

  num_leveldirs = 0;
  leveldir_nr = 0;

  num_leveldirs = LoadLevelInfoFromLevelDir(options.level_directory,
					    num_leveldirs);
  num_leveldirs = LoadLevelInfoFromLevelDir(getUserLevelDir(""),
					    num_leveldirs);
  if (num_leveldirs > 1)
    qsort(leveldir, num_leveldirs, sizeof(struct LevelDirInfo),
	  compareLevelDirInfoEntries);
}

static void SaveUserLevelInfo()
{
  char filename[MAX_FILENAME_LEN];
  FILE *file;
  int i;

  sprintf(filename, "%s/%s",
	  getUserLevelDir(getLoginName()), LEVELINFO_FILENAME);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot write level info file '%s'", filename);
    return;
  }

  ldi.name = getLoginName();
  ldi.levels = 100;
  ldi.sort_priority = 300;
  ldi.readonly = FALSE;

  fprintf(file, "%s\n\n",
	  getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER, LEVELINFO_COOKIE));

  for (i=FIRST_LEVELINFO_TOKEN; i<=LAST_LEVELINFO_TOKEN; i++)
    fprintf(file, "%s\n", getSetupLine("", i));

  fclose(file);

  chmod(filename, SETUP_PERMS);
}

void LoadSetup()
{
  char filename[MAX_FILENAME_LEN];
  struct SetupFileList *setup_file_list = NULL;

  /* always start with reliable default values */
  setSetupInfoToDefaults(&setup);

  sprintf(filename, "%s/%s", getSetupDir(), SETUP_FILENAME);

  setup_file_list = loadSetupFileList(filename);

  if (setup_file_list)
  {
    checkSetupFileListIdentifier(setup_file_list, SETUP_COOKIE);
    decodeSetupFileList(setup_file_list);

    setup.direct_draw = !setup.double_buffering;

    freeSetupFileList(setup_file_list);

    /* needed to work around problems with fixed length strings */
    if (strlen(setup.player_name) >= MAX_NAMELEN)
      setup.player_name[MAX_NAMELEN - 1] = '\0';
    else if (strlen(setup.player_name) < MAX_NAMELEN - 1)
    {
      char *new_name = checked_malloc(MAX_NAMELEN);

      strcpy(new_name, setup.player_name);
      free(setup.player_name);
      setup.player_name = new_name;
    }
  }
  else
    Error(ERR_WARN, "using default setup values");
}

static char *getSetupLine(char *prefix, int token_nr)
{
  int i;
  static char entry[MAX_LINE_LEN];
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;
  char *token_text = token_info[token_nr].text;

  /* start with the prefix, token and some spaces to format output line */
  sprintf(entry, "%s%s:", prefix, token_text);
  for (i=strlen(entry); i<TOKEN_VALUE_POSITION; i++)
    strcat(entry, " ");

  /* continue with the token's value (which can have different types) */
  switch (token_type)
  {
    case TYPE_BOOLEAN:
      strcat(entry, (*(boolean *)setup_value ? "true" : "false"));
      break;

    case TYPE_SWITCH:
      strcat(entry, (*(boolean *)setup_value ? "on" : "off"));
      break;

    case TYPE_KEYSYM:
      {
	KeySym keysym = *(KeySym *)setup_value;
	char *keyname = getKeyNameFromKeySym(keysym);

	strcat(entry, getX11KeyNameFromKeySym(keysym));
	for (i=strlen(entry); i<50; i++)
	  strcat(entry, " ");

	/* add comment, if useful */
	if (strcmp(keyname, "(undefined)") != 0 &&
	    strcmp(keyname, "(unknown)") != 0)
	{
	  strcat(entry, "# ");
	  strcat(entry, keyname);
	}
      }
      break;

    case TYPE_INTEGER:
      {
	char buffer[MAX_LINE_LEN];

	sprintf(buffer, "%d", *(int *)setup_value);
	strcat(entry, buffer);
      }
      break;

    case TYPE_STRING:
      strcat(entry, *(char **)setup_value);
      break;

    default:
      break;
  }

  return entry;
}

void SaveSetup()
{
  int i, pnr;
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  InitUserDataDirectory();

  sprintf(filename, "%s/%s", getSetupDir(), SETUP_FILENAME);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintf(file, "%s\n",
	  getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER, SETUP_COOKIE));
  fprintf(file, "\n");

  /* handle global setup values */
  si = setup;
  for (i=FIRST_GLOBAL_SETUP_TOKEN; i<=LAST_GLOBAL_SETUP_TOKEN; i++)
  {
    fprintf(file, "%s\n", getSetupLine("", i));

    /* just to make things nicer :) */
    if (i == SETUP_TOKEN_PLAYER_NAME)
      fprintf(file, "\n");
  }

  /* handle player specific setup values */
  for (pnr=0; pnr<MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);
    fprintf(file, "\n");

    sii = setup.input[pnr];
    for (i=FIRST_PLAYER_SETUP_TOKEN; i<=LAST_PLAYER_SETUP_TOKEN; i++)
      fprintf(file, "%s\n", getSetupLine(prefix, i));
  }

  fclose(file);

  chmod(filename, SETUP_PERMS);
}

void LoadLevelSetup()
{
  char filename[MAX_FILENAME_LEN];

  /* always start with reliable default values */
  leveldir_nr = 0;
  level_nr = 0;

  sprintf(filename, "%s/%s", getSetupDir(), LEVELSETUP_FILENAME);

  if (level_setup_list)
    freeSetupFileList(level_setup_list);

  level_setup_list = loadSetupFileList(filename);

  if (level_setup_list)
  {
    char *last_level_series =
      getTokenValue(level_setup_list, TOKEN_STR_LAST_LEVEL_SERIES);

    leveldir_nr = getLevelSeriesNrFromLevelSeriesName(last_level_series);
    level_nr = getLastPlayedLevelOfLevelSeries(last_level_series);

    checkSetupFileListIdentifier(level_setup_list, LEVELSETUP_COOKIE);
  }
  else
  {
    level_setup_list = newSetupFileList(TOKEN_STR_FILE_IDENTIFIER,
					LEVELSETUP_COOKIE);
    Error(ERR_WARN, "using default setup values");
  }
}

void SaveLevelSetup()
{
  char filename[MAX_FILENAME_LEN];
  struct SetupFileList *list_entry = level_setup_list;
  FILE *file;

  InitUserDataDirectory();

  setTokenValue(level_setup_list,
		TOKEN_STR_LAST_LEVEL_SERIES, leveldir[leveldir_nr].filename);

  setTokenValue(level_setup_list,
		leveldir[leveldir_nr].filename, int2str(level_nr, 0));

  sprintf(filename, "%s/%s", getSetupDir(), LEVELSETUP_FILENAME);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER,
						 LEVELSETUP_COOKIE));
  while (list_entry)
  {
    if (strcmp(list_entry->token, TOKEN_STR_FILE_IDENTIFIER) != 0)
      fprintf(file, "%s\n",
	      getFormattedSetupEntry(list_entry->token, list_entry->value));

    /* just to make things nicer :) */
    if (strcmp(list_entry->token, TOKEN_STR_LAST_LEVEL_SERIES) == 0)
      fprintf(file, "\n");

    list_entry = list_entry->next;
  }

  fclose(file);

  chmod(filename, SETUP_PERMS);
}
