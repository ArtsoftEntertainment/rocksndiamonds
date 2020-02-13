/* 2009-01-25 23:00:34
 *
 * intermediate cave structure
 */

#ifndef CAVE_H
#define CAVE_H

struct CAVE
{
  int width;			/* cave width */
  int height;			/* cave height */

  int player_x[MAX_PLAYERS];	/* player x pos */
  int player_y[MAX_PLAYERS];	/* player y pos */

  int time_seconds;		/* available time (seconds) */
  int gems_needed;		/* emeralds needed */

  int eater_score;		/* score for killing eater */
  int alien_score;		/* score for killing alien */
  int bug_score;		/* score for killing bug */
  int tank_score;		/* score for killing tank */
  int slurp_score;		/* score for slurping alien with spring */
  int nut_score;		/* score for cracking nut to emerald */
  int emerald_score;		/* score for collecting emerald */
  int diamond_score;		/* score for collecting diamond */
  int dynamite_score;		/* score for collecting dynamite */
  int key_score;		/* score for colleting key */
  int lenses_score;		/* score for collecting lenses */
  int magnify_score;		/* score for collecting magnifier */
  int exit_score;		/* score for entering exit */

  int android_move_time;	/* android move reset time */
  int android_clone_time;	/* android clone reset time */
  int ball_time;		/* ball reset time */
  int amoeba_time;		/* amoeba speed */
  int wonderwall_time;		/* wonderwall time */
  int wheel_time;		/* wheel reset time */
  int wheel_x;			/* wheel x pos */
  int wheel_y;			/* wheel y pos */
  int lenses_time;		/* lenses reset time */
  int magnify_time;		/* magnify reset time */
  int wind_time;		/* wind reset time */
  int wind_direction;		/* wind direction */

  int ball_random;		/* ball is random flag */
  int ball_state;		/* ball active flag */
  int wonderwall_state;		/* wonderwall active flag */
  int wheel_cnt;		/* wheel counter */
  int lenses_cnt;		/* lenses counter */
  int magnify_cnt;		/* magnify counter */
  int wind_cnt;			/* wind time counter */

  int num_ball_arrays;		/* number of ball data arrays used */

  short eater_array[8][9];		/* eater data */
  short ball_array[8][8];		/* ball data */
  short android_array[TILE_MAX];	/* android clone data */

  short cave[CAVE_WIDTH][CAVE_HEIGHT];	/* cave data */
};

#endif	// CAVE_H
