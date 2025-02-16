// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// anim.c
// ============================================================================

#include "libgame/libgame.h"

#include "anim.h"
#include "main.h"
#include "tools.h"
#include "files.h"
#include "events.h"
#include "screens.h"
#include "tape.h"


#define DEBUG_ANIM_DELAY		0
#define DEBUG_ANIM_EVENTS		0


// values for global toon animation definition
#define NUM_GLOBAL_TOON_ANIMS		1
#define NUM_GLOBAL_TOON_PARTS		MAX_NUM_TOONS

// values for global animation definition (including toons)
#define NUM_GLOBAL_ANIMS_AND_TOONS	(NUM_GLOBAL_ANIMS +		\
					 NUM_GLOBAL_TOON_ANIMS)
#define NUM_GLOBAL_ANIM_PARTS_AND_TOONS	MAX(NUM_GLOBAL_ANIM_PARTS_ALL,	\
					    NUM_GLOBAL_TOON_PARTS)

#define MAX_GLOBAL_ANIM_LIST		(NUM_GAME_MODES *		\
					 NUM_GLOBAL_ANIMS_AND_TOONS *	\
					 NUM_GLOBAL_ANIM_PARTS_AND_TOONS)

#define ANIM_CLASS_BIT_TITLE_INITIAL	0
#define ANIM_CLASS_BIT_TITLE		1
#define ANIM_CLASS_BIT_MAIN		2
#define ANIM_CLASS_BIT_NAMES		3
#define ANIM_CLASS_BIT_SCORES		4
#define ANIM_CLASS_BIT_SCORESONLY	5
#define ANIM_CLASS_BIT_SUBMENU		6
#define ANIM_CLASS_BIT_MENU		7
#define ANIM_CLASS_BIT_TOONS		8
#define ANIM_CLASS_BIT_NO_TITLE		9

#define NUM_ANIM_CLASSES		10

#define ANIM_CLASS_NONE			0
#define ANIM_CLASS_TITLE_INITIAL	(1 << ANIM_CLASS_BIT_TITLE_INITIAL)
#define ANIM_CLASS_TITLE		(1 << ANIM_CLASS_BIT_TITLE)
#define ANIM_CLASS_MAIN			(1 << ANIM_CLASS_BIT_MAIN)
#define ANIM_CLASS_NAMES		(1 << ANIM_CLASS_BIT_NAMES)
#define ANIM_CLASS_SCORES		(1 << ANIM_CLASS_BIT_SCORES)
#define ANIM_CLASS_SCORESONLY		(1 << ANIM_CLASS_BIT_SCORESONLY)
#define ANIM_CLASS_SUBMENU		(1 << ANIM_CLASS_BIT_SUBMENU)
#define ANIM_CLASS_MENU			(1 << ANIM_CLASS_BIT_MENU)
#define ANIM_CLASS_TOONS		(1 << ANIM_CLASS_BIT_TOONS)
#define ANIM_CLASS_NO_TITLE		(1 << ANIM_CLASS_BIT_NO_TITLE)

#define ANIM_CLASS_TOONS_SCORES		(ANIM_CLASS_TOONS 	|	\
					 ANIM_CLASS_SCORES	|	\
					 ANIM_CLASS_NO_TITLE)

#define ANIM_CLASS_TOONS_SCORESONLY	(ANIM_CLASS_TOONS 	|	\
					 ANIM_CLASS_SCORES	|	\
					 ANIM_CLASS_SCORESONLY	|	\
					 ANIM_CLASS_NO_TITLE)

#define ANIM_CLASS_TOONS_MENU_MAIN	(ANIM_CLASS_TOONS 	|	\
					 ANIM_CLASS_MENU  	|	\
					 ANIM_CLASS_MAIN  	|	\
					 ANIM_CLASS_NO_TITLE)

#define ANIM_CLASS_TOONS_MENU_SUBMENU	(ANIM_CLASS_TOONS 	|	\
					 ANIM_CLASS_MENU  	|	\
					 ANIM_CLASS_SUBMENU	|	\
					 ANIM_CLASS_NO_TITLE)

#define ANIM_CLASS_TOONS_MENU_SUBMENU_2	(ANIM_CLASS_TOONS 	|	\
					 ANIM_CLASS_MENU  	|	\
					 ANIM_CLASS_SUBMENU	|	\
					 ANIM_CLASS_NAMES 	|	\
					 ANIM_CLASS_NO_TITLE)

// values for global animation states
#define ANIM_STATE_INACTIVE		0
#define ANIM_STATE_RESTART		(1 << 0)
#define ANIM_STATE_WAITING		(1 << 1)
#define ANIM_STATE_RUNNING		(1 << 2)

// values for global animation control
#define ANIM_NO_ACTION			0
#define ANIM_START			1
#define ANIM_CONTINUE			2
#define ANIM_STOP			3


struct GlobalAnimPartControlInfo
{
  int old_nr;		// position before mapping animation parts linearly
  int old_anim_nr;	// position before mapping animations linearly

  int nr;
  int anim_nr;
  int mode_nr;

  boolean is_base;	// animation part is base/main/default animation part

  int sound;
  int music;
  int graphic;

  struct GraphicInfo graphic_info;
  struct GraphicInfo control_info;

  boolean class_playfield_or_door;

  int viewport_x;
  int viewport_y;
  int viewport_width;
  int viewport_height;

  int x, y;
  int step_xoffset, step_yoffset;

  int tile_x, tile_y;
  int tile_xoffset, tile_yoffset;

  unsigned int initial_anim_sync_frame;
  unsigned int anim_random_frame;

  DelayCounter step_delay;

  int init_delay_counter;
  int anim_delay_counter;
  int post_delay_counter;

  int fade_delay_counter;
  int fade_alpha;

  boolean init_event_state;
  boolean anim_event_state;

  boolean triggered;
  boolean clickable;
  boolean clicked;

  int drawing_stage;

  int state;
  int last_anim_status;
};

struct GlobalAnimMainControlInfo
{
  struct GlobalAnimPartControlInfo base;
  struct GlobalAnimPartControlInfo part[NUM_GLOBAL_ANIM_PARTS_AND_TOONS];

  int nr;
  int mode_nr;

  struct GraphicInfo control_info;

  int num_parts;	// number of animation parts, but without base part
  int num_parts_all;	// number of animation parts, including base part
  int part_counter;
  int active_part_nr;

  boolean has_base;	// animation has base/main/default animation part

  int last_x, last_y;

  int init_delay_counter;

  int state;

  int last_state, last_active_part_nr;
};

struct GlobalAnimControlInfo
{
  struct GlobalAnimMainControlInfo anim[NUM_GLOBAL_ANIMS_AND_TOONS];

  int nr;
  int num_anims;
};

struct GameModeAnimClass
{
  int game_mode;
  int class;
} game_mode_anim_classes_list[] =
{
  { GAME_MODE_TITLE_INITIAL_1,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_2,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_3,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_4,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_5,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_1,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_2,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_3,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_4,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_5,			ANIM_CLASS_TITLE		},
  { GAME_MODE_NAMES, 			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_LEVELS, 			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_LEVELNR,			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_INFO,			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_SETUP,			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_PSEUDO_NAMESONLY,		ANIM_CLASS_TOONS_MENU_SUBMENU_2	},
  { GAME_MODE_PSEUDO_TYPENAMES,		ANIM_CLASS_TOONS_MENU_SUBMENU_2	},
  { GAME_MODE_PSEUDO_MAINONLY,		ANIM_CLASS_TOONS_MENU_MAIN	},
  { GAME_MODE_PSEUDO_TYPENAME,		ANIM_CLASS_TOONS_MENU_MAIN	},
  { GAME_MODE_PSEUDO_SCORESOLD,		ANIM_CLASS_TOONS_SCORESONLY	},
  { GAME_MODE_PSEUDO_SCORESNEW,		ANIM_CLASS_TOONS_SCORESONLY	},
  { GAME_MODE_SCOREINFO,		ANIM_CLASS_TOONS_SCORES		},
  { GAME_MODE_EDITOR,			ANIM_CLASS_NO_TITLE		},
  { GAME_MODE_PLAYING,			ANIM_CLASS_NO_TITLE		},

  { -1,					-1				}
};

struct AnimClassGameMode
{
  int class_bit;
  int game_mode;
} anim_class_game_modes_list[] =
{
  { ANIM_CLASS_BIT_TITLE_INITIAL,	GAME_MODE_TITLE_INITIAL		},
  { ANIM_CLASS_BIT_TITLE,		GAME_MODE_TITLE			},
  { ANIM_CLASS_BIT_MAIN,		GAME_MODE_MAIN			},
  { ANIM_CLASS_BIT_NAMES,		GAME_MODE_NAMES			},
  { ANIM_CLASS_BIT_SCORES,		GAME_MODE_SCORES		},
  { ANIM_CLASS_BIT_SCORESONLY,		GAME_MODE_PSEUDO_SCORESONLY	},
  { ANIM_CLASS_BIT_SUBMENU,		GAME_MODE_PSEUDO_SUBMENU	},
  { ANIM_CLASS_BIT_MENU,		GAME_MODE_PSEUDO_MENU		},
  { ANIM_CLASS_BIT_TOONS,		GAME_MODE_PSEUDO_TOONS		},
  { ANIM_CLASS_BIT_NO_TITLE,		GAME_MODE_PSEUDO_NO_TITLE	},

  { -1,					-1				}
};

// forward declaration for internal use
static void DoGlobalAnim_DelayAction(struct GlobalAnimPartControlInfo *, int);
static boolean DoGlobalAnim_EventAction(struct GlobalAnimPartControlInfo *);
static void HandleGlobalAnim(int, int);
static void DoAnimationExt(void);
static void ResetGlobalAnim_Clickable(void);
static void ResetGlobalAnim_Clicked(void);

static struct GlobalAnimControlInfo global_anim_ctrl[NUM_GAME_MODES];
static struct GlobalAnimPartControlInfo *global_anim_list[MAX_GLOBAL_ANIM_LIST];
static int num_global_anim_list = 0;

static unsigned int anim_sync_frame = 0;

static int game_mode_anim_classes[NUM_GAME_MODES];
static int anim_class_game_modes[NUM_ANIM_CLASSES];

static int anim_status_last_before_fading = GAME_MODE_DEFAULT;
static int anim_status_last = GAME_MODE_DEFAULT;
static int anim_classes_last = ANIM_CLASS_NONE;

static boolean drawing_to_fading_buffer = FALSE;

static boolean handle_click = FALSE;


// ============================================================================
// generic animation frame calculation
// ============================================================================

int getAnimationFrame(int num_frames, int delay, int mode, int start_frame,
		      int sync_frame)
{
  int frame = 0;

  if (delay < 1)			// delay must be at least 1
    delay = 1;

  sync_frame += start_frame * delay;

  if (mode & ANIM_LOOP)			// looping animation
  {
    frame = (sync_frame % (delay * num_frames)) / delay;
  }
  else if (mode & ANIM_LINEAR)		// linear (non-looping) animation
  {
    frame = sync_frame / delay;

    if (frame > num_frames - 1)
      frame = num_frames - 1;
  }
  else if (mode & ANIM_PINGPONG)	// oscillate (border frames once)
  {
    int max_anim_frames = (num_frames > 1 ? 2 * num_frames - 2 : 1);

    frame = (sync_frame % (delay * max_anim_frames)) / delay;
    frame = (frame < num_frames ? frame : max_anim_frames - frame);
  }
  else if (mode & ANIM_PINGPONG2)	// oscillate (border frames twice)
  {
    int max_anim_frames = 2 * num_frames;

    frame = (sync_frame % (delay * max_anim_frames)) / delay;
    frame = (frame < num_frames ? frame : max_anim_frames - frame - 1);
  }
  else if (mode & ANIM_RANDOM)		// play frames in random order
  {
    // note: expect different frames for the same delay cycle!

    if (gfx.anim_random_frame < 0)
      frame = GetSimpleRandom(num_frames);
    else
      frame = gfx.anim_random_frame % num_frames;
  }
  else if (mode & ANIM_LEVEL_NR)	// play frames by level number
  {
    int level_pos = level_nr - gfx.anim_first_level;

    frame = level_pos % num_frames;
  }
  else if (mode & (ANIM_CE_VALUE | ANIM_CE_SCORE | ANIM_CE_DELAY))
  {
    frame = sync_frame % num_frames;
  }

  if (mode & ANIM_REVERSE)		// use reverse animation direction
    frame = num_frames - frame - 1;

  return frame;
}


// ============================================================================
// global animation functions
// ============================================================================

static int getGlobalAnimationPart(struct GlobalAnimMainControlInfo *anim)
{
  struct GraphicInfo *c = &anim->control_info;
  int last_anim_random_frame = gfx.anim_random_frame;
  int part_nr;

  gfx.anim_random_frame = -1;	// (use simple, ad-hoc random numbers)

  part_nr = getAnimationFrame(anim->num_parts, 1,
			      c->anim_mode, c->anim_start_frame,
			      anim->part_counter);

  gfx.anim_random_frame = last_anim_random_frame;

  return part_nr;
}

static int compareGlobalAnimPartControlInfo(const void *obj1, const void *obj2)
{
  const struct GlobalAnimPartControlInfo *o1 =
    *(struct GlobalAnimPartControlInfo **)obj1;
  const struct GlobalAnimPartControlInfo *o2 =
    *(struct GlobalAnimPartControlInfo **)obj2;
  int compare_result;

  if (o1->control_info.draw_order != o2->control_info.draw_order)
    compare_result = o1->control_info.draw_order - o2->control_info.draw_order;
  else if (o1->mode_nr != o2->mode_nr)
    compare_result = o1->mode_nr - o2->mode_nr;
  else if (o1->anim_nr != o2->anim_nr)
    compare_result = o1->anim_nr - o2->anim_nr;
  else
    compare_result = o1->nr - o2->nr;

  return compare_result;
}

static boolean isPausedOnPlayfieldOrDoor(struct GlobalAnimPartControlInfo *part)
{
  // only pause playfield and door animations when playing
  if (game_status != GAME_MODE_PLAYING)
    return FALSE;

  // do not pause animations when game ended (and engine is running)
  if (checkGameEnded())
    return FALSE;

  // only pause animations on playfield and doors
  if (!part->class_playfield_or_door)
    return FALSE;

  // only pause animations when engine is paused or request dialog is active
  if (!tape.pausing && !game.request_active)
    return FALSE;

  return TRUE;
}

static void InitToonControls(void)
{
  int mode_nr_toons = GAME_MODE_PSEUDO_TOONS;
  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr_toons];
  struct GlobalAnimMainControlInfo *anim = &ctrl->anim[ctrl->num_anims];
  int mode_nr, anim_nr, part_nr;
  int control = IMG_INTERNAL_GLOBAL_TOON_DEFAULT;
  int num_toons = MAX_NUM_TOONS;
  int i;

  if (global.num_toons >= 0 && global.num_toons < MAX_NUM_TOONS)
    num_toons = global.num_toons;

  mode_nr = mode_nr_toons;
  anim_nr = ctrl->num_anims;

  anim->nr = anim_nr;
  anim->mode_nr = mode_nr;
  anim->control_info = graphic_info[control];

  anim->num_parts = 0;
  anim->num_parts_all = 0;
  anim->part_counter = 0;
  anim->active_part_nr = 0;

  anim->has_base = FALSE;

  anim->last_x = POS_OFFSCREEN;
  anim->last_y = POS_OFFSCREEN;

  anim->init_delay_counter = 0;

  anim->state = ANIM_STATE_INACTIVE;

  part_nr = 0;

  for (i = 0; i < num_toons; i++)
  {
    struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];
    int sound = SND_UNDEFINED;
    int music = MUS_UNDEFINED;
    int graphic = IMG_TOON_1 + i;

    control = graphic;

    part->nr = part_nr;
    part->anim_nr = anim_nr;
    part->mode_nr = mode_nr;

    part->is_base = FALSE;

    part->sound = sound;
    part->music = music;
    part->graphic = graphic;

    part->graphic_info = graphic_info[graphic];
    part->control_info = graphic_info[control];

    part->graphic_info.anim_delay *= part->graphic_info.step_delay;

    part->control_info.init_delay_fixed = 0;
    part->control_info.init_delay_random = 150;

    part->control_info.x = ARG_UNDEFINED_VALUE;
    part->control_info.y = ARG_UNDEFINED_VALUE;

    part->initial_anim_sync_frame = 0;
    part->anim_random_frame = -1;

    part->step_delay.count = 0;
    part->step_delay.value = graphic_info[control].step_delay;

    part->state = ANIM_STATE_INACTIVE;
    part->last_anim_status = -1;

    anim->num_parts++;
    anim->num_parts_all++;

    part_nr++;
  }

  ctrl->num_anims++;
}

static void InitGlobalAnimControls(void)
{
  int i, m, a, p;
  int mode_nr, anim_nr, part_nr;
  int sound, music, graphic, control;

  anim_sync_frame = 0;

  for (m = 0; m < NUM_GAME_MODES; m++)
  {
    mode_nr = m;

    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];

    ctrl->nr = mode_nr;
    ctrl->num_anims = 0;

    anim_nr = 0;

    for (a = 0; a < NUM_GLOBAL_ANIMS; a++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      int ctrl_id = GLOBAL_ANIM_ID_CONTROL_FIRST + a;

      control = global_anim_info[ctrl_id].graphic[GLOBAL_ANIM_ID_PART_BASE][m];

      // if no base animation parameters defined, use default values
      if (control == IMG_UNDEFINED)
	control = IMG_INTERNAL_GLOBAL_ANIM_DEFAULT;

      anim->nr = anim_nr;
      anim->mode_nr = mode_nr;
      anim->control_info = graphic_info[control];

      anim->num_parts = 0;
      anim->num_parts_all = 0;
      anim->part_counter = 0;
      anim->active_part_nr = 0;

      anim->has_base = FALSE;

      anim->last_x = POS_OFFSCREEN;
      anim->last_y = POS_OFFSCREEN;

      anim->init_delay_counter = 0;

      anim->state = ANIM_STATE_INACTIVE;

      part_nr = 0;

      for (p = 0; p < NUM_GLOBAL_ANIM_PARTS_ALL; p++)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];

	sound   = global_anim_info[a].sound[p][m];
	music   = global_anim_info[a].music[p][m];
	graphic = global_anim_info[a].graphic[p][m];
	control = global_anim_info[ctrl_id].graphic[p][m];

	if (graphic == IMG_UNDEFINED || graphic_info[graphic].bitmap == NULL ||
	    control == IMG_UNDEFINED)
	  continue;

#if 0
	Debug("anim:InitGlobalAnimControls",
	      "mode == %d, anim = %d, part = %d [%d, %d, %d] [%d]",
	      m, a, p, mode_nr, anim_nr, part_nr, control);
#endif

#if 0
	Debug("anim:InitGlobalAnimControls",
	      "mode == %d, anim = %d, part = %d [%d, %d, %d] [%d]",
	      m, a, p, mode_nr, anim_nr, part_nr, sound);
#endif

	part->old_nr = p;
	part->old_anim_nr = a;

	part->nr = part_nr;
	part->anim_nr = anim_nr;
	part->mode_nr = mode_nr;

	part->sound = sound;
	part->music = music;
	part->graphic = graphic;

	part->graphic_info = graphic_info[graphic];
	part->control_info = graphic_info[control];

	part->initial_anim_sync_frame = 0;
	part->anim_random_frame = -1;

	part->step_delay.count = 0;
	part->step_delay.value = graphic_info[control].step_delay;

	part->state = ANIM_STATE_INACTIVE;
	part->last_anim_status = -1;

	anim->num_parts_all++;

	if (p < GLOBAL_ANIM_ID_PART_BASE)
	{
	  part->is_base = FALSE;

	  anim->num_parts++;
	  part_nr++;
	}
	else
	{
	  part->is_base = TRUE;

	  anim->base = *part;
	  anim->has_base = TRUE;
	}

	// apply special settings to pointer-style animations
	if (isClass(part->control_info.class, "pointer"))
	{
	  // force pointer-style animations to be checked for clicks first
	  part->control_info.draw_order = 1000000;

	  // force pointer-style animations to pass-through clicks
	  part->control_info.style |= STYLE_PASSTHROUGH;
	}
      }

      if (anim->num_parts > 0 || anim->has_base)
      {
	ctrl->num_anims++;
	anim_nr++;
      }
    }
  }

  InitToonControls();

  // create list of all animation parts
  num_global_anim_list = 0;
  for (m = 0; m < NUM_GAME_MODES; m++)
    for (a = 0; a < global_anim_ctrl[m].num_anims; a++)
      for (p = 0; p < global_anim_ctrl[m].anim[a].num_parts_all; p++)
	global_anim_list[num_global_anim_list++] =
	  &global_anim_ctrl[m].anim[a].part[p];

  // sort list of all animation parts according to draw_order and number
  qsort(global_anim_list, num_global_anim_list,
	sizeof(struct GlobalAnimPartControlInfo *),
	compareGlobalAnimPartControlInfo);

  for (i = 0; i < NUM_GAME_MODES; i++)
    game_mode_anim_classes[i] = ANIM_CLASS_NONE;
  for (i = 0; game_mode_anim_classes_list[i].game_mode != -1; i++)
    game_mode_anim_classes[game_mode_anim_classes_list[i].game_mode] =
      game_mode_anim_classes_list[i].class;

  for (i = 0; i < NUM_ANIM_CLASSES; i++)
    anim_class_game_modes[i] = GAME_MODE_DEFAULT;
  for (i = 0; anim_class_game_modes_list[i].game_mode != -1; i++)
    anim_class_game_modes[anim_class_game_modes_list[i].class_bit] =
      anim_class_game_modes_list[i].game_mode;

  anim_status_last_before_fading = GAME_MODE_LOADING;
  anim_status_last = GAME_MODE_LOADING;
  anim_classes_last = ANIM_CLASS_NONE;
}

static void SetGlobalAnimEventsForCustomElements(int list_pos)
{
  int num_events = GetGlobalAnimEventValueCount(list_pos);
  int i;

  for (i = 0; i < num_events; i++)
  {
    int event = GetGlobalAnimEventValue(list_pos, i);

    if (event & ANIM_EVENT_CE_CHANGE)
    {
      int nr = (event >> ANIM_EVENT_CE_BIT) & 0xff;

      if (nr >= 0 && nr < NUM_CUSTOM_ELEMENTS)
	element_info[EL_CUSTOM_START + nr].has_anim_event = TRUE;
    }
  }
}

void InitGlobalAnimEventsForCustomElements(void)
{
  int m, a, p;

  // custom element events for global animations only relevant while playing
  m = GAME_MODE_PLAYING;

  for (a = 0; a < NUM_GLOBAL_ANIMS; a++)
  {
    int ctrl_id = GLOBAL_ANIM_ID_CONTROL_FIRST + a;
    int control = global_anim_info[ctrl_id].graphic[GLOBAL_ANIM_ID_PART_BASE][m];

    // if no base animation parameters defined, use default values
    if (control == IMG_UNDEFINED)
      control = IMG_INTERNAL_GLOBAL_ANIM_DEFAULT;

    SetGlobalAnimEventsForCustomElements(graphic_info[control].init_event);
    SetGlobalAnimEventsForCustomElements(graphic_info[control].anim_event);

    for (p = 0; p < NUM_GLOBAL_ANIM_PARTS_ALL; p++)
    {
      control = global_anim_info[ctrl_id].graphic[p][m];

      if (control == IMG_UNDEFINED)
	continue;

      SetGlobalAnimEventsForCustomElements(graphic_info[control].init_event);
      SetGlobalAnimEventsForCustomElements(graphic_info[control].anim_event);
    }
  }
}

void InitGlobalAnimations(void)
{
  InitGlobalAnimControls();
}

static void BlitGlobalAnimation(struct GlobalAnimPartControlInfo *part,
				Bitmap *src_bitmap, int src_x0, int src_y0,
				int drawing_target)
{
  struct GraphicInfo *g = &part->graphic_info;
  struct GraphicInfo *c = &part->control_info;
  void (*blit_bitmap)(Bitmap *, Bitmap *, int, int, int, int, int, int) =
    (g->draw_masked ? BlitBitmapMasked : BlitBitmap);
  void (*blit_screen)(Bitmap *, int, int, int, int, int, int) =
    (g->draw_masked ? BlitToScreenMasked : BlitToScreen);
  Bitmap *fade_bitmap =
    (drawing_target == DRAW_TO_FADE_SOURCE ? gfx.fade_bitmap_source :
     drawing_target == DRAW_TO_FADE_TARGET ? gfx.fade_bitmap_target : NULL);
  int alpha = ((c->fade_mode & FADE_MODE_FADE) != 0 ? part->fade_alpha : g->alpha);
  int x, y;

  for (y = 0; y < c->stacked_yfactor; y++)
  {
    for (x = 0; x < c->stacked_xfactor; x++)
    {
      int src_x = src_x0;
      int src_y = src_y0;
      int dst_x = part->x + x * (g->width  + c->stacked_xoffset);
      int dst_y = part->y + y * (g->height + c->stacked_yoffset);
      int cut_x = 0;
      int cut_y = 0;
      int width  = g->width;
      int height = g->height;

      if (dst_x < 0)
      {
	width += dst_x;
	cut_x = -dst_x;
	dst_x = 0;
      }
      else if (dst_x > part->viewport_width - g->width)
      {
	width -= (dst_x - (part->viewport_width - g->width));
      }

      if (dst_y < 0)
      {
	height += dst_y;
	cut_y  = -dst_y;
	dst_y = 0;
      }
      else if (dst_y > part->viewport_height - g->height)
      {
	height -= (dst_y - (part->viewport_height - g->height));
      }

      if (width <= 0 || height <= 0)
	continue;

      src_x += cut_x;
      src_y += cut_y;

      dst_x += part->viewport_x;
      dst_y += part->viewport_y;

      SetBitmapAlphaNextBlit(src_bitmap, alpha);

      if (drawing_target == DRAW_TO_SCREEN)
	blit_screen(src_bitmap, src_x, src_y, width, height,
		    dst_x, dst_y);
      else
	blit_bitmap(src_bitmap, fade_bitmap, src_x, src_y, width, height,
		    dst_x, dst_y);
    }
  }
}

static void DrawGlobalAnimationsExt(int drawing_target, int drawing_stage)
{
  int game_mode_anim_action[NUM_GAME_MODES];
  int mode_nr;
  int i;

  if (!setup.global_animations)
    return;

  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_1 &&
      drawing_target == DRAW_TO_SCREEN)
    DoAnimationExt();

  // always start with reliable default values (no animation actions)
  for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
    game_mode_anim_action[mode_nr] = ANIM_NO_ACTION;

  if (global.anim_status != anim_status_last)
  {
    boolean before_fading = (global.anim_status == GAME_MODE_PSEUDO_FADING);
    boolean after_fading  = (anim_status_last   == GAME_MODE_PSEUDO_FADING);
    int anim_classes_next = game_mode_anim_classes[global.anim_status_next];

    if (drawing_target == DRAW_TO_FADE_TARGET)
      after_fading = TRUE;

    // special case: changing from/to these screens is done without fading
    if (global.anim_status == GAME_MODE_PSEUDO_TYPENAME  ||
	global.anim_status == GAME_MODE_PSEUDO_TYPENAMES ||
	anim_status_last   == GAME_MODE_PSEUDO_TYPENAME  ||
	anim_status_last   == GAME_MODE_PSEUDO_TYPENAMES)
      after_fading = TRUE;

    // ---------- part 1 ------------------------------------------------------
    // start or stop global animations by change of game mode
    // (special handling of animations for "current screen" and "all screens")

    if (global.anim_status_next != anim_status_last_before_fading)
    {
      // stop animations for last screen before fading to new screen
      game_mode_anim_action[anim_status_last] = ANIM_STOP;

      // start animations for current screen after fading to new screen
      game_mode_anim_action[global.anim_status] = ANIM_START;
    }

    // start animations for all screens after loading new artwork set
    if (anim_status_last == GAME_MODE_LOADING)
      game_mode_anim_action[GAME_MODE_DEFAULT] = ANIM_START;

    // ---------- part 2 ------------------------------------------------------
    // start or stop global animations by change of animation class
    // (generic handling of animations for "class of screens")

    for (i = 0; i < NUM_ANIM_CLASSES; i++)
    {
      int anim_class_check = (1 << i);
      int anim_class_game_mode = anim_class_game_modes[i];
      int anim_class_last = anim_classes_last & anim_class_check;
      int anim_class_next = anim_classes_next & anim_class_check;

      // stop animations for changed screen class before fading to new screen
      if (before_fading && anim_class_last && !anim_class_next)
	game_mode_anim_action[anim_class_game_mode] = ANIM_STOP;

      // start animations for changed screen class after fading to new screen
      if (after_fading && !anim_class_last && anim_class_next)
	game_mode_anim_action[anim_class_game_mode] = ANIM_START;
    }

    if (drawing_target == DRAW_TO_SCREEN)
    {
      if (after_fading)
      {
	anim_classes_last = anim_classes_next;
	anim_status_last_before_fading = global.anim_status;
      }

      anim_status_last = global.anim_status;

      // start or stop animations determined to be started or stopped above
      for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
	if (game_mode_anim_action[mode_nr] != ANIM_NO_ACTION)
	  HandleGlobalAnim(game_mode_anim_action[mode_nr], mode_nr);
    }
    else if (drawing_target == DRAW_TO_FADE_TARGET)
    {
      drawing_to_fading_buffer = TRUE;

      // start animations determined to be (temporary) started above
      for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
	if (game_mode_anim_action[mode_nr] == ANIM_START)
	  HandleGlobalAnim(ANIM_START, mode_nr);
    }
  }

  // when restarting global animations, do not redraw them, but stop here
  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_RESTART)
    return;

  if (global.anim_status == GAME_MODE_LOADING)
    return;

  for (i = 0; i < num_global_anim_list; i++)
  {
    struct GlobalAnimPartControlInfo *part = global_anim_list[i];
    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[part->mode_nr];
    struct GlobalAnimMainControlInfo *anim = &ctrl->anim[part->anim_nr];
    struct GraphicInfo *g = &part->graphic_info;
    Bitmap *src_bitmap;
    int src_x, src_y;
    int sync_frame;
    int frame;
    int last_anim_random_frame = gfx.anim_random_frame;

    if (!setup.toons &&
	part->graphic >= IMG_TOON_1 &&
	part->graphic <= IMG_TOON_20)
      continue;

    // when preparing source fading buffer, only draw animations to be stopped
    if (drawing_target == DRAW_TO_FADE_SOURCE &&
	game_mode_anim_action[part->mode_nr] != ANIM_STOP)
      continue;

    // when preparing target fading buffer, only draw animations to be started
    if (drawing_target == DRAW_TO_FADE_TARGET &&
	game_mode_anim_action[part->mode_nr] != ANIM_START)
      continue;

    if (!(anim->state & ANIM_STATE_RUNNING))
      continue;

    if (!(part->state & ANIM_STATE_RUNNING))
      continue;

    if (part->drawing_stage != drawing_stage)
      continue;

    // if game is paused, also pause playfield and door animations
    if (isPausedOnPlayfieldOrDoor(part))
      part->initial_anim_sync_frame++;

    sync_frame = anim_sync_frame - part->initial_anim_sync_frame;

    // re-initialize random animation frame after animation delay
    if (g->anim_mode == ANIM_RANDOM &&
	sync_frame % g->anim_delay == 0 &&
	sync_frame > 0)
      part->anim_random_frame = GetSimpleRandom(g->anim_frames);

    gfx.anim_random_frame = part->anim_random_frame;

    frame = getAnimationFrame(g->anim_frames, g->anim_delay,
			      g->anim_mode, g->anim_start_frame,
			      sync_frame);

    gfx.anim_random_frame = last_anim_random_frame;

    getGlobalAnimGraphicSource(part->graphic, frame, &src_bitmap,
			       &src_x, &src_y);

    BlitGlobalAnimation(part, src_bitmap, src_x, src_y, drawing_target);
  }

  if (drawing_target == DRAW_TO_FADE_TARGET)
  {
    // stop animations determined to be (temporary) started above
    for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
      if (game_mode_anim_action[mode_nr] == ANIM_START)
	HandleGlobalAnim(ANIM_STOP, mode_nr);

    drawing_to_fading_buffer = FALSE;
  }
}

void DrawGlobalAnimations(int drawing_target, int drawing_stage)
{
  int last_cursor_mode_override = gfx.cursor_mode_override;

  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_1)
  {
    ResetGlobalAnim_Clickable();

    gfx.cursor_mode_override = CURSOR_UNDEFINED;
  }

  DrawGlobalAnimationsExt(drawing_target, drawing_stage);

  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_2)
  {
    ResetGlobalAnim_Clicked();
  }

  if (gfx.cursor_mode_override != last_cursor_mode_override)
    SetMouseCursor(gfx.cursor_mode);
}

static boolean SetGlobalAnimPart_Viewport(struct GlobalAnimPartControlInfo *part)
{
  int viewport_x;
  int viewport_y;
  int viewport_width;
  int viewport_height;
  boolean changed = FALSE;

  if (part->last_anim_status == global.anim_status &&
      !isClass(part->control_info.class, "pointer"))
    return FALSE;

  part->last_anim_status = global.anim_status;

  part->drawing_stage = DRAW_GLOBAL_ANIM_STAGE_1;

  part->class_playfield_or_door = FALSE;

  if (isClass(part->control_info.class, "window") ||
      isClass(part->control_info.class, "border"))
  {
    viewport_x = 0;
    viewport_y = 0;
    viewport_width  = WIN_XSIZE;
    viewport_height = WIN_YSIZE;

    part->drawing_stage = DRAW_GLOBAL_ANIM_STAGE_2;
  }
  else if (isClass(part->control_info.class, "pointer"))
  {
    int mx = MIN(MAX(0, gfx.mouse_x), WIN_XSIZE - 1);
    int my = MIN(MAX(0, gfx.mouse_y), WIN_YSIZE - 1);

    // prevent displaying off-screen custom mouse cursor in upper left corner
    if (gfx.mouse_x == POS_OFFSCREEN &&
	gfx.mouse_y == POS_OFFSCREEN)
      mx = my = POS_OFFSCREEN;

    viewport_x = mx - part->control_info.x;
    viewport_y = my - part->control_info.y;
    viewport_width  = part->graphic_info.width;
    viewport_height = part->graphic_info.height;

    part->drawing_stage = DRAW_GLOBAL_ANIM_STAGE_3;

    // do not use global animation mouse pointer when reloading artwork
    if (global.anim_status != GAME_MODE_LOADING)
      gfx.cursor_mode_override = CURSOR_NONE;
  }
  else if (isClass(part->control_info.class, "door_1"))
  {
    viewport_x = DX;
    viewport_y = DY;
    viewport_width  = DXSIZE;
    viewport_height = DYSIZE;

    part->class_playfield_or_door = TRUE;
  }
  else if (isClass(part->control_info.class, "door_2"))
  {
    if (part->mode_nr == GAME_MODE_EDITOR)
    {
      viewport_x = EX;
      viewport_y = EY;
      viewport_width  = EXSIZE;
      viewport_height = EYSIZE;
    }
    else
    {
      viewport_x = VX;
      viewport_y = VY;
      viewport_width  = VXSIZE;
      viewport_height = VYSIZE;
    }

    part->class_playfield_or_door = TRUE;
  }
  else		// default: "playfield"
  {
    viewport_x = REAL_SX;
    viewport_y = REAL_SY;
    viewport_width  = FULL_SXSIZE;
    viewport_height = FULL_SYSIZE;

    part->class_playfield_or_door = TRUE;
  }

  if (viewport_x != part->viewport_x ||
      viewport_y != part->viewport_y ||
      viewport_width  != part->viewport_width ||
      viewport_height != part->viewport_height)
  {
    part->viewport_x = viewport_x;
    part->viewport_y = viewport_y;
    part->viewport_width  = viewport_width;
    part->viewport_height = viewport_height;

    if (!isClass(part->control_info.class, "pointer"))
      changed = TRUE;
  }

  return changed;
}

static void PlayGlobalAnimSound(struct GlobalAnimPartControlInfo *part)
{
  int sound = part->sound;

  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  // !!! TODO: ADD STEREO POSITION FOR MOVING ANIMATIONS !!!
  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
  else
    PlaySound(sound);

#if 0
  Debug("anim:PlayGlobalAnimSound", "PLAY SOUND %d.%d.%d: %d",
	 part->anim_nr, part->nr, part->mode_nr, sound);
#endif
}

static void StopGlobalAnimSound(struct GlobalAnimPartControlInfo *part)
{
  int sound = part->sound;

  if (sound == SND_UNDEFINED)
    return;

  StopSound(sound);

#if 0
  Debug("anim:StopGlobalAnimSound", "STOP SOUND %d.%d.%d: %d",
	part->anim_nr, part->nr, part->mode_nr, sound);
#endif
}

static void PlayGlobalAnimMusic(struct GlobalAnimPartControlInfo *part)
{
  int music = part->music;

  if (music == MUS_UNDEFINED)
    return;

  if (!setup.sound_music)
    return;

  if (IS_LOOP_MUSIC(music))
    PlayMusicLoop(music);
  else
    PlayMusic(music);

#if 0
  Debug("anim:PlayGlobalAnimMusic", "PLAY MUSIC %d.%d.%d: %d",
	part->anim_nr, part->nr, part->mode_nr, music);
#endif
}

static void StopGlobalAnimMusic(struct GlobalAnimPartControlInfo *part)
{
  int music = part->music;

  if (music == MUS_UNDEFINED)
    return;

  char *anim_music = getMusicInfoEntryFilename(music);
  char *curr_music = getCurrentlyPlayingMusicFilename();

  // do not stop music if global anim music differs from current music
  if (!strEqual(curr_music, anim_music))
    return;

  StopMusic();

#if 0
  Debug("anim:StopGlobalAnimMusic", "STOP MUSIC %d.%d.%d: %d",
	part->anim_nr, part->nr, part->mode_nr, music);
#endif
}

static void PlayGlobalAnimSoundAndMusic(struct GlobalAnimPartControlInfo *part)
{
  // when drawing animations to fading buffer, do not play sounds or music
  if (drawing_to_fading_buffer)
    return;

  PlayGlobalAnimSound(part);
  PlayGlobalAnimMusic(part);
}

static void StopGlobalAnimSoundAndMusic(struct GlobalAnimPartControlInfo *part)
{
  StopGlobalAnimSound(part);
  StopGlobalAnimMusic(part);
}

static void PlayGlobalAnimSoundIfLoop(struct GlobalAnimPartControlInfo *part)
{
  // when drawing animations to fading buffer, do not play sounds
  if (drawing_to_fading_buffer)
    return;

  // loop sounds only expire when playing
  if (game_status != GAME_MODE_PLAYING)
    return;

  // check if any sound is defined for this animation part
  if (part->sound == SND_UNDEFINED)
    return;

  // normal (non-loop) sounds do not expire when playing
  if (!IS_LOOP_SOUND(part->sound))
    return;

  // prevent expiring loop sounds when playing
  PlayGlobalAnimSound(part);
}

static boolean checkGlobalAnimEvent(int anim_event, int mask)
{
  int mask_anim_only = mask & ~ANIM_EVENT_PART_MASK;
  int mask_ce_only   = mask & ~ANIM_EVENT_PAGE_MASK;

  if (mask & ANIM_EVENT_ANY)
    return (anim_event & ANIM_EVENT_ANY);
  else if (mask & ANIM_EVENT_SELF)
    return (anim_event & ANIM_EVENT_SELF);
  else if (mask & ANIM_EVENT_UNCLICK_ANY)
    return (anim_event & ANIM_EVENT_UNCLICK_ANY);
  else if (mask & ANIM_EVENT_CE_CHANGE)
    return (anim_event == mask ||
	    anim_event == mask_ce_only);
  else
    return (anim_event == mask ||
	    anim_event == mask_anim_only);
}

static boolean isClickablePart(struct GlobalAnimPartControlInfo *part, int mask)
{
  struct GraphicInfo *c = &part->control_info;
  int i;

  if (part->init_event_state)
  {
    int num_init_events = GetGlobalAnimEventValueCount(c->init_event);

    for (i = 0; i < num_init_events; i++)
    {
      int init_event = GetGlobalAnimEventValue(c->init_event, i);

      if (checkGlobalAnimEvent(init_event, mask))
	return TRUE;
    }
  }
  else if (part->anim_event_state)
  {
    int num_anim_events = GetGlobalAnimEventValueCount(c->anim_event);

    for (i = 0; i < num_anim_events; i++)
    {
      int anim_event = GetGlobalAnimEventValue(c->anim_event, i);

      if (checkGlobalAnimEvent(anim_event, mask))
	return TRUE;
    }
  }

  return FALSE;
}

static boolean isInsidePartStacked(struct GlobalAnimPartControlInfo *part,
				   int mx, int my)
{
  struct GraphicInfo *g = &part->graphic_info;
  struct GraphicInfo *c = &part->control_info;
  int part_x = part->viewport_x + part->x;
  int part_y = part->viewport_y + part->y;
  int part_width  = g->width;
  int part_height = g->height;
  int x, y;

  for (y = 0; y < c->stacked_yfactor; y++)
  {
    for (x = 0; x < c->stacked_xfactor; x++)
    {
      int part_stacked_x = part_x + x * (part_width  + c->stacked_xoffset);
      int part_stacked_y = part_y + y * (part_height + c->stacked_yoffset);

      if (mx >= part_stacked_x &&
	  mx <  part_stacked_x + part_width &&
	  my >= part_stacked_y &&
	  my <  part_stacked_y + part_height)
	return TRUE;
    }
  }

  return FALSE;
}

static boolean isClickedPart(struct GlobalAnimPartControlInfo *part,
			     int mx, int my, boolean clicked)
{
  // check if mouse click was detected at all
  if (!clicked)
    return FALSE;

  // check if mouse click is outside the animation part's viewport
  if (mx <  part->viewport_x ||
      mx >= part->viewport_x + part->viewport_width ||
      my <  part->viewport_y ||
      my >= part->viewport_y + part->viewport_height)
    return FALSE;

  // check if mouse click is inside the animation part's (stacked) graphic
  if (isInsidePartStacked(part, mx, my))
    return TRUE;

  return FALSE;
}

static boolean clickBlocked(struct GlobalAnimPartControlInfo *part)
{
  return ((part->control_info.style & STYLE_BLOCK) ? TRUE : FALSE);
}

static boolean clickConsumed(struct GlobalAnimPartControlInfo *part)
{
  return ((part->control_info.style & STYLE_PASSTHROUGH) ? FALSE : TRUE);
}

static void SetGlobalAnimPartTileXY(struct GlobalAnimPartControlInfo *part)
{
  // calculate playfield position (with scrolling) for related CE tile
  // (do not use FX/FY, which are incorrect during envelope requests)
  int FX0 = 2 * TILEX_VAR;	// same as FX during DRAW_TO_FIELDBUFFER
  int FY0 = 2 * TILEY_VAR;	// same as FY during DRAW_TO_FIELDBUFFER
  int fx = getFieldbufferOffsetX_RND(ScreenMovDir, ScreenGfxPos);
  int fy = getFieldbufferOffsetY_RND(ScreenMovDir, ScreenGfxPos);
  int sx = FX0 + SCREENX(part->tile_x) * TILEX_VAR;
  int sy = FY0 + SCREENY(part->tile_y) * TILEY_VAR;
  int cx = SX - REAL_SX;
  int cy = SY - REAL_SY;
  int x = sx - fx + cx;
  int y = sy - fy + cy;

  part->tile_xoffset += part->step_xoffset;
  part->tile_yoffset += part->step_yoffset;

  part->x = x + part->tile_xoffset;
  part->y = y + part->tile_yoffset;
}

static void InitGlobalAnim_Triggered(struct GlobalAnimPartControlInfo *part,
				     boolean *click_consumed,
				     boolean *any_event_action,
				     int event_value, char *info_text)
{
  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[part->mode_nr];

  int gic_anim_nr = part->old_anim_nr + 1;	// X as in "anim_X"
  int gic_part_nr = part->old_nr + 1;		// Y as in "part_Y"
  int mask = event_value | (gic_anim_nr << ANIM_EVENT_ANIM_BIT);

  if (!part->is_base)
    mask |= gic_part_nr << ANIM_EVENT_PART_BIT;

  int anim2_nr;

  for (anim2_nr = 0; anim2_nr < ctrl->num_anims; anim2_nr++)
  {
    struct GlobalAnimMainControlInfo *anim2 = &ctrl->anim[anim2_nr];
    int part2_nr;

    for (part2_nr = 0; part2_nr < anim2->num_parts_all; part2_nr++)
    {
      struct GlobalAnimPartControlInfo *part2 = &anim2->part[part2_nr];

      if (!(part2->state & (ANIM_STATE_RUNNING | ANIM_STATE_WAITING)))
	continue;

      if (isClickablePart(part2, mask))
      {
	part2->triggered = TRUE;
	*click_consumed |= clickConsumed(part); 	// click was on "part"!

#if DEBUG_ANIM_EVENTS
	Debug("anim:InitGlobalAnim_Triggered",
	      "%d.%d TRIGGERED BY %s OF %d.%d",
	      part2->old_anim_nr + 1, part2->old_nr + 1, info_text,
	      part->old_anim_nr + 1, part->old_nr + 1);
#endif
#if 0
	Debug("anim:InitGlobalAnim_Triggered",
	      "%d.%d TRIGGER CLICKED [%d]", anim2_nr, part2_nr,
	      part2->control_info.anim_event_action);
#endif

	// after executing event action, ignore any further actions
	if (!*any_event_action && DoGlobalAnim_EventAction(part2))
	  *any_event_action = TRUE;
      }

#if 0
      struct GraphicInfo *c = &part2->control_info;

      if (isClickablePart(part2, mask))
	Debug("anim:InitGlobalAnim_Triggered",
	      "%d.%d: 0x%08x, 0x%08x [0x%08x] <--- TRIGGERED BY %d.%d",
	      anim2_nr, part2_nr, c->init_event, c->anim_event, mask,
	      anim_nr, part_nr);
      else
	Debug("anim:InitGlobalAnim_Triggered",
	      "%d.%d: 0x%08x, 0x%08x [0x%08x]",
	      anim2_nr, part2_nr, c->init_event, c->anim_event, mask);
#endif
    }
  }
}

static void InitGlobalAnim_Triggered_ByCustomElement(int nr, int page,
						     int x, int y,
						     int trigger_x,
						     int trigger_y)
{
  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[GAME_MODE_PLAYING];

  int event_value = ANIM_EVENT_CE_CHANGE;
  int event_bits = (nr << ANIM_EVENT_CE_BIT) | (page << ANIM_EVENT_PAGE_BIT);
  int mask = event_value | event_bits;
  int anim2_nr;

  for (anim2_nr = 0; anim2_nr < ctrl->num_anims; anim2_nr++)
  {
    struct GlobalAnimMainControlInfo *anim2 = &ctrl->anim[anim2_nr];
    int part2_nr;

    for (part2_nr = 0; part2_nr < anim2->num_parts_all; part2_nr++)
    {
      struct GlobalAnimPartControlInfo *part2 = &anim2->part[part2_nr];

      if (!(part2->state & (ANIM_STATE_RUNNING | ANIM_STATE_WAITING)))
	continue;

      if (isClickablePart(part2, mask) && !part2->triggered)
      {
	struct GraphicInfo *c = &part2->control_info;

	if (c->position == POS_CE ||
	    c->position == POS_CE_TRIGGER)
	{
	  // store CE tile and offset position to handle scrolling
	  part2->tile_x = (c->position == POS_CE_TRIGGER ? trigger_x : x);
	  part2->tile_y = (c->position == POS_CE_TRIGGER ? trigger_y : y);
	  part2->tile_xoffset = c->x;
	  part2->tile_yoffset = c->y;

	  // set resulting animation position relative to CE tile position
	  // (but only for ".init_event", not ".anim_event" type events)
	  if (part2->init_event_state)
	    SetGlobalAnimPartTileXY(part2);

	  // restart animation (by using current sync frame)
	  part2->initial_anim_sync_frame = anim_sync_frame;
	}

	part2->triggered = TRUE;

	// do not trigger any other animation if CE change event was consumed
	if (c->style == STYLE_CONSUME_CE_EVENT)
	  return;

#if 0
	Debug("anim:InitGlobalAnim_Triggered_ByCustomElement",
	      "%d.%d TRIGGERED BY CE %d", anim2_nr, part2_nr, nr + 1);
#endif
      }
    }
  }
}

static void HandleGlobalAnimDelay(struct GlobalAnimPartControlInfo *part,
				  int delay_type, char *info_text)
{
#if DEBUG_ANIM_DELAY
  Debug("anim:HandleGlobalAnimDelay", "%d.%d %s",
	part->old_anim_nr + 1, part->old_nr + 1, info_text);
#endif

  DoGlobalAnim_DelayAction(part, delay_type);
}

static void HandleGlobalAnimEvent(struct GlobalAnimPartControlInfo *part,
				  int event_value, char *info_text)
{
#if DEBUG_ANIM_EVENTS
  Debug("anim:HandleGlobalAnimEvent", "%d.%d %s",
	part->old_anim_nr + 1, part->old_nr + 1, info_text);
#endif

  boolean click_consumed = FALSE;
  boolean any_event_action = FALSE;

  // check if this event is defined to trigger other animations
  InitGlobalAnim_Triggered(part, &click_consumed, &any_event_action,
			   event_value, info_text);
}

static int HandleGlobalAnim_Part(struct GlobalAnimPartControlInfo *part,
				 int state)
{
  if (handle_click && !part->clicked)
    return state;

  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[part->mode_nr];
  struct GlobalAnimMainControlInfo *anim = &ctrl->anim[part->anim_nr];
  struct GraphicInfo *g = &part->graphic_info;
  struct GraphicInfo *c = &part->control_info;
  boolean viewport_changed = SetGlobalAnimPart_Viewport(part);
  int alpha = (g->alpha != -1 ? g->alpha : SDL_ALPHA_OPAQUE);

  // if game is paused, also pause playfield and door animations
  if (isPausedOnPlayfieldOrDoor(part))
    return state;

  if (viewport_changed)
    state |= ANIM_STATE_RESTART;

  if (state & ANIM_STATE_RESTART)
  {
    // when drawing animations to fading buffer, only start fixed animations
    if (drawing_to_fading_buffer && (c->x == ARG_UNDEFINED_VALUE ||
				     c->y == ARG_UNDEFINED_VALUE))
      return ANIM_STATE_INACTIVE;

    ResetDelayCounterExt(&part->step_delay, anim_sync_frame);

    part->init_delay_counter =
      (c->init_delay_fixed + GetSimpleRandom(c->init_delay_random));

    part->anim_delay_counter =
      (c->anim_delay_fixed + GetSimpleRandom(c->anim_delay_random));

    part->post_delay_counter = 0;

    part->init_event_state = (c->init_event != ANIM_EVENT_UNDEFINED);
    part->anim_event_state = (c->anim_event != ANIM_EVENT_UNDEFINED);

    part->initial_anim_sync_frame =
      (g->anim_global_sync || g->anim_global_anim_sync ? 0 :
       anim_sync_frame + part->init_delay_counter);

    // do not re-initialize random animation frame after fade-in
    if (part->anim_random_frame == -1)
      part->anim_random_frame = GetSimpleRandom(g->anim_frames);

    if (c->fade_mode & FADE_MODE_FADE)
    {
      // when fading in screen, first frame is 100 % transparent or opaque
      part->fade_delay_counter = c->fade_delay + 1;
      part->fade_alpha = (c->fade_mode == FADE_MODE_FADE_IN ? 0 : alpha);
    }
    else
    {
      part->fade_delay_counter = 0;
      part->fade_alpha = -1;
    }

    if (c->direction & MV_HORIZONTAL)
    {
      int pos_bottom = part->viewport_height - g->height;

      if (c->position == POS_TOP)
	part->y = 0;
      else if (c->position == POS_UPPER)
	part->y = GetSimpleRandom(pos_bottom / 2);
      else if (c->position == POS_MIDDLE)
	part->y = pos_bottom / 2;
      else if (c->position == POS_LOWER)
	part->y = pos_bottom / 2 + GetSimpleRandom(pos_bottom / 2);
      else if (c->position == POS_BOTTOM)
	part->y = pos_bottom;
      else
	part->y = GetSimpleRandom(pos_bottom);

      if (c->direction == MV_RIGHT)
      {
	part->step_xoffset = c->step_offset;
	part->x = -g->width + part->step_xoffset;
      }
      else
      {
	part->step_xoffset = -c->step_offset;
	part->x = part->viewport_width + part->step_xoffset;
      }

      part->step_yoffset = 0;
    }
    else if (c->direction & MV_VERTICAL)
    {
      int pos_right = part->viewport_width - g->width;

      if (c->position == POS_LEFT)
	part->x = 0;
      else if (c->position == POS_RIGHT)
	part->x = pos_right;
      else
	part->x = GetSimpleRandom(pos_right);

      if (c->direction == MV_DOWN)
      {
	part->step_yoffset = c->step_offset;
	part->y = -g->height + part->step_yoffset;
      }
      else
      {
	part->step_yoffset = -c->step_offset;
	part->y = part->viewport_height + part->step_yoffset;
      }

      part->step_xoffset = 0;
    }
    else
    {
      part->x = 0;
      part->y = 0;

      part->step_xoffset = 0;
      part->step_yoffset = 0;
    }

    if (!isClass(part->control_info.class, "pointer"))
    {
      if (c->x != ARG_UNDEFINED_VALUE)
	part->x = c->x;
      if (c->y != ARG_UNDEFINED_VALUE)
	part->y = c->y;
    }

    if (c->position == POS_LAST &&
	anim->last_x > -g->width  && anim->last_x < part->viewport_width &&
	anim->last_y > -g->height && anim->last_y < part->viewport_height)
    {
      part->x = anim->last_x;
      part->y = anim->last_y;
    }

    if (c->step_xoffset != ARG_UNDEFINED_VALUE)
      part->step_xoffset = c->step_xoffset;
    if (c->step_yoffset != ARG_UNDEFINED_VALUE)
      part->step_yoffset = c->step_yoffset;

    if (part->init_delay_counter == 0 &&
	!part->init_event_state)
    {
      PlayGlobalAnimSoundAndMusic(part);

      HandleGlobalAnimDelay(part, ANIM_DELAY_INIT,  "START [INIT_DELAY]");
      HandleGlobalAnimEvent(part, ANIM_EVENT_START, "START [ANIM]");
    }
    else
    {
      HandleGlobalAnimEvent(part, ANIM_EVENT_INIT, "START [INIT_DELAY/EVENT]");
    }
  }

  if (part->clicked &&
      part->init_event_state)
  {
    if (part->initial_anim_sync_frame > 0)
      part->initial_anim_sync_frame = anim_sync_frame;

    part->init_delay_counter = 1;
    part->init_event_state = FALSE;

    part->clicked = FALSE;
  }

  if (part->clicked &&
      part->anim_event_state)
  {
    part->anim_delay_counter = 1;
    part->anim_event_state = FALSE;

    part->clicked = FALSE;
  }

  if (part->init_delay_counter > 0)
  {
    part->init_delay_counter--;

    if (part->init_delay_counter == 0)
    {
      part->init_event_state = FALSE;

      PlayGlobalAnimSoundAndMusic(part);

      HandleGlobalAnimDelay(part, ANIM_DELAY_INIT,  "START [INIT_DELAY]");
      HandleGlobalAnimEvent(part, ANIM_EVENT_START, "START [ANIM]");

      // continue with state ANIM_STATE_RUNNING (set below)
    }
    else
    {
      return ANIM_STATE_WAITING;
    }
  }

  if (part->init_event_state)
    return ANIM_STATE_WAITING;

  // animation part is now running/visible and therefore clickable
  part->clickable = TRUE;

  // check if moving animation has left the visible screen area
  if ((part->x <= -g->width              && part->step_xoffset <= 0) ||
      (part->x >=  part->viewport_width  && part->step_xoffset >= 0) ||
      (part->y <= -g->height             && part->step_yoffset <= 0) ||
      (part->y >=  part->viewport_height && part->step_yoffset >= 0))
  {
    // do not wait for "anim" events for off-screen animations
    part->anim_event_state = FALSE;

    // do not stop animation before "anim" or "post" counter are finished
    if (part->anim_delay_counter == 0 &&
	part->post_delay_counter == 0)
    {
      StopGlobalAnimSoundAndMusic(part);

      HandleGlobalAnimEvent(part, ANIM_EVENT_END, "END [ANIM/OFF-SCREEN]");

      part->post_delay_counter =
	(c->post_delay_fixed + GetSimpleRandom(c->post_delay_random));

      if (part->post_delay_counter > 0)
	return ANIM_STATE_RUNNING;

      // drawing last frame not needed here -- animation not visible anymore
      return ANIM_STATE_RESTART;
    }
  }

  if (part->anim_delay_counter > 0)
  {
    part->anim_delay_counter--;

    if (part->anim_delay_counter == 0)
    {
      part->anim_event_state = FALSE;

      StopGlobalAnimSoundAndMusic(part);

      HandleGlobalAnimDelay(part, ANIM_DELAY_ANIM, "END [ANIM_DELAY]");
      HandleGlobalAnimEvent(part, ANIM_EVENT_END,  "END [ANIM_DELAY/EVENT]");

      part->post_delay_counter =
	(c->post_delay_fixed + GetSimpleRandom(c->post_delay_random));

      if (part->post_delay_counter > 0)
	return ANIM_STATE_RUNNING;

      // additional state "RUNNING" required to not skip drawing last frame
      return ANIM_STATE_RESTART | ANIM_STATE_RUNNING;
    }
  }

  if (part->post_delay_counter > 0)
  {
    part->post_delay_counter--;

    if (part->post_delay_counter == 0)
    {
      HandleGlobalAnimDelay(part, ANIM_DELAY_POST, "END [POST_DELAY]");
      HandleGlobalAnimEvent(part, ANIM_EVENT_POST, "END [POST_DELAY]");

      return ANIM_STATE_RESTART;
    }

    return ANIM_STATE_WAITING;
  }

  if (part->fade_delay_counter > 0)
  {
    part->fade_delay_counter--;
    part->fade_alpha = alpha * (c->fade_mode == FADE_MODE_FADE_IN ?
				c->fade_delay - part->fade_delay_counter :
				part->fade_delay_counter) / c->fade_delay;
  }

  // special case to prevent expiring loop sounds when playing
  PlayGlobalAnimSoundIfLoop(part);

  if (!DelayReachedExt(&part->step_delay, anim_sync_frame))
    return ANIM_STATE_RUNNING;

#if 0
  {
    static unsigned int last_counter = -1;
    unsigned int counter = Counter();

    Debug("anim:HandleGlobalAnim_Part", "NEXT ANIM PART [%d, %d]",
	  anim_sync_frame, counter - last_counter);

    last_counter = counter;
  }
#endif

  if (c->position == POS_CE ||
      c->position == POS_CE_TRIGGER)
  {
    SetGlobalAnimPartTileXY(part);
  }
  else
  {
    part->x += part->step_xoffset;
    part->y += part->step_yoffset;
  }

  anim->last_x = part->x;
  anim->last_y = part->y;

  return ANIM_STATE_RUNNING;
}

static void HandleGlobalAnim_Main(struct GlobalAnimMainControlInfo *anim,
				  int action)
{
  struct GlobalAnimPartControlInfo *part;
  struct GraphicInfo *c = &anim->control_info;
  int num_parts = anim->num_parts + (anim->has_base ? 1 : 0);
  int state, active_part_nr;
  int i;

#if 0
  Debug("anim:HandleGlobalAnim_Main", "%d, %d => %d",
	 anim->mode_nr, anim->nr, anim->num_parts);
  Debug("anim:HandleGlobalAnim_Main",
	"%d, %d, %d", global.num_toons, setup.toons, num_toons);
#endif

#if 0
  Debug("anim:HandleGlobalAnim_Main", "%s(%d): %d, %d, %d [%d]",
	(action == ANIM_START ? "ANIM_START" :
	 action == ANIM_CONTINUE ? "ANIM_CONTINUE" :
	 action == ANIM_STOP ? "ANIM_STOP" : "(should not happen)"),
	anim->nr,
	anim->state & ANIM_STATE_RESTART,
	anim->state & ANIM_STATE_WAITING,
	anim->state & ANIM_STATE_RUNNING,
	anim->num_parts);
#endif

  switch (action)
  {
    case ANIM_START:
      anim->state = anim->last_state = ANIM_STATE_RESTART;
      anim->active_part_nr = anim->last_active_part_nr = 0;
      anim->part_counter = 0;

      break;

    case ANIM_CONTINUE:
      if (anim->state == ANIM_STATE_INACTIVE)
	return;

      anim->state = anim->last_state;
      anim->active_part_nr = anim->last_active_part_nr;

      break;

    case ANIM_STOP:
      anim->state = ANIM_STATE_INACTIVE;

      for (i = 0; i < num_parts; i++)
	StopGlobalAnimSoundAndMusic(&anim->part[i]);

      return;

    default:
      break;
  }

  if (c->anim_mode & ANIM_ALL || anim->num_parts == 0)
  {
#if 0
    Debug("anim:HandleGlobalAnim_Main", "%d, %d => %d",
	  anim->mode_nr, anim->nr, num_parts);
#endif

    for (i = 0; i < num_parts; i++)
    {
      part = &anim->part[i];

      switch (action)
      {
        case ANIM_START:
	  anim->state = ANIM_STATE_RUNNING;
	  part->state = ANIM_STATE_RESTART;

	  break;

        case ANIM_CONTINUE:
	  if (part->state == ANIM_STATE_INACTIVE)
	    continue;

	  break;

        case ANIM_STOP:
	  part->state = ANIM_STATE_INACTIVE;

	  continue;

        default:
	  break;
      }

      part->state = HandleGlobalAnim_Part(part, part->state);

      // when animation mode is "once", stop after animation was played once
      if (c->anim_mode & ANIM_ONCE &&
	  part->state & ANIM_STATE_RESTART)
	part->state = ANIM_STATE_INACTIVE;
    }

    anim->last_state = anim->state;
    anim->last_active_part_nr = anim->active_part_nr;

    return;
  }

  if (anim->state & ANIM_STATE_RESTART)		// directly after restart
    anim->active_part_nr = getGlobalAnimationPart(anim);

  part = &anim->part[anim->active_part_nr];

  // first set all animation parts to "inactive", ...
  for (i = 0; i < num_parts; i++)
    anim->part[i].state = ANIM_STATE_INACTIVE;

  // ... then set current animation part to "running" ...
  part->state = ANIM_STATE_RUNNING;

  // ... unless it is waiting for an initial event
  if (part->init_event_state)
    part->state = ANIM_STATE_WAITING;

  anim->state = HandleGlobalAnim_Part(part, anim->state);

  if (anim->state & ANIM_STATE_RESTART)
    anim->part_counter++;

  // when animation mode is "once", stop after all animations were played once
  if (c->anim_mode & ANIM_ONCE &&
      anim->part_counter == anim->num_parts)
    anim->state = ANIM_STATE_INACTIVE;

  state = anim->state;
  active_part_nr = anim->active_part_nr;

  // while the animation parts are pausing (waiting or inactive), play the base
  // (main) animation; this corresponds to the "boring player animation" logic
  // (!!! KLUDGE WARNING: I THINK THIS IS A MESS THAT SHOULD BE CLEANED UP !!!)
  if (anim->has_base)
  {
    if (anim->state == ANIM_STATE_WAITING ||
	anim->state == ANIM_STATE_INACTIVE)
    {
      anim->active_part_nr = anim->num_parts;	// part nr of base animation
      part = &anim->part[anim->active_part_nr];

      if (anim->state != anim->last_state)
	part->state = ANIM_STATE_RESTART;

      anim->state = ANIM_STATE_RUNNING;
      part->state = HandleGlobalAnim_Part(part, part->state);
    }
  }

  anim->last_state = state;
  anim->last_active_part_nr = active_part_nr;
}

static void HandleGlobalAnim_Mode(struct GlobalAnimControlInfo *ctrl, int action)
{
  int i;

#if 0
  Debug("anim:HandleGlobalAnim_Mode", "%d => %d", ctrl->nr, ctrl->num_anims);
#endif

  for (i = 0; i < ctrl->num_anims; i++)
    HandleGlobalAnim_Main(&ctrl->anim[i], action);
}

static void HandleGlobalAnim(int action, int game_mode)
{
#if 0
  Debug("anim:HandleGlobalAnim", "mode == %d", game_status);
#endif

  HandleGlobalAnim_Mode(&global_anim_ctrl[game_mode], action);
}

static void DoAnimationExt(void)
{
  int i;

#if 0
  Debug("anim:DoAnimationExt", "%d, %d", anim_sync_frame, Counter());
#endif

  // global animations now synchronized with frame delay of screen update
  anim_sync_frame++;

  for (i = 0; i < NUM_GAME_MODES; i++)
    HandleGlobalAnim(ANIM_CONTINUE, i);

#if 0
  // force screen redraw in next frame to continue drawing global animations
  redraw_mask = REDRAW_ALL;
#endif
}

static void DoGlobalAnim_DelayAction(struct GlobalAnimPartControlInfo *part,
				     int delay_type)
{
  int delay_action =
    (delay_type == ANIM_DELAY_INIT ? part->control_info.init_delay_action :
     delay_type == ANIM_DELAY_ANIM ? part->control_info.anim_delay_action :
     delay_type == ANIM_DELAY_POST ? part->control_info.post_delay_action :
     ANIM_DELAY_ACTION_NONE);

  if (delay_action == ANIM_DELAY_ACTION_NONE)
    return;

  PushUserEvent(USEREVENT_ANIM_DELAY_ACTION, delay_action, 0);
}

static boolean DoGlobalAnim_EventAction(struct GlobalAnimPartControlInfo *part)
{
  int event_action = (part->init_event_state ?
		      part->control_info.init_event_action :
		      part->control_info.anim_event_action);

  if (event_action == ANIM_EVENT_ACTION_NONE)
    return FALSE;

  if (event_action < MAX_IMAGE_FILES)
    PushUserEvent(USEREVENT_ANIM_EVENT_ACTION, event_action, 0);
  else
    OpenURLFromHash(anim_url_hash, event_action);

  // check if further actions are allowed to be executed
  if (part->control_info.style & STYLE_MULTIPLE_ACTIONS)
    return FALSE;

  return TRUE;
}

static void InitGlobalAnim_Clickable(void)
{
  int i;

  for (i = 0; i < num_global_anim_list; i++)
  {
    struct GlobalAnimPartControlInfo *part = global_anim_list[i];

    if (part->triggered)
      part->clicked = TRUE;

    part->triggered = FALSE;
    part->clickable = FALSE;
  }
}

#define ANIM_CLICKED_RESET	0
#define ANIM_CLICKED_PRESSED	1
#define ANIM_CLICKED_RELEASED	2

static boolean InitGlobalAnim_Clicked(int mx, int my, int clicked_event)
{
  boolean click_consumed = FALSE;
  boolean anything_clicked = FALSE;
  boolean any_part_clicked = FALSE;
  boolean any_event_action = FALSE;
  int i;

  // check animation parts in reverse draw order (to stop when clicked)
  for (i = num_global_anim_list - 1; i >= 0; i--)
  {
    struct GlobalAnimPartControlInfo *part = global_anim_list[i];

    // if request dialog is active, only handle pointer-style animations
    if (game.request_active &&
	!isClass(part->control_info.class, "pointer"))
      continue;

    if (clicked_event == ANIM_CLICKED_RESET)
    {
      part->clicked = FALSE;

      continue;
    }

    if (!part->clickable)
      continue;

    if (!(part->state & ANIM_STATE_RUNNING))
      continue;

    // always handle "any" click events (clicking anywhere on screen) ...
    if (clicked_event == ANIM_CLICKED_PRESSED &&
	isClickablePart(part, ANIM_EVENT_ANY))
    {
#if DEBUG_ANIM_EVENTS
      Debug("anim:InitGlobalAnim_Clicked", "%d.%d TRIGGERED BY ANY",
	    part->old_anim_nr + 1, part->old_nr + 1);
#endif

      anything_clicked = part->clicked = TRUE;
      click_consumed |= clickConsumed(part);
    }

    // always handle "unclick:any" events (releasing anywhere on screen) ...
    if (clicked_event == ANIM_CLICKED_RELEASED &&
	isClickablePart(part, ANIM_EVENT_UNCLICK_ANY))
    {
#if DEBUG_ANIM_EVENTS
      Debug("anim:InitGlobalAnim_Clicked", "%d.%d TRIGGERED BY UNCLICK:ANY",
	    part->old_anim_nr + 1, part->old_nr + 1);
#endif

      anything_clicked = part->clicked = TRUE;
      click_consumed |= clickConsumed(part);
    }

    // ... but only handle the first (topmost) clickable animation
    if (any_part_clicked)
      continue;

    if (clicked_event == ANIM_CLICKED_PRESSED &&
	isClickedPart(part, mx, my, TRUE))
    {
#if 0
      Debug("anim:InitGlobalAnim_Clicked", "%d.%d CLICKED [%d]",
	    anim_nr, part_nr, part->control_info.anim_event_action);
#endif

      // after executing event action, ignore any further actions
      if (!any_event_action && DoGlobalAnim_EventAction(part))
	any_event_action = TRUE;

      // determine if mouse clicks should be blocked from other animations
      any_part_clicked |= clickConsumed(part);

      if (isClickablePart(part, ANIM_EVENT_SELF))
      {
#if DEBUG_ANIM_EVENTS
	Debug("anim:InitGlobalAnim_Clicked", "%d.%d TRIGGERED BY SELF",
	      part->old_anim_nr + 1, part->old_nr + 1);
#endif

	anything_clicked = part->clicked = TRUE;
	click_consumed |= clickConsumed(part);
      }

      // determine if mouse clicks should be blocked by this animation
      click_consumed |= clickBlocked(part);

      // check if this click is defined to trigger other animations
      InitGlobalAnim_Triggered(part, &click_consumed, &any_event_action,
			       ANIM_EVENT_CLICK, "CLICK");
    }
  }

  if (anything_clicked)
  {
    handle_click = TRUE;

    for (i = 0; i < NUM_GAME_MODES; i++)
      HandleGlobalAnim(ANIM_CONTINUE, i);

    handle_click = FALSE;

    // prevent ignoring release event if processed within same game frame
    StopProcessingEvents();
  }

  return (click_consumed || any_event_action);
}

static void ResetGlobalAnim_Clickable(void)
{
  InitGlobalAnim_Clickable();
}

static void ResetGlobalAnim_Clicked(void)
{
  InitGlobalAnim_Clicked(-1, -1, ANIM_CLICKED_RESET);
}

void RestartGlobalAnimsByStatus(int status)
{
  int global_anim_status_last = global.anim_status;

  global.anim_status = status;

  // force restarting global animations by changed global animation status
  DrawGlobalAnimationsExt(DRAW_TO_SCREEN, DRAW_GLOBAL_ANIM_STAGE_RESTART);

  global.anim_status = global_anim_status_last;
}

void SetAnimStatusBeforeFading(int status)
{
  anim_status_last_before_fading = status;
}

boolean HandleGlobalAnimClicks(int mx, int my, int button, boolean force_click)
{
  static boolean click_consumed = FALSE;
  static int last_button = 0;
  boolean press_event;
  boolean release_event;
  boolean click_consumed_current = click_consumed;

  if (button != 0 && force_click)
    last_button = 0;

  // check if button state has changed since last invocation
  press_event   = (button != 0 && last_button == 0);
  release_event = (button == 0 && last_button != 0);
  last_button = button;

  if (press_event)
  {
    click_consumed = InitGlobalAnim_Clicked(mx, my, ANIM_CLICKED_PRESSED);
    click_consumed_current = click_consumed;
  }

  if (release_event)
  {
    InitGlobalAnim_Clicked(mx, my, ANIM_CLICKED_RELEASED);
    click_consumed = FALSE;
  }

  return click_consumed_current;
}

int getGlobalAnimSyncFrame(void)
{
  return anim_sync_frame;
}

void HandleGlobalAnimEventByElementChange(int element, int page, int x, int y,
					  int trigger_x, int trigger_y)
{
  if (!IS_CUSTOM_ELEMENT(element))
    return;

  // custom element stored as 0 to 255, change page stored as 1 to 32
  InitGlobalAnim_Triggered_ByCustomElement(element - EL_CUSTOM_START, page + 1,
					   x, y, trigger_x, trigger_y);
}
