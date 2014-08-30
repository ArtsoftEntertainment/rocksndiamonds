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

#include "files.h"
#include "tools.h"
#include "misc.h"
#include "tape.h"
#include "joystick.h"

BOOL CreateNewScoreFile()
{
  int i,j,k;
  char filename[MAX_FILENAME];
  char empty_alias[MAX_NAMELEN];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"w")))
    return(FALSE);

  for(i=0;i<MAX_NAMELEN;i++)
    empty_alias[i] = 0;
  strncpy(empty_alias,EMPTY_ALIAS,MAX_NAMELEN-1);

  fputs(SCORE_COOKIE,file);		/* Formatkennung */
  for(i=0;i<leveldir[leveldir_nr].levels;i++)
  {
    for(j=0;j<MAX_SCORE_ENTRIES;j++)
    {
      for(k=0;k<MAX_NAMELEN;k++)
	fputc(empty_alias[k],file);
      fputc(0,file);
      fputc(0,file);
    }
  }
  fclose(file);

  chmod(filename, SCORE_PERMS);
  return(TRUE);
}

BOOL CreateNewNamesFile(int mode)
{
  char filename[MAX_FILENAME];
  FILE *file;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    level_directory,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  if (!(file=fopen(filename,"w")))
    return(FALSE);

  fputs(NAMES_COOKIE,file);		/* Formatkennung */
  fclose(file);

  chmod(filename, NAMES_PERMS);
  return(TRUE);
}

BOOL LoadLevelInfo()
{
  int i;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s",level_directory,LEVDIR_FILENAME);

  if (!(file=fopen(filename,"r")))
  {
    fprintf(stderr,"%s: cannot load level info '%s'!\n",progname,filename);
    return(FALSE);
  }

  fscanf(file,"%s\n",cookie);
  if (strcmp(cookie,LEVELDIR_COOKIE))	/* ung�ltiges Format? */
  {
    fprintf(stderr,"%s: wrong format of level info file!\n",progname);
    fclose(file);
    return(FALSE);
  }

  num_leveldirs = 0;
  leveldir_nr = 0;
  for(i=0;i<MAX_LEVDIR_ENTRIES;i++)
  {
    fscanf(file,"%s",leveldir[i].filename);
    fscanf(file,"%s",leveldir[i].name);
    fscanf(file,"%d",&leveldir[i].levels);
    fscanf(file,"%d",&leveldir[i].readonly);
    if (feof(file))
      break;

    num_leveldirs++;
  }

  if (!num_leveldirs)
  {
    fprintf(stderr,"%s: empty level info '%s'!\n",progname,filename);
    return(FALSE);
  }

  return(TRUE);
}

void LoadLevel(int level_nr)
{
  int i,x,y;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%d",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);

  if (!(file=fopen(filename,"r")))
  {
/*
    fprintf(stderr,"%s: cannot load level '%s'!\n",progname,filename);
*/
  }
  else
  {
    fgets(cookie,LEVEL_COOKIE_LEN,file);
    fgetc(file);

    if (strcmp(cookie,LEVEL_COOKIE))	/* ung�ltiges Format? */
    {
      fprintf(stderr,"%s: wrong format of level file '%s'!\n",
	      progname,filename);
      fclose(file);
      file = NULL;
    }
  }

  if (file)
  {
    lev_fieldx = level.fieldx = fgetc(file);
    lev_fieldy = level.fieldy = fgetc(file);

    level.time		= (fgetc(file)<<8) | fgetc(file);
    level.edelsteine	= (fgetc(file)<<8) | fgetc(file);
    for(i=0;i<MAX_LEVNAMLEN;i++)
      level.name[i]	= fgetc(file);
    level.name[MAX_LEVNAMLEN-1] = 0;
    for(i=0;i<MAX_LEVSCORE_ENTRIES;i++)
      level.score[i]	= fgetc(file);
    for(i=0;i<4;i++)
      for(y=0;y<3;y++)
	for(x=0;x<3;x++)
	  level.mampfer_inhalt[i][x][y] = fgetc(file);
    level.tempo_amoebe	= fgetc(file);
    level.dauer_sieb	= fgetc(file);
    level.dauer_ablenk	= fgetc(file);
    level.amoebe_inhalt = fgetc(file);

    for(i=0;i<NUM_FREE_LVHD_BYTES;i++)	/* Rest frei / Headergr��e 80 Bytes */
      fgetc(file);

    for(y=0;y<MAX_LEV_FIELDY;y++) 
      for(x=0;x<MAX_LEV_FIELDX;x++) 
	Feld[x][y] = Ur[x][y] = EL_ERDREICH;

    for(y=0;y<lev_fieldy;y++) 
      for(x=0;x<lev_fieldx;x++) 
	Feld[x][y] = Ur[x][y] = fgetc(file);

    fclose(file);

    if (level.time<=10)	/* Mindestspieldauer */
      level.time = 10;
  }
  else
  {
    lev_fieldx = level.fieldx = STD_LEV_FIELDX;
    lev_fieldy = level.fieldy = STD_LEV_FIELDY;

    level.time		= 100;
    level.edelsteine	= 0;
    strncpy(level.name,"Nameless Level",MAX_LEVNAMLEN-1);
    for(i=0;i<MAX_LEVSCORE_ENTRIES;i++)
      level.score[i]	= 10;
    for(i=0;i<4;i++)
      for(y=0;y<3;y++)
	for(x=0;x<3;x++)
	  level.mampfer_inhalt[i][x][y] = EL_FELSBROCKEN;
    level.tempo_amoebe	= 10;
    level.dauer_sieb	= 10;
    level.dauer_ablenk	= 10;
    level.amoebe_inhalt = EL_DIAMANT;

    for(y=0;y<STD_LEV_FIELDY;y++) 
      for(x=0;x<STD_LEV_FIELDX;x++) 
	Feld[x][y] = Ur[x][y] = EL_ERDREICH;
    Feld[0][0] = Ur[0][0] = EL_SPIELFIGUR;
    Feld[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] =
      Ur[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] = EL_AUSGANG_ZU;
  }
}

void LoadLevelTape(int level_nr)
{
  int i;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;
  BOOL levelrec_10 = FALSE;

#ifndef MSDOS
  sprintf(filename,"%s/%s/%d.tape",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#else
  sprintf(filename,"%s/%s/%d.tap",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#endif

  if ((file=fopen(filename,"r")))
  {
    fgets(cookie,LEVELREC_COOKIE_LEN,file);
    fgetc(file);
    if (!strcmp(cookie,LEVELREC_COOKIE_10))	/* old 1.0 tape format */
      levelrec_10 = TRUE;
    else if (strcmp(cookie,LEVELREC_COOKIE))	/* unknown tape format */
    {
      fprintf(stderr,"%s: wrong format of level recording file '%s'!\n",
	      progname,filename);
      fclose(file);
      file = NULL;
    }
  }

  if (!file)
    return;

  tape.random_seed =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
  tape.date =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
  tape.length =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  for(i=0;i<tape.length;i++)
  {
    int j;

    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
    {
      if (levelrec_10 && j>0)
      {
	tape.pos[i].joystickdata[j] = MV_NO_MOVING;
	continue;
      }
      tape.pos[i].joystickdata[j] = fgetc(file);
    }

    tape.pos[i].delay = fgetc(file);

    if (feof(file))
      break;
  }

  fclose(file);

  if (i != tape.length)
    fprintf(stderr,"%s: level recording file '%s' corrupted!\n",
	    progname,filename);

  tape.length_seconds = GetTapeLength();
}

void LoadScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"r")))
  {
    if (!CreateNewScoreFile())
    {
      fprintf(stderr,"%s: cannot create score file '%s'!\n",
	      progname,filename);
    }
    else if (!(file=fopen(filename,"r"))) 
    {
      fprintf(stderr,"%s: cannot load score for level %d!\n",
	      progname,level_nr);
    }
  }

  if (file)
  {
    fgets(cookie,SCORE_COOKIE_LEN,file);
    if (strcmp(cookie,SCORE_COOKIE))	/* ung�ltiges Format? */
    {
      fprintf(stderr,"%s: wrong format of score file!\n",progname);
      fclose(file);
      file = NULL;
    }
  }

  if (file)
  {
    fseek(file,
	  SCORE_COOKIE_LEN-1+level_nr*(MAX_SCORE_ENTRIES*(MAX_NAMELEN+2)),
	  SEEK_SET);
    for(i=0;i<MAX_SCORE_ENTRIES;i++)
    {
      for(j=0;j<MAX_NAMELEN;j++)
	highscore[i].Name[j] = fgetc(file);
      highscore[i].Score = (fgetc(file)<<8) | fgetc(file);
    }
    fclose(file);
  }
  else
  {
    for(i=0;i<MAX_SCORE_ENTRIES;i++)
    {
      strcpy(highscore[i].Name,EMPTY_ALIAS);
      highscore[i].Score = 0;
    }
  }
}

void LoadPlayerInfo(int mode)
{
  int i;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;
  char *login_name = GetLoginName();
  struct PlayerInfo default_player, new_player;
  int version_10_file = FALSE;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    level_directory,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  for(i=0;i<MAX_NAMELEN;i++)
    default_player.login_name[i] = default_player.alias_name[i] = 0;
  strncpy(default_player.login_name,login_name,MAX_NAMELEN-1);
  strncpy(default_player.alias_name,login_name,MAX_NAMELEN-1);
  default_player.handicap = 0;
  default_player.setup = DEFAULT_SETUP;
  default_player.leveldir_nr = 0;
  default_player.level_nr = 0;

  new_player = default_player;

  if (!(file=fopen(filename,"r")))
  {
    if (!CreateNewNamesFile(mode))
    {
      fprintf(stderr,"%s: cannot create names file '%s'!\n",
	      progname,filename);
    }
    else if (!(file=fopen(filename,"r"))) 
    {
      fprintf(stderr,"%s: cannot load player information '%s'!\n",
	      progname,filename);
    }
  }

  if (file)
  {
    fgets(cookie,NAMES_COOKIE_LEN,file);
    if (!strcmp(cookie,NAMES_COOKIE_10))	/* altes Format? */
      version_10_file = TRUE;
    else if (strcmp(cookie,NAMES_COOKIE))	/* ung�ltiges Format? */
    {
      fprintf(stderr,"%s: wrong format of names file '%s'!\n",
	      progname,filename);
      fclose(file);
      file = NULL;
    }
  }

  if (!file)
  {
    *local_player = default_player;
    level_nr = default_player.level_nr;
    return;
  }

  while(1)
  {
    for(i=0;i<MAX_NAMELEN;i++)
      new_player.login_name[i] = fgetc(file);
    for(i=0;i<MAX_NAMELEN;i++)
      new_player.alias_name[i] = fgetc(file);
    new_player.handicap = fgetc(file);
    new_player.setup = (fgetc(file)<<8) | fgetc(file);
    new_player.leveldir_nr = fgetc(file);
    if (!version_10_file)
    {
      new_player.level_nr = fgetc(file);
      for(i=0;i<10;i++)		/* currently unused bytes */
	fgetc(file);
    }
    else
      new_player.level_nr = new_player.handicap;

    if (feof(file))		/* Spieler noch nicht in Liste enthalten */
    {
      new_player = default_player;

      fclose(file);
      if (!(file=fopen(filename,"a")))
      {
	fprintf(stderr,"%s: cannot append new player to names file '%s'!\n",
		progname,filename);
      }
      else
      {
	for(i=0;i<MAX_NAMELEN;i++)
	  fputc(new_player.login_name[i],file);
	for(i=0;i<MAX_NAMELEN;i++)
	  fputc(new_player.alias_name[i],file);
	fputc(new_player.handicap,file);
	fputc(new_player.setup / 256,file);
	fputc(new_player.setup % 256,file);
	fputc(new_player.leveldir_nr,file);
	if (!version_10_file)
	{
	  fputc(new_player.level_nr,file);
	  for(i=0;i<10;i++)	/* currently unused bytes */
	    fputc(0,file);
	}
      }
      break;
    }
    else			/* pr�fen, ob Spieler in Liste enthalten */
      if (!strncmp(new_player.login_name,login_name,MAX_NAMELEN-1))
	break;
  }

  if (mode==PLAYER_SETUP)
  {
    *local_player = new_player;
    if (local_player->leveldir_nr < num_leveldirs)
      leveldir_nr = local_player->leveldir_nr;
    else
      leveldir_nr = 0;
  }
  else
  {
    local_player->handicap = new_player.handicap;
    local_player->level_nr = new_player.level_nr;
  }

  level_nr = local_player->level_nr;
  fclose(file);
}

void SaveLevel(int level_nr)
{
  int i,x,y;
  char filename[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%d",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);

  if (!(file=fopen(filename,"w")))
  {
    fprintf(stderr,"%s: cannot save level file '%s'!\n",progname,filename);
    return;
  }

  fputs(LEVEL_COOKIE,file);		/* Formatkennung */
  fputc(0x0a,file);

  fputc(level.fieldx,file);
  fputc(level.fieldy,file);
  fputc(level.time / 256,file);
  fputc(level.time % 256,file);
  fputc(level.edelsteine / 256,file);
  fputc(level.edelsteine % 256,file);

  for(i=0;i<MAX_LEVNAMLEN;i++)
    fputc(level.name[i],file);
  for(i=0;i<MAX_LEVSCORE_ENTRIES;i++)
    fputc(level.score[i],file);
  for(i=0;i<4;i++)
    for(y=0;y<3;y++)
      for(x=0;x<3;x++)
	fputc(level.mampfer_inhalt[i][x][y],file);
  fputc(level.tempo_amoebe,file);
  fputc(level.dauer_sieb,file);
  fputc(level.dauer_ablenk,file);
  fputc(level.amoebe_inhalt,file);

  for(i=0;i<NUM_FREE_LVHD_BYTES;i++)	/* Rest frei / Headergr��e 80 Bytes */
    fputc(0,file);

  for(y=0;y<lev_fieldy;y++) 
    for(x=0;x<lev_fieldx;x++) 
      fputc(Ur[x][y],file);

  fclose(file);

  chmod(filename, LEVEL_PERMS);
}

void SaveLevelTape(int level_nr)
{
  int i;
  char filename[MAX_FILENAME];
  FILE *file;
  BOOL new_tape = TRUE;

#ifndef MSDOS
  sprintf(filename,"%s/%s/%d.tape",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#else
  sprintf(filename,"%s/%s/%d.tap",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#endif

  /* Testen, ob bereits eine Aufnahme existiert */
  if ((file=fopen(filename,"r")))
  {
    new_tape = FALSE;
    fclose(file);

    if (!AreYouSure("Replace old tape ?",AYS_ASK))
      return;
  }

  if (!(file=fopen(filename,"w")))
  {
    fprintf(stderr,"%s: cannot save level recording file '%s'!\n",
	    progname,filename);
    return;
  }

  fputs(LEVELREC_COOKIE,file);		/* Formatkennung */
  fputc(0x0a,file);

  fputc((tape.random_seed >> 24) & 0xff,file);
  fputc((tape.random_seed >> 16) & 0xff,file);
  fputc((tape.random_seed >>  8) & 0xff,file);
  fputc((tape.random_seed >>  0) & 0xff,file);

  fputc((tape.date >>  24) & 0xff,file);
  fputc((tape.date >>  16) & 0xff,file);
  fputc((tape.date >>   8) & 0xff,file);
  fputc((tape.date >>   0) & 0xff,file);

  fputc((tape.length >>  24) & 0xff,file);
  fputc((tape.length >>  16) & 0xff,file);
  fputc((tape.length >>   8) & 0xff,file);
  fputc((tape.length >>   0) & 0xff,file);

  for(i=0;i<tape.length;i++)
  {
    int j;

    for(j=0; j<MAX_PLAYERS; j++)
      fputc(tape.pos[i].joystickdata[j],file);

    fputc(tape.pos[i].delay,file);
  }

  fclose(file);

  chmod(filename, LEVREC_PERMS);

  tape.changed = FALSE;

  if (new_tape)
    AreYouSure("tape saved !",AYS_CONFIRM);
}

void SaveScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"r+")))
  {
    fprintf(stderr,"%s: cannot save score for level %d!\n",
	    progname,level_nr);
    return;
  }

  fseek(file,
	SCORE_COOKIE_LEN-1+level_nr*(MAX_SCORE_ENTRIES*(MAX_NAMELEN+2)),
	SEEK_SET);
  for(i=0;i<MAX_SCORE_ENTRIES;i++)
  {
    for(j=0;j<MAX_NAMELEN;j++)
      fputc(highscore[i].Name[j],file);
    fputc(highscore[i].Score / 256,file);
    fputc(highscore[i].Score % 256,file);
  }
  fclose(file);
}

void SavePlayerInfo(int mode)
{
  int i;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;
  struct PlayerInfo default_player;
  int version_10_file = FALSE;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    level_directory,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  if (!(file=fopen(filename,"r+")))
  {
    fprintf(stderr,"%s: cannot save player information '%s'!\n",
	    progname,filename);
    return;
  }

  fgets(cookie,NAMES_COOKIE_LEN,file);
  if (!strcmp(cookie,NAMES_COOKIE_10))	/* altes Format? */
    version_10_file = TRUE;
  else if (strcmp(cookie,NAMES_COOKIE))	/* ung�ltiges Format? */
  {
    fprintf(stderr,"%s: wrong format of names file '%s'!\n",
	    progname,filename);
    fclose(file);
    return;
  }

  while(1)
  {
    for(i=0;i<MAX_NAMELEN;i++)
      default_player.login_name[i] = fgetc(file);
    for(i=0;i<MAX_NAMELEN;i++)
      default_player.alias_name[i] = fgetc(file);
    default_player.handicap = fgetc(file);
    default_player.setup = (fgetc(file)<<8) | fgetc(file);
    default_player.leveldir_nr = fgetc(file);
    if (!version_10_file)
    {
      default_player.level_nr = fgetc(file);
      for(i=0;i<10;i++)		/* currently unused bytes */
	fgetc(file);
    }
    else
      default_player.level_nr = default_player.handicap;

    if (feof(file))		/* Spieler noch nicht in Liste enthalten */
      break;
    else			/* pr�fen, ob Spieler in Liste enthalten */
      if (!strncmp(default_player.login_name,
		   local_player->login_name, MAX_NAMELEN-1))
      {
	fseek(file,-(2*MAX_NAMELEN+1+2+1+(version_10_file ? 0 : 11)),SEEK_CUR);
	break;
      }
  }

  local_player->level_nr = level_nr;

  for(i=0;i<MAX_NAMELEN;i++)
    fputc(local_player->login_name[i],file);
  for(i=0;i<MAX_NAMELEN;i++)
    fputc(local_player->alias_name[i],file);
  fputc(local_player->handicap,file);
  fputc(local_player->setup / 256,file);
  fputc(local_player->setup % 256,file);
  fputc(local_player->leveldir_nr,file);
  if (!version_10_file)
  {
    fputc(local_player->level_nr,file);
    for(i=0;i<10;i++)		/* currently unused bytes */
      fputc(0,file);
  }

  fclose(file);
}

void LoadJoystickData()
{
  int i;
  char cookie[256];
  FILE *file;

  if (joystick_status==JOYSTICK_OFF)
    return;

#ifndef MSDOS
  if (!(file=fopen(JOYDAT_FILE,"r")))
    return;

  fscanf(file,"%s",cookie);
  if (strcmp(cookie,JOYSTICK_COOKIE))	/* ung�ltiges Format? */
  {
    fprintf(stderr,"%s: wrong format of joystick file!\n",progname);
    fclose(file);
    return;
  }

  for(i=0;i<2;i++)
  {
    fscanf(file,"%s",cookie);
    fscanf(file, "%d %d %d \n",
	   &joystick[i].xleft, &joystick[i].xmiddle, &joystick[i].xright);
    fscanf(file, "%d %d %d \n",
	   &joystick[i].yupper, &joystick[i].ymiddle, &joystick[i].ylower);
  }
  fclose(file);

  CheckJoystickData();
#else
  load_joystick_data(JOYDAT_FILE);
#endif
}

void SaveJoystickData()
{
  int i;
  FILE *file;

  if (joystick_status==JOYSTICK_OFF)
    return;

#ifndef MSDOS
  CheckJoystickData();

  if (!(file=fopen(JOYDAT_FILE,"w")))
  {
    fprintf(stderr,"%s: cannot save joystick calibration data!\n",progname);
    return;
  }

  fprintf(file,"%s\n",JOYSTICK_COOKIE);	/* Formatkennung */
  for(i=0;i<2;i++)
  {
    fprintf(file,"JOYSTICK_%d_DATA\n",i);
    fprintf(file, "%d %d %d \n",
	    joystick[i].xleft, joystick[i].xmiddle, joystick[i].xright);
    fprintf(file, "%d %d %d \n",
	    joystick[i].yupper, joystick[i].ymiddle, joystick[i].ylower);
  }
  fclose(file);

  chmod(JOYDAT_FILE, JOYDAT_PERMS);
#else
  save_joystick_data(JOYDAT_FILE);
#endif

}
