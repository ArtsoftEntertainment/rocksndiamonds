/* 2000-08-10T16:43:50Z
 *
 * cave data structures
 */

#include "main_em.h"


struct LevelInfo_EM native_em_level;

void setLevelInfoToDefaults_EM(void)
{
  int i, j, x, y;

  native_em_level.file_version = FILE_VERSION_EM_ACTUAL;
  native_em_level.cav = &cav;

  game_em.lev = &lev;
  for (i = 0; i < MAX_PLAYERS; i++)
    game_em.ply[i] = &ply[i];

  cav.width  = 64;
  cav.height = 32;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    cav.player_x[i] = -1;
    cav.player_y[i] = -1;
  }

  cav.time_seconds	= 0;
  cav.gems_needed	= 0;

  cav.eater_score	= 0;
  cav.alien_score	= 0;
  cav.bug_score		= 0;
  cav.tank_score	= 0;
  cav.slurp_score	= 0;
  cav.nut_score		= 0;
  cav.emerald_score	= 0;
  cav.diamond_score	= 0;
  cav.dynamite_score	= 0;
  cav.key_score		= 0;
  cav.lenses_score	= 0;
  cav.magnify_score	= 0;
  cav.exit_score	= 0;

  cav.android_move_time	= 0;
  cav.android_clone_time= 0;
  cav.ball_time		= 0;
  cav.amoeba_time	= 0;
  cav.wonderwall_time	= 0;
  cav.wheel_time	= 0;
  cav.wheel_x		= 0;
  cav.wheel_y		= 0;
  cav.lenses_time	= 0;
  cav.magnify_time	= 0;
  cav.wind_time		= 0;
  cav.wind_direction	= 0;

  cav.num_eater_arrays	= 8;
  cav.num_ball_arrays	= 8;

  cav.testmode		= FALSE;
  cav.teamwork		= FALSE;
  cav.infinite		= TRUE;
  cav.infinite_true	= FALSE;	// default: use original EMC behaviour

  cav.ball_random	= FALSE;
  cav.ball_active	= FALSE;
  cav.wonderwall_active	= FALSE;
  cav.wheel_active	= FALSE;
  cav.lenses_active	= FALSE;
  cav.magnify_active	= FALSE;

  for (i = 0; i < 8; i++)
    for (j = 0; j < 9; j++)
      cav.eater_array[i][j] = Cblank;

  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
      cav.ball_array[i][j] = Cblank;

  for (i = 0; i < GAME_TILE_MAX; i++)
    cav.android_array[i] = Cblank;

  for (x = 0; x < CAVE_WIDTH; x++)
    for (y = 0; y < CAVE_HEIGHT; y++)
      cav.cave[x][y] = Cblank;
}


/* load cave
 * 
 * completely initializes the level structure, ready for a game
 */

#define MAX_EM_LEVEL_SIZE		16384

boolean LoadNativeLevel_EM(char *filename, boolean level_info_only)
{
  unsigned char raw_leveldata[MAX_EM_LEVEL_SIZE];
  int raw_leveldata_length;
  int file_version;
  File *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults_EM();

  if (!(file = openFile(filename, MODE_READ)))
  {
    if (!level_info_only)
      Warn("cannot open level '%s' -- using empty level", filename);

    return FALSE;
  }

  raw_leveldata_length = readFile(file, raw_leveldata, 1, MAX_EM_LEVEL_SIZE);

  closeFile(file);

  if (raw_leveldata_length <= 0)
  {
    Warn("cannot read level '%s' -- using empty level", filename);

    return FALSE;
  }

  file_version = cleanup_em_level(raw_leveldata, raw_leveldata_length, filename);

  if (file_version == FILE_VERSION_EM_UNKNOWN)
  {
    Warn("unknown EM level '%s' -- using empty level", filename);

    return FALSE;
  }

  convert_em_level(raw_leveldata, file_version);
  prepare_em_level();

  return TRUE;
}
