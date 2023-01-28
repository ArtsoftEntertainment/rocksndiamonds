// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// tape.c
// ============================================================================

#include "libgame/libgame.h"

#include "tape.h"
#include "init.h"
#include "game.h"
#include "tools.h"
#include "files.h"
#include "network.h"
#include "anim.h"
#include "api.h"


#define DEBUG_TAPE_WHEN_PLAYING			FALSE

// tape button identifiers
#define TAPE_CTRL_ID_EJECT			0
#define TAPE_CTRL_ID_EXTRA			1
#define TAPE_CTRL_ID_STOP			2
#define TAPE_CTRL_ID_PAUSE			3
#define TAPE_CTRL_ID_RECORD			4
#define TAPE_CTRL_ID_PLAY			5
#define TAPE_CTRL_ID_INSERT_SOLUTION		6
#define TAPE_CTRL_ID_PLAY_SOLUTION		7

#define NUM_TAPE_BUTTONS			8

// values for tape handling
#define TAPE_PAUSE_SECONDS_BEFORE_DEATH		5
#define TAPE_MIN_SECONDS_FOR_UNDO_BUFFER	20

// forward declaration for internal use
static void HandleTapeButtons(struct GadgetInfo *);
static void TapeStopWarpForward(void);
static float GetTapeLengthSecondsFloat(void);
static void CopyTapeToUndoBuffer(void);

static struct GadgetInfo *tape_gadget[NUM_TAPE_BUTTONS];
static struct TapeInfo tape_undo_buffer;


// ============================================================================
// video tape definitions
// ============================================================================

#define VIDEO_INFO_OFF			(VIDEO_STATE_DATE_OFF	|	\
					 VIDEO_STATE_TIME_OFF	|	\
					 VIDEO_STATE_FRAME_OFF)
#define VIDEO_STATE_OFF			(VIDEO_STATE_PLAY_OFF	|	\
					 VIDEO_STATE_REC_OFF	|	\
					 VIDEO_STATE_PAUSE_OFF	|	\
					 VIDEO_STATE_FFWD_OFF	|	\
					 VIDEO_STATE_WARP_OFF	|	\
					 VIDEO_STATE_WARP2_OFF	|	\
					 VIDEO_STATE_PBEND_OFF	|	\
					 VIDEO_STATE_1STEP_OFF)
#define VIDEO_PRESS_OFF			(VIDEO_PRESS_PLAY_OFF	|	\
					 VIDEO_PRESS_REC_OFF	|	\
					 VIDEO_PRESS_PAUSE_OFF	|	\
					 VIDEO_PRESS_STOP_OFF	|	\
					 VIDEO_PRESS_EJECT_OFF)
#define VIDEO_ALL_OFF			(VIDEO_INFO_OFF		|	\
	 				 VIDEO_STATE_OFF	|	\
	 				 VIDEO_PRESS_OFF)

#define VIDEO_INFO_ON			(VIDEO_STATE_DATE_ON	|	\
					 VIDEO_STATE_TIME_ON	|	\
					 VIDEO_STATE_FRAME_ON)
#define VIDEO_STATE_ON			(VIDEO_STATE_PLAY_ON	|	\
					 VIDEO_STATE_REC_ON	|	\
					 VIDEO_STATE_PAUSE_ON	|	\
					 VIDEO_STATE_FFWD_ON	|	\
					 VIDEO_STATE_WARP_ON	|	\
					 VIDEO_STATE_WARP2_ON	|	\
					 VIDEO_STATE_PBEND_ON	|	\
					 VIDEO_STATE_1STEP_ON)
#define VIDEO_PRESS_ON			(VIDEO_PRESS_PLAY_ON	|	\
					 VIDEO_PRESS_REC_ON	|	\
					 VIDEO_PRESS_PAUSE_ON	|	\
					 VIDEO_PRESS_STOP_ON	|	\
					 VIDEO_PRESS_EJECT_ON)
#define VIDEO_ALL_ON			(VIDEO_INFO_ON		|	\
	 				 VIDEO_STATE_ON		|	\
	 				 VIDEO_PRESS_ON)

#define VIDEO_INFO			(VIDEO_INFO_ON | VIDEO_INFO_OFF)
#define VIDEO_STATE			(VIDEO_STATE_ON | VIDEO_STATE_OFF)
#define VIDEO_PRESS			(VIDEO_PRESS_ON | VIDEO_PRESS_OFF)
#define VIDEO_ALL			(VIDEO_ALL_ON | VIDEO_ALL_OFF)

#define NUM_TAPE_FUNCTIONS		11
#define NUM_TAPE_FUNCTION_PARTS		2
#define NUM_TAPE_FUNCTION_STATES	2


// ============================================================================
// video display functions
// ============================================================================

static void DrawVideoDisplay_Graphics(unsigned int state, unsigned int value)
{
  int i, j, k;

  static struct
  {
    int graphic;
    struct XY *pos;
  }
  video_pos[NUM_TAPE_FUNCTIONS][NUM_TAPE_FUNCTION_PARTS] =
  {
    {
      { IMG_GFX_TAPE_LABEL_PLAY,		&tape.label.play	},
      { IMG_GFX_TAPE_SYMBOL_PLAY,		&tape.symbol.play	},
    },
    {
      { IMG_GFX_TAPE_LABEL_RECORD,		&tape.label.record	},
      { IMG_GFX_TAPE_SYMBOL_RECORD,		&tape.symbol.record	},
    },
    {
      { IMG_GFX_TAPE_LABEL_PAUSE,		&tape.label.pause	},
      { IMG_GFX_TAPE_SYMBOL_PAUSE,		&tape.symbol.pause	},
    },
    {
      { IMG_GFX_TAPE_LABEL_DATE,		&tape.label.date	},
      { -1,					NULL			},
    },
    {
      { IMG_GFX_TAPE_LABEL_TIME,		&tape.label.time	},
      { -1,					NULL			},
    },
    {
      // (no label for displaying optional frame)
      { -1,					NULL			},
      { -1,					NULL			},
    },
    {
      { IMG_GFX_TAPE_LABEL_FAST_FORWARD,	&tape.label.fast_forward  },
      { IMG_GFX_TAPE_SYMBOL_FAST_FORWARD,	&tape.symbol.fast_forward },
    },
    {
      { IMG_GFX_TAPE_LABEL_WARP_FORWARD,	&tape.label.warp_forward  },
      { IMG_GFX_TAPE_SYMBOL_WARP_FORWARD,	&tape.symbol.warp_forward },
    },
    {
      { IMG_GFX_TAPE_LABEL_WARP_FORWARD_BLIND,	&tape.label.warp_forward_blind},
      { IMG_GFX_TAPE_SYMBOL_WARP_FORWARD_BLIND, &tape.symbol.warp_forward_blind},
    },
    {
      { IMG_GFX_TAPE_LABEL_PAUSE_BEFORE_END,	&tape.label.pause_before_end  },
      { IMG_GFX_TAPE_SYMBOL_PAUSE_BEFORE_END,	&tape.symbol.pause_before_end },
    },
    {
      { IMG_GFX_TAPE_LABEL_SINGLE_STEP,		&tape.label.single_step	 },
      { IMG_GFX_TAPE_SYMBOL_SINGLE_STEP,	&tape.symbol.single_step },
    },
  };

  for (k = 0; k < NUM_TAPE_FUNCTION_STATES; k++)	// on or off states
  {
    for (i = 0; i < NUM_TAPE_FUNCTIONS; i++)		// record, play, ...
    {
      for (j = 0; j < NUM_TAPE_FUNCTION_PARTS; j++)	// label or symbol
      {
	int graphic = video_pos[i][j].graphic;
	struct XY *pos = video_pos[i][j].pos;

	if (graphic == -1 ||
	    pos->x == -1 ||
	    pos->y == -1)
	  continue;

	if (state & (1 << (i * 2 + k)))
	{
	  struct GraphicInfo *gfx_bg = &graphic_info[IMG_BACKGROUND_TAPE];
	  struct GraphicInfo *gfx = &graphic_info[graphic];
	  Bitmap *gd_bitmap;
	  int gd_x, gd_y;
	  int skip_value =
	    (j == 0 ? VIDEO_DISPLAY_SYMBOL_ONLY : VIDEO_DISPLAY_LABEL_ONLY);

	  if (value == skip_value)
	    continue;

	  if (k == 1)		// on
	  {
	    gd_bitmap = gfx->bitmap;
	    gd_x = gfx->src_x;
	    gd_y = gfx->src_y;
	  }
	  else			// off
	  {
	    gd_bitmap = gfx_bg->bitmap;
	    gd_x = gfx_bg->src_x + pos->x;
	    gd_y = gfx_bg->src_y + pos->y;
	  }

	  // some tape graphics may be undefined -- only draw if defined
	  if (gd_bitmap != NULL)
	    BlitBitmap(gd_bitmap, drawto, gd_x, gd_y, gfx->width, gfx->height,
		       VX + pos->x, VY + pos->y);

	  redraw_mask |= REDRAW_DOOR_2;
	}
      }
    }
  }
}


#define DATETIME_NONE			(0)

#define DATETIME_DATE_YYYY		(1 << 0)
#define DATETIME_DATE_YY		(1 << 1)
#define DATETIME_DATE_MON		(1 << 2)
#define DATETIME_DATE_MM		(1 << 3)
#define DATETIME_DATE_DD		(1 << 4)

#define DATETIME_TIME_HH		(1 << 5)
#define DATETIME_TIME_MIN		(1 << 6)
#define DATETIME_TIME_MM		(1 << 7)
#define DATETIME_TIME_SS		(1 << 8)

#define DATETIME_FRAME			(1 << 9)

#define DATETIME_XOFFSET_1		(1 << 10)
#define DATETIME_XOFFSET_2		(1 << 11)

#define DATETIME_DATE			(DATETIME_DATE_YYYY	|	\
					 DATETIME_DATE_YY	|	\
					 DATETIME_DATE_MON	|	\
					 DATETIME_DATE_MM	|	\
					 DATETIME_DATE_DD)

#define DATETIME_TIME			(DATETIME_TIME_HH	|	\
					 DATETIME_TIME_MIN	|	\
					 DATETIME_TIME_MM	|	\
					 DATETIME_TIME_SS)

#define MAX_DATETIME_STRING_SIZE	32

static void DrawVideoDisplay_DateTime(unsigned int state, unsigned int value)
{
  int i;

  static char *month_shortnames[] =
  {
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
  };

  static struct
  {
    struct TextPosInfo *pos;
    int type;
  }
  datetime_info[] =
  {
    { &tape.text.date,		DATETIME_DATE_DD			},
    { &tape.text.date,		DATETIME_DATE_MON | DATETIME_XOFFSET_1	},
    { &tape.text.date,		DATETIME_DATE_YY  | DATETIME_XOFFSET_2	},
    { &tape.text.date_yyyy,	DATETIME_DATE_YYYY			},
    { &tape.text.date_yy,	DATETIME_DATE_YY			},
    { &tape.text.date_mon,	DATETIME_DATE_MON			},
    { &tape.text.date_mm,	DATETIME_DATE_MM			},
    { &tape.text.date_dd,	DATETIME_DATE_DD			},

    { &tape.text.time,		DATETIME_TIME_MIN			},
    { &tape.text.time,		DATETIME_TIME_SS  | DATETIME_XOFFSET_1	},
    { &tape.text.time_hh,	DATETIME_TIME_HH			},
    { &tape.text.time_mm,	DATETIME_TIME_MM			},
    { &tape.text.time_ss,	DATETIME_TIME_SS			},

    { &tape.text.frame,		DATETIME_FRAME				},

    { NULL,			DATETIME_NONE				},
  };

  for (i = 0; datetime_info[i].pos != NULL; i++)
  {
    struct TextPosInfo *pos = datetime_info[i].pos;
    int type = datetime_info[i].type;
    int xpos, ypos;

    if (pos->x == -1 &&
	pos->y == -1)
      continue;

    xpos = VX + pos->x + (type & DATETIME_XOFFSET_1 ? pos->xoffset  :
			  type & DATETIME_XOFFSET_2 ? pos->xoffset2 : 0);
    ypos = VY + pos->y;

    if ((type & DATETIME_DATE) && (state & VIDEO_STATE_DATE_ON))
    {
      char s[MAX_DATETIME_STRING_SIZE];
      int year2 = value / 10000;
      int year4 = (year2 < 70 ? 2000 + year2 : 1900 + year2);
      int month_index_raw = (value / 100) % 100;
      int month_index = month_index_raw % 12;	// prevent invalid index
      int month = month_index + 1;
      int day = value % 100;

      strcpy(s, (type & DATETIME_DATE_YYYY ? int2str(year4, 4) :
		 type & DATETIME_DATE_YY   ? int2str(year2, 2) :
		 type & DATETIME_DATE_MON  ? month_shortnames[month_index] :
		 type & DATETIME_DATE_MM   ? int2str(month, 2) :
		 type & DATETIME_DATE_DD   ? int2str(day, 2) : ""));

      DrawText(xpos, ypos, s, pos->font);
    }
    else if ((type & DATETIME_TIME) && (state & VIDEO_STATE_TIME_ON))
    {
      char s[MAX_DATETIME_STRING_SIZE];
      int hh = (value / 3600) % 100;
      int min = value / 60;
      int mm = (value / 60) % 60;
      int ss = value % 60;

      strcpy(s, (type & DATETIME_TIME_HH  ? int2str(hh, 2) :
		 type & DATETIME_TIME_MIN ? int2str(min, 2) :
		 type & DATETIME_TIME_MM  ? int2str(mm, 2) :
		 type & DATETIME_TIME_SS  ? int2str(ss, 2) : ""));

      DrawText(xpos, ypos, s, pos->font);
    }
    else if ((type & DATETIME_FRAME) && (state & VIDEO_STATE_FRAME_ON))
    {
      DrawText(xpos, ypos, int2str(value, pos->size), pos->font);
    }
  }
}

void DrawVideoDisplay(unsigned int state, unsigned int value)
{
  DrawVideoDisplay_Graphics(state, value);
  DrawVideoDisplay_DateTime(state, value);
}

static void DrawVideoDisplayLabel(unsigned int state)
{
  DrawVideoDisplay(state, VIDEO_DISPLAY_LABEL_ONLY);
}

static void DrawVideoDisplaySymbol(unsigned int state)
{
  DrawVideoDisplay(state, VIDEO_DISPLAY_SYMBOL_ONLY);
}

static void DrawVideoDisplayCurrentState(void)
{
  int state = 0;

  DrawVideoDisplay(VIDEO_STATE_OFF, 0);

  if (tape.pausing)
    state |= VIDEO_STATE_PAUSE_ON;

  if (tape.recording)
  {
    state |= VIDEO_STATE_REC_ON;

    if (tape.single_step)
      state |= VIDEO_STATE_1STEP_ON;
  }
  else if (tape.playing)
  {
    state |= VIDEO_STATE_PLAY_ON;

    if (!tape.pausing)
    {
      if (tape.deactivate_display)
	state |= VIDEO_STATE_WARP2_ON;
      else if (tape.warp_forward)
	state |= VIDEO_STATE_WARP_ON;
      else if (tape.fast_forward)
	state |= VIDEO_STATE_FFWD_ON;

      if (tape.pause_before_end)
	state |= VIDEO_STATE_PBEND_ON;
    }
  }

  // draw labels and symbols separately to prevent labels overlapping symbols
  DrawVideoDisplayLabel(state);
  DrawVideoDisplaySymbol(state);
}

void DrawCompleteVideoDisplay(void)
{
  struct GraphicInfo *g_tape = &graphic_info[IMG_BACKGROUND_TAPE];

  // draw tape background
  BlitBitmap(g_tape->bitmap, drawto, g_tape->src_x, g_tape->src_y,
	     gfx.vxsize, gfx.vysize, gfx.vx, gfx.vy);

  // draw tape buttons (forced)
  RedrawOrRemapTapeButtons();

  DrawVideoDisplay(VIDEO_ALL_OFF, 0);

  if (tape.recording)
  {
    DrawVideoDisplay(VIDEO_STATE_REC_ON, 0);
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, tape.length_frames);

    if (tape.pausing)
      DrawVideoDisplay(VIDEO_STATE_PAUSE_ON, 0);
  }
  else if (tape.playing)
  {
    DrawVideoDisplay(VIDEO_STATE_PLAY_ON, 0);
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, 0);

    if (tape.pausing)
      DrawVideoDisplay(VIDEO_STATE_PAUSE_ON, 0);
  }
  else if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, tape.length_frames);
  }

  BlitBitmap(drawto, bitmap_db_door_2, gfx.vx, gfx.vy, gfx.vxsize, gfx.vysize,
	     0, 0);
}

void TapeDeactivateDisplayOn(void)
{
  SetDrawDeactivationMask(REDRAW_FIELD);
  audio.sound_deactivated = TRUE;
}

void TapeDeactivateDisplayOff(boolean redraw_display)
{
  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;

  if (redraw_display)
  {
    RedrawPlayfield();

    UpdateGameDoorValues();
    DrawGameDoorValues();
  }
}


// ============================================================================
// tape logging functions
// ============================================================================

struct AutoPlayInfo
{
  LevelDirTree *leveldir;
  boolean all_levelsets;
  int last_level_nr;
  int level_nr;
  int num_levels_played;
  int num_levels_solved;
  int num_tapes_patched;
  int num_tape_missing;
  boolean level_failed[MAX_TAPES_PER_SET];
  char *tape_filename;
};

static char tape_patch_info[MAX_OUTPUT_LINESIZE];

static void PrintTapeReplayHeader(struct AutoPlayInfo *autoplay)
{
  PrintLine("=", 79);

  if (global.autoplay_mode == AUTOPLAY_MODE_FIX)
    Print("Automatically fixing level tapes\n");
  else if (global.autoplay_mode == AUTOPLAY_MODE_UPLOAD)
    Print("Automatically uploading level tapes\n");
  else
    Print("Automatically playing level tapes\n");

  PrintLine("-", 79);
  Print("Level series identifier: '%s'\n", autoplay->leveldir->identifier);
  Print("Level series name:       '%s'\n", autoplay->leveldir->name);
  Print("Level series author:     '%s'\n", autoplay->leveldir->author);
  Print("Number of levels:        %d\n",   autoplay->leveldir->levels);
  PrintLine("=", 79);
  Print("\n");

  DrawInitTextItem(autoplay->leveldir->name);
}

static void PrintTapeReplayProgress(boolean replay_finished)
{
  static unsigned int counter_last = -1;
  unsigned int counter = Counter();
  unsigned int counter_seconds  = counter / 1000;

  if (!replay_finished)
  {
    unsigned int counter_delay = 50;

    if (counter > counter_last + counter_delay)
    {
      PrintNoLog("\r");
      PrintNoLog("Tape %03d %s[%02d:%02d]: [%02d:%02d] - playing tape ... ",
		 level_nr,  tape_patch_info,
		 tape.length_seconds / 60, tape.length_seconds % 60,
		 TapeTime / 60, TapeTime % 60);

      counter_last = counter;
    }
  }
  else
  {
    float tape_length_seconds = GetTapeLengthSecondsFloat();

    PrintNoLog("\r");
    Print("Tape %03d %s[%02d:%02d]: (%02d:%02d.%03d / %.2f %%) - %s.\n",
	  level_nr, tape_patch_info,
	  tape.length_seconds / 60, tape.length_seconds % 60,
	  counter_seconds / 60, counter_seconds % 60, counter % 1000,
	  (float)counter / tape_length_seconds / 10,
	  tape.auto_play_level_fixed ? "solved and fixed" :
	  tape.auto_play_level_solved ? "solved" :
	  tape.auto_play_level_not_fixable ? "NOT SOLVED, NOT FIXED" :
	  "NOT SOLVED");

    counter_last = -1;
  }
}

static void PrintTapeReplaySummary(struct AutoPlayInfo *autoplay)
{
  char *autoplay_status =
    (autoplay->num_levels_played == autoplay->num_levels_solved &&
     autoplay->num_levels_played > 0 ? " OK " : "WARN");
  int autoplay_percent =
    (autoplay->num_levels_played ?
     autoplay->num_levels_solved * 100 / autoplay->num_levels_played : 0);
  int i;

  Print("\n");
  PrintLine("=", 79);
  Print("Number of levels played: %d\n", autoplay->num_levels_played);
  Print("Number of levels solved: %d (%d%%)\n", autoplay->num_levels_solved,
	(autoplay->num_levels_played ?
	 autoplay->num_levels_solved * 100 / autoplay->num_levels_played : 0));
  if (global.autoplay_mode == AUTOPLAY_MODE_FIX)
    Print("Number of tapes fixed: %d\n", autoplay->num_tapes_patched);
  PrintLine("-", 79);
  Print("Summary (for automatic parsing by scripts):\n");

  if (autoplay->tape_filename)
  {
    Print("TAPEFILE [%s] '%s', %d, %d, %d",
	  autoplay_status,
	  autoplay->leveldir->identifier,
	  autoplay->last_level_nr,
	  game.score_final,
	  game.score_time_final);
  }
  else
  {
    Print("LEVELDIR [%s] '%s', SOLVED %d/%d (%d%%)",
	  autoplay_status,
	  autoplay->leveldir->identifier,
	  autoplay->num_levels_solved,
	  autoplay->num_levels_played,
	  autoplay_percent);

    if (autoplay->num_levels_played != autoplay->num_levels_solved)
    {
      Print(", FAILED:");
      for (i = 0; i < MAX_TAPES_PER_SET; i++)
	if (autoplay->level_failed[i])
	  Print(" %03d", i);
    }
  }

  Print("\n");
  PrintLine("=", 79);
}

static FILE *tape_log_file;

static void OpenTapeLogfile(void)
{
  if (!(tape_log_file = fopen(options.tape_log_filename, MODE_WRITE)))
    Warn("cannot write tape logfile '%s'", options.tape_log_filename);
}

static void WriteTapeLogfile(byte action[MAX_TAPE_ACTIONS])
{
  int i;

  for (i = 0; i < MAX_TAPE_ACTIONS; i++)
    putFile8Bit(tape_log_file, action[i]);
}

static void CloseTapeLogfile(void)
{
  fclose(tape_log_file);
}


// ============================================================================
// tape control functions
// ============================================================================

void TapeSetDateFromEpochSeconds(time_t epoch_seconds)
{
  struct tm *lt = localtime(&epoch_seconds);

  tape.date = 10000 * (lt->tm_year % 100) + 100 * lt->tm_mon + lt->tm_mday;
}

void TapeSetDateFromNow(void)
{
  TapeSetDateFromEpochSeconds(time(NULL));
}

void TapeErase(void)
{
  int i;

  CopyTapeToUndoBuffer();

  tape.counter = 0;
  tape.length = 0;
  tape.length_frames = 0;
  tape.length_seconds = 0;

  tape.score_tape_basename[0] = '\0';

  if (leveldir_current)
  {
    strncpy(tape.level_identifier, leveldir_current->identifier,
	    MAX_FILENAME_LEN);
    tape.level_identifier[MAX_FILENAME_LEN] = '\0';
  }

  tape.level_nr = level_nr;
  tape.pos[tape.counter].delay = 0;
  tape.changed = TRUE;
  tape.solved = FALSE;

  tape.random_seed = InitRND(level.random_seed);

  tape.file_version = FILE_VERSION_ACTUAL;
  tape.game_version = GAME_VERSION_ACTUAL;
  tape.engine_version = level.game_version;

  tape.property_bits = TAPE_PROPERTY_NONE;

  TapeSetDateFromNow();

  for (i = 0; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  tape.centered_player_nr_next = -1;
  tape.set_centered_player = FALSE;

  // set flags for game actions to default values (may be overwritten later)
  tape.use_key_actions = TRUE;
  tape.use_mouse_actions = FALSE;
}

static void TapeRewind(void)
{
  tape.counter = 0;
  tape.delay_played = 0;
  tape.pause_before_end = FALSE;
  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.deactivate_display = FALSE;
  tape.auto_play = (global.autoplay_leveldir != NULL);
  tape.auto_play_level_solved = FALSE;
  tape.auto_play_level_fixed = FALSE;
  tape.auto_play_level_not_fixable = FALSE;
  tape.quick_resume = FALSE;
  tape.single_step = FALSE;

  tape.centered_player_nr_next = -1;
  tape.set_centered_player = FALSE;

  InitRND(tape.random_seed);
}

static void TapeSetRandomSeed(int random_seed)
{
  tape.random_seed = InitRND(random_seed);
}

void TapeStartRecording(int random_seed)
{
  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  TapeErase();
  TapeRewind();
  TapeSetRandomSeed(random_seed);

  tape.recording = TRUE;

  DrawVideoDisplay(VIDEO_STATE_REC_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_FRAME_ON, 0);

  MapTapeWarpButton();

  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;

  // required here to update video display if tape door is closed
  if (GetDoorState() & DOOR_CLOSE_2)
    OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);
}

static void TapeStartGameRecording(void)
{
  StartGameActions(network.enabled, TRUE, level.random_seed);
}

static void TapeAppendRecording(void)
{
  if (!tape.playing || !tape.pausing)
    return;

  // stop playing
  tape.playing = FALSE;
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.pause_before_end = FALSE;
  tape.deactivate_display = FALSE;

  // start recording
  tape.recording = TRUE;
  tape.changed = TRUE;
  tape.solved = FALSE;

  // set current delay (for last played move)
  tape.pos[tape.counter].delay = tape.delay_played;

  tape.property_bits |= TAPE_PROPERTY_REPLAYED;

  // set current date
  TapeSetDateFromNow();

  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF | VIDEO_STATE_REC_ON, 0);

  UpdateAndDisplayGameControlValues();
}

void TapeHaltRecording(void)
{
  // only advance tape counter if any input events have been recorded
  if (tape.pos[tape.counter].delay > 0)
    tape.counter++;

  // initialize delay for next tape entry (to be able to continue recording)
  if (tape.counter < MAX_TAPE_LEN)
    tape.pos[tape.counter].delay = 0;

  tape.length = tape.counter;
  tape.length_frames = GetTapeLengthFrames();
  tape.length_seconds = GetTapeLengthSeconds();
}

void TapeStopRecording(void)
{
  if (tape.recording)
    TapeHaltRecording();

  tape.recording = FALSE;
  tape.pausing = FALSE;

  DrawVideoDisplay(VIDEO_STATE_REC_OFF, 0);
  MapTapeEjectButton();
}

boolean TapeAddAction(byte action[MAX_TAPE_ACTIONS])
{
  int i;

  if (tape.pos[tape.counter].delay > 0)		// already stored action
  {
    boolean changed_events = FALSE;

    for (i = 0; i < MAX_TAPE_ACTIONS; i++)
      if (tape.pos[tape.counter].action[i] != action[i])
	changed_events = TRUE;

    if (changed_events || tape.pos[tape.counter].delay >= 255)
    {
      if (tape.counter >= MAX_TAPE_LEN - 1)
	return FALSE;

      tape.counter++;
      tape.pos[tape.counter].delay = 0;
    }
    else
      tape.pos[tape.counter].delay++;
  }

  if (tape.pos[tape.counter].delay == 0)	// store new action
  {
    for (i = 0; i < MAX_TAPE_ACTIONS; i++)
      tape.pos[tape.counter].action[i] = action[i];

    tape.pos[tape.counter].delay++;
  }

  tape.changed = TRUE;

  return TRUE;
}

void TapeRecordAction(byte action_raw[MAX_TAPE_ACTIONS])
{
  byte action[MAX_TAPE_ACTIONS];
  int i;

  if (!tape.recording)		// (record action even when tape is paused)
    return;

  for (i = 0; i < MAX_TAPE_ACTIONS; i++)
    action[i] = action_raw[i];

  if (tape.use_key_actions && tape.set_centered_player)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      if (tape.centered_player_nr_next == i ||
	  tape.centered_player_nr_next == -1)
	action[i] |= KEY_SET_FOCUS;

    tape.set_centered_player = FALSE;
  }

  if (GameFrameDelay != GAME_FRAME_DELAY)
    tape.property_bits |= TAPE_PROPERTY_GAME_SPEED;

  if (setup.small_game_graphics || SCR_FIELDX >= 2 * SCR_FIELDX_DEFAULT)
    tape.property_bits |= TAPE_PROPERTY_SMALL_GRAPHICS;

  if (!TapeAddAction(action))
    TapeStopRecording();
}

void TapeTogglePause(boolean toggle_mode)
{
  if (tape.playing && tape.pausing && (toggle_mode & TAPE_TOGGLE_PLAY_PAUSE))
  {
    // continue playing in normal mode
    tape.fast_forward = FALSE;
    tape.warp_forward = FALSE;
    tape.deactivate_display = FALSE;

    tape.pause_before_end = FALSE;
  }

  tape.pausing = !tape.pausing;

  if (tape.single_step && (toggle_mode & TAPE_TOGGLE_MANUAL))
    tape.single_step = FALSE;

  if (tape.single_step)
    tape.property_bits |= TAPE_PROPERTY_SINGLE_STEP;

  if (tape.pausing)
    tape.property_bits |= TAPE_PROPERTY_PAUSE_MODE;

  DrawVideoDisplayCurrentState();

  if (tape.deactivate_display)
  {
    if (tape.pausing)
      TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);
    else
      TapeDeactivateDisplayOn();
  }

  if (tape.quick_resume)
  {
    tape.quick_resume = FALSE;

    TapeStopWarpForward();
    TapeAppendRecording();

    if (!CheckEngineSnapshotSingle())
      SaveEngineSnapshotSingle();

    // restart step/move snapshots after quick loading tape
    SaveEngineSnapshotToListInitial();

    // do not map undo/redo buttons after quick loading tape
    return;
  }

  if (game_status == GAME_MODE_PLAYING)
  {
    if (setup.show_load_save_buttons &&
	setup.show_undo_redo_buttons &&
	CheckEngineSnapshotList())
    {
      if (tape.pausing)
	MapUndoRedoButtons();
      else if (!tape.single_step)
	MapLoadSaveButtons();
    }

    ModifyPauseButtons();
  }

  // stop tape when leaving auto-pause after completely replaying tape
  if (tape.playing && !tape.pausing && tape.counter >= tape.length)
    TapeStop();
}

void TapeStartPlaying(void)
{
  if (TAPE_IS_EMPTY(tape))
    return;

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  TapeRewind();

  tape.playing = TRUE;

  DrawVideoDisplay(VIDEO_STATE_PLAY_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_FRAME_ON, 0);

  MapTapeWarpButton();

  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;
}

static void TapeStartGamePlaying(void)
{
  TapeStartPlaying();

  InitGame();
}

void TapeStopPlaying(void)
{
  tape.playing = FALSE;
  tape.pausing = FALSE;

  if (tape.warp_forward)
    TapeStopWarpForward();

  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF, 0);
  MapTapeEjectButton();
}

byte *TapePlayAction(void)
{
  int update_delay = FRAMES_PER_SECOND / 2;
  boolean update_video_display = (FrameCounter % update_delay == 0);
  boolean update_draw_label_on = ((FrameCounter / update_delay) % 2 == 1);
  static byte action[MAX_TAPE_ACTIONS];
  int i;

  if (!tape.playing || tape.pausing)
    return NULL;

  if (tape.pause_before_end)  // stop some seconds before end of tape
  {
    if (TapeTime > (int)tape.length_seconds - TAPE_PAUSE_SECONDS_BEFORE_DEATH)
    {
      TapeStopWarpForward();
      TapeTogglePause(TAPE_TOGGLE_MANUAL);

      if (setup.autorecord_after_replay)
	TapeAppendRecording();

      return NULL;
    }
  }

  if (tape.counter >= tape.length)	// end of tape reached
  {
    if (!tape.auto_play)
    {
      TapeStopWarpForward();
      TapeTogglePause(TAPE_TOGGLE_MANUAL);

      if (setup.autorecord_after_replay)
	TapeAppendRecording();
    }
    else
    {
      TapeStop();
    }

    return NULL;
  }

  if (update_video_display && !tape.deactivate_display)
  {
    int state = 0;

    if (tape.warp_forward)
      state |= VIDEO_STATE_WARP(update_draw_label_on);
    else if (tape.fast_forward)
      state |= VIDEO_STATE_FFWD(update_draw_label_on);

    if (tape.pause_before_end)
      state |= VIDEO_STATE_PBEND(update_draw_label_on);

    // draw labels and symbols separately to prevent labels overlapping symbols
    DrawVideoDisplayLabel(state);
    DrawVideoDisplaySymbol(state);
  }

  for (i = 0; i < MAX_TAPE_ACTIONS; i++)
    action[i] = tape.pos[tape.counter].action[i];

#if DEBUG_TAPE_WHEN_PLAYING
  DebugContinued("", "%05d", FrameCounter);
  for (i = 0; i < MAX_TAPE_ACTIONS; i++)
    DebugContinued("", "   %08x", action[i]);
  DebugContinued("tape:play", "\n");
#endif

  tape.set_centered_player = FALSE;
  tape.centered_player_nr_next = -999;

  if (tape.use_key_actions)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      if (action[i] & KEY_SET_FOCUS)
      {
	tape.set_centered_player = TRUE;
	tape.centered_player_nr_next =
	  (tape.centered_player_nr_next == -999 ? i : -1);
      }

      action[i] &= ~KEY_SET_FOCUS;
    }
  }

  tape.delay_played++;
  if (tape.delay_played >= tape.pos[tape.counter].delay)
  {
    tape.counter++;
    tape.delay_played = 0;
  }

  if (tape.auto_play)
    PrintTapeReplayProgress(FALSE);

  if (options.tape_log_filename != NULL)
    WriteTapeLogfile(action);

  return action;
}

void TapeStop(void)
{
  if (tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_MANUAL);

  TapeStopRecording();
  TapeStopPlaying();

  DrawVideoDisplay(VIDEO_STATE_OFF, 0);

  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, tape.length_frames);
  }
}

static void TapeStopGameOrTape(boolean stop_game)
{
  if (score_info_tape_play || (!tape.playing && stop_game))
    RequestQuitGame(FALSE);
  else
    TapeStop();
}

void TapeStopGame(void)
{
  if (game_status == GAME_MODE_MAIN)
    return;

  TapeStopGameOrTape(TRUE);
}

void TapeStopTape(void)
{
  TapeStopGameOrTape(FALSE);
}

unsigned int GetTapeLengthFrames(void)
{
  unsigned int tape_length_frames = 0;
  int i;

  if (TAPE_IS_EMPTY(tape))
    return 0;

  for (i = 0; i < tape.length; i++)
    tape_length_frames += tape.pos[i].delay;

  return tape_length_frames;
}

unsigned int GetTapeLengthSeconds(void)
{
  return (GetTapeLengthFrames() * GAME_FRAME_DELAY / 1000);
}

static float GetTapeLengthSecondsFloat(void)
{
  return ((float)GetTapeLengthFrames() * GAME_FRAME_DELAY / 1000);
}

static void TapeStartWarpForward(int mode)
{
  tape.fast_forward = (mode & AUTOPLAY_FFWD);
  tape.warp_forward = (mode & AUTOPLAY_WARP);
  tape.deactivate_display = (mode & AUTOPLAY_WARP_NO_DISPLAY);

  tape.pausing = FALSE;

  if (tape.deactivate_display)
    TapeDeactivateDisplayOn();

  DrawVideoDisplayCurrentState();
}

static void TapeStopWarpForward(void)
{
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.deactivate_display = FALSE;

  tape.pause_before_end = FALSE;

  TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);

  DrawVideoDisplayCurrentState();
}

static void TapeSingleStep(void)
{
  if (network.enabled)
    return;

  if (!tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_MANUAL);

  tape.single_step = !tape.single_step;

  DrawVideoDisplay(VIDEO_STATE_1STEP(tape.single_step), 0);
}

void TapeQuickSave(void)
{
  if (game_status != GAME_MODE_PLAYING)
  {
    Request("No game that could be saved!", REQ_CONFIRM);

    return;
  }

  if (!tape.recording)
  {
    Request("No recording that could be saved!", REQ_CONFIRM);

    return;
  }

  TapeHaltRecording();		// prepare tape for saving on-the-fly

  if (TAPE_IS_EMPTY(tape))
  {
    Request("No tape that could be saved!", REQ_CONFIRM);

    return;
  }

  tape.property_bits |= TAPE_PROPERTY_SNAPSHOT;

  if (SaveTapeChecked(tape.level_nr))
    SaveEngineSnapshotSingle();
}

void TapeQuickLoad(void)
{
  char *filename = getTapeFilename(level_nr);

  if (!fileExists(filename))
  {
    Request("No tape for this level!", REQ_CONFIRM);

    return;
  }

  if (tape.recording && !Request("Stop recording and load tape?",
				 REQ_ASK | REQ_STAY_CLOSED))
  {
    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

    return;
  }

  if (game_status != GAME_MODE_PLAYING && game_status != GAME_MODE_MAIN)
    return;

  if (CheckEngineSnapshotSingle())
  {
    TapeStartGamePlaying();

    LoadEngineSnapshotSingle();

    DrawCompleteVideoDisplay();

    tape.playing = TRUE;
    tape.pausing = TRUE;

    TapeStopWarpForward();
    TapeAppendRecording();

    // restart step/move snapshots after quick loading tape
    SaveEngineSnapshotToListInitial();

    if (FrameCounter > 0)
      return;
  }

  TapeStop();
  TapeErase();

  LoadTape(level_nr);

  if (!TAPE_IS_EMPTY(tape))
  {
    TapeStartGamePlaying();
    TapeStartWarpForward(AUTOPLAY_MODE_WARP_NO_DISPLAY);

    tape.quick_resume = TRUE;
    tape.property_bits |= TAPE_PROPERTY_SNAPSHOT;
  }
  else	// this should not happen (basically checked above)
  {
    int reopen_door = (game_status == GAME_MODE_PLAYING ? REQ_REOPEN : 0);

    Request("No tape for this level!", REQ_CONFIRM | reopen_door);
  }
}

static boolean checkRestartGame(char *message)
{
  if (game_status == GAME_MODE_MAIN)
    return TRUE;

  if (!hasStartedNetworkGame())
    return FALSE;

  if (level_editor_test_game)
    return TRUE;

  if (game.all_players_gone)
    return TRUE;

  if (!setup.ask_on_quit_game)
    return TRUE;

  if (Request(message, REQ_ASK | REQ_STAY_CLOSED))
    return TRUE;

  OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

  return FALSE;
}

void TapeRestartGame(void)
{
  if (score_info_tape_play)
  {
    TapeStartGamePlaying();

    return;
  }

  if (!checkRestartGame("Restart game?"))
    return;

  StartGameActions(network.enabled, setup.autorecord, level.random_seed);
}

void TapeReplayAndPauseBeforeEnd(void)
{
  if (score_info_tape_play)
    return;

  if (TAPE_IS_EMPTY(tape) && !tape.recording)
  {
    Request("No tape for this level!", REQ_CONFIRM);

    return;
  }

  if (!checkRestartGame("Replay game and pause before end?"))
    return;

  TapeStop();
  TapeStartGamePlaying();
  TapeStartWarpForward(AUTOPLAY_MODE_WARP_NO_DISPLAY);

  tape.pause_before_end = TRUE;
  tape.quick_resume = TRUE;
}

boolean hasSolutionTape(void)
{
  boolean tape_file_exists = fileExists(getSolutionTapeFilename(level_nr));
  boolean level_has_tape = (level.game_engine_type == GAME_ENGINE_TYPE_SP &&
			    level.native_sp_level->demo.is_available);

  return (tape_file_exists || level_has_tape);
}

boolean InsertSolutionTape(void)
{
  if (!hasSolutionTape())
  {
    Request("No solution tape for this level!", REQ_CONFIRM);

    return FALSE;
  }

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  // if tape recorder already contains a tape, remove it without asking
  TapeErase();

  LoadSolutionTape(level_nr);

  DrawCompleteVideoDisplay();

  if (TAPE_IS_EMPTY(tape))
  {
    Request("Loading solution tape for this level failed!", REQ_CONFIRM);

    return FALSE;
  }

  return TRUE;
}

boolean PlaySolutionTape(void)
{
  if (!InsertSolutionTape())
    return FALSE;

  TapeStartGamePlaying();

  return TRUE;
}

static boolean PlayScoreTape_WaitForDownload(void)
{
  DelayCounter download_delay = { 10000 };

  ResetDelayCounter(&download_delay);

  // wait for score tape to be successfully downloaded (and fail on timeout)
  while (!server_scores.tape_downloaded)
  {
    if (DelayReached(&download_delay))
      return FALSE;

    UPDATE_BUSY_STATE_NOT_LOADING();

    Delay(20);
  }

  return TRUE;
}

boolean PlayScoreTape(int entry_nr)
{
  struct ScoreEntry *entry = &scores.entry[entry_nr];
  char *tape_filename =
    (entry->id == -1 ?
     getScoreTapeFilename(entry->tape_basename, level_nr) :
     getScoreCacheTapeFilename(entry->tape_basename, level_nr));
  boolean download_tape = (!fileExists(tape_filename));

  if (download_tape && entry->id == -1)
  {
    FadeSkipNextFadeIn();

    Request("Cannot find score tape!", REQ_CONFIRM);

    return FALSE;
  }

  server_scores.tape_downloaded = FALSE;

  if (download_tape)
    ApiGetScoreTapeAsThread(level_nr, entry->id, entry->tape_basename);

  SetGameStatus(GAME_MODE_PLAYING);

  FadeOut(REDRAW_FIELD);

  if (download_tape && !PlayScoreTape_WaitForDownload())
  {
    SetGameStatus(GAME_MODE_SCOREINFO);
    ClearField();

    Request("Cannot download score tape from score server!", REQ_CONFIRM);

    return FALSE;
  }

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  // if tape recorder already contains a tape, remove it without asking
  TapeErase();

  if (entry->id == -1)
    LoadScoreTape(entry->tape_basename, level_nr);
  else
    LoadScoreCacheTape(entry->tape_basename, level_nr);

  if (TAPE_IS_EMPTY(tape))
  {
    SetGameStatus(GAME_MODE_SCOREINFO);
    ClearField();

    Request("Cannot load score tape for this level!", REQ_CONFIRM);

    return FALSE;
  }

  FadeSkipNextFadeOut();

  TapeStartGamePlaying();

  score_info_tape_play = TRUE;

  return TRUE;
}

static boolean checkTapesFromSameLevel(struct TapeInfo *t1, struct TapeInfo *t2)
{
  return (strEqual(t1->level_identifier, t2->level_identifier) &&
	  t1->level_nr == t2->level_nr);
}

static void CopyTape(struct TapeInfo *tape_from, struct TapeInfo *tape_to)
{
  *tape_to = *tape_from;
}

static void SwapTapes(struct TapeInfo *t1, struct TapeInfo *t2)
{
  struct TapeInfo tmp = *t1;

  *t1 = *t2;
  *t2 = tmp;
}

static void CopyTapeToUndoBuffer(void)
{
  // copy tapes to undo buffer if large enough (or larger than last undo tape)
  // or if the last undo tape is from a different level set or level number
  if (tape.length_seconds >= TAPE_MIN_SECONDS_FOR_UNDO_BUFFER ||
      tape.length_seconds >= tape_undo_buffer.length_seconds ||
      !checkTapesFromSameLevel(&tape, &tape_undo_buffer))
  {
    CopyTape(&tape, &tape_undo_buffer);
  }
}

void UndoTape(void)
{
  // only undo tapes from same level set and with same level number
  if (!checkTapesFromSameLevel(&tape, &tape_undo_buffer))
    return;

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  // swap last recorded tape with undo buffer, so undo can be reversed
  SwapTapes(&tape, &tape_undo_buffer);

  DrawCompleteVideoDisplay();
}

void FixTape_ForceSinglePlayer(void)
{
  int i;

  /* fix single-player tapes that contain player input for more than one
     player (due to a bug in 3.3.1.2 and earlier versions), which results
     in playing levels with more than one player in multi-player mode,
     even though the tape was originally recorded in single-player mode */

  // remove player input actions for all players but the first one
  for (i = 1; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  tape.changed = TRUE;
}


// ----------------------------------------------------------------------------
// tape autoplay functions
// ----------------------------------------------------------------------------

static TreeInfo *getNextValidAutoPlayEntry(TreeInfo *node)
{
  node = getNextValidTreeInfoEntry(node);

  while (node && node->is_copy)
    node = getNextValidTreeInfoEntry(node);

  return node;
}

static TreeInfo *getFirstValidAutoPlayEntry(TreeInfo *node)
{
  node = getFirstValidTreeInfoEntry(node);

  if (node && node->is_copy)
    return getNextValidAutoPlayEntry(node);

  return node;
}

static void AutoPlayTapes_SetScoreEntry(int score, int time)
{
  char *name = (options.mytapes ? setup.player_name : options.player_name);

  // set unique basename for score tape (for uploading to score server)
  strcpy(tape.score_tape_basename, getScoreTapeBasename(name));

  // store score in first score entry
  scores.last_added = 0;

  struct ScoreEntry *entry = &scores.entry[scores.last_added];

  strncpy(entry->tape_basename, tape.score_tape_basename, MAX_FILENAME_LEN);
  strncpy(entry->name, setup.player_name, MAX_PLAYER_NAME_LEN);

  entry->score = score;
  entry->time = time;

  PrintNoLog("- uploading score tape to score server ... ");

  server_scores.uploaded = FALSE;
}

static boolean AutoPlayTapes_WaitForUpload(void)
{
  DelayCounter upload_delay = { 10000 };

  ResetDelayCounter(&upload_delay);

  // wait for score tape to be successfully uploaded (and fail on timeout)
  while (!server_scores.uploaded)
  {
    if (DelayReached(&upload_delay))
    {
      PrintNoLog("\r");
      Print("- uploading score tape to score server - TIMEOUT.\n");

      if (program.headless)
	Fail("cannot upload score tape to score server");

      return FALSE;
    }

    UPDATE_BUSY_STATE();

    Delay(20);
  }

  PrintNoLog("\r");
  Print("- uploading score tape to score server - uploaded.\n");

  return TRUE;
}

static int AutoPlayTapesExt(boolean initialize)
{
  static struct AutoPlayInfo autoplay;
  static int num_tapes = 0;
  static int patch_nr = 0;
  static char *patch_name[] =
  {
    "original tape",
    "em_random_bug",
    "screen_34x34",

    NULL
  };
  static int patch_version_first[] =
  {
    VERSION_IDENT(0,0,0,0),
    VERSION_IDENT(3,3,1,0),
    VERSION_IDENT(0,0,0,0),

    -1
  };
  static int patch_version_last[] =
  {
    VERSION_IDENT(9,9,9,9),
    VERSION_IDENT(4,0,1,1),
    VERSION_IDENT(4,2,2,0),

    -1
  };
  static byte patch_property_bit[] =
  {
    TAPE_PROPERTY_NONE,
    TAPE_PROPERTY_EM_RANDOM_BUG,
    TAPE_PROPERTY_NONE,

    -1
  };
  LevelDirTree *leveldir_current_last = leveldir_current;
  boolean init_level_set = FALSE;
  int level_nr_last = level_nr;
  int i;

  if (!initialize)
  {
    if (global.autoplay_mode == AUTOPLAY_MODE_FIX)
    {
      if (tape.auto_play_level_solved)
      {
	if (patch_nr > 0)
	{
	  // level solved by patched tape -- save fixed tape
	  char *filename = getTapeFilename(level_nr);
	  char *filename_orig = getStringCat2(filename, ".orig");

	  // create backup from old tape, if not yet existing
	  if (!fileExists(filename_orig))
	    rename(filename, filename_orig);

	  SaveTapeToFilename(filename);

	  tape.auto_play_level_fixed = TRUE;
	  autoplay.num_tapes_patched++;
	}

	// continue with next tape
	patch_nr = 0;
      }
      else if (patch_name[patch_nr + 1] != NULL)
      {
	// level not solved by patched tape -- continue with next patch
	patch_nr++;
      }
      else
      {
	// level not solved by any patched tape -- continue with next tape
	tape.auto_play_level_not_fixable = TRUE;
	patch_nr = 0;
      }
    }

    // just finished auto-playing tape
    PrintTapeReplayProgress(TRUE);

    if (options.tape_log_filename != NULL)
      CloseTapeLogfile();

    if (global.autoplay_mode == AUTOPLAY_MODE_SAVE &&
	tape.auto_play_level_solved)
    {
      AutoPlayTapes_SetScoreEntry(game.score_final, game.score_time_final);

      if (leveldir_current)
      {
	// the tape's level set identifier may differ from current level set
	strncpy(tape.level_identifier, leveldir_current->identifier,
		MAX_FILENAME_LEN);
	tape.level_identifier[MAX_FILENAME_LEN] = '\0';

	// the tape's level number may differ from current level number
	tape.level_nr = level_nr;
      }

      // save score tape to upload to server; may be required for some reasons:
      // * level set identifier in solution tapes may differ from level set
      // * level set identifier is missing (old-style tape without INFO chunk)
      // * solution tape may have native format (like Supaplex solution files)

      SaveScoreTape(level_nr);
      SaveServerScore(level_nr, TRUE);

      AutoPlayTapes_WaitForUpload();
    }

    if (patch_nr == 0)
      autoplay.num_levels_played++;

    if (tape.auto_play_level_solved)
      autoplay.num_levels_solved++;

    if (level_nr >= 0 && level_nr < MAX_TAPES_PER_SET)
      autoplay.level_failed[level_nr] = !tape.auto_play_level_solved;
  }
  else
  {
    if (strEqual(global.autoplay_leveldir, "ALL"))
    {
      autoplay.all_levelsets = TRUE;

      // tape mass-uploading only allowed for private tapes
      if (global.autoplay_mode == AUTOPLAY_MODE_UPLOAD)
	options.mytapes = TRUE;
    }

    if ((global.autoplay_mode == AUTOPLAY_MODE_SAVE ||
	 global.autoplay_mode == AUTOPLAY_MODE_UPLOAD) &&
	!options.mytapes &&
	options.player_name == NULL)
    {
      Fail("specify player name when uploading solution tapes");
    }

    if (global.autoplay_mode != AUTOPLAY_MODE_UPLOAD)
      DrawCompleteVideoDisplay();

    if (program.headless)
    {
      audio.sound_enabled = FALSE;
      setup.engine_snapshot_mode = getStringCopy(STR_SNAPSHOT_MODE_OFF);
    }

    if (strSuffix(global.autoplay_leveldir, ".tape"))
    {
      autoplay.tape_filename = global.autoplay_leveldir;

      if (!fileExists(autoplay.tape_filename))
	Fail("tape file '%s' does not exist", autoplay.tape_filename);

      LoadTapeFromFilename(autoplay.tape_filename);

      if (tape.no_valid_file)
	Fail("cannot load tape file '%s'", autoplay.tape_filename);

      if (tape.no_info_chunk && !options.identifier)
	Fail("cannot get levelset from tape file '%s'", autoplay.tape_filename);

      if (tape.no_info_chunk && !options.level_nr)
	Fail("cannot get level nr from tape file '%s'", autoplay.tape_filename);

      global.autoplay_leveldir = tape.level_identifier;

      if (options.identifier != NULL)
	global.autoplay_leveldir = options.identifier;

      if (options.level_nr != NULL)
	tape.level_nr = atoi(options.level_nr);

      if (tape.level_nr >= 0 && tape.level_nr < MAX_TAPES_PER_SET)
        global.autoplay_level[tape.level_nr] = TRUE;

      global.autoplay_all = FALSE;
      options.mytapes = FALSE;
    }

    if (autoplay.all_levelsets)
    {
      // start auto-playing first level set
      autoplay.leveldir = getFirstValidAutoPlayEntry(leveldir_first);
    }
    else
    {
      // auto-play selected level set
      autoplay.leveldir = getTreeInfoFromIdentifier(leveldir_first,
						    global.autoplay_leveldir);
    }

    if (autoplay.leveldir == NULL)
      Fail("no such level identifier: '%s'", global.autoplay_leveldir);

    // only private tapes may be modified
    if (global.autoplay_mode == AUTOPLAY_MODE_FIX)
      options.mytapes = TRUE;

    // set timestamp for batch tape upload
    global.autoplay_time = time(NULL);

    num_tapes = 0;

    init_level_set = TRUE;
  }

  while (1)
  {
    if (init_level_set)
    {
      leveldir_current = autoplay.leveldir;

      if (autoplay.leveldir->first_level < 0)
	autoplay.leveldir->first_level = 0;
      if (autoplay.leveldir->last_level >= MAX_TAPES_PER_SET)
	autoplay.leveldir->last_level = MAX_TAPES_PER_SET - 1;

      autoplay.level_nr = autoplay.leveldir->first_level;

      autoplay.num_levels_played = 0;
      autoplay.num_levels_solved = 0;
      autoplay.num_tapes_patched = 0;
      autoplay.num_tape_missing = 0;

      for (i = 0; i < MAX_TAPES_PER_SET; i++)
	autoplay.level_failed[i] = FALSE;

      PrintTapeReplayHeader(&autoplay);

      init_level_set = FALSE;
    }

    if (autoplay.all_levelsets && global.autoplay_mode == AUTOPLAY_MODE_UPLOAD)
    {
      boolean skip_levelset = FALSE;

      if (!directoryExists(getTapeDir(autoplay.leveldir->subdir)))
      {
	Print("No tape directory for this level set found -- skipping.\n");

	skip_levelset = TRUE;
      }

      if (CheckTapeDirectoryUploadsComplete(autoplay.leveldir->subdir))
      {
	Print("All tapes for this level set already uploaded -- skipping.\n");

	skip_levelset = TRUE;
      }

      if (skip_levelset)
      {
	PrintTapeReplaySummary(&autoplay);

	// continue with next level set
	autoplay.leveldir = getNextValidAutoPlayEntry(autoplay.leveldir);

	// all level sets processed
	if (autoplay.leveldir == NULL)
	  break;

	init_level_set = TRUE;

	continue;
      }
    }

    if (global.autoplay_mode != AUTOPLAY_MODE_FIX || patch_nr == 0)
      level_nr = autoplay.level_nr++;

    UPDATE_BUSY_STATE();

    // check if all tapes for this level set have been processed
    if (level_nr > autoplay.leveldir->last_level)
    {
      PrintTapeReplaySummary(&autoplay);

      if (!autoplay.all_levelsets)
	break;

      if (global.autoplay_mode == AUTOPLAY_MODE_UPLOAD)
	MarkTapeDirectoryUploadsAsComplete(autoplay.leveldir->subdir);

      // continue with next level set
      autoplay.leveldir = getNextValidAutoPlayEntry(autoplay.leveldir);

      // all level sets processed
      if (autoplay.leveldir == NULL)
	break;

      init_level_set = TRUE;

      continue;
    }

    // set patch info (required for progress output)
    strcpy(tape_patch_info, "");
    if (global.autoplay_mode == AUTOPLAY_MODE_FIX)
      snprintf(tape_patch_info, MAX_OUTPUT_LINESIZE, "[%-13s] ",
	       patch_name[patch_nr]);

    if (!global.autoplay_all && !global.autoplay_level[level_nr])
      continue;

    // speed things up in case of missing private tapes (skip loading level)
    if (options.mytapes && !fileExists(getTapeFilename(level_nr)))
    {
      autoplay.num_tape_missing++;

      Print("Tape %03d: (no tape found)\n", level_nr);

      continue;
    }

    TapeErase();
    TapeRewind();	// needed to reset "tape.auto_play_level_solved"

    LoadLevel(level_nr);

    if (level.no_level_file || level.no_valid_file)
    {
      Print("Tape %03d: (no level found)\n", level_nr);

      continue;
    }

#if 0
    // ACTIVATE THIS FOR LOADING/TESTING OF LEVELS ONLY
    Print("Tape %03d: (only testing level)\n", level_nr);
    continue;
#endif

    if (autoplay.tape_filename)
      LoadTapeFromFilename(autoplay.tape_filename);
    else if (options.mytapes)
      LoadTape(level_nr);
    else
      LoadSolutionTape(level_nr);

    if (tape.no_valid_file)
    {
      autoplay.num_tape_missing++;

      Print("Tape %03d: (no tape found)\n", level_nr);

      continue;
    }

    if (global.autoplay_mode == AUTOPLAY_MODE_FIX)
    {
      boolean skip_patch = FALSE;

      if (tape.engine_version < patch_version_first[patch_nr] ||
	  tape.engine_version > patch_version_last[patch_nr])
      {
	Print("Tape %03d %s[%02d:%02d]: (%s %d.%d.%d.%d) - skipped.\n",
	      level_nr,  tape_patch_info,
	      tape.length_seconds / 60, tape.length_seconds % 60,
	      "not suitable for version",
	      (tape.engine_version / 1000000) % 100,
	      (tape.engine_version / 10000  ) % 100,
	      (tape.engine_version / 100    ) % 100,
	      (tape.engine_version          ) % 100);

	skip_patch = TRUE;
      }

      if (strEqual(patch_name[patch_nr], "screen_34x34") &&
	  tape.num_participating_players == 1)
      {
	Print("Tape %03d %s[%02d:%02d]: (%s) - skipped.\n",
	      level_nr,  tape_patch_info,
	      tape.length_seconds / 60, tape.length_seconds % 60,
	      "not suitable for single player tapes");

	skip_patch = TRUE;
      }

      if (skip_patch)
      {
	if (patch_name[patch_nr + 1] != NULL)
	{
	  // continue with next patch
	  patch_nr++;
	}
	else
	{
	  // continue with next tape
	  patch_nr = 0;
	}

	continue;
      }

      if (strEqual(patch_name[patch_nr], "screen_34x34"))
      {
	tape.scr_fieldx = SCR_FIELDX_DEFAULT * 2;
	tape.scr_fieldy = SCR_FIELDY_DEFAULT * 2;
      }
      else
      {
	tape.property_bits |= patch_property_bit[patch_nr];
      }
    }

    num_tapes++;

    if (global.autoplay_mode == AUTOPLAY_MODE_UPLOAD)
    {
      boolean use_temporary_tape_file = FALSE;

      Print("Tape %03d:\n", level_nr);

      AutoPlayTapes_SetScoreEntry(0, 0);

      if (autoplay.tape_filename == NULL)
      {
	autoplay.tape_filename = (options.mytapes ? getTapeFilename(level_nr) :
				  getDefaultSolutionTapeFilename(level_nr));

	if (!fileExists(autoplay.tape_filename))
	{
	  // non-standard or incorrect solution tape -- save to temporary file
	  autoplay.tape_filename = getTemporaryTapeFilename();

	  SaveTapeToFilename(autoplay.tape_filename);

	  use_temporary_tape_file = TRUE;
	}
      }

      SaveServerScoreFromFile(level_nr, TRUE, autoplay.tape_filename);

      boolean success = AutoPlayTapes_WaitForUpload();

      if (use_temporary_tape_file)
        unlink(autoplay.tape_filename);

      // required for uploading multiple tapes
      autoplay.tape_filename = NULL;

      if (!success)
      {
	num_tapes = -num_tapes;

	break;
      }

      continue;
    }

    InitCounter();

    if (options.tape_log_filename != NULL)
      OpenTapeLogfile();

    TapeStartGamePlaying();
    TapeStartWarpForward(global.autoplay_mode);

    autoplay.last_level_nr = level_nr;

    return num_tapes;
  }

  if (global.autoplay_mode == AUTOPLAY_MODE_UPLOAD)
  {
    Print("\n");
    PrintLine("=", 79);

    if (num_tapes >= 0)
      Print("SUMMARY: %d tapes uploaded.\n", num_tapes);
    else
      Print("SUMMARY: Uploading tapes failed.\n");

    PrintLine("=", 79);
  }

  // clear timestamp for batch tape upload (required after interactive upload)
  global.autoplay_time = 0;

  // exit if running headless or if visually auto-playing tapes
  if (program.headless || global.autoplay_mode != AUTOPLAY_MODE_UPLOAD)
    CloseAllAndExit(0);

  // when running interactively, restore last selected level set and number
  leveldir_current = leveldir_current_last;
  level_nr = level_nr_last;

  return num_tapes;
}

int AutoPlayTapes(void)
{
  return AutoPlayTapesExt(TRUE);
}

int AutoPlayTapesContinue(void)
{
  return AutoPlayTapesExt(FALSE);
}


// ----------------------------------------------------------------------------
// tape patch functions
// ----------------------------------------------------------------------------

static boolean PatchTape(struct TapeInfo *tape, char *mode)
{
  Print("[%d.%d.%d.%d]: ",
	(tape->engine_version / 1000000) % 100,
	(tape->engine_version / 10000  ) % 100,
	(tape->engine_version / 100    ) % 100,
	(tape->engine_version          ) % 100);

  if (strEqual(mode, "info"))
  {
    Print("property bits == 0x%02x\n", tape->property_bits);

    return FALSE;
  }

  boolean unpatch_tape = FALSE;
  boolean use_property_bit = FALSE;
  byte property_bitmask = 0;

  if (strSuffix(mode, ":0") ||
      strSuffix(mode, ":off") ||
      strSuffix(mode, ":clear"))
    unpatch_tape = TRUE;

  if (strEqual(mode, "em_random_bug") || strPrefix(mode, "em_random_bug:"))
  {
    // this bug was introduced in version 3.3.1.0 and fixed in version 4.0.1.2
    if (tape->engine_version <  VERSION_IDENT(3,3,1,0) ||
	tape->engine_version >= VERSION_IDENT(4,0,1,2))
    {
      Print("This tape version cannot be patched against EM random bug!\n");

      return FALSE;
    }

    property_bitmask = TAPE_PROPERTY_EM_RANDOM_BUG;

    use_property_bit = TRUE;
  }
  else if (strEqual(mode, "screen_34x34") || strPrefix(mode, "screen_34x34:"))
  {
    // this bug only affects team mode tapes
    if (tape->num_participating_players == 1)
    {
      Print("Only team mode tapes can be patched against screen size bug!\n");

      return FALSE;
    }

    // this bug (that always existed before) was fixed in version 4.2.2.1
    if (tape->engine_version >= VERSION_IDENT(4,2,2,1))
    {
      Print("This tape version cannot be patched against screen size bug!\n");

      return FALSE;
    }

    int factor = (unpatch_tape ? 1 : 2);
    int scr_fieldx_new = SCR_FIELDX_DEFAULT * factor;
    int scr_fieldy_new = SCR_FIELDY_DEFAULT * factor;

    if (scr_fieldx_new == tape->scr_fieldx &&
	scr_fieldy_new == tape->scr_fieldy)
    {
      Print("Tape already patched for '%s'!\n", mode);

      return FALSE;
    }

    tape->scr_fieldx = scr_fieldx_new;
    tape->scr_fieldy = scr_fieldy_new;
  }
  else
  {
    Print("Unknown patch mode '%s'!\n", mode);

    return FALSE;
  }

  // patching tapes using property bits may be used for several patch modes
  if (use_property_bit)
  {
    byte property_bits = tape->property_bits;
    boolean set_property_bit = (unpatch_tape ? FALSE : TRUE);

    if (set_property_bit)
      property_bits |= property_bitmask;
    else
      property_bits &= ~property_bitmask;

    if (property_bits == tape->property_bits)
    {
      Print("Tape already patched for '%s'!\n", mode);

      return FALSE;
    }

    tape->property_bits = property_bits;
  }

  Print("Patching for '%s' ... ", mode);

  return TRUE;
}

void PatchTapes(void)
{
  static LevelDirTree *patchtapes_leveldir = NULL;
  static int num_tapes_found = 0;
  static int num_tapes_patched = 0;
  char *mode = global.patchtapes_mode;
  int i;

  if (strEqual(mode, "help"))
  {
    PrintLine("=", 79);
    Print("Supported patch modes:\n");
    Print("- \"em_random_bug\"   - use 64-bit random value bug for EM engine\n");
    Print("- \"screen_34x34\"    - force visible playfield size of 34 x 34\n");
    PrintLine("-", 79);
    Print("Supported modifiers:\n");
    Print("- add \":0\", \":off\" or \":clear\" to patch mode to un-patch tape file\n");
    PrintLine("=", 79);

    CloseAllAndExit(0);
  }

  patchtapes_leveldir = getTreeInfoFromIdentifier(leveldir_first,
						  global.patchtapes_leveldir);

  if (patchtapes_leveldir == NULL)
    Fail("no such level identifier: '%s'", global.patchtapes_leveldir);

  leveldir_current = patchtapes_leveldir;

  if (patchtapes_leveldir->first_level < 0)
    patchtapes_leveldir->first_level = 0;
  if (patchtapes_leveldir->last_level >= MAX_TAPES_PER_SET)
    patchtapes_leveldir->last_level = MAX_TAPES_PER_SET - 1;

  PrintLine("=", 79);
  Print("Patching level tapes for patch mode '%s'\n", mode);
  PrintLine("-", 79);
  Print("Level series identifier: '%s'\n", patchtapes_leveldir->identifier);
  Print("Level series name:       '%s'\n", patchtapes_leveldir->name);
  Print("Level series author:     '%s'\n", patchtapes_leveldir->author);
  Print("Number of levels:        %d\n",   patchtapes_leveldir->levels);
  PrintLine("=", 79);
  Print("\n");

  int first_level = patchtapes_leveldir->first_level;
  int last_level = patchtapes_leveldir->last_level;

  for (i = first_level; i <= last_level; i++)
  {
    if (!global.patchtapes_all && !global.patchtapes_level[i])
      continue;

    Print("Tape %03d: ", i);

    TapeErase();
    LoadTape(i);

    if (tape.no_valid_file)
    {
      Print("(no tape found)\n");

      continue;
    }

    num_tapes_found++;

    if (PatchTape(&tape, mode))
    {
      char *filename = getTapeFilename(i);
      char *filename_orig = getStringCat2(filename, ".orig");

      if (!fileExists(filename_orig))
	rename(filename, filename_orig);

      SaveTapeToFilename(filename);

      Print("patched tape saved.\n");

      num_tapes_patched++;
    }
  }

  Print("\n");
  PrintLine("=", 79);
  Print("Number of tapes found: %d\n", num_tapes_found);
  Print("Number of tapes patched: %d\n", num_tapes_patched);
  PrintLine("=", 79);

  CloseAllAndExit(0);
}


// ---------- new tape button stuff -------------------------------------------

static struct
{
  int graphic;
  struct XY *pos;
  int gadget_id;
  char *infotext;
} tapebutton_info[NUM_TAPE_BUTTONS] =
{
  {
    IMG_GFX_TAPE_BUTTON_EJECT,		&tape.button.eject,
    TAPE_CTRL_ID_EJECT,			"eject tape"
  },
  {
    // (same position as "eject" button)
    IMG_GFX_TAPE_BUTTON_EXTRA,		&tape.button.eject,
    TAPE_CTRL_ID_EXTRA,			"extra functions"
  },
  {
    IMG_GFX_TAPE_BUTTON_STOP,		&tape.button.stop,
    TAPE_CTRL_ID_STOP,			"stop tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_PAUSE,		&tape.button.pause,
    TAPE_CTRL_ID_PAUSE,			"pause tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_RECORD,		&tape.button.record,
    TAPE_CTRL_ID_RECORD,		"record tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_PLAY,		&tape.button.play,
    TAPE_CTRL_ID_PLAY,			"play tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_INSERT_SOLUTION,&tape.button.insert_solution,
    TAPE_CTRL_ID_INSERT_SOLUTION,	"insert solution tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_PLAY_SOLUTION,	&tape.button.play_solution,
    TAPE_CTRL_ID_PLAY_SOLUTION,		"play solution tape"
  }
};

void CreateTapeButtons(void)
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
  {
    int graphic = tapebutton_info[i].graphic;
    struct GraphicInfo *gfx = &graphic_info[graphic];
    struct XY *pos = tapebutton_info[i].pos;
    struct GadgetInfo *gi;
    int gd_x = gfx->src_x;
    int gd_y = gfx->src_y;
    int gd_xp = gfx->src_x + gfx->pressed_xoffset;
    int gd_yp = gfx->src_y + gfx->pressed_yoffset;
    int id = i;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_IMAGE_ID, graphic,
		      GDI_INFO_TEXT, tapebutton_info[i].infotext,
		      GDI_X, VX + pos->x,
		      GDI_Y, VY + pos->y,
		      GDI_WIDTH, gfx->width,
		      GDI_HEIGHT, gfx->height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gfx->bitmap, gd_x, gd_y,
		      GDI_DESIGN_PRESSED, gfx->bitmap, gd_xp, gd_yp,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, GD_EVENT_RELEASED,
		      GDI_CALLBACK_ACTION, HandleTapeButtons,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    tape_gadget[id] = gi;
  }
}

void FreeTapeButtons(void)
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    FreeGadget(tape_gadget[i]);
}

void MapTapeEjectButton(void)
{
  UnmapGadget(tape_gadget[TAPE_CTRL_ID_EXTRA]);
  MapGadget(tape_gadget[TAPE_CTRL_ID_EJECT]);
}

void MapTapeWarpButton(void)
{
  UnmapGadget(tape_gadget[TAPE_CTRL_ID_EJECT]);
  MapGadget(tape_gadget[TAPE_CTRL_ID_EXTRA]);
}

void MapTapeButtons(void)
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    if (i != TAPE_CTRL_ID_EXTRA)
      MapGadget(tape_gadget[i]);

  if (tape.recording || tape.playing)
    MapTapeWarpButton();

  if (tape.show_game_buttons)
    MapGameButtonsOnTape();
}

void UnmapTapeButtons(void)
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    UnmapGadget(tape_gadget[i]);

  if (tape.show_game_buttons)
    UnmapGameButtonsOnTape();
}

void RedrawTapeButtons(void)
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    RedrawGadget(tape_gadget[i]);

  if (tape.show_game_buttons)
    RedrawGameButtonsOnTape();
}

void RedrawOrRemapTapeButtons(void)
{
  if (tape_gadget[TAPE_CTRL_ID_PLAY]->mapped)
  {
    // tape buttons already mapped
    RedrawTapeButtons();
  }
  else
  {
    UnmapTapeButtons();
    MapTapeButtons();
  }
}

static void HandleTapeButtonsExt(int id)
{
  if (game_status != GAME_MODE_MAIN && game_status != GAME_MODE_PLAYING)
    return;

  switch (id)
  {
    case TAPE_CTRL_ID_EJECT:
      TapeStop();

      if (TAPE_IS_EMPTY(tape))
      {
	LoadTape(level_nr);

	if (TAPE_IS_EMPTY(tape))
	  Request("No tape for this level!", REQ_CONFIRM);
      }
      else
      {
	if (tape.changed)
	  SaveTapeChecked(level_nr);

	TapeErase();
      }

      DrawCompleteVideoDisplay();
      break;

    case TAPE_CTRL_ID_EXTRA:
      if (tape.playing)
      {
	tape.pause_before_end = !tape.pause_before_end;

	DrawVideoDisplayCurrentState();
      }
      else if (tape.recording)
      {
	TapeSingleStep();
      }

      break;

    case TAPE_CTRL_ID_STOP:
      TapeStopTape();

      break;

    case TAPE_CTRL_ID_PAUSE:
      TapeTogglePause(TAPE_TOGGLE_MANUAL);

      break;

    case TAPE_CTRL_ID_RECORD:
      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartGameRecording();
      }
      else if (tape.pausing)
      {
	if (tape.playing)			// PLAY -> PAUSE -> RECORD
	  TapeAppendRecording();
	else
	  TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }

      break;

    case TAPE_CTRL_ID_PLAY:
      if (tape.recording && tape.pausing)	// PAUSE -> RECORD
      {
	// ("TAPE_IS_EMPTY(tape)" is TRUE here -- probably fix this)

	TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }

      if (TAPE_IS_EMPTY(tape))
	break;

      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartGamePlaying();
      }
      else if (tape.playing)
      {
	if (tape.pausing)			// PAUSE -> PLAY
	{
	  TapeTogglePause(TAPE_TOGGLE_MANUAL | TAPE_TOGGLE_PLAY_PAUSE);
	}
	else if (!tape.fast_forward)		// PLAY -> FFWD
	{
	  tape.fast_forward = TRUE;
	}
	else if (!tape.warp_forward)		// FFWD -> WARP
	{
	  tape.warp_forward = TRUE;
	}
	else if (!tape.deactivate_display)	// WARP -> WARP BLIND
	{
	  tape.deactivate_display = TRUE;

	  TapeDeactivateDisplayOn();
	}
	else					// WARP BLIND -> PLAY
	{
	  tape.fast_forward = FALSE;
	  tape.warp_forward = FALSE;
	  tape.deactivate_display = FALSE;

	  TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);
	}

	DrawVideoDisplayCurrentState();
      }

      break;

    case TAPE_CTRL_ID_INSERT_SOLUTION:
      InsertSolutionTape();

      break;

    case TAPE_CTRL_ID_PLAY_SOLUTION:
      PlaySolutionTape();

      break;

    default:
      break;
  }
}

static void HandleTapeButtons(struct GadgetInfo *gi)
{
  HandleTapeButtonsExt(gi->custom_id);
}

void HandleTapeButtonKeys(Key key)
{
  boolean eject_button_is_active = TAPE_IS_STOPPED(tape);
  boolean extra_button_is_active = !eject_button_is_active;

  if (key == setup.shortcut.tape_eject && eject_button_is_active)
    HandleTapeButtonsExt(TAPE_CTRL_ID_EJECT);
  else if (key == setup.shortcut.tape_extra && extra_button_is_active)
    HandleTapeButtonsExt(TAPE_CTRL_ID_EXTRA);
  else if (key == setup.shortcut.tape_stop)
    HandleTapeButtonsExt(TAPE_CTRL_ID_STOP);
  else if (key == setup.shortcut.tape_pause)
    HandleTapeButtonsExt(TAPE_CTRL_ID_PAUSE);
  else if (key == setup.shortcut.tape_record)
    HandleTapeButtonsExt(TAPE_CTRL_ID_RECORD);
  else if (key == setup.shortcut.tape_play)
    HandleTapeButtonsExt(TAPE_CTRL_ID_PLAY);
}
