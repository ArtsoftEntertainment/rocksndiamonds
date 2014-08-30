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
*  game.c                                                  *
***********************************************************/

#include "game.h"
#include "misc.h"
#include "tools.h"
#include "screens.h"
#include "sound.h"
#include "init.h"
#include "buttons.h"
#include "files.h"
#include "tape.h"
#include "joystick.h"
#include "network.h"

void GetPlayerConfig()
{
  int old_joystick_nr = setup.joystick_nr;

  if (sound_status==SOUND_OFF)
    local_player->setup &= ~SETUP_SOUND;
  if (!sound_loops_allowed)
  {
    local_player->setup &= ~SETUP_SOUND_LOOPS;
    local_player->setup &= ~SETUP_SOUND_MUSIC;
  }

  setup.sound_on = setup.sound_simple_on = SETUP_SOUND_ON(local_player->setup);
  setup.sound_loops_on = SETUP_SOUND_LOOPS_ON(local_player->setup);
  setup.sound_music_on = SETUP_SOUND_MUSIC_ON(local_player->setup);
  setup.toons_on = SETUP_TOONS_ON(local_player->setup);
  setup.direct_draw_on = SETUP_DIRECT_DRAW_ON(local_player->setup);
  setup.fading_on = SETUP_FADING_ON(local_player->setup);
  setup.autorecord_on = SETUP_AUTO_RECORD_ON(local_player->setup);
  setup.joystick_nr = SETUP_2ND_JOYSTICK_ON(local_player->setup);
  setup.quick_doors = SETUP_QUICK_DOORS_ON(local_player->setup);
  setup.scroll_delay_on = SETUP_SCROLL_DELAY_ON(local_player->setup);
  setup.soft_scrolling_on = SETUP_SOFT_SCROLL_ON(local_player->setup);

#ifndef MSDOS
  if (setup.joystick_nr != old_joystick_nr)
  {
    if (joystick_device)
      close(joystick_device);
    InitJoystick();
  }
#endif
}

void InitGame()
{
  int i,j, x,y;
  boolean emulate_bd = TRUE;	/* unless non-BOULDERDASH elements found */
  boolean emulate_sb = TRUE;	/* unless non-SOKOBAN     elements found */

  /* don't play tapes over network */
  network_playing = (options.network && !tape.playing);

  for(i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    player->index_nr = i;
    player->element_nr = EL_SPIELER1 + i;

    player->present = FALSE;
    player->active = FALSE;

    /*
    player->local = FALSE;
    */

    player->score = 0;
    player->gems_still_needed = level.edelsteine;
    player->sokobanfields_still_needed = 0;
    player->lights_still_needed = 0;
    player->friends_still_needed = 0;

    for(j=0; j<4; j++)
      player->key[j] = FALSE;

    player->dynamite = 0;
    player->dynabomb_count = 0;
    player->dynabomb_size = 0;
    player->dynabombs_left = 0;
    player->dynabomb_xl = FALSE;

    player->MovDir = MV_NO_MOVING;
    player->MovPos = 0;
    player->Pushing = FALSE;
    player->GfxPos = 0;
    player->Frame = 0;

    player->actual_frame_counter = 0;

    player->frame_reset_delay = 0;

    player->push_delay = 0;
    player->push_delay_value = 5;

    player->move_delay = 0;
    player->last_move_dir = MV_NO_MOVING;

    player->snapped = FALSE;

    player->gone = FALSE;

    player->last_jx = player->last_jy = 0;
    player->jx = player->jy = 0;

    DigField(player, 0,0,0,0,DF_NO_PUSH);
    SnapField(player, 0,0);


    /* TEST TEST TEST */

    /*
    stored_player[i].active = TRUE;
    */

    /* TEST TEST TEST */

    player->LevelSolved = FALSE;
    player->GameOver = FALSE;
  }

  /*
  local_player->active = TRUE;
  local_player->local = TRUE;
  */

  network_player_action_received = FALSE;

  /* initial null action */
  if (network_playing)
    SendToServer_MovePlayer(MV_NO_MOVING);

  ZX = ZY = -1;

  MampferNr = 0;
  FrameCounter = 0;
  TimeFrames = 0;
  TimeLeft = level.time;

  ScreenMovDir = MV_NO_MOVING;
  ScreenMovPos = 0;
  ScreenGfxPos = 0;

  AllPlayersGone = SiebAktiv = FALSE;

  for(i=0;i<MAX_NUM_AMOEBA;i++)
    AmoebaCnt[i] = AmoebaCnt2[i] = 0;

  for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
  {
    Feld[x][y] = Ur[x][y];
    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    Store[x][y] = Store2[x][y] = StorePlayer[x][y] = 0;
    Frame[x][y] = 0;
    AmoebaNr[x][y] = 0;
    JustHit[x][y] = 0;

    if (emulate_bd && !IS_BD_ELEMENT(Feld[x][y]))
      emulate_bd = FALSE;
    if (emulate_sb && !IS_SB_ELEMENT(Feld[x][y]))
      emulate_sb = FALSE;

    switch(Feld[x][y])
    {
      case EL_SPIELFIGUR:
	Feld[x][y] = EL_SPIELER1;
	/* no break! */
      case EL_SPIELER1:
      case EL_SPIELER2:
      case EL_SPIELER3:
      case EL_SPIELER4:
      {
	struct PlayerInfo *player = &stored_player[Feld[x][y] - EL_SPIELER1];
	int jx = player->jx, jy = player->jy;

	/*
	player->active = TRUE;
	*/

	player->present = TRUE;
	if (player->connected)
	{
	  player->active = TRUE;

	  printf("Player %d activated.\n", player->element_nr);
	  printf("[Local player is %d and currently %s.]\n",
		 local_player->element_nr,
		 local_player->active ? "active" : "not active");
	}

	/* remove potentially duplicate players */
	if (StorePlayer[jx][jy] == Feld[x][y])
	  StorePlayer[jx][jy] = 0;

	StorePlayer[x][y] = Feld[x][y];
	Feld[x][y] = EL_LEERRAUM;
	player->jx = player->last_jx = x;
	player->jy = player->last_jy = y;

	break;
      }
      case EL_BADEWANNE:
	if (x<lev_fieldx-1 && Feld[x+1][y]==EL_SALZSAEURE)
	  Feld[x][y] = EL_BADEWANNE1;
	else if (x>0 && Feld[x-1][y]==EL_SALZSAEURE)
	  Feld[x][y] = EL_BADEWANNE2;
	else if (y>0 && Feld[x][y-1]==EL_BADEWANNE1)
	  Feld[x][y] = EL_BADEWANNE3;
	else if (y>0 && Feld[x][y-1]==EL_SALZSAEURE)
	  Feld[x][y] = EL_BADEWANNE4;
	else if (y>0 && Feld[x][y-1]==EL_BADEWANNE2)
	  Feld[x][y] = EL_BADEWANNE5;
	break;
      case EL_KAEFER_R:
      case EL_KAEFER_O:
      case EL_KAEFER_L:
      case EL_KAEFER_U:
      case EL_KAEFER:
      case EL_FLIEGER_R:
      case EL_FLIEGER_O:
      case EL_FLIEGER_L:
      case EL_FLIEGER_U:
      case EL_FLIEGER:
      case EL_BUTTERFLY_R:
      case EL_BUTTERFLY_O:
      case EL_BUTTERFLY_L:
      case EL_BUTTERFLY_U:
      case EL_BUTTERFLY:
      case EL_FIREFLY_R:
      case EL_FIREFLY_O:
      case EL_FIREFLY_L:
      case EL_FIREFLY_U:
      case EL_FIREFLY:
      case EL_PACMAN_R:
      case EL_PACMAN_O:
      case EL_PACMAN_L:
      case EL_PACMAN_U:
      case EL_MAMPFER:
      case EL_MAMPFER2:
      case EL_ROBOT:
      case EL_PACMAN:
	InitMovDir(x,y);
	break;
      case EL_AMOEBE_VOLL:
      case EL_AMOEBE_BD:
	InitAmoebaNr(x,y);
	break;
      case EL_TROPFEN:
	if (y==lev_fieldy-1)
	{
	  Feld[x][y] = EL_AMOEBING;
	  Store[x][y] = EL_AMOEBE_NASS;
	}
	break;
      case EL_DYNAMIT:
	MovDelay[x][y] = 96;
	break;
      case EL_BIRNE_AUS:
	local_player->lights_still_needed++;
	break;
      case EL_SOKOBAN_FELD_LEER:
	local_player->sokobanfields_still_needed++;
	break;
      case EL_MAULWURF:
      case EL_PINGUIN:
	local_player->friends_still_needed++;
	break;
      case EL_SCHWEIN:
      case EL_DRACHE:
	MovDir[x][y] = 1<<RND(4);
	break;
      default:
	break;
    }
  }

  /* check if any connected player was not found in playfield */
  for(i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->connected && !player->present)
    {
      printf("Oops!\n");


      for(j=0; j<MAX_PLAYERS; j++)
      {
	struct PlayerInfo *some_player = &stored_player[j];
	int jx = some_player->jx, jy = some_player->jy;

	/* assign first free player found that is present in the playfield */
	if (some_player->present && !some_player->connected)
	{
	  player->present = TRUE;
	  player->active = TRUE;
	  some_player->present = FALSE;

	  StorePlayer[jx][jy] = player->element_nr;
	  player->jx = player->last_jx = jx;
	  player->jy = player->last_jy = jy;

	  break;
	}
      }
    }
  }

  for(i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    printf("Player %d: present == %d, connected == %d, active == %d.\n",
	   i+1,
	   player->present,
	   player->connected,
	   player->active);
    if (local_player == player)
      printf("Player %d is local player.\n", i+1);
  }


  game_emulation = (emulate_bd ? EMU_BOULDERDASH :
		    emulate_sb ? EMU_SOKOBAN : EMU_NONE);

  scroll_x = scroll_y = -1;
  if (local_player->jx >= MIDPOSX-1)
    scroll_x = (local_player->jx <= lev_fieldx-MIDPOSX ?
		local_player->jx - MIDPOSX :
		lev_fieldx - SCR_FIELDX + 1);
  if (local_player->jy >= MIDPOSY-1)
    scroll_y = (local_player->jy <= lev_fieldy-MIDPOSY ?
		local_player->jy - MIDPOSY :
		lev_fieldy - SCR_FIELDY + 1);

  CloseDoor(DOOR_CLOSE_1);

  DrawLevel();
  DrawAllPlayers();
  FadeToFront();

  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX5,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_LEVEL,DOOR_GFX_PAGEY1+YY_LEVEL,
	      int2str(level_nr,2),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_EMERALDS,DOOR_GFX_PAGEY1+YY_EMERALDS,
	      int2str(local_player->gems_still_needed,3),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_DYNAMITE,DOOR_GFX_PAGEY1+YY_DYNAMITE,
	      int2str(local_player->dynamite,3),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_SCORE,DOOR_GFX_PAGEY1+YY_SCORE,
	      int2str(local_player->score,5),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_TIME,DOOR_GFX_PAGEY1+YY_TIME,
	      int2str(TimeLeft,3),FS_SMALL,FC_YELLOW);

  DrawGameButton(BUTTON_GAME_STOP);
  DrawGameButton(BUTTON_GAME_PAUSE);
  DrawGameButton(BUTTON_GAME_PLAY);
  DrawSoundDisplay(BUTTON_SOUND_MUSIC  | (BUTTON_ON * setup.sound_music_on));
  DrawSoundDisplay(BUTTON_SOUND_LOOPS  | (BUTTON_ON * setup.sound_loops_on));
  DrawSoundDisplay(BUTTON_SOUND_SIMPLE | (BUTTON_ON * setup.sound_simple_on));
  XCopyArea(display,drawto,pix[PIX_DB_DOOR],gc,
	    DX+GAME_CONTROL_XPOS,DY+GAME_CONTROL_YPOS,
	    GAME_CONTROL_XSIZE,2*GAME_CONTROL_YSIZE,
	    DOOR_GFX_PAGEX1+GAME_CONTROL_XPOS,
	    DOOR_GFX_PAGEY1+GAME_CONTROL_YPOS);

  OpenDoor(DOOR_OPEN_1);

  if (setup.sound_music_on)
    PlaySoundLoop(background_loop[level_nr % num_bg_loops]);

  XAutoRepeatOff(display);


  for (i=0;i<4;i++)
    printf("Spieler %d %saktiv.\n",
	   i+1, (stored_player[i].active ? "" : "nicht "));

}

void InitMovDir(int x, int y)
{
  int i, element = Feld[x][y];
  static int xy[4][2] =
  {
    { 0,+1 },
    { +1,0 },
    { 0,-1 },
    { -1,0 }
  };
  static int direction[2][4] =
  {
    { MV_RIGHT, MV_UP,   MV_LEFT,  MV_DOWN },
    { MV_LEFT,  MV_DOWN, MV_RIGHT, MV_UP }
  };

  switch(element)
  {
    case EL_KAEFER_R:
    case EL_KAEFER_O:
    case EL_KAEFER_L:
    case EL_KAEFER_U:
      Feld[x][y] = EL_KAEFER;
      MovDir[x][y] = direction[0][element-EL_KAEFER_R];
      break;
    case EL_FLIEGER_R:
    case EL_FLIEGER_O:
    case EL_FLIEGER_L:
    case EL_FLIEGER_U:
      Feld[x][y] = EL_FLIEGER;
      MovDir[x][y] = direction[0][element-EL_FLIEGER_R];
      break;
    case EL_BUTTERFLY_R:
    case EL_BUTTERFLY_O:
    case EL_BUTTERFLY_L:
    case EL_BUTTERFLY_U:
      Feld[x][y] = EL_BUTTERFLY;
      MovDir[x][y] = direction[0][element-EL_BUTTERFLY_R];
      break;
    case EL_FIREFLY_R:
    case EL_FIREFLY_O:
    case EL_FIREFLY_L:
    case EL_FIREFLY_U:
      Feld[x][y] = EL_FIREFLY;
      MovDir[x][y] = direction[0][element-EL_FIREFLY_R];
      break;
    case EL_PACMAN_R:
    case EL_PACMAN_O:
    case EL_PACMAN_L:
    case EL_PACMAN_U:
      Feld[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element-EL_PACMAN_R];
      break;
    default:
      MovDir[x][y] = 1<<RND(4);
      if (element != EL_KAEFER &&
	  element != EL_FLIEGER &&
	  element != EL_BUTTERFLY &&
	  element != EL_FIREFLY)
	break;

      for(i=0;i<4;i++)
      {
	int x1,y1;

	x1 = x+xy[i][0];
	y1 = y+xy[i][1];

	if (!IN_LEV_FIELD(x1,y1) || !IS_FREE(x1,y1))
	{
	  if (element==EL_KAEFER || element==EL_BUTTERFLY)
	  {
	    MovDir[x][y] = direction[0][i];
	    break;
	  }
	  else if (element==EL_FLIEGER || element==EL_FIREFLY)
	  {
	    MovDir[x][y] = direction[1][i];
	    break;
	  }
	}
      }
      break;
  }
}

void InitAmoebaNr(int x, int y)
{
  int i;
  int group_nr = AmoebeNachbarNr(x,y);

  if (group_nr==0)
  {
    for(i=1;i<MAX_NUM_AMOEBA;i++)
    {
      if (AmoebaCnt[i]==0)
      {
	group_nr = i;
	break;
      }
    }
  }

  AmoebaNr[x][y] = group_nr;
  AmoebaCnt[group_nr]++;
  AmoebaCnt2[group_nr]++;
}

void GameWon()
{
  int hi_pos;
  int bumplevel = FALSE;

  local_player->LevelSolved = FALSE;

  if (TimeLeft)
  {
    if (setup.sound_loops_on)
      PlaySoundExt(SND_SIRR, PSND_MAX_VOLUME, PSND_MAX_RIGHT, PSND_LOOP);

    while(TimeLeft > 0)
    {
      if (!setup.sound_loops_on)
	PlaySoundStereo(SND_SIRR, PSND_MAX_RIGHT);
      if (TimeLeft && !(TimeLeft % 10))
	RaiseScore(level.score[SC_ZEITBONUS]);
      if (TimeLeft > 100 && !(TimeLeft % 10))
	TimeLeft -= 10;
      else
	TimeLeft--;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft,3), FS_SMALL, FC_YELLOW);
      BackToFront();
      Delay(10);
    }

    if (setup.sound_loops_on)
      StopSound(SND_SIRR);
  }

  FadeSounds();

  /* Hero disappears */
  DrawLevelField(ExitX, ExitY);
  BackToFront();

  if (tape.playing)
    return;

  CloseDoor(DOOR_CLOSE_1);

  if (tape.recording)
  {
    TapeStop();
    SaveLevelTape(tape.level_nr);	/* Ask to save tape */
  }

  if (level_nr == local_player->handicap &&
      level_nr < leveldir[leveldir_nr].levels-1)
  { 
    local_player->handicap++; 
    bumplevel = TRUE;
    SavePlayerInfo(PLAYER_LEVEL);
  }

  if ((hi_pos=NewHiScore()) >= 0) 
  {
    game_status = HALLOFFAME;
    DrawHallOfFame(hi_pos);
    if (bumplevel && TAPE_IS_EMPTY(tape))
      level_nr++;
  }
  else
  {
    game_status = MAINMENU;
    if (bumplevel && TAPE_IS_EMPTY(tape))
      level_nr++;
    DrawMainMenu();
  }

  BackToFront();
}

boolean NewHiScore()
{
  int k,l;
  int position = -1;

  LoadScore(level_nr);

  if (!strcmp(local_player->alias_name,EMPTY_ALIAS) ||
      local_player->score < highscore[MAX_SCORE_ENTRIES-1].Score) 
    return(-1);

  for(k=0;k<MAX_SCORE_ENTRIES;k++) 
  {
    if (local_player->score > highscore[k].Score)
    {
      /* Spieler kommt in Highscore-Liste */

      if (k<MAX_SCORE_ENTRIES-1)
      {
	int m = MAX_SCORE_ENTRIES-1;

#ifdef ONE_PER_NAME
	for(l=k;l<MAX_SCORE_ENTRIES;l++)
	  if (!strcmp(local_player->alias_name,highscore[l].Name))
	    m = l;
	if (m==k)	/* Spieler �berschreibt seine alte Position */
	  goto put_into_list;
#endif

	for(l=m;l>k;l--)
	{
	  strcpy(highscore[l].Name,highscore[l-1].Name);
	  highscore[l].Score = highscore[l-1].Score;
	}
      }

#ifdef ONE_PER_NAME
      put_into_list:
#endif
      sprintf(highscore[k].Name,local_player->alias_name);
      highscore[k].Score = local_player->score; 
      position = k;
      break;
    }

#ifdef ONE_PER_NAME
    else if (!strcmp(local_player->alias_name,highscore[k].Name))
      break;	/* Spieler schon mit besserer Punktzahl in der Liste */
#endif

  }

  if (position>=0) 
    SaveScore(level_nr);

  return(position);
}

void InitMovingField(int x, int y, int direction)
{
  int newx = x + (direction==MV_LEFT ? -1 : direction==MV_RIGHT ? +1 : 0);
  int newy = y + (direction==MV_UP   ? -1 : direction==MV_DOWN  ? +1 : 0);

  MovDir[x][y] = direction;
  MovDir[newx][newy] = direction;
  if (Feld[newx][newy] == EL_LEERRAUM)
    Feld[newx][newy] = EL_BLOCKED;
}

void Moving2Blocked(int x, int y, int *goes_to_x, int *goes_to_y)
{
  int direction = MovDir[x][y];
  int newx = x + (direction==MV_LEFT ? -1 : direction==MV_RIGHT ? +1 : 0);
  int newy = y + (direction==MV_UP   ? -1 : direction==MV_DOWN  ? +1 : 0);

  *goes_to_x = newx;
  *goes_to_y = newy;
}

void Blocked2Moving(int x, int y, int *comes_from_x, int *comes_from_y)
{
  int oldx = x, oldy = y;
  int direction = MovDir[x][y];

  if (direction==MV_LEFT)
    oldx++;
  else if (direction==MV_RIGHT)
    oldx--;
  else if (direction==MV_UP)
    oldy++;
  else if (direction==MV_DOWN)
    oldy--;

  *comes_from_x = oldx;
  *comes_from_y = oldy;
}

int MovingOrBlocked2Element(int x, int y)
{
  int element = Feld[x][y];

  if (element==EL_BLOCKED)
  {
    int oldx,oldy;

    Blocked2Moving(x,y,&oldx,&oldy);
    return(Feld[oldx][oldy]);
  }
  else
    return(element);
}

static void RemoveField(int x, int y)
{
  Feld[x][y] = EL_LEERRAUM;
  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;
}

void RemoveMovingField(int x, int y)
{
  int oldx = x,oldy = y, newx = x,newy = y;

  if (Feld[x][y] != EL_BLOCKED && !IS_MOVING(x,y))
    return;

  if (IS_MOVING(x,y))
  {
    Moving2Blocked(x,y,&newx,&newy);
    if (Feld[newx][newy] != EL_BLOCKED)
      return;
  }
  else if (Feld[x][y]==EL_BLOCKED)
  {
    Blocked2Moving(x,y,&oldx,&oldy);
    if (!IS_MOVING(oldx,oldy))
      return;
  }

  if (Feld[x][y]==EL_BLOCKED &&
      (Store[oldx][oldy]==EL_MORAST_LEER ||
       Store[oldx][oldy]==EL_SIEB_LEER ||
       Store[oldx][oldy]==EL_SIEB2_LEER ||
       Store[oldx][oldy]==EL_AMOEBE_NASS))
  {
    Feld[oldx][oldy] = Store[oldx][oldy];
    Store[oldx][oldy] = Store2[oldx][oldy] = 0;
  }
  else
    Feld[oldx][oldy] = EL_LEERRAUM;

  Feld[newx][newy] = EL_LEERRAUM;
  MovPos[oldx][oldy] = MovDir[oldx][oldy] = MovDelay[oldx][oldy] = 0;
  MovPos[newx][newy] = MovDir[newx][newy] = MovDelay[newx][newy] = 0;

  DrawLevelField(oldx,oldy);
  DrawLevelField(newx,newy);
}

void DrawDynamite(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic = el2gfx(Feld[x][y]);
  int phase;

  if (!IN_SCR_FIELD(sx,sy) || IS_PLAYER(x,y))
    return;

  if (Store[x][y])
    DrawGraphic(sx,sy, el2gfx(Store[x][y]));

  if (Feld[x][y]==EL_DYNAMIT)
  {
    if ((phase = (96-MovDelay[x][y])/12) > 6)
      phase = 6;
  }
  else
  {
    if ((phase = ((96-MovDelay[x][y])/6) % 8) > 3)
      phase = 7-phase;
  }

  if (Store[x][y])
    DrawGraphicThruMask(sx,sy, graphic + phase);
  else
    DrawGraphic(sx,sy, graphic + phase);
}

void CheckDynamite(int x, int y)
{
  if (MovDelay[x][y])		/* neues Dynamit / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (!(MovDelay[x][y] % 12))
	PlaySoundLevel(x,y,SND_ZISCH);

      if (Feld[x][y]==EL_DYNAMIT && !(MovDelay[x][y] % 12))
	DrawDynamite(x,y);
      else if (Feld[x][y]==EL_DYNABOMB && !(MovDelay[x][y] % 6))
	DrawDynamite(x,y);

      return;
    }
  }

  StopSound(SND_ZISCH);
  Bang(x,y);
}

void Explode(int ex, int ey, int phase, int mode)
{
  int x,y;
  int num_phase = 9, delay = 2;
  int last_phase = num_phase*delay;
  int half_phase = (num_phase/2)*delay;

  if (phase==0)			/* Feld 'Store' initialisieren */
  {
    int center_element = Feld[ex][ey];

    if (IS_MOVING(ex,ey) || IS_BLOCKED(ex,ey))
    {
      center_element = MovingOrBlocked2Element(ex,ey);
      RemoveMovingField(ex,ey);
    }

    for(y=ey-1;y<ey+2;y++) for(x=ex-1;x<ex+2;x++)
    {
      int element = Feld[x][y];

      if (IS_MOVING(x,y) || IS_BLOCKED(x,y))
      {
	element = MovingOrBlocked2Element(x,y);
	RemoveMovingField(x,y);
      }

      if (!IN_LEV_FIELD(x,y) || IS_MASSIV(element) || element==EL_BURNING)
	continue;

      if ((mode!=EX_NORMAL || center_element==EL_AMOEBA2DIAM) &&
	  (x!=ex || y!=ey))
	continue;

      if (element==EL_EXPLODING)
	element = Store2[x][y];

      if (IS_PLAYER(ex,ey))
      {
	switch(StorePlayer[ex][ey])
	{
	  case EL_SPIELER2:
	    Store[x][y] = EL_EDELSTEIN_ROT;
	    break;
	  case EL_SPIELER3:
	    Store[x][y] = EL_EDELSTEIN;
	    break;
	  case EL_SPIELER4:
	    Store[x][y] = EL_EDELSTEIN_LILA;
	    break;
	  case EL_SPIELER1:
	  default:
	    Store[x][y] = EL_EDELSTEIN_GELB;
	    break;
	}
      }
      else if (center_element==EL_MAULWURF)
	Store[x][y] = EL_EDELSTEIN_ROT;
      else if (center_element==EL_PINGUIN)
	Store[x][y] = EL_EDELSTEIN_LILA;
      else if (center_element==EL_KAEFER)
	Store[x][y] = ((x==ex && y==ey) ? EL_DIAMANT : EL_EDELSTEIN);
      else if (center_element==EL_BUTTERFLY)
	Store[x][y] = EL_EDELSTEIN_BD;
      else if (center_element==EL_MAMPFER)
	Store[x][y] = level.mampfer_inhalt[MampferNr][x-ex+1][y-ey+1];
      else if (center_element==EL_AMOEBA2DIAM)
	Store[x][y] = level.amoebe_inhalt;
      else if (element==EL_ERZ_EDEL)
	Store[x][y] = EL_EDELSTEIN;
      else if (element==EL_ERZ_DIAM)
	Store[x][y] = EL_DIAMANT;
      else if (element==EL_ERZ_EDEL_BD)
	Store[x][y] = EL_EDELSTEIN_BD;
      else if (element==EL_ERZ_EDEL_GELB)
	Store[x][y] = EL_EDELSTEIN_GELB;
      else if (element==EL_ERZ_EDEL_ROT)
	Store[x][y] = EL_EDELSTEIN_ROT;
      else if (element==EL_ERZ_EDEL_LILA)
	Store[x][y] = EL_EDELSTEIN_LILA;
      else if (!IS_PFORTE(Store[x][y]))
	Store[x][y] = EL_LEERRAUM;

      if (x!=ex || y!=ey || center_element==EL_AMOEBA2DIAM || mode==EX_BORDER)
	Store2[x][y] = element;

      if (AmoebaNr[x][y] &&
	  (element==EL_AMOEBE_VOLL ||
	   element==EL_AMOEBE_BD ||
	   element==EL_AMOEBING))
      {
	AmoebaCnt[AmoebaNr[x][y]]--;
	AmoebaCnt2[AmoebaNr[x][y]]--;
      }

      Feld[x][y] = EL_EXPLODING;
      MovDir[x][y] = MovPos[x][y] = 0;
      AmoebaNr[x][y] = 0;
      Frame[x][y] = 1;
      Stop[x][y] = TRUE;
    }

    if (center_element==EL_MAMPFER)
      MampferNr = (MampferNr+1) % 4;

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

  Frame[x][y] = (phase<last_phase ? phase+1 : 0);

  if (phase==half_phase)
  {
    int element = Store2[x][y];

    if (IS_PLAYER(x,y))
      KillHero(PLAYERINFO(x,y));
    else if (IS_EXPLOSIVE(element))
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x,y);
    }
    else if (element==EL_AMOEBA2DIAM)
      AmoebeUmwandeln(x,y);
  }

  if (phase==last_phase)
  {
    int element;

    element = Feld[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    if (CAN_MOVE(element) || COULD_MOVE(element))
      InitMovDir(x,y);
    DrawLevelField(x,y);
  }
  else if (!(phase%delay) && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
  {
    if (phase==delay)
      ErdreichAnbroeckeln(SCREENX(x),SCREENY(y));

    DrawGraphic(SCREENX(x),SCREENY(y),GFX_EXPLOSION+(phase/delay-1));
  }
}

void DynaExplode(int ex, int ey)
{
  int i,j;
  struct PlayerInfo *player = &stored_player[Store2[ex][ey] - EL_SPIELER1];
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  Store2[ex][ey] = 0;	/* delete player information */

  Explode(ex,ey,0,EX_CENTER);

  for(i=0;i<4;i++)
  {
    for(j=1; j<=player->dynabomb_size; j++)
    {
      int x = ex+j*xy[i%4][0];
      int y = ey+j*xy[i%4][1];
      int element;

      if (!IN_LEV_FIELD(x,y) || IS_MASSIV(Feld[x][y]))
	break;

      element = Feld[x][y];
      Explode(x,y,0,EX_BORDER);

      if (element != EL_LEERRAUM &&
	  element != EL_ERDREICH &&
	  element != EL_EXPLODING &&
	  !player->dynabomb_xl)
	break;
    }
  }

  player->dynabombs_left++;
}

void Bang(int x, int y)
{
  int element = Feld[x][y];

  PlaySoundLevel(x,y,SND_ROAAAR);

  switch(element)
  {
    case EL_KAEFER:
    case EL_FLIEGER:
    case EL_BUTTERFLY:
    case EL_FIREFLY:
    case EL_MAMPFER:
    case EL_MAMPFER2:
    case EL_ROBOT:
    case EL_PACMAN:
      RaiseScoreElement(element);
      Explode(x,y,0,EX_NORMAL);
      break;
    case EL_DYNABOMB:
    case EL_DYNABOMB_NR:
    case EL_DYNABOMB_SZ:
    case EL_DYNABOMB_XL:
      DynaExplode(x,y);
      break;
    case EL_BIRNE_AUS:
    case EL_BIRNE_EIN:
      Explode(x,y,0,EX_CENTER);
      break;
    default:
      Explode(x,y,0,EX_NORMAL);
      break;
  }
}

void Blurb(int x, int y)
{
  int element = Feld[x][y];

  if (element!=EL_BLURB_LEFT && element!=EL_BLURB_RIGHT) /* Anfang */
  {
    PlaySoundLevel(x,y,SND_BLURB);
    if (IN_LEV_FIELD(x-1,y) && IS_FREE(x-1,y) &&
	(!IN_LEV_FIELD(x-1,y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x-1,y-1))))
    {
      Feld[x-1][y] = EL_BLURB_LEFT;
    }
    if (IN_LEV_FIELD(x+1,y) && IS_FREE(x+1,y) &&
	(!IN_LEV_FIELD(x+1,y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x+1,y-1))))
    {
      Feld[x+1][y] = EL_BLURB_RIGHT;
    }
  }
  else							 /* Blubbern */
  {
    int graphic = (element==EL_BLURB_LEFT ? GFX_BLURB_LEFT : GFX_BLURB_RIGHT);

    if (!MovDelay[x][y])	/* neue Phase / noch nicht gewartet */
      MovDelay[x][y] = 9;

    if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
	DrawGraphic(SCREENX(x),SCREENY(y),graphic+4-MovDelay[x][y]/2);

      if (!MovDelay[x][y])
      {
	Feld[x][y] = EL_LEERRAUM;
	DrawLevelField(x,y);
      }
    }
  }
}

void Impact(int x, int y)
{
  boolean lastline = (y==lev_fieldy-1);
  boolean object_hit = FALSE;
  int element = Feld[x][y];
  int smashed = 0;

  /* Element darunter ber�hrt? */
  if (!lastline)
  {
    if (Feld[x][y+1] == EL_PLAYER_IS_LEAVING)
      return;

    object_hit = (!IS_FREE(x,y+1) && (!IS_MOVING(x,y+1) ||
				      MovDir[x][y+1]!=MV_DOWN ||
				      MovPos[x][y+1]<=TILEY/2));
    if (object_hit)
      smashed = MovingOrBlocked2Element(x,y+1);
  }

  /* Auftreffendes Element f�llt in Salzs�ure */
  if (!lastline && smashed==EL_SALZSAEURE)
  {
    Blurb(x,y);
    return;
  }

  /* Auftreffendes Element ist Bombe */
  if (element==EL_BOMBE && (lastline || object_hit))
  {
    Bang(x,y);
    return;
  }

  /* Auftreffendes Element ist S�uretropfen */
  if (element==EL_TROPFEN && (lastline || object_hit))
  {
    if (object_hit && IS_PLAYER(x,y+1))
      KillHero(PLAYERINFO(x,y+1));
    else if (object_hit && (smashed==EL_MAULWURF || smashed==EL_PINGUIN))
      Bang(x,y+1);
    else
    {
      Feld[x][y] = EL_AMOEBING;
      Store[x][y] = EL_AMOEBE_NASS;
    }
    return;
  }

  /* Welches Element kriegt was auf die R�be? */
  if (!lastline && object_hit)
  {
    if (CAN_CHANGE(element) && 
	(smashed==EL_SIEB_LEER || smashed==EL_SIEB2_LEER) && !SiebAktiv)
      SiebAktiv = level.dauer_sieb * FRAMES_PER_SECOND;

    if (IS_PLAYER(x,y+1))
    {
      KillHero(PLAYERINFO(x,y+1));
      return;
    }
    else if (smashed==EL_MAULWURF || smashed==EL_PINGUIN)
    {
      Bang(x,y+1);
      return;
    }
    else if (element==EL_EDELSTEIN_BD)
    {
      if (IS_ENEMY(smashed) && IS_BD_ELEMENT(smashed))
      {
	Bang(x,y+1);
	return;
      }
    }
    else if (element==EL_FELSBROCKEN)
    {
      if (IS_ENEMY(smashed) || smashed==EL_BOMBE || smashed==EL_SONDE ||
	  smashed==EL_SCHWEIN || smashed==EL_DRACHE)
      {
	Bang(x,y+1);
	return;
      }
      else if (!IS_MOVING(x,y+1))
      {
	if (smashed==EL_BIRNE_AUS || smashed==EL_BIRNE_EIN)
	{
	  Bang(x,y+1);
	  return;
	}
	else if (smashed==EL_KOKOSNUSS)
	{
	  Feld[x][y+1] = EL_CRACKINGNUT;
	  PlaySoundLevel(x,y,SND_KNACK);
	  RaiseScoreElement(EL_KOKOSNUSS);
	  return;
	}
	else if (smashed==EL_DIAMANT)
	{
	  Feld[x][y+1] = EL_LEERRAUM;
	  PlaySoundLevel(x,y,SND_QUIRK);
	  return;
	}
      }
    }
  }

  /* Ger�usch beim Durchqueren des Siebes */
  if (!lastline && (Feld[x][y+1]==EL_SIEB_LEER || Feld[x][y+1]==EL_SIEB2_LEER))
  {
    PlaySoundLevel(x,y,SND_QUIRK);
    return;
  }

  /* Ger�usch beim Auftreffen */
  if (lastline || object_hit)
  {
    int sound;

    switch(element)
    {
      case EL_EDELSTEIN:
      case EL_EDELSTEIN_BD:
      case EL_EDELSTEIN_GELB:
      case EL_EDELSTEIN_ROT:
      case EL_EDELSTEIN_LILA:
      case EL_DIAMANT:
        sound = SND_PLING;
	break;
      case EL_KOKOSNUSS:
	sound = SND_KLUMPF;
	break;
      case EL_FELSBROCKEN:
	sound = SND_KLOPF;
	break;
      case EL_SCHLUESSEL:
      case EL_SCHLUESSEL1:
      case EL_SCHLUESSEL2:
      case EL_SCHLUESSEL3:
      case EL_SCHLUESSEL4:
	sound = SND_KINK;
	break;
      case EL_ZEIT_VOLL:
      case EL_ZEIT_LEER:
	sound = SND_DENG;
	break;
      default:
	sound = -1;
        break;
    }

    if (sound>=0)
      PlaySoundLevel(x,y,sound);
  }
}

void TurnRound(int x, int y)
{
  static struct
  {
    int x,y;
  } move_xy[] =
  {
    { 0,0 },
    {-1,0 },
    {+1,0 },
    { 0,0 },
    { 0,-1 },
    { 0,0 }, { 0,0 }, { 0,0 },
    { 0,+1 }
  };
  static struct
  {
    int left,right,back;
  } turn[] =
  {
    { 0,	0,		0 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT },
    { 0,	0,		0 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN },
    { 0,0,0 },	{ 0,0,0 },	{ 0,0,0 },
    { MV_RIGHT,	MV_LEFT,	MV_UP }
  };

  int element = Feld[x][y];
  int old_move_dir = MovDir[x][y];
  int left_dir = turn[old_move_dir].left;
  int right_dir = turn[old_move_dir].right;
  int back_dir = turn[old_move_dir].back;

  int left_dx = move_xy[left_dir].x, left_dy = move_xy[left_dir].y;
  int right_dx = move_xy[right_dir].x, right_dy = move_xy[right_dir].y;
  int move_dx = move_xy[old_move_dir].x, move_dy = move_xy[old_move_dir].y;
  int back_dx = move_xy[back_dir].x, back_dy = move_xy[back_dir].y;

  int left_x = x+left_dx, left_y = y+left_dy;
  int right_x = x+right_dx, right_y = y+right_dy;
  int move_x = x+move_dx, move_y = y+move_dy;

  if (element==EL_KAEFER || element==EL_BUTTERFLY)
  {
    TestIfBadThingHitsOtherBadThing(x,y);

    if (IN_LEV_FIELD(right_x,right_y) &&
	IS_FREE_OR_PLAYER(right_x,right_y))
      MovDir[x][y] = right_dir;
    else if (!IN_LEV_FIELD(move_x,move_y) ||
	     !IS_FREE_OR_PLAYER(move_x,move_y))
      MovDir[x][y] = left_dir;

    if (element==EL_KAEFER && MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element==EL_BUTTERFLY)	/* && MovDir[x][y]==left_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element==EL_FLIEGER || element==EL_FIREFLY)
  {
    TestIfBadThingHitsOtherBadThing(x,y);

    if (IN_LEV_FIELD(left_x,left_y) &&
	IS_FREE_OR_PLAYER(left_x,left_y))
      MovDir[x][y] = left_dir;
    else if (!IN_LEV_FIELD(move_x,move_y) ||
	     !IS_FREE_OR_PLAYER(move_x,move_y))
      MovDir[x][y] = right_dir;

    if (element==EL_FLIEGER && MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element==EL_FIREFLY)	/* && MovDir[x][y]==right_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element==EL_MAMPFER)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x,left_y) &&
	(IS_FREE_OR_PLAYER(left_x,left_y) ||
	 Feld[left_x][left_y] == EL_DIAMANT))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x,right_y) &&
	(IS_FREE_OR_PLAYER(right_x,right_y) ||
	 Feld[right_x][right_y] == EL_DIAMANT))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16+16*RND(3);
  }
  else if (element==EL_MAMPFER2)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x,left_y) &&
	(IS_FREE_OR_PLAYER(left_x,left_y) ||
	 IS_MAMPF2(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x,right_y) &&
	(IS_FREE_OR_PLAYER(right_x,right_y) ||
	 IS_MAMPF2(Feld[right_x][right_y])))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16+16*RND(3);
  }
  else if (element==EL_PACMAN)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x,left_y) &&
	(IS_FREE_OR_PLAYER(left_x,left_y) ||
	 IS_AMOEBOID(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x,right_y) &&
	(IS_FREE_OR_PLAYER(right_x,right_y) ||
	 IS_AMOEBOID(Feld[right_x][right_y])))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 6+RND(40);
  }
  else if (element==EL_SCHWEIN)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;
    boolean should_turn_left = FALSE, should_turn_right = FALSE;
    boolean should_move_on = FALSE;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (IN_LEV_FIELD(left_x,left_y) &&
	(IS_FREE(left_x,left_y) || IS_GEM(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x,right_y) &&
	(IS_FREE(right_x,right_y) || IS_GEM(Feld[right_x][right_y])))
      can_turn_right = TRUE;
    if (IN_LEV_FIELD(move_x,move_y) &&
	(IS_FREE(move_x,move_y) || IS_GEM(Feld[move_x][move_y])))
      can_move_on = TRUE;

    if (can_turn_left &&
	(!can_move_on ||
	 (IN_LEV_FIELD(x+back_dx+left_dx,y+back_dy+left_dy) &&
	  !IS_FREE(x+back_dx+left_dx,y+back_dy+left_dy))))
      should_turn_left = TRUE;
    if (can_turn_right &&
	(!can_move_on ||
	 (IN_LEV_FIELD(x+back_dx+right_dx,y+back_dy+right_dy) &&
	  !IS_FREE(x+back_dx+right_dx,y+back_dy+right_dy))))
      should_turn_right = TRUE;
    if (can_move_on &&
	(!can_turn_left || !can_turn_right ||
	 (IN_LEV_FIELD(x+move_dx+left_dx,y+move_dy+left_dy) &&
	  !IS_FREE(x+move_dx+left_dx,y+move_dy+left_dy)) ||
	 (IN_LEV_FIELD(x+move_dx+right_dx,y+move_dy+right_dy) &&
	  !IS_FREE(x+move_dx+right_dx,y+move_dy+right_dy))))
      should_move_on = TRUE;

    if (should_turn_left || should_turn_right || should_move_on)
    {
      if (should_turn_left && should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/3 ? left_dir :
			rnd < 2*rnd_value/3 ? right_dir :
			old_move_dir);
      else if (should_turn_left && should_turn_right)
	MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
      else if (should_turn_left && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : old_move_dir);
      else if (should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/2 ? right_dir : old_move_dir);
      else if (should_turn_left)
	MovDir[x][y] = left_dir;
      else if (should_turn_right)
	MovDir[x][y] = right_dir;
      else if (should_move_on)
	MovDir[x][y] = old_move_dir;
    }
    else if (can_move_on && rnd > rnd_value/8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value/8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    if (!IS_FREE(x+move_xy[MovDir[x][y]].x,y+move_xy[MovDir[x][y]].y) &&
	!IS_GEM(Feld[x+move_xy[MovDir[x][y]].x][y+move_xy[MovDir[x][y]].y]))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element==EL_DRACHE)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (IN_LEV_FIELD(left_x,left_y) && IS_FREE(left_x,left_y))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x,right_y) && IS_FREE(right_x,right_y))
      can_turn_right = TRUE;
    if (IN_LEV_FIELD(move_x,move_y) && IS_FREE(move_x,move_y))
      can_move_on = TRUE;

    if (can_move_on && rnd > rnd_value/8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value/8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    if (!IS_FREE(x+move_xy[MovDir[x][y]].x,y+move_xy[MovDir[x][y]].y))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element==EL_ROBOT || element==EL_SONDE ||
	   element==EL_MAULWURF || element==EL_PINGUIN)
  {
    int attr_x = -1, attr_y = -1;

    if (AllPlayersGone)
    {
      attr_x = ExitX;
      attr_y = ExitY;
    }
    else
    {
      int i;

      for(i=0; i<MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	if (!player->active || player->gone)
	  continue;

	if (attr_x == -1 || ABS(jx-x)+ABS(jy-y) < ABS(attr_x-x)+ABS(attr_y-y))
	{
	  attr_x = jx;
	  attr_y = jy;
	}
      }
    }

    if (element==EL_ROBOT && ZX>=0 && ZY>=0)
    {
      attr_x = ZX;
      attr_y = ZY;
    }

    if (element==EL_MAULWURF || element==EL_PINGUIN)
    {
      int i;
      static int xy[4][2] =
      {
	{ 0,-1 },
	{ -1,0 },
	{ +1,0 },
	{ 0,+1 }
      };

      for(i=0;i<4;i++)
      {
    	int ex = x + xy[i%4][0];
    	int ey = y + xy[i%4][1];

    	if (IN_LEV_FIELD(ex,ey) && Feld[ex][ey] == EL_AUSGANG_AUF)
	{
	  attr_x = ex;
 	  attr_y = ey;
	  break;
	}
      }
    }

    MovDir[x][y] = MV_NO_MOVING;
    if (attr_x<x)
      MovDir[x][y] |= (AllPlayersGone ? MV_RIGHT : MV_LEFT);
    else if (attr_x>x)
      MovDir[x][y] |= (AllPlayersGone ? MV_LEFT : MV_RIGHT);
    if (attr_y<y)
      MovDir[x][y] |= (AllPlayersGone ? MV_DOWN : MV_UP);
    else if (attr_y>y)
      MovDir[x][y] |= (AllPlayersGone ? MV_UP : MV_DOWN);

    if (element==EL_ROBOT)
    {
      int newx, newy;

      if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
	MovDir[x][y] &= (RND(2) ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
      Moving2Blocked(x,y,&newx,&newy);

      if (IN_LEV_FIELD(newx,newy) && IS_FREE_OR_PLAYER(newx,newy))
	MovDelay[x][y] = 8+8*!RND(3);
      else
	MovDelay[x][y] = 16;
    }
    else
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
	Moving2Blocked(x,y,&newx,&newy);

	if (IN_LEV_FIELD(newx,newy) &&
	    (IS_FREE(newx,newy) ||
	     Feld[newx][newy] == EL_SALZSAEURE ||
	     ((element == EL_MAULWURF || element==EL_PINGUIN) &&
	      (Feld[newx][newy] == EL_AUSGANG_AUF ||
	       IS_MAMPF3(Feld[newx][newy])))))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
	Moving2Blocked(x,y,&newx,&newy);

	if (IN_LEV_FIELD(newx,newy) &&
	    (IS_FREE(newx,newy) ||
	     Feld[newx][newy] == EL_SALZSAEURE ||
	     ((element == EL_MAULWURF || element==EL_PINGUIN) &&
	      (Feld[newx][newy] == EL_AUSGANG_AUF ||
	       IS_MAMPF3(Feld[newx][newy])))))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
  }
}

static boolean JustBeingPushed(int x, int y)
{
  int i;

  for(i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->active && !player->gone &&
	player->Pushing && player->MovPos)
    {
      int next_jx = player->jx + (player->jx - player->last_jx);
      int next_jy = player->jy + (player->jy - player->last_jy);

      if (x == next_jx && y == next_jy)
	return(TRUE);
    }
  }

  return(FALSE);
}

void StartMoving(int x, int y)
{
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

  if (CAN_FALL(element) && y<lev_fieldy-1)
  {
    if ((x>0 && IS_PLAYER(x-1,y)) || (x<lev_fieldx-1 && IS_PLAYER(x+1,y)))
      if (JustBeingPushed(x,y))
	return;

    if (element==EL_MORAST_VOLL)
    {
      if (IS_FREE(x,y+1))
      {
	InitMovingField(x,y,MV_DOWN);
	Feld[x][y] = EL_FELSBROCKEN;
	Store[x][y] = EL_MORAST_LEER;
      }
      else if (Feld[x][y+1]==EL_MORAST_LEER)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_MORAST_LEER;
	Feld[x][y+1] = EL_MORAST_VOLL;
      }
    }
    else if (element==EL_FELSBROCKEN && Feld[x][y+1]==EL_MORAST_LEER)
    {
      InitMovingField(x,y,MV_DOWN);
      Store[x][y] = EL_MORAST_VOLL;
    }
    else if (element==EL_SIEB_VOLL)
    {
      if (IS_FREE(x,y+1))
      {
	InitMovingField(x,y,MV_DOWN);
	Feld[x][y] = EL_CHANGED(Store2[x][y]);
	Store[x][y] = EL_SIEB_LEER;
      }
      else if (Feld[x][y+1]==EL_SIEB_LEER)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY/4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_SIEB_LEER;
	Feld[x][y+1] = EL_SIEB_VOLL;
	Store2[x][y+1] = EL_CHANGED(Store2[x][y]);
	Store2[x][y] = 0;
      }
    }
    else if (element==EL_SIEB2_VOLL)
    {
      if (IS_FREE(x,y+1))
      {
	InitMovingField(x,y,MV_DOWN);
	Feld[x][y] = EL_CHANGED2(Store2[x][y]);
	Store[x][y] = EL_SIEB2_LEER;
      }
      else if (Feld[x][y+1]==EL_SIEB2_LEER)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY/4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_SIEB2_LEER;
	Feld[x][y+1] = EL_SIEB2_VOLL;
	Store2[x][y+1] = EL_CHANGED2(Store2[x][y]);
	Store2[x][y] = 0;
      }
    }
    else if (SiebAktiv && CAN_CHANGE(element) &&
	     (Feld[x][y+1]==EL_SIEB_LEER || Feld[x][y+1]==EL_SIEB2_LEER))
    {
      InitMovingField(x,y,MV_DOWN);
      Store[x][y] =
	(Feld[x][y+1]==EL_SIEB_LEER ? EL_SIEB_VOLL : EL_SIEB2_VOLL);
      Store2[x][y+1] = element;
    }
    else if (CAN_SMASH(element) && Feld[x][y+1]==EL_SALZSAEURE)
    {
      Blurb(x,y);
      InitMovingField(x,y,MV_DOWN);
      Store[x][y] = EL_SALZSAEURE;
    }
    else if (CAN_SMASH(element) && Feld[x][y+1]==EL_BLOCKED && JustHit[x][y])
    {
      Impact(x,y);
    }
    else if (IS_FREE(x,y+1))
    {
      InitMovingField(x,y,MV_DOWN);
    }
    else if (element==EL_TROPFEN)
    {
      Feld[x][y] = EL_AMOEBING;
      Store[x][y] = EL_AMOEBE_NASS;
    }
    else if (IS_SLIPPERY(Feld[x][y+1]) && !Store[x][y+1])
    {
      boolean left  = (x>0 && IS_FREE(x-1,y) &&
		       (IS_FREE(x-1,y+1) || Feld[x-1][y+1]==EL_SALZSAEURE));
      boolean right = (x<lev_fieldx-1 && IS_FREE(x+1,y) &&
		       (IS_FREE(x+1,y+1) || Feld[x+1][y+1]==EL_SALZSAEURE));

      if (left || right)
      {
	if (left && right && game_emulation != EMU_BOULDERDASH)
	  left = !(right = RND(2));

	InitMovingField(x,y,left ? MV_LEFT : MV_RIGHT);
      }
    }
  }
  else if (CAN_MOVE(element))
  {
    int newx,newy;

    if (element == EL_SONDE && JustBeingPushed(x,y))
      return;

    if (!MovDelay[x][y])	/* neuer Schritt / noch nicht gewartet */
    {
      /* Alle Figuren, die nach jeden Schritt die Richtung wechseln k�nnen.
       * (MAMPFER, MAMPFER2 und PACMAN laufen bis zur n�chsten Wand.)
       */

      if (element!=EL_MAMPFER && element!=EL_MAMPFER2 && element!=EL_PACMAN)
      {
	TurnRound(x,y);
	if (MovDelay[x][y] && (element == EL_KAEFER || element == EL_FLIEGER))
	  DrawLevelField(x,y);
      }
    }

    if (MovDelay[x][y])		/* neuer Schritt / in Wartezustand */
    {
      MovDelay[x][y]--;

      if (element==EL_ROBOT || element==EL_MAMPFER || element==EL_MAMPFER2)
      {
	int phase = MovDelay[x][y] % 8;

	if (phase>3)
	  phase = 7-phase;

	if (IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
	  DrawGraphic(SCREENX(x),SCREENY(y), el2gfx(element)+phase);

	if ((element==EL_MAMPFER || element==EL_MAMPFER2)
	    && MovDelay[x][y]%4==3)
	  PlaySoundLevel(x,y,SND_NJAM);
      }
      else if (element==EL_DRACHE)
      {
	int i;
	int dir = MovDir[x][y];
	int dx = (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
	int dy = (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);
	int graphic = (dir == MV_LEFT	? GFX_FLAMMEN_LEFT :
		       dir == MV_RIGHT	? GFX_FLAMMEN_RIGHT :
		       dir == MV_UP	? GFX_FLAMMEN_UP :
		       dir == MV_DOWN	? GFX_FLAMMEN_DOWN : GFX_LEERRAUM);
	int phase = FrameCounter % 2;

	for(i=1;i<=3;i++)
	{
	  int xx = x + i*dx, yy = y + i*dy;
	  int sx = SCREENX(xx), sy = SCREENY(yy);

	  if (!IN_LEV_FIELD(xx,yy) ||
	      IS_SOLID(Feld[xx][yy]) || Feld[xx][yy]==EL_EXPLODING)
	    break;

	  if (MovDelay[x][y])
	  {
	    int flamed = MovingOrBlocked2Element(xx,yy);

	    if (IS_ENEMY(flamed) || IS_EXPLOSIVE(flamed))
	      Bang(xx,yy);
	    else
	      RemoveMovingField(xx,yy);

	    Feld[xx][yy] = EL_BURNING;
	    if (IN_SCR_FIELD(sx,sy))
	      DrawGraphic(sx,sy, graphic + phase*3 + i-1);
	  }
	  else
	  {
	    if (Feld[xx][yy] == EL_BURNING)
	      Feld[xx][yy] = EL_LEERRAUM;
	    DrawLevelField(xx,yy);
	  }
	}
      }

      if (MovDelay[x][y])
	return;
    }

    if (element==EL_KAEFER || element==EL_BUTTERFLY)
    {
      PlaySoundLevel(x,y,SND_KLAPPER);
    }
    else if (element==EL_FLIEGER || element==EL_FIREFLY)
    {
      PlaySoundLevel(x,y,SND_ROEHR);
    }

    /* neuer Schritt / Wartezustand beendet */

    Moving2Blocked(x,y,&newx,&newy);	/* wohin soll's gehen? */

    if (IS_ENEMY(element) && IS_PLAYER(newx,newy))
    {
      /* Spieler erwischt */
      MovDir[x][y] = 0;
      KillHero(PLAYERINFO(newx,newy));
      return;
    }
    else if ((element == EL_MAULWURF || element == EL_PINGUIN ||
	      element==EL_ROBOT || element==EL_SONDE) &&
	     IN_LEV_FIELD(newx,newy) &&
	     MovDir[x][y]==MV_DOWN && Feld[newx][newy]==EL_SALZSAEURE)
    {
      Blurb(x,y);
      Store[x][y] = EL_SALZSAEURE;
    }
    else if ((element == EL_MAULWURF || element == EL_PINGUIN) &&
	     IN_LEV_FIELD(newx,newy))
    {
      if (Feld[newx][newy] == EL_AUSGANG_AUF)
      {
	Feld[x][y] = EL_LEERRAUM;
	DrawLevelField(x,y);

	PlaySoundLevel(newx,newy,SND_BUING);
	if (IN_SCR_FIELD(SCREENX(newx),SCREENY(newy)))
	  DrawGraphicThruMask(SCREENX(newx),SCREENY(newy),el2gfx(element));

	local_player->friends_still_needed--;
	if (!local_player->friends_still_needed &&
	    !local_player->GameOver && AllPlayersGone)
	  local_player->LevelSolved = local_player->GameOver = TRUE;

	return;
      }
      else if (IS_MAMPF3(Feld[newx][newy]))
      {
	if (DigField(local_player, newx,newy, 0,0, DF_DIG) == MF_MOVING)
	  DrawLevelField(newx,newy);
	else
	  MovDir[x][y] = MV_NO_MOVING;
      }
      else if (!IS_FREE(newx,newy))
      {
	if (IS_PLAYER(x,y))
	  DrawPlayerField(x,y);
	else
	  DrawLevelField(x,y);
	return;
      }
    }
    else if (element == EL_SCHWEIN && IN_LEV_FIELD(newx,newy))
    {
      if (IS_GEM(Feld[newx][newy]))
      {
	if (IS_MOVING(newx,newy))
	  RemoveMovingField(newx,newy);
	else
	{
	  Feld[newx][newy] = EL_LEERRAUM;
	  DrawLevelField(newx,newy);
	}
      }
      else if (!IS_FREE(newx,newy))
      {
	if (IS_PLAYER(x,y))
	  DrawPlayerField(x,y);
	else
	  DrawLevelField(x,y);
	return;
      }
    }
    else if (element==EL_DRACHE && IN_LEV_FIELD(newx,newy))
    {
      if (!IS_FREE(newx,newy))
      {
	if (IS_PLAYER(x,y))
	  DrawPlayerField(x,y);
	else
	  DrawLevelField(x,y);
	return;
      }
      else
      {
	boolean wanna_flame = !RND(10);
	int dx = newx - x, dy = newy - y;
	int newx1 = newx+1*dx, newy1 = newy+1*dy;
	int newx2 = newx+2*dx, newy2 = newy+2*dy;
	int element1 = (IN_LEV_FIELD(newx1,newy1) ?
			MovingOrBlocked2Element(newx1,newy1) : EL_BETON);
	int element2 = (IN_LEV_FIELD(newx2,newy2) ?
			MovingOrBlocked2Element(newx2,newy2) : EL_BETON);

	if ((wanna_flame || IS_ENEMY(element1) || IS_ENEMY(element2)) &&
	    element1 != EL_DRACHE && element2 != EL_DRACHE &&
	    element1 != EL_BURNING && element2 != EL_BURNING)
	{
	  if (IS_PLAYER(x,y))
	    DrawPlayerField(x,y);
	  else
	    DrawLevelField(x,y);

	  MovDelay[x][y] = 50;
	  Feld[newx][newy] = EL_BURNING;
	  if (IN_LEV_FIELD(newx1,newy1) && Feld[newx1][newy1] == EL_LEERRAUM)
	    Feld[newx1][newy1] = EL_BURNING;
	  if (IN_LEV_FIELD(newx2,newy2) && Feld[newx2][newy2] == EL_LEERRAUM)
	    Feld[newx2][newy2] = EL_BURNING;
	  return;
	}
      }
    }
    else if (element==EL_MAMPFER && IN_LEV_FIELD(newx,newy) &&
	     Feld[newx][newy]==EL_DIAMANT)
    {
      if (IS_MOVING(newx,newy))
	RemoveMovingField(newx,newy);
      else
      {
	Feld[newx][newy] = EL_LEERRAUM;
	DrawLevelField(newx,newy);
      }
    }
    else if (element==EL_MAMPFER2 && IN_LEV_FIELD(newx,newy) &&
	     IS_MAMPF2(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Feld[newx][newy]==EL_AMOEBE_VOLL || Feld[newx][newy]==EL_AMOEBE_BD)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (IS_MOVING(newx,newy))
	RemoveMovingField(newx,newy);
      else
      {
	Feld[newx][newy] = EL_LEERRAUM;
	DrawLevelField(newx,newy);
      }
    }
    else if (element==EL_PACMAN && IN_LEV_FIELD(newx,newy) &&
	     IS_AMOEBOID(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Feld[newx][newy]==EL_AMOEBE_VOLL || Feld[newx][newy]==EL_AMOEBE_BD)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      Feld[newx][newy] = EL_LEERRAUM;
      DrawLevelField(newx,newy);
    }
    else if (!IN_LEV_FIELD(newx,newy) || !IS_FREE(newx,newy))
    {					/* gegen Wand gelaufen */
      TurnRound(x,y);

      if (element == EL_KAEFER || element == EL_FLIEGER)
	DrawLevelField(x,y);
      else if (element == EL_BUTTERFLY || element == EL_FIREFLY)
	DrawGraphicAnimation(x,y, el2gfx(element), 2, 4, ANIM_NORMAL);
      else if (element==EL_SONDE)
	DrawGraphicAnimation(x,y, GFX_SONDE_START, 8, 2, ANIM_NORMAL);

      return;
    }

    if (element==EL_ROBOT && IN_SCR_FIELD(x,y))
      PlaySoundLevel(x,y,SND_SCHLURF);

    InitMovingField(x,y,MovDir[x][y]);
  }

  if (MovDir[x][y])
    ContinueMoving(x,y);
}

void ContinueMoving(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];
  int dx = (direction==MV_LEFT ? -1 : direction==MV_RIGHT ? +1 : 0);
  int dy = (direction==MV_UP   ? -1 : direction==MV_DOWN  ? +1 : 0);
  int horiz_move = (dx!=0);
  int newx = x + dx, newy = y + dy;
  int step = (horiz_move ? dx : dy) * TILEX/8;

  if (CAN_FALL(element) && horiz_move)
    step*=2;
  else if (element==EL_TROPFEN)
    step/=2;
  else if (Store[x][y]==EL_MORAST_VOLL || Store[x][y]==EL_MORAST_LEER)
    step/=4;

  MovPos[x][y] += step;

  if (ABS(MovPos[x][y])>=TILEX)		/* Zielfeld erreicht */
  {
    Feld[x][y] = EL_LEERRAUM;
    Feld[newx][newy] = element;

    if (Store[x][y]==EL_MORAST_VOLL)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_MORAST_VOLL;
      element = EL_MORAST_VOLL;
    }
    else if (Store[x][y]==EL_MORAST_LEER)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_MORAST_LEER;
    }
    else if (Store[x][y]==EL_SIEB_VOLL)
    {
      Store[x][y] = 0;
      element = Feld[newx][newy] = (SiebAktiv ? EL_SIEB_VOLL : EL_SIEB_TOT);
    }
    else if (Store[x][y]==EL_SIEB_LEER)
    {
      Store[x][y] = Store2[x][y] = 0;
      Feld[x][y] = (SiebAktiv ? EL_SIEB_LEER : EL_SIEB_TOT);
    }
    else if (Store[x][y]==EL_SIEB2_VOLL)
    {
      Store[x][y] = 0;
      element = Feld[newx][newy] = (SiebAktiv ? EL_SIEB2_VOLL : EL_SIEB2_TOT);
    }
    else if (Store[x][y]==EL_SIEB2_LEER)
    {
      Store[x][y] = Store2[x][y] = 0;
      Feld[x][y] = (SiebAktiv ? EL_SIEB2_LEER : EL_SIEB2_TOT);
    }
    else if (Store[x][y]==EL_SALZSAEURE)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_SALZSAEURE;
      element = EL_SALZSAEURE;
    }
    else if (Store[x][y]==EL_AMOEBE_NASS)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_AMOEBE_NASS;
    }

    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    MovDelay[newx][newy] = 0;

    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;

    DrawLevelField(x,y);
    DrawLevelField(newx,newy);

    Stop[newx][newy] = TRUE;
    JustHit[x][newy] = 3;

    if (DONT_TOUCH(element))	/* K�fer oder Flieger */
    {
      TestIfBadThingHitsHero(newx,newy);
      TestIfBadThingHitsFriend(newx,newy);
      TestIfBadThingHitsOtherBadThing(newx,newy);
    }
    else if (element == EL_PINGUIN)
      TestIfFriendHitsBadThing(newx,newy);

    if (CAN_SMASH(element) && direction==MV_DOWN &&
	(newy==lev_fieldy-1 || !IS_FREE(x,newy+1)))
      Impact(x,newy);
  }
  else				/* noch in Bewegung */
    DrawLevelField(x,y);
}

int AmoebeNachbarNr(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int group_nr = 0;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  for(i=0;i<4;i++)
  {
    int x = ax+xy[i%4][0];
    int y = ay+xy[i%4][1];

    if (!IN_LEV_FIELD(x,y))
      continue;

    if (Feld[x][y]==element && AmoebaNr[x][y]>0)
      group_nr = AmoebaNr[x][y];
  }

  return(group_nr);
}

void AmoebenVereinigen(int ax, int ay)
{
  int i,x,y,xx,yy;
  int new_group_nr = AmoebaNr[ax][ay];
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  if (!new_group_nr)
    return;

  for(i=0;i<4;i++)
  {
    x = ax+xy[i%4][0];
    y = ay+xy[i%4][1];

    if (!IN_LEV_FIELD(x,y))
      continue;

    if ((Feld[x][y]==EL_AMOEBE_VOLL ||
	 Feld[x][y]==EL_AMOEBE_BD ||
	 Feld[x][y]==EL_AMOEBE_TOT) &&
	AmoebaNr[x][y] != new_group_nr)
    {
      int old_group_nr = AmoebaNr[x][y];

      AmoebaCnt[new_group_nr] += AmoebaCnt[old_group_nr];
      AmoebaCnt[old_group_nr] = 0;
      AmoebaCnt2[new_group_nr] += AmoebaCnt2[old_group_nr];
      AmoebaCnt2[old_group_nr] = 0;

      for(yy=0;yy<lev_fieldy;yy++) for(xx=0;xx<lev_fieldx;xx++)
	if (AmoebaNr[xx][yy]==old_group_nr)
	  AmoebaNr[xx][yy] = new_group_nr;
    }
  }
}

void AmoebeUmwandeln(int ax, int ay)
{
  int i,x,y;
  int group_nr = AmoebaNr[ax][ay];
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  if (Feld[ax][ay]==EL_AMOEBE_TOT)
  {
    for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
    {
      if (Feld[x][y]==EL_AMOEBE_TOT && AmoebaNr[x][y]==group_nr)
      {
	AmoebaNr[x][y] = 0;
	Feld[x][y] = EL_AMOEBA2DIAM;
      }
    }
    Bang(ax,ay);
  }
  else
  {
    for(i=0;i<4;i++)
    {
      x = ax+xy[i%4][0];
      y = ay+xy[i%4][1];

      if (!IN_LEV_FIELD(x,y))
	continue;

      if (Feld[x][y]==EL_AMOEBA2DIAM)
	Bang(x,y);
    }
  }
}

void AmoebeUmwandeln2(int ax, int ay, int new_element)
{
  int x,y;
  int group_nr = AmoebaNr[ax][ay];
  boolean done = FALSE;

  for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
  {
    if (AmoebaNr[x][y]==group_nr &&
	(Feld[x][y]==EL_AMOEBE_TOT ||
	 Feld[x][y]==EL_AMOEBE_BD ||
	 Feld[x][y]==EL_AMOEBING))
    {
      AmoebaNr[x][y] = 0;
      Feld[x][y] = new_element;
      DrawLevelField(x,y);
      done = TRUE;
    }
  }

  if (done)
    PlaySoundLevel(ax,ay,new_element==EL_FELSBROCKEN ? SND_KLOPF : SND_PLING);
}

void AmoebeWaechst(int x, int y)
{
  static long sound_delay = 0;
  static int sound_delay_value = 0;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay, sound_delay_value))
    {
      PlaySoundLevel(x,y,SND_AMOEBE);
      sound_delay_value = 30;
    }
  }

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
      DrawGraphic(SCREENX(x),SCREENY(y),GFX_AMOEBING+3-MovDelay[x][y]/2);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      DrawLevelField(x,y);
    }
  }
}

void AmoebeAbleger(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int newax = ax, neway = ay;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  if (!level.tempo_amoebe)
  {
    Feld[ax][ay] = EL_AMOEBE_TOT;
    DrawLevelField(ax,ay);
    return;
  }

  if (!MovDelay[ax][ay])	/* neue Amoebe / noch nicht gewartet */
    MovDelay[ax][ay] = RND(FRAMES_PER_SECOND * 25/(1+level.tempo_amoebe));

  if (MovDelay[ax][ay])		/* neue Amoebe / in Wartezustand */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (element==EL_AMOEBE_NASS)	/* tropfende Am�be */
  {
    int start = RND(4);
    int x = ax+xy[start][0];
    int y = ay+xy[start][1];

    if (!IN_LEV_FIELD(x,y))
      return;

    if (IS_FREE(x,y) ||
	Feld[x][y]==EL_ERDREICH || Feld[x][y]==EL_MORAST_LEER)
    {
      newax = x;
      neway = y;
    }

    if (newax==ax && neway==ay)
      return;
  }
  else				/* normale oder "gef�llte" Am�be */
  {
    int start = RND(4);
    boolean waiting_for_player = FALSE;

    for(i=0;i<4;i++)
    {
      int j = (start+i)%4;
      int x = ax+xy[j][0];
      int y = ay+xy[j][1];

      if (!IN_LEV_FIELD(x,y))
	continue;

      if (IS_FREE(x,y) ||
	  Feld[x][y]==EL_ERDREICH || Feld[x][y]==EL_MORAST_LEER)
      {
	newax = x;
	neway = y;
	break;
      }
      else if (IS_PLAYER(x,y))
	waiting_for_player = TRUE;
    }

    if (newax==ax && neway==ay)
    {
      if (i==4 && !waiting_for_player)
      {
	Feld[ax][ay] = EL_AMOEBE_TOT;
	DrawLevelField(ax,ay);
	AmoebaCnt[AmoebaNr[ax][ay]]--;

	if (AmoebaCnt[AmoebaNr[ax][ay]]<=0)	/* Am�be vollst�ndig tot */
	{
	  if (element==EL_AMOEBE_VOLL)
	    AmoebeUmwandeln(ax,ay);
	  else if (element==EL_AMOEBE_BD)
	    AmoebeUmwandeln2(ax,ay,level.amoebe_inhalt);
	}
      }
      return;
    }
    else if (element==EL_AMOEBE_VOLL || element==EL_AMOEBE_BD)
    {
      int new_group_nr = AmoebaNr[ax][ay];

      AmoebaNr[newax][neway] = new_group_nr;
      AmoebaCnt[new_group_nr]++;
      AmoebaCnt2[new_group_nr]++;
      AmoebenVereinigen(newax,neway);

      if (AmoebaCnt2[new_group_nr] >= 200 && element==EL_AMOEBE_BD)
      {
	AmoebeUmwandeln2(newax,neway,EL_FELSBROCKEN);
	return;
      }
    }
  }

  if (element!=EL_AMOEBE_NASS || neway<ay || !IS_FREE(newax,neway) ||
      (neway==lev_fieldy-1 && newax!=ax))
  {
    Feld[newax][neway] = EL_AMOEBING;
    Store[newax][neway] = element;
  }
  else if (neway==ay)
    Feld[newax][neway] = EL_TROPFEN;
  else
  {
    InitMovingField(ax,ay,MV_DOWN);
    Feld[ax][ay] = EL_TROPFEN;
    Store[ax][ay] = EL_AMOEBE_NASS;
    ContinueMoving(ax,ay);
    return;
  }

  DrawLevelField(newax,neway);
}

void Life(int ax, int ay)
{
  int x1,y1,x2,y2;
  static int life[4] = { 2,3,3,3 };	/* "Life"-Parameter */
  int life_time = 40;
  int element = Feld[ax][ay];

  if (Stop[ax][ay])
    return;

  if (!MovDelay[ax][ay])	/* neue Phase / noch nicht gewartet */
    MovDelay[ax][ay] = life_time;

  if (MovDelay[ax][ay])		/* neue Phase / in Wartezustand */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  for(y1=-1;y1<2;y1++) for(x1=-1;x1<2;x1++)
  {
    int xx = ax+x1, yy = ay+y1;
    int nachbarn = 0;

    if (!IN_LEV_FIELD(xx,yy))
      continue;

    for(y2=-1;y2<2;y2++) for(x2=-1;x2<2;x2++)
    {
      int x = xx+x2, y = yy+y2;

      if (!IN_LEV_FIELD(x,y) || (x==xx && y==yy))
	continue;

      if (((Feld[x][y]==element || (element==EL_LIFE && IS_PLAYER(x,y))) &&
	   !Stop[x][y]) ||
	  (IS_FREE(x,y) && Stop[x][y]))
	nachbarn++;
    }

    if (xx==ax && yy==ay)		/* mittleres Feld mit Amoebe */
    {
      if (nachbarn<life[0] || nachbarn>life[1])
      {
	Feld[xx][yy] = EL_LEERRAUM;
	if (!Stop[xx][yy])
	  DrawLevelField(xx,yy);
	Stop[xx][yy] = TRUE;
      }
    }
    else if (IS_FREE(xx,yy) || Feld[xx][yy]==EL_ERDREICH)
    {					/* Randfeld ohne Amoebe */
      if (nachbarn>=life[2] && nachbarn<=life[3])
      {
	Feld[xx][yy] = element;
	MovDelay[xx][yy] = (element==EL_LIFE ? 0 : life_time-1);
	if (!Stop[xx][yy])
	  DrawLevelField(xx,yy);
	Stop[xx][yy] = TRUE;
      }
    }
  }
}

void Ablenk(int x, int y)
{
  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = level.dauer_ablenk * FRAMES_PER_SECOND;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
	DrawGraphic(SCREENX(x),SCREENY(y),GFX_ABLENK+MovDelay[x][y]%4);
      if (!(MovDelay[x][y]%4))
	PlaySoundLevel(x,y,SND_MIEP);
      return;
    }
  }

  Feld[x][y] = EL_ABLENK_AUS;
  DrawLevelField(x,y);
  if (ZX==x && ZY==y)
    ZX = ZY = -1;
}

void Birne(int x, int y)
{
  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 800;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (!(MovDelay[x][y]%5))
      {
	if (!(MovDelay[x][y]%10))
	  Feld[x][y]=EL_ABLENK_EIN;
	else
	  Feld[x][y]=EL_ABLENK_AUS;
	DrawLevelField(x,y);
	Feld[x][y]=EL_ABLENK_EIN;
      }
      return;
    }
  }

  Feld[x][y]=EL_ABLENK_AUS;
  DrawLevelField(x,y);
  if (ZX==x && ZY==y)
    ZX=ZY=-1;
}

void Blubber(int x, int y)
{
  if (y > 0 && IS_MOVING(x,y-1) && MovDir[x][y-1] == MV_DOWN)
    DrawLevelField(x,y-1);
  else
    DrawGraphicAnimation(x,y, GFX_GEBLUBBER, 4, 10, ANIM_NORMAL);
}

void NussKnacken(int x, int y)
{
  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 7;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
      DrawGraphic(SCREENX(x),SCREENY(y),GFX_CRACKINGNUT+3-MovDelay[x][y]/2);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_EDELSTEIN;
      DrawLevelField(x,y);
    }
  }
}

void SiebAktivieren(int x, int y, int typ)
{
  if (!(SiebAktiv % 4) && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
    DrawGraphic(SCREENX(x),SCREENY(y),
		(typ==1 ? GFX_SIEB_VOLL : GFX_SIEB2_VOLL)+3-(SiebAktiv%16)/4);
}

void AusgangstuerPruefen(int x, int y)
{
  if (!local_player->gems_still_needed &&
      !local_player->sokobanfields_still_needed &&
      !local_player->lights_still_needed)
  {
    Feld[x][y] = EL_AUSGANG_ACT;

    PlaySoundLevel(x < LEVELX(BX1) ? LEVELX(BX1) :
		   (x > LEVELX(BX2) ? LEVELX(BX2) : x),
		   y < LEVELY(BY1) ? LEVELY(BY1) :
		   (y > LEVELY(BY2) ? LEVELY(BY2) : y),
		   SND_OEFFNEN);
  }
}

void AusgangstuerOeffnen(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 5*delay;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    int tuer;

    MovDelay[x][y]--;
    tuer = MovDelay[x][y]/delay;
    if (!(MovDelay[x][y]%delay) && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
      DrawGraphic(SCREENX(x),SCREENY(y),GFX_AUSGANG_AUF-tuer);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_AUSGANG_AUF;
      DrawLevelField(x,y);
    }
  }
}

void AusgangstuerBlinken(int x, int y)
{
  DrawGraphicAnimation(x,y, GFX_AUSGANG_AUF, 4, 4, ANIM_OSCILLATE);
}

void EdelsteinFunkeln(int x, int y)
{
  if (!IN_SCR_FIELD(SCREENX(x),SCREENY(y)) || IS_MOVING(x,y))
    return;

  if (Feld[x][y] == EL_EDELSTEIN_BD)
    DrawGraphicAnimation(x,y, GFX_EDELSTEIN_BD, 4, 4, ANIM_REVERSE);
  else
  {
    if (!MovDelay[x][y])	/* neue Phase / noch nicht gewartet */
      MovDelay[x][y] = 11 * !SimpleRND(500);

    if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
    {
      MovDelay[x][y]--;

      if (setup.direct_draw_on && MovDelay[x][y])
	SetDrawtoField(DRAW_BUFFERED);

      DrawGraphic(SCREENX(x),SCREENY(y), el2gfx(Feld[x][y]));

      if (MovDelay[x][y])
      {
	int phase = (MovDelay[x][y]-1)/2;

	if (phase > 2)
	  phase = 4-phase;

	DrawGraphicThruMask(SCREENX(x),SCREENY(y), GFX_FUNKELN_WEISS + phase);

	if (setup.direct_draw_on)
	{
	  int dest_x,dest_y;

	  dest_x = FX + SCREENX(x)*TILEX;
	  dest_y = FY + SCREENY(y)*TILEY;

	  XCopyArea(display,drawto_field,window,gc,
		    dest_x,dest_y, TILEX,TILEY, dest_x,dest_y);
	  SetDrawtoField(DRAW_DIRECT);
	}
      }
    }
  }
}

void MauerWaechst(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 3*delay;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    int phase;

    MovDelay[x][y]--;
    phase = 2-MovDelay[x][y]/delay;
    if (!(MovDelay[x][y]%delay) && IN_SCR_FIELD(SCREENX(x),SCREENY(y)))
      DrawGraphic(SCREENX(x),SCREENY(y),
		  (MovDir[x][y] == MV_LEFT  ? GFX_MAUER_LEFT  :
		   MovDir[x][y] == MV_RIGHT ? GFX_MAUER_RIGHT :
		   MovDir[x][y] == MV_UP    ? GFX_MAUER_UP    :
		                              GFX_MAUER_DOWN  ) + phase);

    if (!MovDelay[x][y])
    {
      if (MovDir[x][y] == MV_LEFT)
      {
	if (IN_LEV_FIELD(x-1,y) && IS_MAUER(Feld[x-1][y]))
	  DrawLevelField(x-1,y);
      }
      else if (MovDir[x][y] == MV_RIGHT)
      {
	if (IN_LEV_FIELD(x+1,y) && IS_MAUER(Feld[x+1][y]))
	  DrawLevelField(x+1,y);
      }
      else if (MovDir[x][y] == MV_UP)
      {
	if (IN_LEV_FIELD(x,y-1) && IS_MAUER(Feld[x][y-1]))
	  DrawLevelField(x,y-1);
      }
      else
      {
	if (IN_LEV_FIELD(x,y+1) && IS_MAUER(Feld[x][y+1]))
	  DrawLevelField(x,y+1);
      }

      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      MovDir[x][y] = MV_NO_MOVING;
      DrawLevelField(x,y);
    }
  }
}

void MauerAbleger(int ax, int ay)
{
  int element = Feld[ax][ay];
  boolean oben_frei = FALSE, unten_frei = FALSE;
  boolean links_frei = FALSE, rechts_frei = FALSE;
  boolean oben_massiv = FALSE, unten_massiv = FALSE;
  boolean links_massiv = FALSE, rechts_massiv = FALSE;

  if (!MovDelay[ax][ay])	/* neue Mauer / noch nicht gewartet */
    MovDelay[ax][ay] = 6;

  if (MovDelay[ax][ay])		/* neue Mauer / in Wartezustand */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (IN_LEV_FIELD(ax,ay-1) && IS_FREE(ax,ay-1))
    oben_frei = TRUE;
  if (IN_LEV_FIELD(ax,ay+1) && IS_FREE(ax,ay+1))
    unten_frei = TRUE;
  if (IN_LEV_FIELD(ax-1,ay) && IS_FREE(ax-1,ay))
    links_frei = TRUE;
  if (IN_LEV_FIELD(ax+1,ay) && IS_FREE(ax+1,ay))
    rechts_frei = TRUE;

  if (element == EL_MAUER_Y || element == EL_MAUER_XY)
  {
    if (oben_frei)
    {
      Feld[ax][ay-1] = EL_MAUERND;
      Store[ax][ay-1] = element;
      MovDir[ax][ay-1] = MV_UP;
      if (IN_SCR_FIELD(SCREENX(ax),SCREENY(ay-1)))
  	DrawGraphic(SCREENX(ax),SCREENY(ay-1),GFX_MAUER_UP);
    }
    if (unten_frei)
    {
      Feld[ax][ay+1] = EL_MAUERND;
      Store[ax][ay+1] = element;
      MovDir[ax][ay+1] = MV_DOWN;
      if (IN_SCR_FIELD(SCREENX(ax),SCREENY(ay+1)))
  	DrawGraphic(SCREENX(ax),SCREENY(ay+1),GFX_MAUER_DOWN);
    }
  }

  if (element == EL_MAUER_X || element == EL_MAUER_XY ||
      element == EL_MAUER_LEBT)
  {
    if (links_frei)
    {
      Feld[ax-1][ay] = EL_MAUERND;
      Store[ax-1][ay] = element;
      MovDir[ax-1][ay] = MV_LEFT;
      if (IN_SCR_FIELD(SCREENX(ax-1),SCREENY(ay)))
  	DrawGraphic(SCREENX(ax-1),SCREENY(ay),GFX_MAUER_LEFT);
    }
    if (rechts_frei)
    {
      Feld[ax+1][ay] = EL_MAUERND;
      Store[ax+1][ay] = element;
      MovDir[ax+1][ay] = MV_RIGHT;
      if (IN_SCR_FIELD(SCREENX(ax+1),SCREENY(ay)))
  	DrawGraphic(SCREENX(ax+1),SCREENY(ay),GFX_MAUER_RIGHT);
    }
  }

  if (element == EL_MAUER_LEBT && (links_frei || rechts_frei))
    DrawLevelField(ax,ay);

  if (!IN_LEV_FIELD(ax,ay-1) || IS_MAUER(Feld[ax][ay-1]))
    oben_massiv = TRUE;
  if (!IN_LEV_FIELD(ax,ay+1) || IS_MAUER(Feld[ax][ay+1]))
    unten_massiv = TRUE;
  if (!IN_LEV_FIELD(ax-1,ay) || IS_MAUER(Feld[ax-1][ay]))
    links_massiv = TRUE;
  if (!IN_LEV_FIELD(ax+1,ay) || IS_MAUER(Feld[ax+1][ay]))
    rechts_massiv = TRUE;

  if (((oben_massiv && unten_massiv) ||
       element == EL_MAUER_X || element == EL_MAUER_LEBT) &&
      ((links_massiv && rechts_massiv) ||
       element == EL_MAUER_Y))
    Feld[ax][ay] = EL_MAUERWERK;
}

void CheckForDragon(int x, int y)
{
  int i,j;
  boolean dragon_found = FALSE;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  for(i=0;i<4;i++)
  {
    for(j=0;j<4;j++)
    {
      int xx = x + j*xy[i][0], yy = y + j*xy[i][1];

      if (IN_LEV_FIELD(xx,yy) &&
	  (Feld[xx][yy] == EL_BURNING || Feld[xx][yy] == EL_DRACHE))
      {
	if (Feld[xx][yy] == EL_DRACHE)
	  dragon_found = TRUE;
      }
      else
	break;
    }
  }

  if (!dragon_found)
  {
    for(i=0;i<4;i++)
    {
      for(j=0;j<3;j++)
      {
  	int xx = x + j*xy[i][0], yy = y + j*xy[i][1];
  
  	if (IN_LEV_FIELD(xx,yy) && Feld[xx][yy] == EL_BURNING)
  	{
	  Feld[xx][yy] = EL_LEERRAUM;
	  DrawLevelField(xx,yy);
  	}
  	else
  	  break;
      }
    }
  }
}

void PlayerActions(struct PlayerInfo *player, byte player_action)
{
  static byte stored_player_action[MAX_PLAYERS];
  static int num_stored_actions = 0;
  boolean moved = FALSE, snapped = FALSE, bombed = FALSE;
  int jx = player->jx, jy = player->jy;
  int left	= player_action & JOY_LEFT;
  int right	= player_action & JOY_RIGHT;
  int up	= player_action & JOY_UP;
  int down	= player_action & JOY_DOWN;
  int button1	= player_action & JOY_BUTTON_1;
  int button2	= player_action & JOY_BUTTON_2;
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);

  stored_player_action[player->index_nr] = 0;
  num_stored_actions++;

  if (!player->active || player->gone)
    return;

  if (player_action)
  {
    player->frame_reset_delay = 0;

    if (button1)
      snapped = SnapField(player, dx,dy);
    else
    {
      if (button2)
	bombed = PlaceBomb(player);
      moved = MoveFigure(player, dx,dy);
    }

    if (tape.recording && (moved || snapped || bombed))
    {
      if (bombed && !moved)
	player_action &= JOY_BUTTON;

      stored_player_action[player->index_nr] = player_action;

      /* this allows cycled sequences of PlayerActions() */
      if (num_stored_actions >= MAX_PLAYERS)
      {
	TapeRecordAction(stored_player_action);
	num_stored_actions = 0;
      }
    }
    else if (tape.playing && snapped)
      SnapField(player, 0,0);			/* stop snapping */
  }
  else
  {
    DigField(player, 0,0, 0,0, DF_NO_PUSH);
    SnapField(player, 0,0);
    if (++player->frame_reset_delay > MoveSpeed)
      player->Frame = 0;
  }

  if (tape.playing && !tape.pausing && !player_action &&
      tape.counter < tape.length)
  {
    int next_joy =
      tape.pos[tape.counter].action[player->index_nr] & (JOY_LEFT|JOY_RIGHT);

    if (next_joy == JOY_LEFT || next_joy == JOY_RIGHT)
    {
      int dx = (next_joy == JOY_LEFT ? -1 : +1);

      if (IN_LEV_FIELD(jx+dx,jy) && IS_PUSHABLE(Feld[jx+dx][jy]))
      {
	int el = Feld[jx+dx][jy];
	int push_delay = (IS_SB_ELEMENT(el) || el==EL_SONDE ? 2 : 10);

	if (tape.delay_played + push_delay >= tape.pos[tape.counter].delay)
	{
	  player->MovDir = next_joy;
	  player->Frame = FrameCounter % 4;
	  player->Pushing = TRUE;
	}
      }
    }
  }
}

void GameActions(byte player_action)
{
  static long action_delay = 0;
  long action_delay_value;
  int sieb_x = 0, sieb_y = 0;
  int i, x,y, element;
  int *recorded_player_action;

  if (game_status != PLAYING)
    return;

#ifdef DEBUG
  action_delay_value =
    (tape.playing && tape.fast_forward ? FFWD_FRAME_DELAY : GameFrameDelay);
#else
  action_delay_value =
    (tape.playing && tape.fast_forward ? FFWD_FRAME_DELAY : GAME_FRAME_DELAY);
#endif

  /* main game synchronization point */
  WaitUntilDelayReached(&action_delay, action_delay_value);

  if (network_playing && !network_player_action_received)
  {
    /*
#ifdef DEBUG
    printf("DEBUG: try to get network player actions in time\n");
#endif
    */

    /* last chance to get network player actions without main loop delay */
    HandleNetworking();

    if (game_status != PLAYING)
      return;

    if (!network_player_action_received)
    {
      /*
#ifdef DEBUG
      printf("DEBUG: failed to get network player actions in time\n");
#endif
      */
      return;
    }
  }


  if (tape.pausing || (tape.playing && !TapePlayDelay()))
    return;
  else if (tape.recording)
    TapeRecordDelay();


  if (tape.playing)
    recorded_player_action = TapePlayAction();
  else
    recorded_player_action = NULL;

  if (network_playing)
    SendToServer_MovePlayer(player_action);

  for(i=0; i<MAX_PLAYERS; i++)
  {
    /*
    int actual_player_action =
      (options.network ? network_player_action[i] : player_action);
      */

    int actual_player_action =
      (network_playing ? network_player_action[i] : player_action);

    /*
    int actual_player_action = network_player_action[i];
    */

    /*
    int actual_player_action = player_action;
    */

    /* TEST TEST TEST */

    /*
    if (i != TestPlayer && !stored_player[i].MovPos)
      actual_player_action = 0;
    */

    if (!options.network && i != TestPlayer)
      actual_player_action = 0;

    /* TEST TEST TEST */

    if (recorded_player_action)
      actual_player_action = recorded_player_action[i];

    PlayerActions(&stored_player[i], actual_player_action);
    ScrollFigure(&stored_player[i], SCROLL_GO_ON);

    network_player_action[i] = 0;
  }

  network_player_action_received = FALSE;

  ScrollScreen(NULL, SCROLL_GO_ON);

  /*
  if (tape.pausing || (tape.playing && !TapePlayDelay()))
    return;
  else if (tape.recording)
    TapeRecordDelay();
  */

  FrameCounter++;
  TimeFrames++;


  /*
  printf("FrameCounter == %d, RND(100) == %d\n", FrameCounter, RND(100));
  */


  for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
  {
    Stop[x][y] = FALSE;
    if (JustHit[x][y]>0)
      JustHit[x][y]--;

#if DEBUG
    if (IS_BLOCKED(x,y))
    {
      int oldx,oldy;

      Blocked2Moving(x,y,&oldx,&oldy);
      if (!IS_MOVING(oldx,oldy))
      {
	printf("GameActions(): (BLOCKED=>MOVING) context corrupted!\n");
	printf("GameActions(): BLOCKED: x = %d, y = %d\n",x,y);
	printf("GameActions(): !MOVING: oldx = %d, oldy = %d\n",oldx,oldy);
	printf("GameActions(): This should never happen!\n");
      }
    }
#endif
  }

  for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
  {
    element = Feld[x][y];

    if (IS_INACTIVE(element))
      continue;

    if (!IS_MOVING(x,y) && (CAN_FALL(element) || CAN_MOVE(element)))
    {
      StartMoving(x,y);

      if (IS_GEM(element))
	EdelsteinFunkeln(x,y);
    }
    else if (IS_MOVING(x,y))
      ContinueMoving(x,y);
    else if (element==EL_DYNAMIT || element==EL_DYNABOMB)
      CheckDynamite(x,y);
    else if (element==EL_EXPLODING)
      Explode(x,y,Frame[x][y],EX_NORMAL);
    else if (element==EL_AMOEBING)
      AmoebeWaechst(x,y);
    else if (IS_AMOEBALIVE(element))
      AmoebeAbleger(x,y);
    else if (element==EL_LIFE || element==EL_LIFE_ASYNC)
      Life(x,y);
    else if (element==EL_ABLENK_EIN)
      Ablenk(x,y);
    else if (element==EL_SALZSAEURE)
      Blubber(x,y);
    else if (element==EL_BLURB_LEFT || element==EL_BLURB_RIGHT)
      Blurb(x,y);
    else if (element==EL_CRACKINGNUT)
      NussKnacken(x,y);
    else if (element==EL_AUSGANG_ZU)
      AusgangstuerPruefen(x,y);
    else if (element==EL_AUSGANG_ACT)
      AusgangstuerOeffnen(x,y);
    else if (element==EL_AUSGANG_AUF)
      AusgangstuerBlinken(x,y);
    else if (element==EL_MAUERND)
      MauerWaechst(x,y);
    else if (element==EL_MAUER_LEBT ||
	     element==EL_MAUER_X ||
	     element==EL_MAUER_Y ||
	     element==EL_MAUER_XY)
      MauerAbleger(x,y);
    else if (element==EL_BURNING)
      CheckForDragon(x,y);

    if (SiebAktiv)
    {
      boolean sieb = FALSE;
      int jx = local_player->jx, jy = local_player->jy;

      if (element==EL_SIEB_LEER || element==EL_SIEB_VOLL ||
	  Store[x][y]==EL_SIEB_LEER)
      {
	SiebAktivieren(x, y, 1);
	sieb = TRUE;
      }
      else if (element==EL_SIEB2_LEER || element==EL_SIEB2_VOLL ||
	       Store[x][y]==EL_SIEB2_LEER)
      {
	SiebAktivieren(x, y, 2);
	sieb = TRUE;
      }

      /* play the element sound at the position nearest to the player */
      if (sieb && ABS(x-jx)+ABS(y-jy) < ABS(sieb_x-jx)+ABS(sieb_y-jy))
      {
	sieb_x = x;
	sieb_y = y;
      }
    }
  }

  if (SiebAktiv)
  {
    if (!(SiebAktiv%4))
      PlaySoundLevel(sieb_x,sieb_y,SND_MIEP);
    SiebAktiv--;
    if (!SiebAktiv)
    {
      for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
      {
	element = Feld[x][y];
	if (element==EL_SIEB_LEER || element==EL_SIEB_VOLL)
	{
	  Feld[x][y] = EL_SIEB_TOT;
	  DrawLevelField(x,y);
	}
	else if (element==EL_SIEB2_LEER || element==EL_SIEB2_VOLL)
	{
	  Feld[x][y] = EL_SIEB2_TOT;
	  DrawLevelField(x,y);
	}
      }
    }
  }

  if (TimeLeft>0 && TimeFrames>=(1000/GameFrameDelay) && !tape.pausing)
  {
    TimeFrames = 0;
    TimeLeft--;

    if (tape.recording || tape.playing)
      DrawVideoDisplay(VIDEO_STATE_TIME_ON,level.time-TimeLeft);

    if (TimeLeft<=10)
      PlaySoundStereo(SND_GONG,PSND_MAX_RIGHT);

    DrawText(DX_TIME,DY_TIME,int2str(TimeLeft,3),FS_SMALL,FC_YELLOW);

    if (!TimeLeft)
      for(i=0; i<MAX_PLAYERS; i++)
	KillHero(&stored_player[i]);
  }

  DrawAllPlayers();
}

static boolean AllPlayersInSight(struct PlayerInfo *player, int x, int y)
{
  int min_x = x, min_y = y, max_x = x, max_y = y;
  int i;

  for(i=0; i<MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active || stored_player[i].gone ||
	&stored_player[i] == player)
      continue;

    min_x = MIN(min_x, jx);
    min_y = MIN(min_y, jy);
    max_x = MAX(max_x, jx);
    max_y = MAX(max_y, jy);
  }

  return(max_x - min_x < SCR_FIELDX && max_y - min_y < SCR_FIELDY);
}

static boolean AllPlayersInVisibleScreen()
{
  int i;

  for(i=0; i<MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active || stored_player[i].gone)
      continue;

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
      return(FALSE);
  }

  return(TRUE);
}

void ScrollLevel(int dx, int dy)
{
  int softscroll_offset = (setup.soft_scrolling_on ? TILEX : 0);
  int x,y;

  /*
  ScreenGfxPos = local_player->GfxPos;
  */

  XCopyArea(display,drawto_field,drawto_field,gc,
	    FX + TILEX*(dx==-1) - softscroll_offset,
	    FY + TILEY*(dy==-1) - softscroll_offset,
	    SXSIZE - TILEX*(dx!=0) + 2*softscroll_offset,
	    SYSIZE - TILEY*(dy!=0) + 2*softscroll_offset,
	    FX + TILEX*(dx==1) - softscroll_offset,
	    FY + TILEY*(dy==1) - softscroll_offset);

  if (dx)
  {
    x = (dx==1 ? BX1 : BX2);
    for(y=BY1; y<=BY2; y++)
      DrawScreenField(x,y);
  }
  if (dy)
  {
    y = (dy==1 ? BY1 : BY2);
    for(x=BX1; x<=BX2; x++)
      DrawScreenField(x,y);
  }

  redraw_mask |= REDRAW_FIELD;
}

boolean MoveFigureOneStep(struct PlayerInfo *player,
			  int dx, int dy, int real_dx, int real_dy)
{
  int jx = player->jx, jy = player->jy;
  int new_jx = jx+dx, new_jy = jy+dy;
  int element;
  int can_move;

  if (player->gone || (!dx && !dy))
    return(MF_NO_ACTION);

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NO_MOVING);

  if (!IN_LEV_FIELD(new_jx,new_jy))
    return(MF_NO_ACTION);

  if (!options.network && !AllPlayersInSight(player, new_jx,new_jy))
    return(MF_NO_ACTION);

  element = MovingOrBlocked2Element(new_jx,new_jy);

  if (DONT_GO_TO(element))
  {
    if (element==EL_SALZSAEURE && dx==0 && dy==1)
    {
      Blurb(jx,jy);
      Feld[jx][jy] = EL_SPIELFIGUR;
      InitMovingField(jx,jy,MV_DOWN);
      Store[jx][jy] = EL_SALZSAEURE;
      ContinueMoving(jx,jy);
      BuryHero(player);
    }
    else
      KillHero(player);

    return(MF_MOVING);
  }

  can_move = DigField(player, new_jx,new_jy, real_dx,real_dy, DF_DIG);
  if (can_move != MF_MOVING)
    return(can_move);

  StorePlayer[jx][jy] = 0;
  player->last_jx = jx;
  player->last_jy = jy;
  jx = player->jx = new_jx;
  jy = player->jy = new_jy;
  StorePlayer[jx][jy] = player->element_nr;

  player->MovPos = (dx > 0 || dy > 0 ? -1 : 1) * 7*TILEX/8;

  ScrollFigure(player, SCROLL_INIT);

  return(MF_MOVING);
}

boolean MoveFigure(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int old_jx = jx, old_jy = jy;
  int moved = MF_NO_ACTION;

  if (player->gone || (!dx && !dy))
    return(FALSE);

  if (!FrameReached(&player->move_delay,MoveSpeed) && !tape.playing)
    return(FALSE);

  if (player->last_move_dir & (MV_LEFT | MV_RIGHT))
  {
    if (!(moved |= MoveFigureOneStep(player, 0,dy, dx,dy)))
      moved |= MoveFigureOneStep(player, dx,0, dx,dy);
  }
  else
  {
    if (!(moved |= MoveFigureOneStep(player, dx,0, dx,dy)))
      moved |= MoveFigureOneStep(player, 0,dy, dx,dy);
  }

  jx = player->jx;
  jy = player->jy;



  /*
  if (moved & MF_MOVING && player == local_player)
  */

  if (moved & MF_MOVING && !ScreenMovPos &&
      (player == local_player || !options.network))
  {
    int old_scroll_x = scroll_x, old_scroll_y = scroll_y;
    int offset = (setup.scroll_delay_on ? 3 : 0);

    /*
    if (player == local_player)
    {
      printf("MOVING LOCAL PLAYER && SCROLLING\n");
    }
    */

    if (!IN_VIS_FIELD(SCREENX(jx),SCREENY(jy)))
    {
      /* actual player has left the screen -- scroll in that direction */
      if (jx != old_jx)		/* player has moved horizontally */
	scroll_x += (jx - old_jx);
      else			/* player has moved vertically */
	scroll_y += (jy - old_jy);
    }
    else
    {
      if (jx != old_jx)		/* player has moved horizontally */
      {
	if ((scroll_x < jx-MIDPOSX-offset || scroll_x > jx-MIDPOSX+offset) &&
	    jx >= MIDPOSX-1-offset && jx <= lev_fieldx-(MIDPOSX-offset))
	  scroll_x = jx-MIDPOSX + (scroll_x < jx-MIDPOSX ? -offset : offset);

	/* don't scroll more than one field at a time */
	scroll_x = old_scroll_x + SIGN(scroll_x - old_scroll_x);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_LEFT && scroll_x > old_scroll_x) ||
	    (player->MovDir == MV_RIGHT && scroll_x < old_scroll_x))
	  scroll_x = old_scroll_x;
      }
      else			/* player has moved vertically */
      {
	if ((scroll_y < jy-MIDPOSY-offset || scroll_y > jy-MIDPOSY+offset) &&
	    jy >= MIDPOSY-1-offset && jy <= lev_fieldy-(MIDPOSY-offset))
	  scroll_y = jy-MIDPOSY + (scroll_y < jy-MIDPOSY ? -offset : offset);

	/* don't scroll more than one field at a time */
	scroll_y = old_scroll_y + SIGN(scroll_y - old_scroll_y);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_UP && scroll_y > old_scroll_y) ||
	    (player->MovDir == MV_DOWN && scroll_y < old_scroll_y))
	  scroll_y = old_scroll_y;
      }
    }

#if 0
    if (player == local_player)
    {
      if ((scroll_x < jx-MIDPOSX-offset || scroll_x > jx-MIDPOSX+offset) &&
	  jx >= MIDPOSX-1-offset && jx <= lev_fieldx-(MIDPOSX-offset))
	scroll_x = jx-MIDPOSX + (scroll_x < jx-MIDPOSX ? -offset : offset);
      if ((scroll_y < jy-MIDPOSY-offset || scroll_y > jy-MIDPOSY+offset) &&
	  jy >= MIDPOSY-1-offset && jy <= lev_fieldy-(MIDPOSY-offset))
	scroll_y = jy-MIDPOSY + (scroll_y < jy-MIDPOSY ? -offset : offset);

      /* don't scroll more than one field at a time */
      scroll_x = old_scroll_x + SIGN(scroll_x - old_scroll_x);
      scroll_y = old_scroll_y + SIGN(scroll_y - old_scroll_y);
    }
#endif

    if (scroll_x != old_scroll_x || scroll_y != old_scroll_y)
    {
      if (!options.network && !AllPlayersInVisibleScreen())
      {
	scroll_x = old_scroll_x;
	scroll_y = old_scroll_y;
      }
      else
      {
	ScrollScreen(player, SCROLL_INIT);
	ScrollLevel(old_scroll_x - scroll_x, old_scroll_y - scroll_y);
      }
    }
  }

  if (!(moved & MF_MOVING) && !player->Pushing)
    player->Frame = 0;
  else
    player->Frame = (player->Frame + 1) % 4;

  if (moved & MF_MOVING)
  {
    if (old_jx != jx && old_jy == jy)
      player->MovDir = (old_jx < jx ? MV_RIGHT : MV_LEFT);
    else if (old_jx == jx && old_jy != jy)
      player->MovDir = (old_jy < jy ? MV_DOWN : MV_UP);

    DrawLevelField(jx,jy);	/* f�r "ErdreichAnbroeckeln()" */

    player->last_move_dir = player->MovDir;
  }
  else
    player->last_move_dir = MV_NO_MOVING;

  TestIfHeroHitsBadThing(jx,jy);

  if (player->gone)
    RemoveHero(player);

  return(moved);
}

void ScrollFigure(struct PlayerInfo *player, int mode)
{
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;

  if (!player->active || player->gone || !player->MovPos)
    return;

  if (mode == SCROLL_INIT)
  {
    player->actual_frame_counter = FrameCounter;
    player->GfxPos = ScrollStepSize * (player->MovPos / ScrollStepSize);

    if (Feld[last_jx][last_jy] == EL_LEERRAUM)
      Feld[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;

    DrawPlayer(player);
    return;
  }
  else if (!FrameReached(&player->actual_frame_counter,1))
    return;

  player->MovPos += (player->MovPos > 0 ? -1 : 1) * TILEX/8;
  player->GfxPos = ScrollStepSize * (player->MovPos / ScrollStepSize);

  if (Feld[last_jx][last_jy] == EL_PLAYER_IS_LEAVING)
    Feld[last_jx][last_jy] = EL_LEERRAUM;

  DrawPlayer(player);

  if (!player->MovPos)
  {
    player->last_jx = jx;
    player->last_jy = jy;

    if (Feld[jx][jy] == EL_AUSGANG_AUF)
    {
      RemoveHero(player);

      if (!local_player->friends_still_needed)
	player->LevelSolved = player->GameOver = TRUE;
    }
  }
}

void ScrollScreen(struct PlayerInfo *player, int mode)
{
  static long screen_frame_counter = 0;

  if (mode == SCROLL_INIT)
  {
    screen_frame_counter = FrameCounter;
    ScreenMovDir = player->MovDir;
    ScreenMovPos = player->MovPos;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    return;
  }
  else if (!FrameReached(&screen_frame_counter,1))
    return;

  if (ScreenMovPos)
  {
    ScreenMovPos += (ScreenMovPos > 0 ? -1 : 1) * TILEX/8;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    redraw_mask |= REDRAW_FIELD;
  }
  else
    ScreenMovDir = MV_NO_MOVING;
}

void TestIfGoodThingHitsBadThing(int goodx, int goody)
{
  int i, killx = goodx, killy = goody;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };
  static int harmless[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  for(i=0; i<4; i++)
  {
    int x,y, element;

    x = goodx + xy[i][0];
    y = goody + xy[i][1];
    if (!IN_LEV_FIELD(x,y))
      continue;

    element = Feld[x][y];

    if (DONT_TOUCH(element))
    {
      if (MovDir[x][y] == harmless[i])
	continue;

      killx = x;
      killy = y;
      break;
    }
  }

  if (killx != goodx || killy != goody)
  {
    if (IS_PLAYER(goodx,goody))
      KillHero(PLAYERINFO(goodx,goody));
    else
      Bang(goodx,goody);
  }
}

void TestIfBadThingHitsGoodThing(int badx, int bady)
{
  int i, killx = badx, killy = bady;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };
  static int harmless[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  for(i=0; i<4; i++)
  {
    int x,y, element;

    x = badx + xy[i][0];
    y = bady + xy[i][1];
    if (!IN_LEV_FIELD(x,y))
      continue;

    element = Feld[x][y];

    if (IS_PLAYER(x,y))
    {
      killx = x;
      killy = y;
      break;
    }
    else if (element == EL_PINGUIN)
    {
      if (MovDir[x][y] == harmless[i] && IS_MOVING(x,y))
	continue;

      killx = x;
      killy = y;
      break;
    }
  }

  if (killx != badx || killy != bady)
  {
    if (IS_PLAYER(killx,killy))
      KillHero(PLAYERINFO(killx,killy));
    else
      Bang(killx,killy);
  }
}

void TestIfHeroHitsBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x,y);
}

void TestIfBadThingHitsHero(int x, int y)
{
  /*
  TestIfGoodThingHitsBadThing(JX,JY);
  */

  TestIfBadThingHitsGoodThing(x,y);
}

void TestIfFriendHitsBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x,y);
}

void TestIfBadThingHitsFriend(int x, int y)
{
  TestIfBadThingHitsGoodThing(x,y);
}

void TestIfBadThingHitsOtherBadThing(int badx, int bady)
{
  int i, killx = badx, killy = bady;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  for(i=0; i<4; i++)
  {
    int x,y, element;

    x=badx + xy[i][0];
    y=bady + xy[i][1];
    if (!IN_LEV_FIELD(x,y))
      continue;

    element = Feld[x][y];
    if (IS_AMOEBOID(element) || element == EL_LIFE ||
	element == EL_AMOEBING || element == EL_TROPFEN)
    {
      killx = x;
      killy = y;
      break;
    }
  }

  if (killx != badx || killy != bady)
    Bang(badx,bady);
}

void KillHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (player->gone)
    return;

  if (IS_PFORTE(Feld[jx][jy]))
    Feld[jx][jy] = EL_LEERRAUM;

  Bang(jx,jy);
  BuryHero(player);
}

void BuryHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (player->gone)
    return;

  PlaySoundLevel(jx,jy, SND_AUTSCH);
  PlaySoundLevel(jx,jy, SND_LACHEN);

  player->GameOver = TRUE;
  RemoveHero(player);
}

void RemoveHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int i, found = FALSE;

  player->gone = TRUE;
  StorePlayer[jx][jy] = 0;

  for(i=0; i<MAX_PLAYERS; i++)
    if (stored_player[i].active && !stored_player[i].gone)
      found = TRUE;

  if (!found)
    AllPlayersGone = TRUE;

  ExitX = ZX = jx;
  ExitY = ZY = jy;
}

int DigField(struct PlayerInfo *player,
	     int x, int y, int real_dx, int real_dy, int mode)
{
  int jx = player->jx, jy = player->jy;
  int dx = x - jx, dy = y - jy;
  int element;

  if (!player->MovPos)
    player->Pushing = FALSE;

  if (mode == DF_NO_PUSH)
  {
    player->push_delay = 0;
    return(MF_NO_ACTION);
  }

  if (IS_MOVING(x,y) || IS_PLAYER(x,y))
    return(MF_NO_ACTION);

  element = Feld[x][y];

  switch(element)
  {
    case EL_LEERRAUM:
      break;

    case EL_ERDREICH:
      Feld[x][y] = EL_LEERRAUM;
      break;

    case EL_EDELSTEIN:
    case EL_EDELSTEIN_BD:
    case EL_EDELSTEIN_GELB:
    case EL_EDELSTEIN_ROT:
    case EL_EDELSTEIN_LILA:
    case EL_DIAMANT:
      RemoveField(x,y);
      local_player->gems_still_needed -= (element == EL_DIAMANT ? 3 : 1);
      if (local_player->gems_still_needed < 0)
	local_player->gems_still_needed = 0;
      RaiseScoreElement(element);
      DrawText(DX_EMERALDS, DY_EMERALDS,
	       int2str(local_player->gems_still_needed, 3),
	       FS_SMALL, FC_YELLOW);
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_DYNAMIT_AUS:
      RemoveField(x,y);
      player->dynamite++;
      RaiseScoreElement(EL_DYNAMIT);
      DrawText(DX_DYNAMITE, DY_DYNAMITE,
	       int2str(local_player->dynamite, 3),
	       FS_SMALL, FC_YELLOW);
      PlaySoundLevel(x,y,SND_PONG);
      break;

    case EL_DYNABOMB_NR:
      RemoveField(x,y);
      player->dynabomb_count++;
      player->dynabombs_left++;
      RaiseScoreElement(EL_DYNAMIT);
      PlaySoundLevel(x,y,SND_PONG);
      break;

    case EL_DYNABOMB_SZ:
      RemoveField(x,y);
      player->dynabomb_size++;
      RaiseScoreElement(EL_DYNAMIT);
      PlaySoundLevel(x,y,SND_PONG);
      break;

    case EL_DYNABOMB_XL:
      RemoveField(x,y);
      player->dynabomb_xl = TRUE;
      RaiseScoreElement(EL_DYNAMIT);
      PlaySoundLevel(x,y,SND_PONG);
      break;

    case EL_SCHLUESSEL1:
    case EL_SCHLUESSEL2:
    case EL_SCHLUESSEL3:
    case EL_SCHLUESSEL4:
    {
      int key_nr = element-EL_SCHLUESSEL1;

      RemoveField(x,y);
      player->key[key_nr] = TRUE;
      RaiseScoreElement(EL_SCHLUESSEL);
      DrawMiniGraphicExt(drawto,gc,
			 DX_KEYS+key_nr*MINI_TILEX,DY_KEYS,
			 GFX_SCHLUESSEL1+key_nr);
      DrawMiniGraphicExt(window,gc,
			 DX_KEYS+key_nr*MINI_TILEX,DY_KEYS,
			 GFX_SCHLUESSEL1+key_nr);
      PlaySoundLevel(x,y,SND_PONG);
      break;
    }

    case EL_ABLENK_AUS:
      Feld[x][y] = EL_ABLENK_EIN;
      ZX = x;
      ZY = y;
      DrawLevelField(x,y);
      return(MF_ACTION);
      break;

    case EL_FELSBROCKEN:
    case EL_BOMBE:
    case EL_KOKOSNUSS:
    case EL_ZEIT_LEER:
      if (dy || mode==DF_SNAP)
	return(MF_NO_ACTION);

      player->Pushing = TRUE;

      if (!IN_LEV_FIELD(x+dx,y+dy) || !IS_FREE(x+dx,y+dy))
	return(MF_NO_ACTION);

      if (real_dy)
      {
	if (IN_LEV_FIELD(jx,jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	  return(MF_NO_ACTION);
      }

      if (player->push_delay == 0)
	player->push_delay = FrameCounter;
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !tape.playing)
	return(MF_NO_ACTION);

      RemoveField(x,y);
      Feld[x+dx][y+dy] = element;

      player->push_delay_value = 2+RND(8);

      DrawLevelField(x+dx,y+dy);
      if (element==EL_FELSBROCKEN)
	PlaySoundLevel(x+dx,y+dy,SND_PUSCH);
      else if (element==EL_KOKOSNUSS)
	PlaySoundLevel(x+dx,y+dy,SND_KNURK);
      else
	PlaySoundLevel(x+dx,y+dy,SND_KLOPF);
      break;

    case EL_PFORTE1:
    case EL_PFORTE2:
    case EL_PFORTE3:
    case EL_PFORTE4:
      if (!player->key[element-EL_PFORTE1])
	return(MF_NO_ACTION);
      break;

    case EL_PFORTE1X:
    case EL_PFORTE2X:
    case EL_PFORTE3X:
    case EL_PFORTE4X:
      if (!player->key[element-EL_PFORTE1X])
	return(MF_NO_ACTION);
      break;

    case EL_AUSGANG_ZU:
    case EL_AUSGANG_ACT:
      /* T�r ist (noch) nicht offen! */
      return(MF_NO_ACTION);
      break;

    case EL_AUSGANG_AUF:
      if (mode==DF_SNAP)
	return(MF_NO_ACTION);

      PlaySoundLevel(x,y,SND_BUING);

      /*
      player->gone = TRUE;
      PlaySoundLevel(x,y,SND_BUING);

      if (!local_player->friends_still_needed)
	player->LevelSolved = player->GameOver = TRUE;
      */

      break;

    case EL_BIRNE_AUS:
      Feld[x][y] = EL_BIRNE_EIN;
      local_player->lights_still_needed--;
      DrawLevelField(x,y);
      PlaySoundLevel(x,y,SND_DENG);
      return(MF_ACTION);
      break;

    case EL_ZEIT_VOLL:
      Feld[x][y] = EL_ZEIT_LEER;
      TimeLeft += 10;
      DrawText(DX_TIME,DY_TIME,int2str(TimeLeft,3),FS_SMALL,FC_YELLOW);
      DrawLevelField(x,y);
      PlaySoundStereo(SND_GONG,PSND_MAX_RIGHT);
      return(MF_ACTION);
      break;

    case EL_SOKOBAN_FELD_LEER:
      break;

    case EL_SOKOBAN_FELD_VOLL:
    case EL_SOKOBAN_OBJEKT:
    case EL_SONDE:
      if (mode==DF_SNAP)
	return(MF_NO_ACTION);

      player->Pushing = TRUE;

      if (!IN_LEV_FIELD(x+dx,y+dy)
	  || (!IS_FREE(x+dx,y+dy)
	      && (Feld[x+dx][y+dy] != EL_SOKOBAN_FELD_LEER
		  || !IS_SB_ELEMENT(element))))
	return(MF_NO_ACTION);

      if (dx && real_dy)
      {
	if (IN_LEV_FIELD(jx,jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	  return(MF_NO_ACTION);
      }
      else if (dy && real_dx)
      {
	if (IN_LEV_FIELD(jx+real_dx,jy) && !IS_SOLID(Feld[jx+real_dx][jy]))
	  return(MF_NO_ACTION);
      }

      if (player->push_delay == 0)
	player->push_delay = FrameCounter;
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !tape.playing)
	return(MF_NO_ACTION);

      if (IS_SB_ELEMENT(element))
      {
	if (element == EL_SOKOBAN_FELD_VOLL)
	{
	  Feld[x][y] = EL_SOKOBAN_FELD_LEER;
	  local_player->sokobanfields_still_needed++;
	}
	else
	  RemoveField(x,y);

	if (Feld[x+dx][y+dy] == EL_SOKOBAN_FELD_LEER)
	{
	  Feld[x+dx][y+dy] = EL_SOKOBAN_FELD_VOLL;
	  local_player->sokobanfields_still_needed--;
	  if (element == EL_SOKOBAN_OBJEKT)
	    PlaySoundLevel(x,y,SND_DENG);
	}
	else
	  Feld[x+dx][y+dy] = EL_SOKOBAN_OBJEKT;
      }
      else
      {
	RemoveField(x,y);
	Feld[x+dx][y+dy] = element;
      }

      player->push_delay_value = 2;

      DrawLevelField(x,y);
      DrawLevelField(x+dx,y+dy);
      PlaySoundLevel(x+dx,y+dy,SND_PUSCH);

      if (IS_SB_ELEMENT(element) &&
	  local_player->sokobanfields_still_needed == 0 &&
	  game_emulation == EMU_SOKOBAN)
      {
	player->LevelSolved = player->GameOver = TRUE;
	PlaySoundLevel(x,y,SND_BUING);
      }

      break;

    case EL_MAULWURF:
    case EL_PINGUIN:
    case EL_SCHWEIN:
    case EL_DRACHE:
      break;

    default:
      return(MF_NO_ACTION);
      break;
  }

  player->push_delay = 0;

  return(MF_MOVING);
}

boolean SnapField(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int x = jx + dx, y = jy + dy;

  if (player->gone || !IN_LEV_FIELD(x,y))
    return(FALSE);

  if (dx && dy)
    return(FALSE);

  if (!dx && !dy)
  {
    player->snapped = FALSE;
    return(FALSE);
  }

  if (player->snapped)
    return(FALSE);

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NO_MOVING);

  if (!DigField(player, x,y, 0,0, DF_SNAP))
    return(FALSE);

  player->snapped = TRUE;
  DrawLevelField(x,y);
  BackToFront();

  return(TRUE);
}

boolean PlaceBomb(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int element;

  if (player->gone || player->MovPos)
    return(FALSE);

  element = Feld[jx][jy];

  if ((player->dynamite==0 && player->dynabombs_left==0) ||
      element==EL_DYNAMIT || element==EL_DYNABOMB || element==EL_EXPLODING)
    return(FALSE);

  if (element != EL_LEERRAUM)
    Store[jx][jy] = element;

  if (player->dynamite)
  {
    Feld[jx][jy] = EL_DYNAMIT;
    MovDelay[jx][jy] = 96;
    player->dynamite--;
    DrawText(DX_DYNAMITE, DY_DYNAMITE, int2str(local_player->dynamite, 3),
	     FS_SMALL, FC_YELLOW);
    if (IN_SCR_FIELD(SCREENX(jx),SCREENY(jy)))
      DrawGraphicThruMask(SCREENX(jx),SCREENY(jy),GFX_DYNAMIT);
  }
  else
  {
    Feld[jx][jy] = EL_DYNABOMB;
    Store2[jx][jy] = player->element_nr;	/* for DynaExplode() */
    MovDelay[jx][jy] = 96;
    player->dynabombs_left--;
    if (IN_SCR_FIELD(SCREENX(jx),SCREENY(jy)))
      DrawGraphicThruMask(SCREENX(jx),SCREENY(jy),GFX_DYNABOMB);
  }

  return(TRUE);
}

void PlaySoundLevel(int x, int y, int sound_nr)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int volume, stereo;
  int silence_distance = 8;

  if ((!setup.sound_simple_on && !IS_LOOP_SOUND(sound_nr)) ||
      (!setup.sound_loops_on && IS_LOOP_SOUND(sound_nr)))
    return;

  if (!IN_LEV_FIELD(x,y) ||
      sx < -silence_distance || sx >= SCR_FIELDX+silence_distance ||
      sy < -silence_distance || sy >= SCR_FIELDY+silence_distance)
    return;

  volume = PSND_MAX_VOLUME;
#ifndef MSDOS
  stereo = (sx-SCR_FIELDX/2)*12;
#else
  stereo = PSND_MIDDLE+(2*sx-(SCR_FIELDX-1))*5;
  if(stereo > PSND_MAX_RIGHT) stereo = PSND_MAX_RIGHT;
  if(stereo < PSND_MAX_LEFT) stereo = PSND_MAX_LEFT;
#endif

  if (!IN_SCR_FIELD(sx,sy))
  {
    int dx = ABS(sx-SCR_FIELDX/2)-SCR_FIELDX/2;
    int dy = ABS(sy-SCR_FIELDY/2)-SCR_FIELDY/2;

    volume -= volume*(dx > dy ? dx : dy)/silence_distance;
  }

  PlaySoundExt(sound_nr, volume, stereo, PSND_NO_LOOP);
}

void RaiseScore(int value)
{
  local_player->score += value;
  DrawText(DX_SCORE, DY_SCORE, int2str(local_player->score, 5),
	   FS_SMALL, FC_YELLOW);
}

void RaiseScoreElement(int element)
{
  switch(element)
  {
    case EL_EDELSTEIN:
    case EL_EDELSTEIN_BD:
    case EL_EDELSTEIN_GELB:
    case EL_EDELSTEIN_ROT:
    case EL_EDELSTEIN_LILA:
      RaiseScore(level.score[SC_EDELSTEIN]);
      break;
    case EL_DIAMANT:
      RaiseScore(level.score[SC_DIAMANT]);
      break;
    case EL_KAEFER:
    case EL_BUTTERFLY:
      RaiseScore(level.score[SC_KAEFER]);
      break;
    case EL_FLIEGER:
    case EL_FIREFLY:
      RaiseScore(level.score[SC_FLIEGER]);
      break;
    case EL_MAMPFER:
    case EL_MAMPFER2:
      RaiseScore(level.score[SC_MAMPFER]);
      break;
    case EL_ROBOT:
      RaiseScore(level.score[SC_ROBOT]);
      break;
    case EL_PACMAN:
      RaiseScore(level.score[SC_PACMAN]);
      break;
    case EL_KOKOSNUSS:
      RaiseScore(level.score[SC_KOKOSNUSS]);
      break;
    case EL_DYNAMIT:
      RaiseScore(level.score[SC_DYNAMIT]);
      break;
    case EL_SCHLUESSEL:
      RaiseScore(level.score[SC_SCHLUESSEL]);
      break;
    default:
      break;
  }
}
