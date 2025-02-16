// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// system.h
// ============================================================================

#ifndef SYSTEM_H
#define SYSTEM_H

#include "platform.h"
#include "types.h"


#if defined(PLATFORM_MAC)
#include "macosx.h"
#elif defined(PLATFORM_WINDOWS)
#include "windows.h"
#elif defined(PLATFORM_ANDROID)
#include "android.h"
#elif defined(PLATFORM_EMSCRIPTEN)
#include "emscripten.h"
#endif

#include "sdl.h"


// the additional 'b' is needed for Win32 to open files in binary mode
#define MODE_READ			"rb"
#define MODE_WRITE			"wb"
#define MODE_APPEND			"ab"

#define DEFAULT_DEPTH			0

#define BLIT_OPAQUE			0
#define BLIT_MASKED			1
#define BLIT_INVERSE			2
#define BLIT_ON_BACKGROUND		3

// values for fullscreen status
#define FULLSCREEN_NOT_AVAILABLE	FALSE
#define FULLSCREEN_AVAILABLE		TRUE

// values for window scaling
#define WINDOW_SCALING_NOT_AVAILABLE	FALSE
#define WINDOW_SCALING_AVAILABLE	TRUE

#define MIN_WINDOW_SCALING_PERCENT	30
#define STD_WINDOW_SCALING_PERCENT	100
#define MAX_WINDOW_SCALING_PERCENT	400
#define STEP_WINDOW_SCALING_PERCENT	10

// values for window scaling quality
#define SCALING_QUALITY_NEAREST		"nearest"
#define SCALING_QUALITY_LINEAR		"linear"
#define SCALING_QUALITY_BEST		"best"

#define SCALING_QUALITY_DEFAULT		SCALING_QUALITY_LINEAR

// values for screen rendering mode
#define STR_SPECIAL_RENDERING_OFF	"stream_texture_only"
#define STR_SPECIAL_RENDERING_BITMAP	"bitmap_and_stream_texture"
#define STR_SPECIAL_RENDERING_TARGET	"target_texture_only"
#define STR_SPECIAL_RENDERING_DOUBLE	"stream_and_target_texture"

#if defined(PLATFORM_EMSCRIPTEN)
#define STR_SPECIAL_RENDERING_DEFAULT	STR_SPECIAL_RENDERING_BITMAP
#else
#define STR_SPECIAL_RENDERING_DEFAULT	STR_SPECIAL_RENDERING_DOUBLE
#endif

#define SPECIAL_RENDERING_OFF		0
#define SPECIAL_RENDERING_BITMAP	1
#define SPECIAL_RENDERING_TARGET	2
#define SPECIAL_RENDERING_DOUBLE	3

// values for vertical screen retrace synchronization (vsync)
#define STR_VSYNC_MODE_OFF		"off"
#define STR_VSYNC_MODE_NORMAL		"normal"
#define STR_VSYNC_MODE_ADAPTIVE		"adaptive"

#define STR_VSYNC_MODE_DEFAULT		STR_VSYNC_MODE_OFF

#define VSYNC_MODE_OFF			0
#define VSYNC_MODE_NORMAL		1
#define VSYNC_MODE_ADAPTIVE		-1

#define VSYNC_MODE_DEFAULT		VSYNC_MODE_OFF

#define VSYNC_MODE_STR_TO_INT(s)					\
  (strEqual((s), STR_VSYNC_MODE_NORMAL)		? VSYNC_MODE_NORMAL :	\
   strEqual((s), STR_VSYNC_MODE_ADAPTIVE)	? VSYNC_MODE_ADAPTIVE :	\
   VSYNC_MODE_OFF)

#define VSYNC_MODE_INT_TO_STR(i)					\
  ((i) == VSYNC_MODE_NORMAL		? STR_VSYNC_MODE_NORMAL :	\
   (i) == VSYNC_MODE_ADAPTIVE		? STR_VSYNC_MODE_ADAPTIVE :	\
   STR_VSYNC_MODE_OFF)

// values for network server settings
#define STR_NETWORK_AUTO_DETECT		"auto_detect_network_server"
#define STR_NETWORK_AUTO_DETECT_SETUP	"(auto detect network server)"

// values for API server settings
#define API_SERVER_HOSTNAME		"api.artsoft.org"
#define API_SERVER_PORT			80
#define API_SERVER_METHOD		"POST"
#define API_SERVER_URI_ADD		"/api/scores/add"
#define API_SERVER_URI_GET		"/api/scores/get"
#define API_SERVER_URI_GETTAPE		"/api/scores/gettape"
#define API_SERVER_URI_RENAME		"/api/players/rename"
#define API_SERVER_URI_RESETUUID	"/api/players/resetuuid"

#if defined(TESTING)
#undef API_SERVER_HOSTNAME
#define API_SERVER_HOSTNAME		"api-test.artsoft.org"
#define TEST_PREFIX			"test."
#else
#define TEST_PREFIX			""
#endif

// values for touch control
#define TOUCH_CONTROL_OFF		"off"
#define TOUCH_CONTROL_VIRTUAL_BUTTONS	"virtual_buttons"
#define TOUCH_CONTROL_WIPE_GESTURES	"wipe_gestures"
#define TOUCH_CONTROL_FOLLOW_FINGER	"follow_finger"

#if defined(PLATFORM_ANDROID)
#define TOUCH_CONTROL_DEFAULT		TOUCH_CONTROL_VIRTUAL_BUTTONS
#else
#define TOUCH_CONTROL_DEFAULT		TOUCH_CONTROL_OFF
#endif

#define TOUCH_MOVE_DISTANCE_DEFAULT	2
#define TOUCH_DROP_DISTANCE_DEFAULT	5
#define TOUCH_TRANSPARENCY_DEFAULT	50

#define ALPHA_FROM_TRANSPARENCY(x)	((100 - x) * SDL_ALPHA_OPAQUE / 100)
#define ALPHA_FADING_STEPSIZE(x)	((x) / 25)

// values for special settings for mobile devices
#if defined(PLATFORM_ANDROID)
#define HAS_TOUCH_DEVICE
#define USE_TOUCH_INPUT_OVERLAY
#define USE_COMPLETE_DISPLAY
#define HAS_SCREEN_KEYBOARD
#define SCREEN_KEYBOARD_POS(h)		((h) * 40 / 100)
#endif

// values for drag-and-drop support (some parts not added before SDL 2.0.5)
#if !SDL_VERSION_ATLEAST(2,0,5)
#define SDL_DROPTEXT			(SDL_DROPFILE + 1)
#define SDL_DROPBEGIN			(SDL_DROPFILE + 2)
#define SDL_DROPCOMPLETE		(SDL_DROPFILE + 3)
#endif

// default input keys
#define DEFAULT_KEY_LEFT		KSYM_Left
#define DEFAULT_KEY_RIGHT		KSYM_Right
#define DEFAULT_KEY_UP			KSYM_Up
#define DEFAULT_KEY_DOWN		KSYM_Down
#if defined(PLATFORM_MAC)
#define DEFAULT_KEY_SNAP		KSYM_Control_L
#define DEFAULT_KEY_DROP		KSYM_KP_Enter
#else
#define DEFAULT_KEY_SNAP		KSYM_Control_L
#define DEFAULT_KEY_DROP		KSYM_Control_R
#endif
#define DEFAULT_KEY_OKAY		KSYM_Return
#define DEFAULT_KEY_CANCEL		KSYM_Escape

// default shortcut keys
#define DEFAULT_KEY_SAVE_GAME		KSYM_F1
#define DEFAULT_KEY_LOAD_GAME		KSYM_F2
#define DEFAULT_KEY_RESTART_GAME	KSYM_F3
#define DEFAULT_KEY_PAUSE_BEFORE_END	KSYM_F4
#define DEFAULT_KEY_TOGGLE_PAUSE	KSYM_space
#define DEFAULT_KEY_FOCUS_PLAYER_1	KSYM_F5
#define DEFAULT_KEY_FOCUS_PLAYER_2	KSYM_F6
#define DEFAULT_KEY_FOCUS_PLAYER_3	KSYM_F7
#define DEFAULT_KEY_FOCUS_PLAYER_4	KSYM_F8
#define DEFAULT_KEY_FOCUS_PLAYER_ALL	KSYM_F9
#define DEFAULT_KEY_TAPE_EJECT		KSYM_UNDEFINED
#define DEFAULT_KEY_TAPE_EXTRA		KSYM_UNDEFINED
#define DEFAULT_KEY_TAPE_STOP		KSYM_UNDEFINED
#define DEFAULT_KEY_TAPE_PAUSE		KSYM_UNDEFINED
#define DEFAULT_KEY_TAPE_RECORD		KSYM_UNDEFINED
#define DEFAULT_KEY_TAPE_PLAY		KSYM_UNDEFINED
#define DEFAULT_KEY_SOUND_SIMPLE	KSYM_UNDEFINED
#define DEFAULT_KEY_SOUND_LOOPS		KSYM_UNDEFINED
#define DEFAULT_KEY_SOUND_MUSIC		KSYM_UNDEFINED
#define DEFAULT_KEY_SNAP_LEFT		KSYM_UNDEFINED
#define DEFAULT_KEY_SNAP_RIGHT		KSYM_UNDEFINED
#define DEFAULT_KEY_SNAP_UP		KSYM_UNDEFINED
#define DEFAULT_KEY_SNAP_DOWN		KSYM_UNDEFINED
#define DEFAULT_KEY_SPEED_FAST		KSYM_f
#define DEFAULT_KEY_SPEED_SLOW		KSYM_s
#define DEFAULT_KEY_SUICIDE		KSYM_F12
#define DEFAULT_KEY_TOGGLE_PANEL	KSYM_Shift_L

// default debug setup keys and values
#define DEFAULT_FRAME_DELAY_0		20		// 100 % speed
#define DEFAULT_FRAME_DELAY_1		500		// 4 % speed
#define DEFAULT_FRAME_DELAY_2		250		// 8 % speed
#define DEFAULT_FRAME_DELAY_3		125		// 16 % speed
#define DEFAULT_FRAME_DELAY_4		60		// 33 % speed
#define DEFAULT_FRAME_DELAY_5		40		// 50 % speed
#define DEFAULT_FRAME_DELAY_6		30		// 66 % speed
#define DEFAULT_FRAME_DELAY_7		10		// 200 % speed
#define DEFAULT_FRAME_DELAY_8		5		// 400 % speed
#define DEFAULT_FRAME_DELAY_9		0		// maximum speed

#define DEFAULT_KEY_FRAME_DELAY_0	KSYM_0
#define DEFAULT_KEY_FRAME_DELAY_1	KSYM_1
#define DEFAULT_KEY_FRAME_DELAY_2	KSYM_2
#define DEFAULT_KEY_FRAME_DELAY_3	KSYM_3
#define DEFAULT_KEY_FRAME_DELAY_4	KSYM_4
#define DEFAULT_KEY_FRAME_DELAY_5	KSYM_5
#define DEFAULT_KEY_FRAME_DELAY_6	KSYM_6
#define DEFAULT_KEY_FRAME_DELAY_7	KSYM_7
#define DEFAULT_KEY_FRAME_DELAY_8	KSYM_8
#define DEFAULT_KEY_FRAME_DELAY_9	KSYM_9

#define NUM_DEBUG_FRAME_DELAY_KEYS	10

#define DEFAULT_FRAME_DELAY_USE_MOD_KEY	FALSE
#define DEFAULT_FRAME_DELAY_GAME_ONLY	TRUE

// values for key_status
#define KEY_NOT_PRESSED			FALSE
#define KEY_RELEASED			FALSE
#define KEY_PRESSED			TRUE

// values for button status
#define MB_NOT_PRESSED			FALSE
#define MB_NOT_RELEASED			TRUE
#define MB_RELEASED			FALSE
#define MB_PRESSED			TRUE
#define MB_MENU_CHOICE			FALSE
#define MB_MENU_MARK			TRUE
#define MB_MENU_INITIALIZE		(-1)
#define MB_MENU_LEAVE			(-2)
#define MB_MENU_CONTINUE		(-3)
#define MB_LEFTBUTTON			1
#define MB_MIDDLEBUTTON			2
#define MB_RIGHTBUTTON			3
#define MB_WHEEL_UP			4
#define MB_WHEEL_DOWN			5
#define MB_WHEEL_LEFT			6
#define MB_WHEEL_RIGHT			7
#define IS_WHEEL_BUTTON_VERTICAL(b)	((b) == MB_WHEEL_UP ||		\
					 (b) == MB_WHEEL_DOWN)
#define IS_WHEEL_BUTTON_HORIZONTAL(b)	((b) == MB_WHEEL_LEFT ||	\
					 (b) == MB_WHEEL_RIGHT)
#define IS_WHEEL_BUTTON(b)		(IS_WHEEL_BUTTON_VERTICAL(b) ||	\
					 IS_WHEEL_BUTTON_HORIZONTAL(b))
#define DEFAULT_WHEEL_STEPS		3

#define BUTTON_STEPSIZE(b)		((b) == MB_LEFTBUTTON   ?  1 :	\
					 (b) == MB_MIDDLEBUTTON ?  5 :	\
					 (b) == MB_RIGHTBUTTON  ? 10 : 1)

// values for move directions
#define MV_BIT_LEFT			0
#define MV_BIT_RIGHT			1
#define MV_BIT_UP			2
#define MV_BIT_DOWN	       		3

#define NUM_DIRECTIONS			4

// diagonal movement directions are used in a different contect than buttons
#define MV_BIT_UPLEFT			4
#define MV_BIT_UPRIGHT			5
#define MV_BIT_DOWNLEFT			6
#define MV_BIT_DOWNRIGHT		7

#define NUM_DIRECTIONS_FULL		8

// values for special "button" bitmasks
#define BUTTON_1			4
#define BUTTON_2			5

#define NUM_PLAYER_ACTIONS		6

// values for special "focus player" and "suicide" bitmasks
#define BIT_SET_FOCUS			6
#define BIT_SUICIDE			7

// values for drawing stages for global animations
#define DRAW_GLOBAL_ANIM_STAGE_1	1
#define DRAW_GLOBAL_ANIM_STAGE_2	2
#define DRAW_GLOBAL_ANIM_STAGE_3	3
#define DRAW_GLOBAL_ANIM_STAGE_RESTART	4

// values for drawing target (various functions)
#define DRAW_TO_BACKBUFFER		0
#define DRAW_TO_FIELDBUFFER		1
#define DRAW_TO_SCREEN			2
#define DRAW_TO_FADE_SOURCE		3
#define DRAW_TO_FADE_TARGET		4

// values for move directions and special "button" key bitmasks
#define MV_NONE				0
#define MV_LEFT				(1 << MV_BIT_LEFT)
#define MV_RIGHT			(1 << MV_BIT_RIGHT)
#define MV_UP				(1 << MV_BIT_UP)
#define MV_DOWN				(1 << MV_BIT_DOWN)

#define MV_UPLEFT			(MV_UP   | MV_LEFT)
#define MV_UPRIGHT			(MV_UP   | MV_RIGHT)
#define MV_DOWNLEFT			(MV_DOWN | MV_LEFT)
#define MV_DOWNRIGHT			(MV_DOWN | MV_RIGHT)

#define MV_HORIZONTAL			(MV_LEFT | MV_RIGHT)
#define MV_VERTICAL			(MV_UP   | MV_DOWN)
#define MV_ALL_DIRECTIONS		(MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN)
#define MV_ANY_DIRECTION		(MV_ALL_DIRECTIONS)
#define MV_NO_DIRECTION			(MV_NONE)

#define KEY_BUTTON_1			(1 << BUTTON_1)
#define KEY_BUTTON_2			(1 << BUTTON_2)
#define KEY_BUTTON_SNAP			KEY_BUTTON_1
#define KEY_BUTTON_DROP			KEY_BUTTON_2
#define KEY_MOTION			(MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN)
#define KEY_BUTTON			(KEY_BUTTON_1 | KEY_BUTTON_2)
#define KEY_ACTION			(KEY_MOTION | KEY_BUTTON)

#define KEY_SET_FOCUS			(1 << BIT_SET_FOCUS)
#define KEY_SUICIDE			(1 << BIT_SUICIDE)

#define MV_DIR_FROM_BIT(x)		((x) < NUM_DIRECTIONS ? 1 << (x) :	  \
					 (x) == MV_BIT_UPLEFT    ? MV_UPLEFT    : \
					 (x) == MV_BIT_UPRIGHT   ? MV_UPRIGHT   : \
					 (x) == MV_BIT_DOWNLEFT  ? MV_DOWNLEFT  : \
					 (x) == MV_BIT_DOWNRIGHT ? MV_DOWNRIGHT : \
					 MV_NONE)

#define MV_DIR_TO_BIT(x)		((x) == MV_LEFT      ? MV_BIT_LEFT      : \
					 (x) == MV_RIGHT     ? MV_BIT_RIGHT     : \
					 (x) == MV_UP        ? MV_BIT_UP        : \
					 (x) == MV_DOWN      ? MV_BIT_DOWN      : \
					 (x) == MV_UPLEFT    ? MV_BIT_UPLEFT    : \
					 (x) == MV_UPRIGHT   ? MV_BIT_UPRIGHT   : \
					 (x) == MV_DOWNLEFT  ? MV_BIT_DOWNLEFT  : \
					 (x) == MV_DOWNRIGHT ? MV_BIT_DOWNRIGHT : \
					 MV_BIT_DOWN)

#define MV_DIR_OPPOSITE(x)		((x) == MV_LEFT      ? MV_RIGHT     : \
					 (x) == MV_RIGHT     ? MV_LEFT      : \
					 (x) == MV_UP        ? MV_DOWN      : \
					 (x) == MV_DOWN      ? MV_UP        : \
					 (x) == MV_UPLEFT    ? MV_DOWNRIGHT : \
					 (x) == MV_UPRIGHT   ? MV_DOWNLEFT  : \
					 (x) == MV_DOWNLEFT  ? MV_UPRIGHT   : \
					 (x) == MV_DOWNRIGHT ? MV_UPLEFT    : \
					 MV_NONE)

// values for animation mode (frame order and direction)
// (stored in level files -- never change existing values)
#define ANIM_NONE			0
#define ANIM_LOOP			(1 << 0)
#define ANIM_LINEAR			(1 << 1)
#define ANIM_PINGPONG			(1 << 2)
#define ANIM_PINGPONG2			(1 << 3)
#define ANIM_RANDOM			(1 << 4)
#define ANIM_CE_VALUE			(1 << 5)
#define ANIM_CE_SCORE			(1 << 6)
#define ANIM_CE_DELAY			(1 << 7)
#define ANIM_REVERSE			(1 << 8)
#define ANIM_OPAQUE_PLAYER		(1 << 9)
#define ANIM_LEVEL_NR			(1 << 10)

// values for special (non game element) animation modes
// (not stored in level files -- can be changed, if needed)
#define ANIM_HORIZONTAL			(1 << 11)
#define ANIM_VERTICAL			(1 << 12)
#define ANIM_CENTERED			(1 << 13)
#define ANIM_STATIC_PANEL		(1 << 14)
#define ANIM_ALL			(1 << 15)
#define ANIM_ONCE			(1 << 16)
#define ANIM_TILED			(1 << 17)
#define ANIM_RANDOM_STATIC		(1 << 18)

#define ANIM_DEFAULT			ANIM_LOOP

// values for special global animation events
#define ANIM_EVENT_UNDEFINED		-1
#define ANIM_EVENT_NONE			0
#define ANIM_EVENT_SELF			(1 << 0)
#define ANIM_EVENT_ANY			(1 << 1)
#define ANIM_EVENT_CLICK		(1 << 2)
#define ANIM_EVENT_INIT			(1 << 3)
#define ANIM_EVENT_START		(1 << 4)
#define ANIM_EVENT_END			(1 << 5)
#define ANIM_EVENT_POST			(1 << 6)
#define ANIM_EVENT_UNCLICK_ANY		(1 << 7)
#define ANIM_EVENT_CE_CHANGE		(1 << 8)

// event mask:  bits 0-15
// CE number:   bits 16-23
// anim number: bits 16-23
// page number: bits 24-31
// part number: bits 24-31
#define ANIM_EVENT_CE_BIT		16
#define ANIM_EVENT_ANIM_BIT		16
#define ANIM_EVENT_PAGE_BIT		24
#define ANIM_EVENT_PART_BIT		24

#define ANIM_EVENT_CE_MASK		(0xff << ANIM_EVENT_CE_BIT)
#define ANIM_EVENT_ANIM_MASK		(0xff << ANIM_EVENT_ANIM_BIT)
#define ANIM_EVENT_PAGE_MASK		(0xff << ANIM_EVENT_PAGE_BIT)
#define ANIM_EVENT_PART_MASK		(0xff << ANIM_EVENT_PART_BIT)

#define ANIM_EVENT_DEFAULT		ANIM_EVENT_NONE

// values for special global animation event actions
#define ANIM_EVENT_ACTION_NONE		-1

// values for special global animation delay types
#define ANIM_DELAY_UNDEFINED		-1
#define ANIM_DELAY_NONE			0
#define ANIM_DELAY_INIT			1
#define ANIM_DELAY_ANIM			2
#define ANIM_DELAY_POST			3

// values for special global animation delay actions
#define ANIM_DELAY_ACTION_NONE		-1

// values for special drawing classes
#define CLASS_NONE			0

// values used for game panel graphics
#define CLASS_MM_ENGINE_ONLY		(1 << 0)
#define CLASS_EXTRA_PANEL_ITEMS		(1 << 1)
#define CLASS_BD_PRE_HATCHING		(1 << 2)
#define CLASS_BD_POST_HATCHING		(1 << 3)

#define CLASS_MAX			((1 << 4) - 1)

#define CLASS_DEFAULT			CLASS_NONE

// values for special drawing styles and event handling
#define STYLE_NONE			0

// values used for crumbled graphics
#define STYLE_ACCURATE_BORDERS		(1 << 0)
#define STYLE_INNER_CORNERS		(1 << 1)

// values used for game panel graphics
#define STYLE_REVERSE			(1 << 2)
#define STYLE_LEFTMOST_POSITION		(1 << 3)

// values used for global animations
#define STYLE_BLOCK			(1 << 4)
#define STYLE_PASSTHROUGH		(1 << 5)
#define STYLE_MULTIPLE_ACTIONS		(1 << 6)
#define STYLE_CONSUME_CE_EVENT		(1 << 7)

#define STYLE_DEFAULT			STYLE_NONE

// values for fade mode
#define FADE_TYPE_NONE			0
#define FADE_TYPE_FADE_IN		(1 << 0)
#define FADE_TYPE_FADE_OUT		(1 << 1)
#define FADE_TYPE_TRANSFORM		(1 << 2)
#define FADE_TYPE_CROSSFADE		(1 << 3)
#define FADE_TYPE_MELT			(1 << 4)
#define FADE_TYPE_CURTAIN		(1 << 5)
#define FADE_TYPE_SKIP			(1 << 6)

#define FADE_MODE_NONE			(FADE_TYPE_NONE)
#define FADE_MODE_FADE_IN		(FADE_TYPE_FADE_IN)
#define FADE_MODE_FADE_OUT		(FADE_TYPE_FADE_OUT)
#define FADE_MODE_FADE			(FADE_TYPE_FADE_IN | FADE_TYPE_FADE_OUT)
#define FADE_MODE_TRANSFORM		(FADE_TYPE_TRANSFORM | FADE_TYPE_FADE_IN)
#define FADE_MODE_CROSSFADE		(FADE_MODE_TRANSFORM | FADE_TYPE_CROSSFADE)
#define FADE_MODE_MELT			(FADE_MODE_TRANSFORM | FADE_TYPE_MELT)
#define FADE_MODE_CURTAIN		(FADE_MODE_TRANSFORM | FADE_TYPE_CURTAIN)
#define FADE_MODE_SKIP_FADE_IN		(FADE_TYPE_SKIP | FADE_TYPE_FADE_IN)
#define FADE_MODE_SKIP_FADE_OUT		(FADE_TYPE_SKIP | FADE_TYPE_FADE_OUT)

#define FADE_MODE_DEFAULT		FADE_MODE_FADE

#define AUTO_DELAY_UNIT_MS		0
#define AUTO_DELAY_UNIT_FRAMES		1

#define AUTO_DELAY_UNIT_DEFAULT		AUTO_DELAY_UNIT_MS

// values for toon positions
#define POS_UNDEFINED			-1
#define POS_LEFT			0
#define POS_RIGHT			1
#define POS_TOP				2
#define POS_UPPER			3
#define POS_MIDDLE			4
#define POS_LOWER			5
#define POS_BOTTOM			6
#define POS_ANY				7
#define POS_CE				8
#define POS_CE_TRIGGER			9
#define POS_LAST			10

// values for text alignment
#define ALIGN_LEFT			(1 << 0)
#define ALIGN_RIGHT			(1 << 1)
#define ALIGN_CENTER			(1 << 2)
#define ALIGN_DEFAULT			ALIGN_LEFT

#define VALIGN_TOP			(1 << 0)
#define VALIGN_BOTTOM			(1 << 1)
#define VALIGN_MIDDLE			(1 << 2)
#define VALIGN_DEFAULT			VALIGN_TOP

#define ALIGNED_XPOS(x, w, a)		((a) == ALIGN_CENTER  ? (x) - (w) / 2 :	\
					 (a) == ALIGN_RIGHT   ? (x) - (w) : (x))
#define ALIGNED_YPOS(y, h, v)		((v) == VALIGN_MIDDLE ? (y) - (h) / 2 :	\
					 (v) == VALIGN_BOTTOM ? (y) - (h) : (y))
#define ALIGNED_TEXT_XPOS(p)		ALIGNED_XPOS((p)->x, (p)->width,  (p)->align)
#define ALIGNED_TEXT_YPOS(p)		ALIGNED_YPOS((p)->y, (p)->height, (p)->valign)
#define ALIGNED_VP_XPOS(p)		ALIGNED_TEXT_XPOS(p)
#define ALIGNED_VP_YPOS(p)		ALIGNED_TEXT_YPOS(p)

// values for redraw_mask
#define REDRAW_NONE			(0)
#define REDRAW_ALL			(1 << 0)
#define REDRAW_FIELD			(1 << 1)
#define REDRAW_DOOR_1			(1 << 2)
#define REDRAW_DOOR_2			(1 << 3)
#define REDRAW_DOOR_3			(1 << 4)
#define REDRAW_FPS			(1 << 5)

#define REDRAW_DOORS			(REDRAW_DOOR_1 | \
					 REDRAW_DOOR_2 | \
					 REDRAW_DOOR_3)

#define IN_GFX_FIELD_PLAY(x, y)		(x >= gfx.sx && x < gfx.sx + gfx.sxsize && \
					 y >= gfx.sy && y < gfx.sy + gfx.sysize)
#define IN_GFX_FIELD_FULL(x, y)		(x >= gfx.real_sx && \
					 x <  gfx.real_sx + gfx.full_sxsize && \
					 y >= gfx.real_sy && \
					 y <  gfx.real_sy + gfx.full_sysize)
#define IN_GFX_DOOR_1(x, y)		(x >= gfx.dx && x < gfx.dx + gfx.dxsize && \
					 y >= gfx.dy && y < gfx.dy + gfx.dysize)
#define IN_GFX_DOOR_2(x, y)		(x >= gfx.vx && x < gfx.vx + gfx.vxsize && \
					 y >= gfx.vy && y < gfx.vy + gfx.vysize)
#define IN_GFX_DOOR_3(x, y)		(x >= gfx.ex && x < gfx.ex + gfx.exsize && \
					 y >= gfx.ey && y < gfx.ey + gfx.eysize)

// values for mouse cursor
#define CURSOR_UNDEFINED		-1
#define CURSOR_DEFAULT			0
#define CURSOR_NONE			1
#define CURSOR_PLAYFIELD		2

// fundamental game speed values
#define ONE_SECOND_DELAY		1000	// delay value for one second
#define MENU_FRAME_DELAY		20	// frame delay in milliseconds
#define GAME_FRAME_DELAY		20	// frame delay in milliseconds
#define FFWD_FRAME_DELAY		10	// 200% speed for fast forward
#define MIN_VSYNC_FRAME_DELAY		15	// minimum value for vsync to keep
#define MAX_VSYNC_FRAME_DELAY		16	// maximum value for vsync to work
#define FRAMES_PER_SECOND		(ONE_SECOND_DELAY / GAME_FRAME_DELAY)
#define FRAMES_PER_SECOND_SP		35
#define FRAMES_PER_SECOND_PAL		50
#define FRAMES_PER_SECOND_NTSC		60

// maximum playfield size supported by libgame functions
#define MAX_PLAYFIELD_WIDTH		128
#define MAX_PLAYFIELD_HEIGHT		128

// maximum number of parallel players supported by libgame functions
#define MAX_PLAYERS			4

// maximum number of player names
#define MAX_PLAYER_NAMES		12

// maximum allowed length of player name
#define MAX_PLAYER_NAME_LEN		10

// maximum number of levels in a level set
#define MAX_LEVELS			1000

// maximum number of global animation and parts
#define MAX_GLOBAL_ANIMS		32
#define MAX_GLOBAL_ANIM_PARTS		32

// maximum number of level specific colors in color template
#define MAX_LEVEL_COLORS		8

// minimum/maximum/default x/y grid size for virtual buttons
#define MIN_GRID_XSIZE			3
#define MIN_GRID_YSIZE			3
#define MAX_GRID_XSIZE			32
#define MAX_GRID_YSIZE			32
#define GRID_REAL_WIDTH			MAX(1, MAX(video.screen_width,	\
						   video.screen_height))
#define GRID_REAL_HEIGHT		MAX(1, MIN(video.screen_width,	\
						   video.screen_height))
#define DEFAULT_GRID_XSIZE_0		18
#define DEFAULT_GRID_YSIZE_0		MIN(MAX(MIN_GRID_YSIZE,		\
						DEFAULT_GRID_XSIZE_0 *	\
						GRID_REAL_HEIGHT /	\
						GRID_REAL_WIDTH),	\
					    MAX_GRID_YSIZE)
#define DEFAULT_GRID_XSIZE_1		13
#define DEFAULT_GRID_YSIZE_1		MIN(MAX(MIN_GRID_YSIZE,		\
						DEFAULT_GRID_XSIZE_1 *	\
						GRID_REAL_WIDTH /	\
						GRID_REAL_HEIGHT),	\
					    MAX_GRID_YSIZE)

#define DEFAULT_GRID_XSIZE(n)		((n) == 0 ? DEFAULT_GRID_XSIZE_0 : \
					 DEFAULT_GRID_XSIZE_1)
#define DEFAULT_GRID_YSIZE(n)		((n) == 0 ? DEFAULT_GRID_YSIZE_0 : \
					 DEFAULT_GRID_YSIZE_1)

#define GRID_ACTIVE_NR()		(video.screen_width >	\
					 video.screen_height ? 0 : 1)

// values for grid button characters for virtual buttons
#define CHAR_GRID_BUTTON_NONE		' '
#define CHAR_GRID_BUTTON_LEFT		'<'
#define CHAR_GRID_BUTTON_RIGHT		'>'
#define CHAR_GRID_BUTTON_UP		'^'
#define CHAR_GRID_BUTTON_DOWN		'v'
#define CHAR_GRID_BUTTON_SNAP		'1'
#define CHAR_GRID_BUTTON_DROP		'2'

#define GET_ACTION_FROM_GRID_BUTTON(c)	((c) == CHAR_GRID_BUTTON_LEFT ?  \
					 JOY_LEFT :			 \
					 (c) == CHAR_GRID_BUTTON_RIGHT ? \
					 JOY_RIGHT :			 \
					 (c) == CHAR_GRID_BUTTON_UP ?    \
					 JOY_UP :			 \
					 (c) == CHAR_GRID_BUTTON_DOWN ?  \
					 JOY_DOWN :			 \
					 (c) == CHAR_GRID_BUTTON_SNAP ?  \
					 JOY_BUTTON_1 :			 \
					 (c) == CHAR_GRID_BUTTON_DROP ?  \
					 JOY_BUTTON_2 :			 \
					 JOY_NO_ACTION)

// maximum number of level sets in the level set history
#define MAX_LEVELDIR_HISTORY		100

// default name for empty highscore entry
#define EMPTY_PLAYER_NAME		"no name"

// default name for unknown player names
#define ANONYMOUS_NAME			"anonymous"

// default for other unknown names
#define UNKNOWN_NAME			"unknown"

// default name for new levels
#define NAMELESS_LEVEL_NAME		"nameless level"

// default text for non-existant artwork
#define NOT_AVAILABLE			"(not available)"

// default value for undefined filename
#define UNDEFINED_FILENAME		"[NONE]"

// default value for undefined levelset
#define UNDEFINED_LEVELSET		"[NONE]"

// default value for undefined password
#define UNDEFINED_PASSWORD		"[undefined]"

// default value for undefined string parameter
#define ARG_UNDEFINED_STRING		"[undefined]"

// default value for default string parameter
#define ARG_DEFAULT			"[DEFAULT]"

// default values for undefined numerical parameter (as string and integer)
#define ARG_UNDEFINED			"-1000000"
#define ARG_UNDEFINED_VALUE		(-1000000)

// default value for off-screen positions
#define POS_OFFSCREEN			(-1000000)

// definitions for game base path and sub-directories
#ifndef BASE_PATH
#define BASE_PATH			"."
#endif

// directory names
#define GRAPHICS_DIRECTORY		"graphics"
#define SOUNDS_DIRECTORY		"sounds"
#define MUSIC_DIRECTORY			"music"
#define LEVELS_DIRECTORY		"levels"
#define TAPES_DIRECTORY			"tapes"
#define SCORES_DIRECTORY		"scores"
#define DOCS_DIRECTORY			"docs"
#define ELEMENTS_DIRECTORY		"elements"
#define CREDITS_DIRECTORY		"credits"
#define PROGRAM_INFO_DIRECTORY		"program"
#define LEVELSET_INFO_DIRECTORY		"levelset"
#define LEVEL_INFO_DIRECTORY		"levels"
#define STORIES_DIRECTORY		"stories"
#define CACHE_DIRECTORY			"cache"
#define CONF_DIRECTORY			"conf"
#define NETWORK_DIRECTORY		"network"
#define USERS_DIRECTORY			"users"
#define PERSISTENT_DIRECTORY		"/persistent"

#define GFX_CLASSIC_SUBDIR		"gfx_classic"
#define SND_CLASSIC_SUBDIR		"snd_classic"
#define MUS_CLASSIC_SUBDIR		"mus_classic"

#define GFX_DEFAULT_SUBDIR		(setup.internal.default_graphics_set)
#define SND_DEFAULT_SUBDIR		(setup.internal.default_sounds_set)
#define MUS_DEFAULT_SUBDIR		(setup.internal.default_music_set)

#define GFX_FALLBACK_FILENAME		(setup.internal.fallback_graphics_file)
#define SND_FALLBACK_FILENAME		(setup.internal.fallback_sounds_file)
#define MUS_FALLBACK_FILENAME		(setup.internal.fallback_music_file)

#define DEFAULT_LEVELSET		(setup.internal.default_level_series)

// file names and filename extensions
#define LEVELSETUP_DIRECTORY		"levelsetup"
#define SETUP_FILENAME			"setup.conf"
#define USERSETUP_FILENAME		"usersetup.conf"
#define AUTOSETUP_FILENAME		"autosetup.conf"
#define LEVELSETUP_FILENAME		"levelsetup.conf"
#define SERVERSETUP_FILENAME		"serversetup.conf"
#define EDITORSETUP_FILENAME		"editorsetup.conf"
#define EDITORCASCADE_FILENAME		"editorcascade.conf"
#define HELPANIM_FILENAME		"helpanim.conf"
#define HELPTEXT_FILENAME		"helptext.conf"
#define LEVELINFO_FILENAME		"levelinfo.conf"
#define GRAPHICSINFO_FILENAME		"graphicsinfo.conf"
#define SOUNDSINFO_FILENAME		"soundsinfo.conf"
#define MUSICINFO_FILENAME		"musicinfo.conf"
#define ARTWORKINFO_CACHE_FILE		"artworkinfo.cache"
#define LEVELTEMPLATE_FILENAME		"template.level"
#define UPLOADED_FILENAME		".uploaded"
#define LEVELFILE_EXTENSION		"level"
#define TAPEFILE_EXTENSION		"tape"
#define SCOREFILE_EXTENSION		"score"

#define GAMECONTROLLER_BASENAME		"gamecontrollerdb.txt"

#define FALLBACK_TEXT_FILENAME		"fallback.txt"

#define STRING_PARENT_DIRECTORY		".."
#define STRING_TOP_DIRECTORY		"/"

#define CHAR_PATH_SEPARATOR_UNIX	'/'
#define CHAR_PATH_SEPARATOR_DOS		'\\'

#define STRING_PATH_SEPARATOR_UNIX	"/"
#define STRING_PATH_SEPARATOR_DOS	"\\"

#define STRING_NEWLINE_UNIX		"\n"
#define STRING_NEWLINE_DOS		"\r\n"

#if defined(PLATFORM_WINDOWS)
#define CHAR_PATH_SEPARATOR		CHAR_PATH_SEPARATOR_DOS
#define STRING_PATH_SEPARATOR		STRING_PATH_SEPARATOR_DOS
#define STRING_NEWLINE			STRING_NEWLINE_DOS
#else
#define CHAR_PATH_SEPARATOR		CHAR_PATH_SEPARATOR_UNIX
#define STRING_PATH_SEPARATOR		STRING_PATH_SEPARATOR_UNIX
#define STRING_NEWLINE			STRING_NEWLINE_UNIX
#endif


// areas in bitmap PIX_DOOR
// meaning in PIX_DB_DOOR: (3 PAGEs)
// PAGEX1: 1. buffer for DOOR_1
// PAGEX2: 2. buffer for DOOR_1
// PAGEX3: buffer for animations

// these values are hard-coded to be able to use them in initialization
#define DOOR_GFX_PAGE_WIDTH		100	// should be set to "gfx.dxsize"
#define DOOR_GFX_PAGE_HEIGHT		280	// should be set to "gfx.dysize"

#define DOOR_GFX_PAGESIZE		(DOOR_GFX_PAGE_WIDTH)
#define DOOR_GFX_PAGEX1			(0 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX2			(1 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX3			(2 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX4			(3 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX5			(4 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX6			(5 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX7			(6 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX8			(7 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEY1			(0)
#define DOOR_GFX_PAGEY2			(DOOR_GFX_PAGE_HEIGHT)


// macros for version handling
#define VERSION_PART_1(x)		((VersionSubType)(((x) >> 56) & 0xff))
#define VERSION_PART_2(x)		((VersionSubType)(((x) >> 48) & 0xff))
#define VERSION_PART_3(x)		((VersionSubType)(((x) >> 40) & 0xff))
#define VERSION_PART_4(x)		((VersionSubType)(((x) >> 32) & 0xff))
#define VERSION_PART_5(x)		((VersionSubType)(((x) >> 24) & 0xff))
#define VERSION_PART_6(x)		((VersionSubType)(((x) >> 16) & 0xff))
#define VERSION_PART_7(x)		((VersionSubType)(((x)) & 0xffff))

#define VERSION_SUPER(x)		VERSION_PART_1(x)
#define VERSION_MAJOR(x)		VERSION_PART_2(x)
#define VERSION_MINOR(x)		VERSION_PART_3(x)
#define VERSION_PATCH(x)		VERSION_PART_4(x)
#define VERSION_STABLE(x)		VERSION_PART_5(x)
#define VERSION_EXTRA(x)		VERSION_PART_6(x)
#define VERSION_BUILD(x)		VERSION_PART_7(x)

#define VERSION_IDENT_STANDARD(a,b,c,d)	(((VersionType)(a) << 56) |	\
					 ((VersionType)(b) << 48) |	\
					 ((VersionType)(c) << 40) |	\
					 ((VersionType)(d) << 32))
#define VERSION_IDENT_EXTENDED(e,f,g)	(((VersionType)(e) << 24) |	\
					 ((VersionType)(f) << 16) |	\
					 ((VersionType)(g)))

#define VERSION_IDENT(a,b,c,d)			(VERSION_IDENT_STANDARD(a,b,c,d))
#define VERSION_IDENT_FULL(a,b,c,d, e,f,g)	(VERSION_IDENT_STANDARD(a,b,c,d) |	\
						 VERSION_IDENT_EXTENDED(e,f,g))


// macros for parent/child process identification
#if defined(PLATFORM_UNIX)
#define IS_PARENT_PROCESS()		(audio.mixer_pid != getpid())
#define IS_CHILD_PROCESS()		(audio.mixer_pid == getpid())
#define HAS_CHILD_PROCESS()		(audio.mixer_pid > 0)
#else
#define IS_PARENT_PROCESS()		TRUE
#define IS_CHILD_PROCESS()		FALSE
#define HAS_CHILD_PROCESS()		FALSE
#endif


// values for artwork type
#define ARTWORK_TYPE_GRAPHICS		0
#define ARTWORK_TYPE_SOUNDS		1
#define ARTWORK_TYPE_MUSIC		2

#define NUM_ARTWORK_TYPES		3


// values for tree type (chosen to match artwork type)
#define TREE_TYPE_UNDEFINED		-1
#define TREE_TYPE_GRAPHICS_DIR		ARTWORK_TYPE_GRAPHICS
#define TREE_TYPE_SOUNDS_DIR		ARTWORK_TYPE_SOUNDS
#define TREE_TYPE_MUSIC_DIR		ARTWORK_TYPE_MUSIC
#define TREE_TYPE_LEVEL_DIR		3
#define TREE_TYPE_LEVEL_NR		4
#define TREE_TYPE_PLAYER_NAME		5
#define TREE_TYPE_SCORE_ENTRY		6

#define NUM_BASE_TREE_TYPES		4
#define NUM_TREE_TYPES			7

#define TREE_TYPE_IS_DIR(type)		((type) == TREE_TYPE_GRAPHICS_DIR ||	\
					 (type) == TREE_TYPE_SOUNDS_DIR ||	\
					 (type) == TREE_TYPE_MUSIC_DIR ||	\
					 (type) == TREE_TYPE_LEVEL_DIR)

#define INFOTEXT_UNDEFINED		""
#define INFOTEXT_GRAPHICS_DIR		"Custom Graphics"
#define INFOTEXT_SOUNDS_DIR		"Custom Sounds"
#define INFOTEXT_MUSIC_DIR		"Custom Music"
#define INFOTEXT_LEVEL_DIR		"Level Sets"
#define INFOTEXT_LEVEL_NR		"Levels"
#define INFOTEXT_PLAYER_NAME		"Players & Teams"
#define INFOTEXT_SCORE_ENTRY		"Hall of Fame"

#define BACKLINK_TEXT_MAIN		".. (main menu)"
#define BACKLINK_TEXT_SETUP		".. (setup menu)"
#define BACKLINK_TEXT_PARENT		".. (parent directory)"
#define BACKLINK_TEXT_BACK		"back"
#define BACKLINK_TEXT_NEXT		"next"

#define TREE_INFOTEXT(t)									\
			((t) == TREE_TYPE_SCORE_ENTRY  ? INFOTEXT_SCORE_ENTRY :			\
			 (t) == TREE_TYPE_PLAYER_NAME  ? INFOTEXT_PLAYER_NAME :			\
			 (t) == TREE_TYPE_LEVEL_NR     ? INFOTEXT_LEVEL_NR :			\
			 (t) == TREE_TYPE_LEVEL_DIR    ? INFOTEXT_LEVEL_DIR :			\
			 (t) == TREE_TYPE_GRAPHICS_DIR ? INFOTEXT_GRAPHICS_DIR :		\
			 (t) == TREE_TYPE_SOUNDS_DIR   ? INFOTEXT_SOUNDS_DIR :			\
			 (t) == TREE_TYPE_MUSIC_DIR    ? INFOTEXT_MUSIC_DIR :			\
			 INFOTEXT_UNDEFINED)

#define TREE_BACKLINK_TEXT(t)									\
			((t) == TREE_TYPE_SCORE_ENTRY  ? BACKLINK_TEXT_BACK :			\
			 (t) == TREE_TYPE_LEVEL_DIR    ? BACKLINK_TEXT_MAIN :			\
			 BACKLINK_TEXT_SETUP)

#define TREE_USERDIR(t)										\
			((t) == TREE_TYPE_LEVEL_DIR    ? getUserLevelDir(NULL) :		\
			 (t) == TREE_TYPE_GRAPHICS_DIR ? getUserGraphicsDir() :			\
			 (t) == TREE_TYPE_SOUNDS_DIR   ? getUserSoundsDir() :			\
			 (t) == TREE_TYPE_MUSIC_DIR    ? getUserMusicDir() :			\
			 NULL)

#define TREE_FIRST_NODE_PTR(t)									\
			((t) == TREE_TYPE_LEVEL_DIR    ? &leveldir_first :			\
			 (t) == TREE_TYPE_GRAPHICS_DIR ? &artwork.gfx_first :			\
			 (t) == TREE_TYPE_SOUNDS_DIR   ? &artwork.snd_first :			\
			 (t) == TREE_TYPE_MUSIC_DIR    ? &artwork.mus_first :			\
			 NULL)

#define TREE_FIRST_NODE(t)									\
			((t) == TREE_TYPE_LEVEL_DIR    ? leveldir_first :			\
			 (t) == TREE_TYPE_GRAPHICS_DIR ? artwork.gfx_first :			\
			 (t) == TREE_TYPE_SOUNDS_DIR   ? artwork.snd_first :			\
			 (t) == TREE_TYPE_MUSIC_DIR    ? artwork.mus_first :			\
			 NULL)

// values for artwork handling
#define LEVELDIR_ARTWORK_SET_PTR(leveldir, type)						\
			((type) == ARTWORK_TYPE_GRAPHICS ? &leveldir->graphics_set :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? &leveldir->sounds_set :		\
			 &leveldir->music_set)

#define LEVELDIR_ARTWORK_SET(leveldir, type)							\
			((type) == ARTWORK_TYPE_GRAPHICS ? leveldir->graphics_set :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? leveldir->sounds_set :		\
			 leveldir->music_set)

#define LEVELDIR_ARTWORK_PATH_PTR(leveldir, type)						\
			((type) == ARTWORK_TYPE_GRAPHICS ? &leveldir->graphics_path :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? &leveldir->sounds_path :		\
			 &leveldir->music_path)

#define LEVELDIR_ARTWORK_PATH(leveldir, type)							\
			((type) == ARTWORK_TYPE_GRAPHICS ? leveldir->graphics_path :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? leveldir->sounds_path :		\
			 leveldir->music_path)

#define SETUP_ARTWORK_SET(setup, type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? setup.graphics_set :			\
			 (type) == ARTWORK_TYPE_SOUNDS   ? setup.sounds_set :			\
			 setup.music_set)

#define SETUP_OVERRIDE_ARTWORK(setup, type)							\
			((type) == ARTWORK_TYPE_GRAPHICS ? setup.override_level_graphics :	\
			 (type) == ARTWORK_TYPE_SOUNDS   ? setup.override_level_sounds :	\
			 setup.override_level_music)

#define GFX_OVERRIDE_ARTWORK(type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? gfx.override_level_graphics :	\
			 (type) == ARTWORK_TYPE_SOUNDS   ? gfx.override_level_sounds :		\
			 gfx.override_level_music)

#define ARTWORK_FIRST_NODE(artwork, type)							\
			((type) == ARTWORK_TYPE_GRAPHICS ? artwork.gfx_first :			\
			 (type) == ARTWORK_TYPE_SOUNDS   ? artwork.snd_first :			\
			 artwork.mus_first)

#define ARTWORK_CURRENT_PTR(artwork, type)							\
			((type) == ARTWORK_TYPE_GRAPHICS ? &artwork.gfx_current :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? &artwork.snd_current :		\
			 &artwork.mus_current)

#define ARTWORK_CURRENT(artwork, type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? artwork.gfx_current :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? artwork.snd_current :		\
			 artwork.mus_current)

#define ARTWORK_CURRENT_IDENTIFIER_PTR(artwork, type)						\
			((type) == ARTWORK_TYPE_GRAPHICS ? &artwork.gfx_current_identifier :	\
			 (type) == ARTWORK_TYPE_SOUNDS   ? &artwork.snd_current_identifier :	\
			 &artwork.mus_current_identifier)

#define ARTWORK_CURRENT_IDENTIFIER(artwork, type)						\
			((type) == ARTWORK_TYPE_GRAPHICS ? artwork.gfx_current_identifier :	\
			 (type) == ARTWORK_TYPE_SOUNDS   ? artwork.snd_current_identifier :	\
			 artwork.mus_current_identifier)

#define ARTWORKINFO_FILENAME(type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? GRAPHICSINFO_FILENAME :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? SOUNDSINFO_FILENAME :		\
			 (type) == ARTWORK_TYPE_MUSIC    ? MUSICINFO_FILENAME :			\
			 "")

#define ARTWORK_DIRECTORY(type)									\
			((type) == ARTWORK_TYPE_GRAPHICS ? GRAPHICS_DIRECTORY :			\
			 (type) == ARTWORK_TYPE_SOUNDS   ? SOUNDS_DIRECTORY :			\
			 (type) == ARTWORK_TYPE_MUSIC    ? MUSIC_DIRECTORY :			\
			 "")

#define OPTIONS_ARTWORK_DIRECTORY(type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? options.graphics_directory :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? options.sounds_directory :		\
			 (type) == ARTWORK_TYPE_MUSIC    ? options.music_directory :		\
			 "")

#define USER_ARTWORK_DIRECTORY(type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? getUserGraphicsDir() :		\
			 (type) == ARTWORK_TYPE_SOUNDS   ? getUserSoundsDir() :			\
			 (type) == ARTWORK_TYPE_MUSIC    ? getUserMusicDir() :			\
			 "")

#define ARTWORK_DEFAULT_SUBDIR(type)								\
			((type) == ARTWORK_TYPE_GRAPHICS ? GFX_DEFAULT_SUBDIR :			\
			 (type) == ARTWORK_TYPE_SOUNDS   ? SND_DEFAULT_SUBDIR :			\
			 MUS_DEFAULT_SUBDIR)

#define UPDATE_BUSY_STATE()			\
{						\
  if (gfx.draw_busy_anim_function != NULL)	\
    gfx.draw_busy_anim_function(TRUE);		\
}
#define UPDATE_BUSY_STATE_NOT_LOADING()		\
{						\
  if (gfx.draw_busy_anim_function != NULL)	\
    gfx.draw_busy_anim_function(FALSE);		\
}


// type definitions

typedef uint64_t	VersionType;
typedef uint8_t		VersionSubType;


// structure definitions

struct VersionInfo
{
  // this structure is currently only used to document the content bytes of "VersionType"

  // standard version info
  VersionSubType super;		// incremented for extraordinary changes (almost never)
  VersionSubType major;		// incremented for major changes
  VersionSubType minor;		// incremented for minor changes
  VersionSubType patch;		// incremented for bug fixes only

  // extended version info
  VersionSubType stable;	// set to "1" for stable versions, "0" for test versions
  VersionSubType extra;		// set to extra (test or pre-release) version number
  unsigned short build;		// set to build number
};

struct ProgramInfo
{
  char *command_basepath;	// path to the program binary
  char *command_basename;	// base filename of the program binary

  char *config_filename;	// optional global program config filename

  char *maindata_path;		// main game data (installation) directory

  char *userdata_subdir;	// personal user game data directory
  char *userdata_path;		// resulting full path to game data directory

  char *program_basename;
  char *program_title;
  char *window_title;

  char *icon_filename;

  char *cookie_prefix;

  char *log_filename;		// filename for log messages
  FILE *log_file;		// file handle for log files
  FILE *log_file_default;	// default log file handle

  VersionSubType version_super;
  VersionSubType version_major;
  VersionSubType version_minor;
  VersionSubType version_patch;
  VersionType version_ident;

  char *version_string;

  char *(*window_title_function)(void);
  void (*exit_message_function)(char *, va_list);
  void (*exit_function)(int);

  int api_thread_count;

  boolean headless;
};

struct NetworkInfo
{
  boolean enabled;
  boolean connected;
  boolean serveronly;

  char *server_host;
  int server_port;

  SDL_Thread *server_thread;
  boolean is_server_thread;
};

struct RuntimeInfo
{
  boolean uses_touch_device;

  boolean use_api_server;
};

struct OptionInfo
{
  char *server_host;
  int server_port;

  char *base_directory;
  char *level_directory;
  char *graphics_directory;
  char *sounds_directory;
  char *music_directory;
  char *docs_directory;
  char *conf_directory;

  char *execute_command;
  char *tape_log_filename;

  char *special_flags;
  char *debug_mode;

  char *player_name;
  char *identifier;
  char *level_nr;

  char *drop_file;

  int display_nr;

  boolean mytapes;
  boolean serveronly;
  boolean network;
  boolean verbose;
  boolean debug;
};

struct VideoSystemInfo
{
  int default_depth;
  int width, height, depth;
  int window_width, window_height;
  int display_width, display_height;
  int screen_width, screen_height;
  int screen_xoffset, screen_yoffset;

  boolean fullscreen_available;
  boolean fullscreen_enabled;
  boolean fullscreen_initial;

  boolean window_scaling_available;
  int window_scaling_percent;
  char *window_scaling_quality;
  int screen_rendering_mode;
  int vsync_mode;

  unsigned int frame_counter;
  DelayCounter frame_delay;

  boolean shifted_up;
  int shifted_up_pos;
  int shifted_up_pos_last;
  DelayCounter shifted_up_delay;

  boolean initialized;
};

struct AudioSystemInfo
{
  boolean sound_available;
  boolean loops_available;
  boolean music_available;

  boolean sound_enabled;
  boolean sound_deactivated;	// for temporarily disabling sound

  int mixer_pipe[2];
  int mixer_pid;
  char *device_name;
  int device_fd;

  int sample_rate;

  int num_channels;
  int music_channel;
  int first_sound_channel;
};

struct FontBitmapInfo
{
  Bitmap *bitmap;

  int src_x, src_y;		// start position of font characters
  int width, height;		// width / height of font characters

  int offset_x;			// offset to next font character
  int offset_y;			// offset to next font character

  int draw_xoffset;		// offset for drawing font characters
  int draw_yoffset;		// offset for drawing font characters

  int num_chars;
  int num_chars_per_line;
};

struct GfxInfo
{
  int sx, sy;
  int sxsize, sysize;
  int real_sx, real_sy;
  int full_sxsize, full_sysize;
  int scrollbuffer_width, scrollbuffer_height;

  int game_tile_size, standard_tile_size;

  int dx, dy;
  int dxsize, dysize;

  int vx, vy;
  int vxsize, vysize;

  int ex, ey;
  int exsize, eysize;

  int win_xsize, win_ysize;

  int draw_deactivation_mask;
  int draw_background_mask;

  Bitmap *field_save_buffer;

  Bitmap *background_bitmap;
  int background_bitmap_mask;

  Bitmap *fade_bitmap_backup;
  Bitmap *fade_bitmap_source;
  Bitmap *fade_bitmap_target;
  Bitmap *fade_bitmap_black;

  int fade_border_source_status;
  int fade_border_target_status;
  Bitmap *masked_border_bitmap_ptr;

  Bitmap *final_screen_bitmap;

  boolean clipping_enabled;
  int clip_x, clip_y;
  int clip_width, clip_height;

  boolean override_level_graphics;
  boolean override_level_sounds;
  boolean override_level_music;

  boolean draw_init_text;

  int num_fonts;
  struct FontBitmapInfo *font_bitmap_info;
  int (*select_font_function)(int);
  int (*get_font_from_token_function)(char *);
  char * (*get_token_from_font_function)(int);

  int anim_random_frame;
  int anim_first_level;

  void (*draw_busy_anim_function)(boolean);
  void (*draw_global_anim_function)(int, int);
  void (*draw_global_border_function)(int);
  void (*draw_tile_cursor_function)(int, int);
  void (*draw_envelope_request_function)(int);

  int cursor_mode;
  int cursor_mode_override;
  int cursor_mode_final;
  int mouse_x, mouse_y;

  boolean has_reduced_color_template;
  boolean has_extended_color_template;
};

struct TileCursorInfo
{
  boolean enabled;		// tile cursor generally enabled or disabled
  boolean active;		// tile cursor activated (depending on game)
  boolean moving;		// tile cursor moving to target position

  int xpos, ypos;		// tile cursor level playfield position
  int x, y;			// tile cursor current screen position
  int target_x, target_y;	// tile cursor target screen position

  int sx, sy;			// tile cursor screen start position

  boolean xsn_debug;		// enable or disable XSN debugging
};

struct OverlayInfo
{
  boolean enabled;		// overlay generally enabled or disabled
  boolean active;		// overlay activated (depending on game mode)

  boolean show_grid;

  int grid_xsize;
  int grid_ysize;

  char grid_button[MAX_GRID_XSIZE][MAX_GRID_YSIZE];
  char grid_button_highlight;

  int grid_button_action;
};

struct JoystickInfo
{
  int status;
  int nr[MAX_PLAYERS];		// joystick number for each player
};

struct SetupJoystickInfo
{
  char *device_name;		// device name of player's joystick

  int xleft, xmiddle, xright;
  int yupper, ymiddle, ylower;
  int snap, drop;
};

struct SetupKeyboardInfo
{
  Key left, right, up, down;
  Key snap, drop;
};

struct SetupTouchInfo
{
  char *control_type;
  int move_distance;
  int drop_distance;

  int grid_xsize[2];
  int grid_ysize[2];

  char grid_button[2][MAX_GRID_XSIZE][MAX_GRID_YSIZE];

  int transparency;		// in percent (0 == opaque, 100 == invisible)
  boolean draw_outlined;
  boolean draw_pressed;

  boolean grid_initialized;

  boolean overlay_buttons;
};

struct SetupInputInfo
{
  boolean use_joystick;
  struct SetupJoystickInfo joy;
  struct SetupKeyboardInfo key;
};

struct SetupEditorInfo
{
  boolean el_boulderdash;
  boolean el_boulderdash_native;
  boolean el_boulderdash_effects;
  boolean el_boulderdash_scanned;
  boolean el_emerald_mine;
  boolean el_emerald_mine_club;
  boolean el_more;
  boolean el_sokoban;
  boolean el_supaplex;
  boolean el_diamond_caves;
  boolean el_dx_boulderdash;

  boolean el_mirror_magic;
  boolean el_deflektor;

  boolean el_chars;
  boolean el_steel_chars;

  boolean el_classic;
  boolean el_custom;
  boolean el_user_defined;
  boolean el_dynamic;

  boolean el_headlines;

  boolean el_by_game;
  boolean el_by_type;

  boolean show_element_token;
  boolean fast_game_start;

  boolean show_read_only_warning;

  boolean use_template_for_new_levels;
};

struct SetupAutoSetupInfo
{
  int editor_zoom_tilesize;
};

struct SetupLevelSetupInfo
{
  char *last_level_series[MAX_LEVELDIR_HISTORY + 1];
};

struct SetupEditorCascadeInfo
{
  boolean el_bd;
  boolean el_bdx;
  boolean el_bdx_effects;
  boolean el_bdx_scanned;
  boolean el_em;
  boolean el_emc;
  boolean el_rnd;
  boolean el_sb;
  boolean el_sp;
  boolean el_dc;
  boolean el_dx;
  boolean el_mm;
  boolean el_df;
  boolean el_chars;
  boolean el_steel_chars;
  boolean el_ce;
  boolean el_ge;
  boolean el_es;
  boolean el_ref;
  boolean el_user;
  boolean el_dynamic;
};

struct SetupShortcutInfo
{
  Key save_game;
  Key load_game;
  Key restart_game;
  Key pause_before_end;
  Key toggle_pause;

  Key focus_player[MAX_PLAYERS];
  Key focus_player_all;

  Key tape_eject;
  Key tape_extra;
  Key tape_stop;
  Key tape_pause;
  Key tape_record;
  Key tape_play;

  Key sound_simple;
  Key sound_loops;
  Key sound_music;

  Key snap_left;
  Key snap_right;
  Key snap_up;
  Key snap_down;

  Key speed_fast;
  Key speed_slow;

  Key suicide;
  Key toggle_panel;
};

struct SetupSystemInfo
{
  char *sdl_renderdriver;
  char *sdl_videodriver;
  char *sdl_audiodriver;
  int audio_fragment_size;
};

struct SetupInternalInfo
{
  char *program_title;
  char *program_version;
  char *program_author;
  char *program_email;
  char *program_website;
  char *program_copyright;
  char *program_company;

  char *program_icon_file;

  char *default_graphics_set;
  char *default_sounds_set;
  char *default_music_set;

  char *fallback_graphics_file;
  char *fallback_sounds_file;
  char *fallback_music_file;

  char *default_level_series;

  int default_window_width;
  int default_window_height;

  boolean choose_from_top_leveldir;
  boolean show_scaling_in_title;
  boolean create_user_levelset;
  boolean info_screens_from_main;

  boolean menu_game;
  boolean menu_engines;
  boolean menu_editor;
  boolean menu_graphics;
  boolean menu_sound;
  boolean menu_artwork;
  boolean menu_input;
  boolean menu_touch;
  boolean menu_shortcuts;
  boolean menu_exit;
  boolean menu_save_and_exit;

  boolean menu_shortcuts_various;
  boolean menu_shortcuts_focus;
  boolean menu_shortcuts_tape;
  boolean menu_shortcuts_sound;
  boolean menu_shortcuts_snap;
  boolean menu_shortcuts_speed;
  boolean menu_shortcuts_engine;

  boolean info_title;
  boolean info_elements;
  boolean info_music;
  boolean info_credits;
  boolean info_program;
  boolean info_version;
  boolean info_levelset;
  boolean info_level;
  boolean info_story;
  boolean info_exit;
};

struct SetupDebugInfo
{
  int frame_delay[10];
  Key frame_delay_key[10];
  boolean frame_delay_use_mod_key;
  boolean frame_delay_game_only;
  boolean show_frames_per_second;
  int xsn_mode;
  int xsn_percent;
};

struct SetupInfo
{
  char *player_name;
  char *player_uuid;
  int player_version;

  boolean multiple_users;

  boolean sound;
  boolean sound_loops;
  boolean sound_music;
  boolean sound_simple;
  boolean toons;
  boolean global_animations;
  boolean scroll_delay;
  boolean forced_scroll_delay;
  int scroll_delay_value;
  char *engine_snapshot_mode;
  int engine_snapshot_memory;
  boolean fade_screens;
  boolean autorecord;
  boolean autorecord_after_replay;
  boolean auto_pause_on_start;
  boolean show_titlescreen;
  boolean show_level_story;
  boolean quick_doors;
  boolean team_mode;
  boolean handicap;
  boolean skip_levels;
  boolean allow_skipping_levels;
  boolean increment_levels;
  boolean auto_play_next_level;
  boolean count_score_after_game;
  boolean show_scores_after_game;
  boolean time_limit;
  boolean fullscreen;
  int window_scaling_percent;
  char *window_scaling_quality;
  char *screen_rendering_mode;
  char *vsync_mode;
  boolean ask_on_escape;
  boolean ask_on_escape_editor;
  boolean ask_on_save_tape;
  boolean ask_on_game_over;
  boolean ask_on_quit_game;
  boolean ask_on_quit_program;
  boolean quick_switch;
  boolean input_on_focus;
  boolean prefer_aga_graphics;
  boolean prefer_lowpass_sounds;
  boolean show_extra_panel_items;
  boolean game_speed_extended;
  int game_frame_delay;
  int default_game_engine_type;
  boolean bd_multiple_lives;
  boolean bd_skip_uncovering;
  boolean bd_skip_hatching;
  boolean bd_scroll_delay;
  boolean bd_show_invisible_outbox;
  int bd_smooth_movements;		// not boolean -- can also be "MODE_AUTO"
  int bd_pushing_graphics;		// not boolean -- can also be "MODE_AUTO"
  int bd_up_down_graphics;		// not boolean -- can also be "MODE_AUTO"
  int bd_falling_sounds;		// not boolean -- can also be "MODE_AUTO"
  int bd_palette_c64;
  int bd_palette_c64dtv;
  int bd_palette_atari;
  int bd_default_color_type;
  int bd_random_colors;
  boolean sp_show_border_elements;
  boolean small_game_graphics;
  boolean show_load_save_buttons;
  boolean show_undo_redo_buttons;
  boolean show_menu_to_save_setup;
  char *scores_in_highscore_list;

  char *graphics_set;
  char *sounds_set;
  char *music_set;
  int override_level_graphics;		// not boolean -- can also be "MODE_AUTO"
  int override_level_sounds;		// not boolean -- can also be "MODE_AUTO"
  int override_level_music;		// not boolean -- can also be "MODE_AUTO"

  int volume_simple;
  int volume_loops;
  int volume_music;
  boolean audio_sample_rate_44100;

  boolean network_mode;
  int network_player_nr;
  char *network_server_hostname;

  boolean use_api_server;
  char *api_server_hostname;
  char *api_server_password;
  boolean ask_for_uploading_tapes;
  boolean ask_for_remaining_tapes;
  boolean provide_uploading_tapes;
  boolean ask_for_using_api_server;
  boolean has_remaining_tapes;

  struct SetupAutoSetupInfo auto_setup;
  struct SetupLevelSetupInfo level_setup;

  struct SetupEditorInfo editor;
  struct SetupEditorCascadeInfo editor_cascade;
  struct SetupShortcutInfo shortcut;
  struct SetupInputInfo input[MAX_PLAYERS];
  struct SetupTouchInfo touch;
  struct SetupSystemInfo system;
  struct SetupInternalInfo internal;
  struct SetupDebugInfo debug;

  struct OptionInfo options;
};

struct UserInfo
{
  int nr;
};

#define ZIP_FILE_OK				0
#define ZIP_FILE_ERROR_CANNOT_OPEN		1
#define ZIP_FILE_ERROR_DIR_NOT_FOUND		2
#define ZIP_FILE_ERROR_DIR_EXISTS		3
#define ZIP_FILE_ERROR_FILE_INVALID		4
#define ZIP_FILE_ERROR_CONF_INVALID		5
#define ZIP_FILE_ERROR_CONF_NOT_FOUND		6
#define ZIP_FILE_ERROR_UNKNOWN			9

struct ZipFileInfo
{
  int state;

  char *top_dir_path;
  char *top_dir_conf_filename;
};

struct TreeInfo
{
  struct TreeInfo **node_top;	// topmost node in tree
  struct TreeInfo *node_parent;	// parent level directory info
  struct TreeInfo *node_group;	// level group sub-directory info
  struct TreeInfo *next;	// next level series structure node

  int cl_first;			// internal control field for setup screen
  int cl_cursor;		// internal control field for setup screen

  int type;			// type of tree content

  // fields for "type == TREE_TYPE_LEVEL_DIR"

  char *subdir;			// tree info sub-directory basename (may be ".")
  char *fullpath;		// complete path relative to tree base directory
  char *basepath;		// absolute base path of tree base directory
  char *identifier;		// identifier string for configuration files
  char *name;			// tree info name, as displayed in selection menues
  char *name_sorting;		// optional sorting name for correct name sorting
  char *author;			// level or artwork author name
  char *year;			// optional year of creation for levels or artwork

  char *program_title;		// optional alternative text for program title
  char *program_copyright;	// optional alternative text for program copyright
  char *program_company;	// optional alternative text for program company

  char *imported_from;		// optional comment for imported levels or artwork
  char *imported_by;		// optional comment for imported levels or artwork
  char *tested_by;		// optional comment to name people who tested a set

  char *graphics_set_ecs;	// special EMC custom graphics set (ECS graphics)
  char *graphics_set_aga;	// special EMC custom graphics set (AGA graphics)
  char *graphics_set;		// optional custom graphics set (level tree only)
  char *sounds_set_default;	// default EMC custom sounds set
  char *sounds_set_lowpass;	// special EMC custom sounds set (lowpass filter)
  char *sounds_set;		// optional custom sounds set (level tree only)
  char *music_set;		// optional custom music set (level tree only)
  char *graphics_path;		// path to optional custom graphics set (level only)
  char *sounds_path;		// path to optional custom sounds set (level only)
  char *music_path;		// path to optional custom music set (level only)

  char *level_filename;		// filename of level file (for packed level file)
  char *level_filetype;		// type of levels in level directory or level file

  char *special_flags;		// flags for special actions performed on level file

  char *empty_level_name;	// name pattern if level title is "nameless level"
  boolean force_level_name;	// force also renaming non-nameless level titles

  int levels;			// number of levels in level series
  int first_level;		// first level number (to allow start with 0 or 1)
  int last_level;		// last level number (automatically calculated)
  int sort_priority;		// sort levels by 'sort_priority' and then by name
  int pos;			// custom position information of node in tree

  boolean latest_engine;	// force level set to use the latest game engine

  boolean level_group;		// directory contains more level series directories
  boolean parent_link;		// entry links back to parent directory
  boolean is_copy;		// this entry is a copy of another entry in the tree
  boolean in_user_dir;		// user defined levels are stored in home directory
  boolean user_defined;		// levels in user directory and marked as "private"
  boolean readonly;		// readonly levels can not be changed with editor
  boolean handicap;		// level set has no handicap when set to "false"
  boolean time_limit;		// level set has no time limit when set to "false"
  boolean skip_levels;		// levels can be skipped when set to "true"

  boolean use_emc_tiles;	// use (swapped) V5/V6 EMC tiles when set to "true"
  boolean info_screens_from_main; // can invoke info screens from main menu
  boolean replay_with_old_engine; // use the old BD engine for playing BDCFF replays

  int color;			// color to use on selection screen for this level
  char *class_desc;		// description of level series class
  int handicap_level;		// number of the lowest unsolved level

  char *infotext;		// optional text to describe the tree type (headline)
};

typedef struct TreeInfo TreeInfo;
typedef struct TreeInfo LevelDirTree;
typedef struct TreeInfo ArtworkDirTree;
typedef struct TreeInfo GraphicsDirTree;
typedef struct TreeInfo SoundsDirTree;
typedef struct TreeInfo MusicDirTree;

struct ArtworkInfo
{
  GraphicsDirTree *gfx_first;
  GraphicsDirTree *gfx_current;
  SoundsDirTree *snd_first;
  SoundsDirTree *snd_current;
  MusicDirTree *mus_first;
  MusicDirTree *mus_current;

  char *gfx_current_identifier;
  char *snd_current_identifier;
  char *mus_current_identifier;
};

struct ValueTextInfo
{
  int value;
  char *text;
};

struct StringValueTextInfo
{
  char *value;
  char *text;
};

struct ConfigInfo
{
  char *token;
  char *value;
};

struct ConfigTypeInfo
{
  char *token;
  char *value;
  int type;
};

struct TokenIntPtrInfo
{
  char *token;
  int *value;
};

struct FileInfo
{
  char *token;

  char *default_filename;
  char *filename;

  char **default_parameter;			// array of file parameters
  char **parameter;				// array of file parameters

  boolean redefined;
  boolean fallback_to_default;
  boolean default_is_cloned;
};

struct SetupFileList
{
  char *token;
  char *value;

  struct SetupFileList *next;
};

struct ListNodeInfo
{
  char *source_filename;			// primary key for node list
  int num_references;
};

struct PropertyMapping
{
  int base_index;
  int ext1_index;
  int ext2_index;
  int ext3_index;

  int artwork_index;
};

struct ArtworkListInfo
{
  int type;					// type of artwork

  int num_file_list_entries;
  int num_dynamic_file_list_entries;
  struct FileInfo *file_list;			// static artwork file array
  struct FileInfo *dynamic_file_list;		// dynamic artwrk file array

  int num_suffix_list_entries;
  struct ConfigTypeInfo *suffix_list;		// parameter suffixes array

  int num_base_prefixes;
  int num_ext1_suffixes;
  int num_ext2_suffixes;
  int num_ext3_suffixes;
  char **base_prefixes;				// base token prefixes array
  char **ext1_suffixes;				// property suffixes array 1
  char **ext2_suffixes;				// property suffixes array 2
  char **ext3_suffixes;				// property suffixes array 3

  int num_ignore_tokens;
  char **ignore_tokens;				// file tokens to be ignored

  int num_property_mapping_entries;
  struct PropertyMapping *property_mapping;	// mapping token -> artwork

  int sizeof_artwork_list_entry;

  struct ListNodeInfo **artwork_list;		// static artwork node array
  struct ListNodeInfo **dynamic_artwork_list;	// dynamic artwrk node array
  struct ListNode *content_list;		// dynamic artwork node list

  void *(*load_artwork)(char *);		// constructor function
  void (*free_artwork)(void *);			// destructor function
};

struct XY
{
  int x, y;
};

struct XYTileSize
{
  int x, y;
  int tile_size;
};

struct Rect
{
  int x, y;
  int width, height;
};

struct RectWithBorder
{
  int x, y;
  int width, height;
  int min_width, min_height;
  int max_width, max_height;
  int margin_left;
  int margin_right;
  int margin_top;
  int margin_bottom;
  int border_left;
  int border_right;
  int border_top;
  int border_bottom;
  int border_size;
  int align_size;
  int align, valign;
};

struct MenuPosInfo
{
  int x, y;
  int width, height;
  int align, valign;
};

struct DoorPartPosInfo
{
  int x, y;
  int step_xoffset;
  int step_yoffset;
  int step_delay;
  int start_step;
  int start_step_opening;
  int start_step_closing;
  boolean draw_masked;
  int sort_priority;
};

struct TextPosInfo
{
  int x, y;
  int xoffset;			// special case for various offsets for drawing
  int xoffset2;			// special case for various offsets for drawing
  int yoffset;			// special case for various offsets for drawing
  int yoffset2;			// special case for various offsets for drawing
  int width, height;
  int align, valign;
  int size;			// also used for suffix ".digits"
  int font, font_alt;
  boolean draw_masked;
  boolean draw_player;		// special case for network player buttons
  int sort_priority;		// also used for suffix ".draw_order"
  int id;

  int direction;		// needed for panel time/health graphics
  int class;			// needed for panel time/health graphics
  int style;			// needed for panel time/health graphics

  int tile_size;		// special case for list of network players
  int border_size;		// special case for list of preview players
  int vertical;			// special case for list of preview players

  boolean redefined;		// redefined by custom artwork
};

struct MouseActionInfo
{
  int lx, ly;
  int button;
  int button_hint;
};

struct LevelSetInfo
{
  int music[MAX_LEVELS];

  boolean level_story_shown[MAX_LEVELS];

  char *identifier;
  int level_nr;
};

struct LevelStats
{
  int played;
  int solved;
};


// ============================================================================
// exported variables
// ============================================================================

extern struct ProgramInfo	program;
extern struct NetworkInfo	network;
extern struct RuntimeInfo	runtime;
extern struct OptionInfo	options;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct GfxInfo		gfx;
extern struct TileCursorInfo	tile_cursor;
extern struct OverlayInfo	overlay;
extern struct ArtworkInfo	artwork;
extern struct JoystickInfo	joystick;
extern struct SetupInfo		setup;
extern struct UserInfo		user;
extern struct ZipFileInfo	zip_file;

extern LevelDirTree	       *leveldir_first_all;
extern LevelDirTree	       *leveldir_first;
extern LevelDirTree	       *leveldir_current;
extern int			level_nr;

extern struct LevelSetInfo	levelset;
extern struct LevelStats	level_stats[];

extern DrawWindow	       *window;
extern DrawBuffer	       *backbuffer;
extern DrawBuffer	       *drawto;

extern int			button_status;
extern boolean			motion_status;
extern int			wheel_steps;
extern boolean			keyrepeat_status;
extern boolean			textinput_status;

extern int			redraw_mask;

extern int			FrameCounter;


// function definitions

void InitProgramInfo(char *, char *, char *, char *, char *, char *, char *,
		     char *, VersionType);
void InitNetworkInfo(boolean, boolean, boolean, char *, int);
void InitRuntimeInfo(void);

void SetWindowTitle(void);
void SetWindowResized(void);

void InitWindowTitleFunction(char *(*window_title_function)(void));
void InitExitMessageFunction(void (*exit_message_function)(char *, va_list));
void InitExitFunction(void (*exit_function)(int));
void InitPlatformDependentStuff(void);
void ClosePlatformDependentStuff(void);

void InitGfxFieldInfo(int, int, int, int, int, int, int, int, Bitmap *);
void InitGfxTileSizeInfo(int, int);
void InitGfxDoor1Info(int, int, int, int);
void InitGfxDoor2Info(int, int, int, int);
void InitGfxDoor3Info(int, int, int, int);
void InitGfxWindowInfo(int, int);
void InitGfxScrollbufferInfo(int, int);
void InitGfxClipRegion(boolean, int, int, int, int);
void InitGfxDrawBusyAnimFunction(void (*draw_busy_anim_function)(boolean));
void InitGfxDrawGlobalAnimFunction(void (*draw_global_anim_function)(int, int));
void InitGfxDrawGlobalBorderFunction(void (*draw_global_border_function)(int));
void InitGfxDrawTileCursorFunction(void (*draw_tile_cursor_function)(int, int));
void InitGfxDrawEnvelopeRequestFunction(void (*draw_envelope_request_function)(int));
void InitGfxCustomArtworkInfo(void);
void InitGfxOtherSettings(void);
void InitTileCursorInfo(void);
void InitOverlayInfo(void);
void SetOverlayGridSizeAndButtons(void);
void SetTileCursorEnabled(boolean);
void SetTileCursorActive(boolean);
void SetTileCursorTargetXY(int, int);
void SetTileCursorXY(int, int);
void SetTileCursorSXSY(int, int);
void SetOverlayEnabled(boolean);
void SetOverlayActive(boolean);
void SetOverlayShowGrid(boolean);
boolean GetOverlayEnabled(void);
boolean GetOverlayActive(void);
void SetDrawDeactivationMask(int);
int GetDrawDeactivationMask(void);
void SetDrawBackgroundMask(int);
int GetDrawBackgroundMask(void);
void SetBackgroundBitmap(Bitmap *, int, int, int, int, int);
void SetRedrawMaskFromArea(int, int, int, int);

void LimitScreenUpdates(boolean);

void InitVideoDefaults(void);
void InitVideoDisplay(void);
void CloseVideoDisplay(void);
void InitVideoBuffer(int, int, int, boolean);
void ResetBitmapAlpha(Bitmap *);
Bitmap *CreateBitmapStruct(void);
Bitmap *CreateBitmap(int, int, int);
void ReCreateBitmap(Bitmap **, int, int);
void FreeBitmap(Bitmap *);
void SetBitmapAlphaNextBlit(Bitmap *, int);
void BlitBitmap(Bitmap *, Bitmap *, int, int, int, int, int, int);
void BlitBitmapTiled(Bitmap *, Bitmap *, int, int, int, int, int, int, int, int);
void FadeRectangle(int, int, int, int, int, int, int,
		   void (*draw_border_function)(void));
void FillRectangle(Bitmap *, int, int, int, int, Pixel);
void ClearRectangle(Bitmap *, int, int, int, int);
void ClearRectangleOnBackground(Bitmap *, int, int, int, int);
void BlitBitmapMasked(Bitmap *, Bitmap *, int, int, int, int, int, int);
boolean DrawingDeactivatedField(void);
boolean DrawingDeactivated(int, int);
boolean DrawingOnBackground(int, int);
boolean DrawingAreaChanged(void);
void BlitBitmapOnBackground(Bitmap *, Bitmap *, int, int, int, int, int, int);
void BlitTexture(Bitmap *, int, int, int, int, int, int);
void BlitTextureMasked(Bitmap *, int, int, int, int, int, int);
void BlitToScreen(Bitmap *, int, int, int, int, int, int);
void BlitToScreenMasked(Bitmap *, int, int, int, int, int, int);
void DrawSimpleWhiteLine(Bitmap *, int, int, int, int);
void DrawLines(Bitmap *, struct XY *, int, Pixel);
Pixel GetPixel(Bitmap *, int, int);
Pixel GetPixelFromRGB(Bitmap *, unsigned int, unsigned int, unsigned int);

void KeyboardAutoRepeatOn(void);
void KeyboardAutoRepeatOff(void);
boolean SetVideoMode(boolean);
void SetVideoFrameDelay(unsigned int);
unsigned int GetVideoFrameDelay(void);
boolean ChangeVideoModeIfNeeded(boolean);

Bitmap *LoadImage(char *);
Bitmap *LoadCustomImage(char *);
void ReloadCustomImage(Bitmap *, char *);

Bitmap *ZoomBitmap(Bitmap *, int, int);
void ReCreateGameTileSizeBitmap(Bitmap **);
void CreateBitmapWithSmallBitmaps(Bitmap **, int, int);
void CreateBitmapTextures(Bitmap **);
void FreeBitmapTextures(Bitmap **);
void ScaleBitmap(Bitmap **, int);

void SetMouseCursor(int);
void UpdateRawMousePosition(int, int);
void UpdateMousePosition(void);

void OpenAudio(void);
void CloseAudio(void);
void SetAudioMode(boolean);

void InitEventFilter(EventFilter);
boolean PendingEvent(void);
void WaitEvent(Event *event);
void PeekEvent(Event *event);
void PumpEvents(void);
void CheckQuitEvent(void);
Key GetEventKey(KeyEvent *);
KeyMod HandleKeyModState(Key, int);
KeyMod GetKeyModState(void);
KeyMod GetKeyModStateFromEvents(void);
void StartTextInput(int, int, int, int);
void StopTextInput(void);
void PushUserEvent(int, int, int);
void PushDropEvent(char *);
boolean PendingEscapeKeyEvent(void);

void InitJoysticks(void);
boolean ReadJoystick(int, int *, int *, boolean *, boolean *);
boolean CheckJoystickOpened(int);
void ClearJoystickState(void);

void InitEmscriptenFilesystem(void);
void SyncEmscriptenFilesystem(void);

#endif // SYSTEM_H
