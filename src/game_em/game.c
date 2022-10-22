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
  boolean north	= FALSE;
  boolean east	= FALSE;
  boolean south	= FALSE;
  boolean west	= FALSE;
  boolean snap	= FALSE;
  boolean drop	= FALSE;

  if (game_em.use_single_button && action & (JOY_BUTTON_1 | JOY_BUTTON_2))
    action |= JOY_BUTTON_1 | JOY_BUTTON_2;

  if (action & JOY_LEFT)
    west = TRUE;

  if (action & JOY_RIGHT)
    east = TRUE;

  if (action & JOY_UP)
    north = TRUE;

  if (action & JOY_DOWN)
    south = TRUE;

  if (action & JOY_BUTTON_1)
    snap = TRUE;

  if (action & JOY_BUTTON_2)
    drop = TRUE;

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

  logic_init();

  game_initscreen();

  RedrawPlayfield_EM(FALSE);
}

void GameActions_EM(byte action[MAX_PLAYERS])
{
  int i;
  boolean any_player_dropping = FALSE;

  game_em.random = game_em.random * 129 + 1;

  frame = (frame + 1) % 8;

  for (i = 0; i < MAX_PLAYERS; i++)
    readjoy(action[i], &ply[i]);

  UpdateEngineValues(CAVE_POS_X(screen_x / TILEX),
		     CAVE_POS_Y(screen_y / TILEY),
		     CAVE_POS_X(ply[0].x),
		     CAVE_POS_Y(ply[0].y));

  logic();

  for (i = 0; i < MAX_PLAYERS; i++)
    if (ply[i].joy_drop &&
	ply[i].dynamite &&
	ply[i].dynamite_cnt > 0 &&
	ply[i].dynamite_cnt < 5)
      any_player_dropping = TRUE;

  boolean single_step_mode_paused =
    CheckSingleStepMode_EM(frame, game_em.any_player_moving,
			   game_em.any_player_snapping, any_player_dropping);

  // draw wrapping around before going to single step pause mode
  if (single_step_mode_paused && logic_check_wrap())
    logic_move();

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
