/* 2009-01-25 23:00:34
 *
 * intermediate cave structure
 */

#ifndef CAVE_H
#define CAVE_H

struct CAVE
{
  int width;			/* playfield width */
  int height;			/* playfield height */

  int player_x[MAX_PLAYERS];	/* player x pos */
  int player_y[MAX_PLAYERS];	/* player y pos */

  int time_seconds;		/* available time (seconds) */
  int required_initial;		/* emeralds needed */

  int alien_score;		/* score for killing alien */
  int amoeba_time;		/* amoeba speed */
  int android_move_cnt_initial;	/* android move counter (initial) */
  int android_move_time;	/* android move reset time */
  int android_clone_cnt_initial;/* android clone counter (initial) */
  int android_clone_time;	/* android clone reset time */
  int ball_cnt;			/* ball counter */
  int ball_pos;			/* ball array pos counter */
  int ball_random;		/* ball is random flag */
  int ball_state_initial;	/* ball active flag (initial) */
  int ball_time;		/* ball reset time */
  int bug_score;		/* score for killing bug */
  int diamond_score;		/* score for collecting diamond */
  int dynamite_score;		/* score for collecting dynamite */
  int eater_pos;		/* eater array pos */
  int eater_score;		/* score for killing eater */
  int emerald_score;		/* score for collecting emerald */
  int exit_score;		/* score for entering exit */
  int key_score;		/* score for colleting key */
  int lenses_cnt_initial;	/* lenses counter (initial) */
  int lenses_score;		/* score for collecting lenses */
  int lenses_time;		/* lenses reset time */
  int magnify_cnt_initial;	/* magnify counter (initial) */
  int magnify_score;		/* score for collecting magnifier */
  int magnify_time;		/* magnify reset time */
  int nut_score;		/* score for cracking nut */
  int shine_cnt;		/* shine counter for emerald/diamond */
  int slurp_score;		/* score for slurping alien */
  int tank_score;		/* score for killing tank */
  int wheel_cnt_initial;	/* wheel counter (initial) */
  int wheel_x_initial;		/* wheel x pos (initial) */
  int wheel_y_initial;		/* wheel y pos (initial) */
  int wheel_time;		/* wheel reset time */
  int wind_cnt_initial;		/* wind counter (initial) */
  int wind_direction_initial;	/* wind direction (initial) */
  int wind_time;		/* wind reset time */
  int wonderwall_state_initial;	/* wonderwall active flag (initial) */
  int wonderwall_time_initial;	/* wonderwall time (initial) */

  int num_ball_arrays;		/* number of ball data arrays used */

  short eater_array[8][9];		/* eater data */
  short ball_array[8][8];		/* ball data */
  short android_array[TILE_MAX];	/* android clone table */

  short cave_raw[CAVE_WIDTH][CAVE_HEIGHT];
};

#endif	// CAVE_H
