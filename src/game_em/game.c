/* 2007-11-06 03:39:32
 *
 * handle interaction between screen and cave
 */

#include "main_em.h"


struct CAVE cav;
struct LOGIC lev;
struct PLAYER ply[MAX_PLAYERS];

struct EngineSnapshotInfo_EM engine_snapshot_em;

/* handle input actions for players */

static void readjoy(byte action, struct PLAYER *ply)
{
  int north = 0, east = 0, south = 0, west = 0;
  int snap = 0, drop = 0;

  if (game_em.use_single_button && action & (JOY_BUTTON_1 | JOY_BUTTON_2))
    action |= JOY_BUTTON_1 | JOY_BUTTON_2;

  if (action & JOY_LEFT)
    west = 1;

  if (action & JOY_RIGHT)
    east = 1;

  if (action & JOY_UP)
    north = 1;

  if (action & JOY_DOWN)
    south = 1;

  if (action & JOY_BUTTON_1)
    snap = 1;

  if (action & JOY_BUTTON_2)
    drop = 1;

  /* always update drop action */
  ply->joy_drop = drop;

  if (ply->joy_stick || (north | east | south | west))	/* (no "| snap"!) */
  {
    ply->joy_n = north;
    ply->joy_e = east;
    ply->joy_s = south;
    ply->joy_w = west;

    /* when storing last action, only update snap action with direction */
    /* (prevents clearing direction if snapping stopped before frame 7) */
    ply->joy_snap = snap;
  }

  /* if no direction was stored before, allow setting snap to current state */
  if (!ply->joy_n &&
      !ply->joy_e &&
      !ply->joy_s &&
      !ply->joy_w)
    ply->joy_snap = snap;

  /* use bug with snap key (mainly TAS keys) sometimes moving the player */
  if (game_em.use_snap_key_bug)
    ply->joy_snap = snap;
}

void InitGameEngine_EM(void)
{
  prepare_em_level();

  game_initscreen();

  RedrawPlayfield_EM(FALSE);
}

void GameActions_EM(byte action[MAX_PLAYERS], boolean warp_mode)
{
  int i;
  boolean any_player_dropping = FALSE;

  game_em.random = game_em.random * 129 + 1;

  frame = (frame + 1) % 8;

  for (i = 0; i < MAX_PLAYERS; i++)
    readjoy(action[i], &ply[i]);

  UpdateEngineValues(screen_x / TILEX, screen_y / TILEY, ply[0].x, ply[0].y);

  logic();

  for (i = 0; i < MAX_PLAYERS; i++)
    if (ply[i].joy_drop &&
	ply[i].dynamite &&
	ply[i].dynamite_cnt > 0 &&
	ply[i].dynamite_cnt < 5)
      any_player_dropping = TRUE;

  CheckSingleStepMode_EM(action, frame, game_em.any_player_moving,
			 game_em.any_player_snapping, any_player_dropping);

  RedrawPlayfield_EM(FALSE);
}

void SaveEngineSnapshotValues_EM(void)
{
  int i;

  engine_snapshot_em.game_em = game_em;
  engine_snapshot_em.lev = lev;

  engine_snapshot_em.frame = frame;
  engine_snapshot_em.screen_x = screen_x;
  engine_snapshot_em.screen_y = screen_y;

  for (i = 0; i < 4; i++)
    engine_snapshot_em.ply[i] = ply[i];
}

void LoadEngineSnapshotValues_EM(void)
{
  int i;

  game_em = engine_snapshot_em.game_em;
  lev = engine_snapshot_em.lev;

  frame = engine_snapshot_em.frame;
  screen_x = engine_snapshot_em.screen_x;
  screen_y = engine_snapshot_em.screen_y;

  for (i = 0; i < 4; i++)
    ply[i] = engine_snapshot_em.ply[i];
}
