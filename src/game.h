// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// game.h
// ============================================================================

#ifndef GAME_H
#define GAME_H

// (not included here due to collisions with Emerald Mine engine definitions)
// #include "main.h"

#define MAX_INVENTORY_SIZE		1000

#define MAX_HEALTH			100

#define STD_NUM_KEYS			4
#define MAX_NUM_KEYS			8

#define NUM_BDX_KEYS			3

#define NUM_BELTS			4
#define NUM_BELT_PARTS			3

#define NUM_PANEL_INVENTORY		8
#define NUM_PANEL_GRAPHICS		8
#define NUM_PANEL_ELEMENTS		8
#define NUM_PANEL_CE_SCORE		8

#define STR_SNAPSHOT_MODE_OFF		"off"
#define STR_SNAPSHOT_MODE_EVERY_STEP	"every_step"
#define STR_SNAPSHOT_MODE_EVERY_MOVE	"every_move"
#define STR_SNAPSHOT_MODE_EVERY_COLLECT	"every_collect"
#define STR_SNAPSHOT_MODE_DEFAULT	STR_SNAPSHOT_MODE_OFF

#define STR_SCORES_TYPE_LOCAL_ONLY	 "local_scores_only"
#define STR_SCORES_TYPE_SERVER_ONLY	 "server_scores_only"
#define STR_SCORES_TYPE_LOCAL_AND_SERVER "local_and_server_scores"
#define STR_SCORES_TYPE_DEFAULT		 STR_SCORES_TYPE_LOCAL_AND_SERVER

#define SNAPSHOT_MODE_OFF		0
#define SNAPSHOT_MODE_EVERY_STEP	1
#define SNAPSHOT_MODE_EVERY_MOVE	2
#define SNAPSHOT_MODE_EVERY_COLLECT	3
#define SNAPSHOT_MODE_DEFAULT		SNAPSHOT_MODE_OFF


struct GamePanelInfo
{
  struct TextPosInfo level_number;
  struct TextPosInfo gems;
  struct TextPosInfo gems_needed;
  struct TextPosInfo gems_collected;
  struct TextPosInfo gems_score;
  struct TextPosInfo inventory_count;
  struct TextPosInfo inventory_first[NUM_PANEL_INVENTORY];
  struct TextPosInfo inventory_last[NUM_PANEL_INVENTORY];
  struct TextPosInfo key[MAX_NUM_KEYS];
  struct TextPosInfo key_white;
  struct TextPosInfo key_white_count;
  struct TextPosInfo score;
  struct TextPosInfo highscore;
  struct TextPosInfo time;
  struct TextPosInfo time_hh;
  struct TextPosInfo time_mm;
  struct TextPosInfo time_ss;
  struct TextPosInfo time_anim;
  struct TextPosInfo health;
  struct TextPosInfo health_anim;
  struct TextPosInfo frame;
  struct TextPosInfo shield_normal;
  struct TextPosInfo shield_normal_time;
  struct TextPosInfo shield_deadly;
  struct TextPosInfo shield_deadly_time;
  struct TextPosInfo exit;
  struct TextPosInfo emc_magic_ball;
  struct TextPosInfo emc_magic_ball_switch;
  struct TextPosInfo light_switch;
  struct TextPosInfo light_switch_time;
  struct TextPosInfo timegate_switch;
  struct TextPosInfo timegate_switch_time;
  struct TextPosInfo switchgate_switch;
  struct TextPosInfo emc_lenses;
  struct TextPosInfo emc_lenses_time;
  struct TextPosInfo emc_magnifier;
  struct TextPosInfo emc_magnifier_time;
  struct TextPosInfo balloon_switch;
  struct TextPosInfo dynabomb_number;
  struct TextPosInfo dynabomb_size;
  struct TextPosInfo dynabomb_power;
  struct TextPosInfo penguins;
  struct TextPosInfo sokoban_objects;
  struct TextPosInfo sokoban_fields;
  struct TextPosInfo robot_wheel;
  struct TextPosInfo conveyor_belt[NUM_BELTS];
  struct TextPosInfo conveyor_belt_switch[NUM_BELTS];
  struct TextPosInfo magic_wall;
  struct TextPosInfo magic_wall_time;
  struct TextPosInfo gravity_state;
  struct TextPosInfo graphic[NUM_PANEL_GRAPHICS];
  struct TextPosInfo element[NUM_PANEL_ELEMENTS];
  struct TextPosInfo element_count[NUM_PANEL_ELEMENTS];
  struct TextPosInfo ce_score[NUM_PANEL_CE_SCORE];
  struct TextPosInfo ce_score_element[NUM_PANEL_CE_SCORE];
  struct TextPosInfo bdx_lives;
  struct TextPosInfo bdx_key[NUM_BDX_KEYS];
  struct TextPosInfo bdx_key_count[NUM_BDX_KEYS];
  struct TextPosInfo bdx_diamond_key;
  struct TextPosInfo bdx_gravity;
  struct TextPosInfo bdx_gravity_next;
  struct TextPosInfo bdx_gravity_time;
  struct TextPosInfo bdx_gravity_state;
  struct TextPosInfo bdx_skeleton;
  struct TextPosInfo bdx_skeleton_count;
  struct TextPosInfo bdx_sweet;
  struct TextPosInfo bdx_pneumatic_hammer;
  struct TextPosInfo bdx_rocket_count;
  struct TextPosInfo bdx_rocket_state;
  struct TextPosInfo bdx_magic_wall;
  struct TextPosInfo bdx_magic_wall_time;
  struct TextPosInfo bdx_creature_switch;
  struct TextPosInfo bdx_expandable_wall_switch;
  struct TextPosInfo bdx_biter_switch_time;
  struct TextPosInfo bdx_replicator;
  struct TextPosInfo bdx_replicator_switch;
  struct TextPosInfo bdx_conveyor_left;
  struct TextPosInfo bdx_conveyor_right;
  struct TextPosInfo bdx_conveyor_switch;
  struct TextPosInfo bdx_conveyor_dir_switch;
  struct TextPosInfo player_name;
  struct TextPosInfo levelset_name;
  struct TextPosInfo level_name;
  struct TextPosInfo level_author;

  // value to determine if panel will be updated or not
  boolean active;

  // value for dynamically showing extra panel items
  boolean show_extra_items;
};

struct GameButtonInfo
{
  struct XY stop;
  struct XY pause;
  struct XY play;

  struct XY undo;
  struct XY redo;

  struct XY save;
  struct XY pause2;
  struct XY load;

  struct XY restart;

  struct XY sound_music;
  struct XY sound_loops;
  struct XY sound_simple;

  struct XY panel_stop;
  struct XY panel_pause;
  struct XY panel_play;

  struct XY panel_restart;

  struct XY panel_sound_music;
  struct XY panel_sound_loops;
  struct XY panel_sound_simple;

  struct XY touch_stop;
  struct XY touch_pause;
  struct XY touch_restart;
};

struct GameSnapshotInfo
{
  int mode;

  byte last_action[MAX_PLAYERS];
  boolean changed_action;
  boolean collected_item;

  boolean save_snapshot;
};

struct GameInfo
{
  // values for control panel
  struct GamePanelInfo panel;
  struct GameButtonInfo button;

  // values for graphics engine customization
  int graphics_engine_version;
  boolean use_native_bd_graphics_engine;
  boolean use_native_emc_graphics_engine;
  boolean use_native_sp_graphics_engine;
  boolean use_masked_pushing;
  boolean use_masked_elements;
  boolean use_masked_elements_initial;
  boolean keep_panel_open_when_restarting;
  int forced_scroll_x;
  int forced_scroll_y;
  int forced_scroll_delay_value;
  int scroll_delay_value;
  int tile_size;

  // values for sound engine customization
  boolean use_native_bd_sound_engine;

  // constant within running game
  VersionType engine_version;
  int emulation;
  int initial_move_delay[MAX_PLAYERS];
  int initial_move_delay_value[MAX_PLAYERS];
  int initial_push_delay_value;

  // flag for single or multi-player mode (needed for playing tapes)
  // (when playing/recording games, this is identical to "setup.team_mode"
  boolean team_mode;

  // flags to handle bugs in and changes between different engine versions
  // (for the latest engine version, these flags should always be "FALSE")
  boolean use_change_when_pushing_bug;
  boolean use_block_last_field_bug;
  boolean max_num_changes_per_frame;
  boolean use_reverse_scan_direction;

  // flags to indicate which game actions are used in this game
  boolean use_key_actions;
  boolean use_mouse_actions;

  // variable within running game
  int yamyam_content_nr;
  boolean robot_wheel_active;
  boolean magic_wall_active;
  int magic_wall_time_left;
  int light_time_left;
  int timegate_time_left;
  int belt_dir[4];
  int belt_dir_nr[4];
  int switchgate_pos;
  int wind_direction;

  boolean explosions_delayed;
  boolean no_level_time_limit;	// (variable only in very special case)
  boolean time_limit;		// forced by levelset config or setup option

  int time_final;		// time (in seconds) or steps left or played
  int score_time_final;		// time (in frames) or steps played

  int score;
  int score_final;

  int health;
  int health_final;

  int gems_still_needed;
  int sokoban_fields_still_needed;
  int sokoban_objects_still_needed;
  int lights_still_needed;
  int players_still_needed;
  int friends_still_needed;

  int robot_wheel_x, robot_wheel_y;
  int exit_x, exit_y;

  boolean all_players_gone;

  // values for the new EMC elements
  int lenses_time_left;
  int magnify_time_left;
  boolean ball_active;
  int ball_content_nr;

  // values for player idle animation (no effect on engine)
  int player_boring_delay_fixed;
  int player_boring_delay_random;
  int player_sleeping_delay_fixed;
  int player_sleeping_delay_random;

  // values for special game initialization control
  boolean restart_level;

  // values for special request dialog control
  boolean request_open;
  boolean request_active;
  boolean envelope_active;
  boolean any_door_active;

  // values for special game control
  int centered_player_nr;
  int centered_player_nr_next;
  boolean set_centered_player;
  boolean set_centered_player_wrap;

  // values for single step mode control
  boolean enter_single_step_mode;

  // values for random number generator initialization after snapshot
  unsigned int num_random_calls;

  // values for game engine snapshot control
  struct GameSnapshotInfo snapshot;

  // values for handling states for solved level and game over
  boolean LevelSolved;
  boolean GamePlayed;
  boolean GameOver;

  boolean LevelSolved_GameWon;
  boolean LevelSolved_GameEnd;
  boolean LevelSolved_SaveTape;
  boolean LevelSolved_SaveScore;

  int LevelSolved_CountingTime;
  int LevelSolved_CountingScore;
  int LevelSolved_CountingHealth;

  boolean RestartGameRequested;
  boolean InitGameRequested;

  boolean InitColorTemplateImagesNeeded;
};

struct PlayerInfo
{
  boolean present;		// player present in level playfield
  boolean connected_locally;	// player connected (locally)
  boolean connected_network;	// player connected (network)
  boolean connected;		// player connected (locally or via network)
  boolean active;		// player present and connected
  boolean mapped;		// player already mapped to input device

  boolean killed;		// player maybe present/active, but killed
  boolean reanimated;		// player maybe killed, but reanimated
  boolean buried;		// player finally killed and removed

  int index_nr;			// player number (0 to 3)
  int index_bit;		// player number bit (1 << 0 to 1 << 3)
  int element_nr;		// element (EL_PLAYER_1 to EL_PLAYER_4)
  int client_nr;		// network client identifier

  byte action;			// action from local input device
  byte effective_action;	/* action acknowledged from network server
				   or summarized over all configured input
				   devices when in single player mode */
  byte programmed_action;	/* action forced by game itself (like moving
				   through doors); overrides other actions */
  byte snap_action;		// action from TAS snap keys

  struct MouseActionInfo mouse_action;		 // (used by MM engine only)
  struct MouseActionInfo effective_mouse_action; // (used by MM engine only)

  int jx, jy, last_jx, last_jy;
  int MovDir, MovPos, GfxDir, GfxPos;
  int Frame, StepFrame;

  int GfxAction;

  int initial_element;		// EL_PLAYER_1 to EL_PLAYER_4 or EL_SP_MURPHY
  int artwork_element;
  boolean use_murphy;

  boolean block_last_field;
  int block_delay_adjustment;	// needed for different engine versions

  boolean can_fall_into_acid;

  boolean gravity;

  int last_move_dir;

  boolean is_active;

  boolean is_waiting;
  boolean is_moving;
  boolean is_auto_moving;
  boolean is_digging;
  boolean is_snapping;
  boolean is_collecting;
  boolean is_pushing;
  boolean is_switching;
  boolean is_dropping;
  boolean is_dropping_pressed;

  boolean is_bored;
  boolean is_sleeping;

  boolean was_waiting;
  boolean was_moving;
  boolean was_snapping;
  boolean was_dropping;

  boolean cannot_move;

  boolean force_dropping;	// needed for single step mode

  int frame_counter_bored;
  int frame_counter_sleeping;

  int anim_delay_counter;
  int post_delay_counter;

  int dir_waiting;
  int action_waiting, last_action_waiting;
  int special_action_bored;
  int special_action_sleeping;

  int num_special_action_bored;
  int num_special_action_sleeping;

  int switch_x, switch_y;
  int drop_x, drop_y;

  int show_envelope;

  int move_delay;
  int move_delay_value;
  int move_delay_value_next;
  int move_delay_reset_counter;

  int push_delay;
  int push_delay_value;

  DelayCounter actual_frame_counter;

  int drop_delay;
  int drop_pressed_delay;

  int step_counter;

  int key[MAX_NUM_KEYS];
  int num_white_keys;
  int dynabomb_count, dynabomb_size, dynabombs_left, dynabomb_xl;
  int shield_normal_time_left;
  int shield_deadly_time_left;

  int last_removed_element;

  int inventory_element[MAX_INVENTORY_SIZE];
  int inventory_infinite_element;
  int inventory_size;
};

extern struct GameInfo		game;
extern struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player;


#ifdef DEBUG
void DEBUG_SetMaximumDynamite(void);
#endif

void GetPlayerConfig(void);
int GetElementFromGroupElement(int);

int getPlayerInventorySize(int);

void UpdateGameDoorValues(void);
void DrawGameDoorValues(void);
void DrawGameDoorValues_ForceRedraw(void);

void UpdateAndDisplayGameControlValues(void);

void InitGameSound(void);
void InitGame(void);

void UpdateEngineValues(int, int, int, int);
void GameEnd(void);

void MergeServerScore(void);

void InitPlayerGfxAnimation(struct PlayerInfo *, int, int);

void Moving2Blocked(int, int, int *, int *);
void Blocked2Moving(int, int, int *, int *);

void DrawDynamite(int, int);

void StartGameActions(boolean, boolean, int);

void GameActions(void);
void GameActions_BD_Main(void);
void GameActions_EM_Main(void);
void GameActions_SP_Main(void);
void GameActions_MM_Main(void);
void GameActions_RND_Main(void);
void GameActions_RND(void);

void ScrollLevel(int, int);

void InitPlayLevelSound(void);
void PlayLevelSound_EM(int, int, int, int);
void PlayLevelSound_SP(int, int, int, int);
void PlayLevelSound_MM(int, int, int, int);
void PlaySound_MM(int);
void PlaySoundLoop_MM(int);
void StopSound_MM(int);

void RaiseScore(int);
void RaiseScoreElement(int);

void RequestQuitGameExt(boolean, boolean, char *);
void RequestQuitGame(boolean);

boolean CheckRestartGame(void);
boolean checkGameRunning(void);
boolean checkGamePlaying(void);
boolean checkGameSolved(void);
boolean checkGameFailed(void);
boolean checkGameEnded(void);
boolean checkRequestActive(void);

unsigned int InitEngineRandom_RND(int);
unsigned int RND(int);

void FreeEngineSnapshotSingle(void);
void FreeEngineSnapshotList(void);
void LoadEngineSnapshotSingle(void);
void SaveEngineSnapshotSingle(void);
boolean CheckSaveEngineSnapshotToList(void);
void SaveEngineSnapshotToList(void);
void SaveEngineSnapshotToListInitial(void);
boolean CheckEngineSnapshotSingle(void);
boolean CheckEngineSnapshotList(void);

void CreateGameButtons(void);
void FreeGameButtons(void);
void MapLoadSaveButtons(void);
void MapUndoRedoButtons(void);
void ModifyPauseButtons(void);
void MapGameButtons(void);
void UnmapGameButtons(void);
void RedrawGameButtons(void);
void MapGameButtonsOnTape(void);
void UnmapGameButtonsOnTape(void);
void RedrawGameButtonsOnTape(void);

void HandleSoundButtonKeys(Key);

#endif
