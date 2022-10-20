// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// tape.h
// ============================================================================

#ifndef TAPE_H
#define TAPE_H


// values for TapeTogglePause()
#define	TAPE_TOGGLE_AUTOMATIC	0
#define	TAPE_TOGGLE_MANUAL	(1 << 0)
#define	TAPE_TOGGLE_PLAY_PAUSE	(1 << 1)

// values for tape properties
#define MAX_TAPE_LEN		(10000 * FRAMES_PER_SECOND) // max.time x fps

// values for tape action array positions
#define TAPE_ACTION_LX		(MAX_PLAYERS + 0)
#define TAPE_ACTION_LY		(MAX_PLAYERS + 1)
#define TAPE_ACTION_BUTTON	(MAX_PLAYERS + 2)

#define MAX_TAPE_ACTIONS	(MAX_PLAYERS + 3)

// values for tape actions stored in tape file
#define TAPE_USE_KEY_ACTIONS_ONLY	0
#define TAPE_USE_MOUSE_ACTIONS_ONLY	1
#define TAPE_USE_KEY_AND_MOUSE_ACTIONS	2

#define TAPE_ACTIONS_DEFAULT		TAPE_USE_KEY_ACTIONS_ONLY

// values for tape properties stored in tape file
#define TAPE_PROPERTY_NONE		0
#define TAPE_PROPERTY_EM_RANDOM_BUG	(1 << 0)
#define TAPE_PROPERTY_GAME_SPEED	(1 << 1)
#define TAPE_PROPERTY_PAUSE_MODE	(1 << 2)
#define TAPE_PROPERTY_SINGLE_STEP	(1 << 3)
#define TAPE_PROPERTY_SNAPSHOT		(1 << 4)
#define TAPE_PROPERTY_REPLAYED		(1 << 5)
#define TAPE_PROPERTY_TAS_KEYS		(1 << 6)
#define TAPE_PROPERTY_SMALL_GRAPHICS	(1 << 7)

// values for score tape basename length (date, time, name hash, no extension)
#define MAX_SCORE_TAPE_BASENAME_LEN	24

// some positions in the video tape control window
#define VIDEO_DISPLAY1_XPOS	5
#define VIDEO_DISPLAY1_YPOS	5
#define VIDEO_DISPLAY2_XPOS	5
#define VIDEO_DISPLAY2_YPOS	41
#define VIDEO_DISPLAY_XSIZE	90
#define VIDEO_DISPLAY_YSIZE	31
#define VIDEO_BUTTON_XSIZE	18
#define VIDEO_BUTTON_YSIZE	18
#define VIDEO_CONTROL_XPOS	5
#define VIDEO_CONTROL_YPOS	77
#define VIDEO_CONTROL_XSIZE	VIDEO_DISPLAY_XSIZE
#define VIDEO_CONTROL_YSIZE	VIDEO_BUTTON_YSIZE

// values for video tape control
#define VIDEO_STATE_PLAY_OFF	(1u << 0)
#define VIDEO_STATE_PLAY_ON	(1u << 1)
#define VIDEO_STATE_REC_OFF	(1u << 2)
#define VIDEO_STATE_REC_ON	(1u << 3)
#define VIDEO_STATE_PAUSE_OFF	(1u << 4)
#define VIDEO_STATE_PAUSE_ON	(1u << 5)
#define VIDEO_STATE_DATE_OFF	(1u << 6)
#define VIDEO_STATE_DATE_ON	(1u << 7)
#define VIDEO_STATE_TIME_OFF	(1u << 8)
#define VIDEO_STATE_TIME_ON	(1u << 9)
#define VIDEO_STATE_FRAME_OFF	(1u << 10)
#define VIDEO_STATE_FRAME_ON	(1u << 11)
#define VIDEO_STATE_FFWD_OFF	(1u << 12)
#define VIDEO_STATE_FFWD_ON	(1u << 13)
#define VIDEO_STATE_WARP_OFF	(1u << 14)
#define VIDEO_STATE_WARP_ON	(1u << 15)
#define VIDEO_STATE_WARP2_OFF	(1u << 16)
#define VIDEO_STATE_WARP2_ON	(1u << 17)
#define VIDEO_STATE_PBEND_OFF	(1u << 18)
#define VIDEO_STATE_PBEND_ON	(1u << 19)
#define VIDEO_STATE_1STEP_OFF	(1u << 20)
#define VIDEO_STATE_1STEP_ON	(1u << 21)

#define VIDEO_PRESS_PLAY_ON	(1u << 22)
#define VIDEO_PRESS_PLAY_OFF	(1u << 23)
#define VIDEO_PRESS_REC_ON	(1u << 24)
#define VIDEO_PRESS_REC_OFF	(1u << 25)
#define VIDEO_PRESS_PAUSE_ON	(1u << 26)
#define VIDEO_PRESS_PAUSE_OFF	(1u << 27)
#define VIDEO_PRESS_STOP_ON	(1u << 28)
#define VIDEO_PRESS_STOP_OFF	(1u << 29)
#define VIDEO_PRESS_EJECT_ON	(1u << 30)
#define VIDEO_PRESS_EJECT_OFF	(1u << 31)

#define VIDEO_STATE_PLAY(x)  ((x) ? VIDEO_STATE_PLAY_ON : VIDEO_STATE_PLAY_OFF)
#define VIDEO_STATE_REC(x)   ((x) ? VIDEO_STATE_REC_ON  : VIDEO_STATE_REC_OFF)
#define VIDEO_STATE_PAUSE(x) ((x) ? VIDEO_STATE_PAUSE_ON: VIDEO_STATE_PAUSE_OFF)
#define VIDEO_STATE_DATE(x)  ((x) ? VIDEO_STATE_DATE_ON : VIDEO_STATE_DATE_OFF)
#define VIDEO_STATE_TIME(x)  ((x) ? VIDEO_STATE_TIME_ON : VIDEO_STATE_TIME_OFF)
#define VIDEO_STATE_FRAME(x) ((x) ? VIDEO_STATE_FRAME_ON: VIDEO_STATE_FRAME_OFF)
#define VIDEO_STATE_FFWD(x)  ((x) ? VIDEO_STATE_FFWD_ON : VIDEO_STATE_FFWD_OFF)
#define VIDEO_STATE_WARP(x)  ((x) ? VIDEO_STATE_WARP_ON : VIDEO_STATE_WARP_OFF)
#define VIDEO_STATE_WARP2(x) ((x) ? VIDEO_STATE_WARP2_ON: VIDEO_STATE_WARP2_OFF)
#define VIDEO_STATE_PBEND(x) ((x) ? VIDEO_STATE_PBEND_ON: VIDEO_STATE_PBEND_OFF)
#define VIDEO_STATE_1STEP(x) ((x) ? VIDEO_STATE_1STEP_ON: VIDEO_STATE_1STEP_OFF)

#define VIDEO_PRESS_PLAY(x)  ((x) ? VIDEO_PRESS_PLAY_ON : VIDEO_PRESS_PLAY_OFF)
#define VIDEO_PRESS_REC(x)   ((x) ? VIDEO_PRESS_REC_ON  : VIDEO_PRESS_REC_OFF)
#define VIDEO_PRESS_PAUSE(x) ((x) ? VIDEO_PRESS_PAUSE_ON: VIDEO_PRESS_PAUSE_OFF)
#define VIDEO_PRESS_STOP(x)  ((x) ? VIDEO_PRESS_STOP_ON : VIDEO_PRESS_STOP_OFF)
#define VIDEO_PRESS_EJECT(x) ((x) ? VIDEO_PRESS_EJECT_ON: VIDEO_PRESS_EJECT_OFF)

// tags to draw video display labels or symbols only
// (negative values to prevent misinterpretation in DrawVideoDisplay(), where
// the variable "value" is also used for tape length -- better fix this)
#define VIDEO_DISPLAY_DEFAULT		0
#define VIDEO_DISPLAY_LABEL_ONLY	-1
#define VIDEO_DISPLAY_SYMBOL_ONLY	-2


struct TapeButtonInfo
{
  struct XY eject;
  struct XY stop;
  struct XY pause;
  struct XY record;
  struct XY play;
  struct XY insert_solution;
  struct XY play_solution;
};

struct TapeSymbolInfo
{
  struct XY eject;
  struct XY stop;
  struct XY pause;
  struct XY record;
  struct XY play;
  struct XY fast_forward;
  struct XY warp_forward;
  struct XY warp_forward_blind;
  struct XY pause_before_end;
  struct XY single_step;
};

struct TapeLabelInfo
{
  struct XY eject;
  struct XY stop;
  struct XY pause;
  struct XY record;
  struct XY play;
  struct XY fast_forward;
  struct XY warp_forward;
  struct XY warp_forward_blind;
  struct XY pause_before_end;
  struct XY single_step;
  struct XY date;
  struct XY time;
};

struct TapeTextInfo
{
  struct TextPosInfo date;
  struct TextPosInfo date_yyyy;
  struct TextPosInfo date_yy;
  struct TextPosInfo date_mon;
  struct TextPosInfo date_mm;
  struct TextPosInfo date_dd;

  struct TextPosInfo time;
  struct TextPosInfo time_hh;
  struct TextPosInfo time_mm;
  struct TextPosInfo time_ss;

  struct TextPosInfo frame;
};

struct TapeInfo
{
  int file_version;	// file format version the tape is stored with
  int game_version;	// game release version the tape was created with
  int engine_version;	// game engine version the tape was recorded with

  char score_tape_basename[MAX_FILENAME_LEN + 1];
  char level_identifier[MAX_FILENAME_LEN + 1];
  int level_nr;
  unsigned int random_seed;
  unsigned int date;
  unsigned int counter;
  unsigned int length;
  unsigned int length_frames;
  unsigned int length_seconds;
  unsigned int delay_played;
  boolean pause_before_end;
  boolean recording, playing, pausing;
  boolean fast_forward;
  boolean warp_forward;
  boolean deactivate_display;
  boolean auto_play;
  boolean auto_play_level_solved;
  boolean auto_play_level_fixed;
  boolean auto_play_level_not_fixable;
  boolean quick_resume;
  boolean single_step;
  boolean changed;
  boolean solved;
  boolean player_participates[MAX_PLAYERS];
  int num_participating_players;
  int centered_player_nr_next;
  boolean set_centered_player;

  // flags to indicate which game actions are stored in this tape
  boolean use_key_actions;
  boolean use_mouse_actions;

  // bits to indicate which tape properties are stored in this tape
  byte property_bits;

  // visible playfield size when recording this tape (for team mode)
  int scr_fieldx;
  int scr_fieldy;

  struct
  {
    byte action[MAX_TAPE_ACTIONS];
    byte delay;
  } pos[MAX_TAPE_LEN];

  struct TapeButtonInfo button;
  struct TapeSymbolInfo symbol;
  struct TapeLabelInfo label;
  struct TapeTextInfo text;

  boolean show_game_buttons;	// show game buttons in tape viewport

  boolean no_info_chunk;	// used to identify old tape file format
  boolean no_valid_file;	// set when tape file missing or invalid
};


void DrawVideoDisplay(unsigned int, unsigned int);
void DrawCompleteVideoDisplay(void);

void TapeDeactivateDisplayOn(void);
void TapeDeactivateDisplayOff(boolean);

void TapeSetDateFromEpochSeconds(time_t);
void TapeSetDateFromNow(void);

void TapeStartRecording(int);
void TapeHaltRecording(void);
void TapeStopRecording(void);
boolean TapeAddAction(byte[MAX_TAPE_ACTIONS]);
void TapeRecordAction(byte[MAX_TAPE_ACTIONS]);
void TapeTogglePause(boolean);
void TapeStartPlaying(void);
void TapeStopPlaying(void);
byte *TapePlayAction(void);
void TapeStop(void);
void TapeStopGame(void);
void TapeStopTape(void);
void TapeErase(void);
unsigned int GetTapeLengthFrames(void);
unsigned int GetTapeLengthSeconds(void);
void TapeQuickSave(void);
void TapeQuickLoad(void);
void TapeRestartGame(void);
void TapeReplayAndPauseBeforeEnd(void);

boolean hasSolutionTape(void);
boolean InsertSolutionTape(void);
boolean PlaySolutionTape(void);
boolean PlayScoreTape(int);

void UndoTape(void);
void FixTape_ForceSinglePlayer(void);

int AutoPlayTapes(void);
int AutoPlayTapesContinue(void);
void PatchTapes(void);

void CreateTapeButtons(void);
void FreeTapeButtons(void);
void MapTapeEjectButton(void);
void MapTapeWarpButton(void);
void MapTapeButtons(void);
void UnmapTapeButtons(void);
void RedrawTapeButtons(void);
void RedrawOrRemapTapeButtons(void);

void HandleTapeButtonKeys(Key);

#endif
