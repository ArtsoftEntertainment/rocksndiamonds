// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_game.c
// ============================================================================

#include <math.h>

#include "main_mm.h"

#include "mm_main.h"
#include "mm_game.h"
#include "mm_tools.h"

// graphic position values for game controls
#define ENERGY_XSIZE		32
#define ENERGY_YSIZE		MAX_LASER_ENERGY
#define OVERLOAD_XSIZE		ENERGY_XSIZE
#define OVERLOAD_YSIZE		MAX_LASER_OVERLOAD

// values for Explode_MM()
#define EX_PHASE_START		0
#define EX_TYPE_NONE		0
#define EX_TYPE_NORMAL		(1 << 0)

// special positions in the game control window (relative to control window)
#define XX_LEVEL		36
#define YY_LEVEL		23
#define XX_KETTLES		29
#define YY_KETTLES		63
#define XX_SCORE		22
#define YY_SCORE		101
#define XX_ENERGY		8
#define YY_ENERGY		158
#define XX_OVERLOAD		60
#define YY_OVERLOAD		YY_ENERGY

// special positions in the game control window (relative to main window)
#define DX_LEVEL		(DX + XX_LEVEL)
#define DY_LEVEL		(DY + YY_LEVEL)
#define DX_KETTLES		(DX + XX_KETTLES)
#define DY_KETTLES		(DY + YY_KETTLES)
#define DX_SCORE		(DX + XX_SCORE)
#define DY_SCORE		(DY + YY_SCORE)
#define DX_ENERGY		(DX + XX_ENERGY)
#define DY_ENERGY		(DY + YY_ENERGY)
#define DX_OVERLOAD		(DX + XX_OVERLOAD)
#define DY_OVERLOAD		(DY + YY_OVERLOAD)

#define IS_LOOP_SOUND(s)	((s) == SND_FUEL)
#define IS_MUSIC_SOUND(s)	((s) == SND_TYGER || (s) == SND_VOYAGER)

// game button identifiers
#define GAME_CTRL_ID_LEFT	0
#define GAME_CTRL_ID_MIDDLE	1
#define GAME_CTRL_ID_RIGHT	2

#define NUM_GAME_BUTTONS	3

// values for DrawLaser()
#define DL_LASER_DISABLED	0
#define DL_LASER_ENABLED	1

// values for 'click_delay_value' in ClickElement()
#define CLICK_DELAY_FIRST	12	// delay (frames) after first click
#define CLICK_DELAY		6	// delay (frames) for pressed butten

#define AUTO_ROTATE_DELAY	CLICK_DELAY
#define INIT_GAME_ACTIONS_DELAY	(ONE_SECOND_DELAY / GAME_FRAME_DELAY)
#define NUM_INIT_CYCLE_STEPS	16
#define PACMAN_MOVE_DELAY	12
#define ENERGY_DELAY		(ONE_SECOND_DELAY / GAME_FRAME_DELAY)
#define HEALTH_DEC_DELAY	3
#define HEALTH_INC_DELAY	9
#define HEALTH_DELAY(x)		((x) ? HEALTH_DEC_DELAY : HEALTH_INC_DELAY)

#define BEGIN_NO_HEADLESS			\
  {						\
    boolean last_headless = program.headless;	\
						\
    program.headless = FALSE;			\

#define END_NO_HEADLESS				\
    program.headless = last_headless;		\
  }						\

// forward declaration for internal use
static int MovingOrBlocked2Element_MM(int, int);
static void Bang_MM(int, int);
static void RaiseScore_MM(int);
static void RaiseScoreElement_MM(int);
static void RemoveMovingField_MM(int, int);
static void InitMovingField_MM(int, int, int);
static void ContinueMoving_MM(int, int);

static void AddLaserEdge(int, int);
static void ScanLaser(void);
static void DrawLaser(int, int);
static boolean HitElement(int, int);
static boolean HitOnlyAnEdge(int);
static boolean HitPolarizer(int, int);
static boolean HitBlock(int, int);
static boolean HitLaserSource(int, int);
static boolean HitLaserDestination(int, int);
static boolean HitReflectingWalls(int, int);
static boolean HitAbsorbingWalls(int, int);
static void RotateMirror(int, int, int);
static boolean ObjHit(int, int, int);
static void DeletePacMan(int, int);
static void MovePacMen(void);

// bitmap for laser beam detection
static Bitmap *laser_bitmap = NULL;

// variables for laser control
static int last_LX = 0, last_LY = 0, last_hit_mask = 0;
static int hold_x = -1, hold_y = -1;

// variables for pacman control
static int pacman_nr = -1;

// various game engine delay counters
static DelayCounter rotate_delay = { AUTO_ROTATE_DELAY };
static DelayCounter pacman_delay = { PACMAN_MOVE_DELAY };
static DelayCounter energy_delay = { ENERGY_DELAY };
static DelayCounter overload_delay = { 0 };

// element mask positions for scanning pixels of MM elements
#define MM_MASK_MCDUFFIN_RIGHT	0
#define MM_MASK_MCDUFFIN_UP	1
#define MM_MASK_MCDUFFIN_LEFT	2
#define MM_MASK_MCDUFFIN_DOWN	3
#define MM_MASK_GRID_1		4
#define MM_MASK_GRID_2		5
#define MM_MASK_GRID_3		6
#define MM_MASK_GRID_4		7
#define MM_MASK_SLOPE_1		8
#define MM_MASK_SLOPE_2		9
#define MM_MASK_SLOPE_3		10
#define MM_MASK_SLOPE_4		11
#define MM_MASK_RECTANGLE	12
#define MM_MASK_CIRCLE		13

#define NUM_MM_MASKS		14

// element masks for scanning pixels of MM elements
static const char mm_masks[NUM_MM_MASKS][16][16 + 1] =
{
  {
    "                ",
    "    XXXXX       ",
    "   XXXXXXX      ",
    "  XXXXXXXXXXX   ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXXX",
    "  XXXXXXXXXXXXXX",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
  },
  {
    "                ",
    "    XXXXXXXX    ",
    "  XXXXXXXXXXXX  ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
  },
  {
    "                ",
    "    XXXXXX      ",
    "  XXXXXXXXX     ",
    " XXXXXXXXXXX    ",
    "XXXXXXXXXXXXX   ",
    "XXXXXXXXXXXXX   ",
    "XXXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
  },
  {
    "                ",
    "    XXXXXX      ",
    "   XXXXXXXX     ",
    "  XXXXXXXXXX    ",
    "  XXXXXXXXXXX   ",
    "  XXXXXXXXXXX   ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXX  XXXXX  ",
  },
  {
    " XXXXXX  XXXXXX ",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "                ",
    "                ",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    " XXXXXX  XXXXXX ",
  },
  {
    " XXXXXX  XXXXXX ",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    " XXXXXX  XXXXXX ",
    " XXXXXX  XXXXXX ",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    " XXXXXX  XXXXXX ",
  },
  {
    "     XX  XXXXX  ",
    "    XXX  XXXX   ",
    "   XXXX  XXX   X",
    "  XXXXXXXXX   XX",
    " XXXXXXXXX   XXX",
    "XXXXXXXXX   XXXX",
    "XXXXXXXX   XXXXX",
    "   XXXX   XXX   ",
    "   XXX   XXXX   ",
    "XXXXX   XXXXXXXX",
    "XXXX   XXXXXXXXX",
    "XXX   XXXXXXXXX ",
    "XX   XXXXXXXXX  ",
    "X   XXX  XXXX   ",
    "   XXXX  XXX    ",
    "  XXXXX  XX     ",
  },
  {
    "  XXXXX  XX     ",
    "   XXXX  XXX    ",
    "X   XXX  XXXX   ",
    "XX   XXXXXXXXX  ",
    "XXX   XXXXXXXXX ",
    "XXXX   XXXXXXXXX",
    "XXXXX   XXXXXXXX",
    "   XXX   XXXX   ",
    "   XXXX   XXX   ",
    "XXXXXXXX   XXXXX",
    "XXXXXXXXX   XXXX",
    " XXXXXXXXX   XXX",
    "  XXXXXXXXX   XX",
    "   XXXX  XXX   X",
    "    XXX  XXXX   ",
    "     XX  XXXXX  ",
  },
  {
    "               X",
    "              XX",
    "             XXX",
    "            XXXX",
    "           XXXXX",
    "          XXXXXX",
    "         XXXXXXX",
    "        XXXXXXXX",
    "       XXXXXXXXX",
    "      XXXXXXXXXX",
    "     XXXXXXXXXXX",
    "    XXXXXXXXXXXX",
    "   XXXXXXXXXXXXX",
    "  XXXXXXXXXXXXXX",
    " XXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
  },
  {
    "X               ",
    "XX              ",
    "XXX             ",
    "XXXX            ",
    "XXXXX           ",
    "XXXXXX          ",
    "XXXXXXX         ",
    "XXXXXXXX        ",
    "XXXXXXXXX       ",
    "XXXXXXXXXX      ",
    "XXXXXXXXXXX     ",
    "XXXXXXXXXXXX    ",
    "XXXXXXXXXXXXX   ",
    "XXXXXXXXXXXXXX  ",
    "XXXXXXXXXXXXXXX ",
    "XXXXXXXXXXXXXXXX",
  },
  {
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXX ",
    "XXXXXXXXXXXXXX  ",
    "XXXXXXXXXXXXX   ",
    "XXXXXXXXXXXX    ",
    "XXXXXXXXXXX     ",
    "XXXXXXXXXX      ",
    "XXXXXXXXX       ",
    "XXXXXXXX        ",
    "XXXXXXX         ",
    "XXXXXX          ",
    "XXXXX           ",
    "XXXX            ",
    "XXX             ",
    "XX              ",
    "X               ",
  },
  {
    "XXXXXXXXXXXXXXXX",
    " XXXXXXXXXXXXXXX",
    "  XXXXXXXXXXXXXX",
    "   XXXXXXXXXXXXX",
    "    XXXXXXXXXXXX",
    "     XXXXXXXXXXX",
    "      XXXXXXXXXX",
    "       XXXXXXXXX",
    "        XXXXXXXX",
    "         XXXXXXX",
    "          XXXXXX",
    "           XXXXX",
    "            XXXX",
    "             XXX",
    "              XX",
    "               X",
  },
  {
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
  },
  {
    "                ",
    "      XXXX      ",
    "    XXXXXXXX    ",
    "   XXXXXXXXXX   ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "   XXXXXXXXXX   ",
    "    XXXXXXXX    ",
    "      XXXX      ",
    "                ",
  },
};

static int get_element_angle(int element)
{
  int element_phase = get_element_phase(element);

  if (IS_MIRROR_FIXED(element) ||
      IS_MCDUFFIN(element) ||
      IS_LASER(element) ||
      IS_RECEIVER(element))
    return 4 * element_phase;
  else if (IS_DF_SLOPE(element))
    return 4 + (element_phase % 2) * 8;
  else
    return element_phase;
}

static int get_opposite_angle(int angle)
{
  int opposite_angle = angle + ANG_RAY_180;

  // make sure "opposite_angle" is in valid interval [0, 15]
  return (opposite_angle + 16) % 16;
}

static int get_mirrored_angle(int laser_angle, int mirror_angle)
{
  int reflected_angle = 16 - laser_angle + mirror_angle;

  // make sure "reflected_angle" is in valid interval [0, 15]
  return (reflected_angle + 16) % 16;
}

static void DrawLaserLines(struct XY *points, int num_points, int mode)
{
  Pixel pixel_drawto = (mode == DL_LASER_ENABLED ? pen_ray     : pen_bg);
  Pixel pixel_buffer = (mode == DL_LASER_ENABLED ? WHITE_PIXEL : BLACK_PIXEL);

  DrawLines(drawto_mm, points, num_points, pixel_drawto);

  BEGIN_NO_HEADLESS
  {
    DrawLines(laser_bitmap, points, num_points, pixel_buffer);
  }
  END_NO_HEADLESS
}

static boolean CheckLaserPixel(int x, int y)
{
  Pixel pixel;

  BEGIN_NO_HEADLESS
  {
    pixel = ReadPixel(laser_bitmap, x, y);
  }
  END_NO_HEADLESS

  return (pixel == WHITE_PIXEL);
}

static void CheckExitMM(void)
{
  int exit_element = EL_EMPTY;
  int exit_x = 0;
  int exit_y = 0;
  int x, y;
  static int xy[4][2] =
  {
    { +1,  0 },
    {  0, -1 },
    { -1,  0 },
    {  0, +1 }
  };

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      if (Tile[x][y] == EL_EXIT_CLOSED)
      {
	// initiate opening animation of exit door
	Tile[x][y] = EL_EXIT_OPENING;

	exit_element = EL_EXIT_OPEN;
	exit_x = x;
	exit_y = y;
      }
      else if (IS_RECEIVER(Tile[x][y]))
      {
	// remove field that blocks receiver
	int phase = Tile[x][y] - EL_RECEIVER_START;
	int blocking_x, blocking_y;

	blocking_x = x + xy[phase][0];
	blocking_y = y + xy[phase][1];

	if (IN_LEV_FIELD(blocking_x, blocking_y))
	{
	  Tile[blocking_x][blocking_y] = EL_EMPTY;

	  DrawField_MM(blocking_x, blocking_y);
	}

	exit_element = EL_RECEIVER;
	exit_x = x;
	exit_y = y;
      }
    }
  }

  if (exit_element != EL_EMPTY)
    PlayLevelSound_MM(exit_x, exit_y, exit_element, MM_ACTION_OPENING);
}

static void SetLaserColor(int brightness)
{
  int color_min = 0x00;
  int color_max = brightness;		// (0x00 <= brightness <= 0xFF)
  int color_up   = color_max * laser.overload_value / MAX_LASER_OVERLOAD;
  int color_down = color_max - color_up;

  pen_ray =
    GetPixelFromRGB(window,
		    (game_mm.laser_red   ? color_max  : color_up),
		    (game_mm.laser_green ? color_down : color_min),
		    (game_mm.laser_blue  ? color_down : color_min));
}

static void InitMovDir_MM(int x, int y)
{
  int element = Tile[x][y];
  static int direction[3][4] =
  {
    { MV_RIGHT, MV_UP,    MV_LEFT,  MV_DOWN },
    { MV_LEFT,  MV_DOWN,  MV_RIGHT, MV_UP   },
    { MV_LEFT,  MV_RIGHT, MV_UP,    MV_DOWN }
  };

  switch (element)
  {
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      Tile[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element - EL_PACMAN_RIGHT];
      break;

    default:
      break;
  }
}

static void InitField(int x, int y, boolean init_game)
{
  int element = Tile[x][y];

  switch (element)
  {
    case EL_DF_EMPTY:
      Tile[x][y] = EL_EMPTY;
      break;

    case EL_KETTLE:
    case EL_CELL:
      if (init_game && native_mm_level.auto_count_kettles)
	game_mm.kettles_still_needed++;
      break;

    case EL_LIGHTBULB_OFF:
      game_mm.lights_still_needed++;
      break;

    default:
      if (IS_MIRROR(element) ||
	  IS_BEAMER_OLD(element) ||
	  IS_BEAMER(element) ||
	  IS_POLAR(element) ||
	  IS_POLAR_CROSS(element) ||
	  IS_DF_MIRROR(element) ||
	  IS_DF_MIRROR_AUTO(element) ||
	  IS_GRID_STEEL_AUTO(element) ||
	  IS_GRID_WOOD_AUTO(element) ||
	  IS_FIBRE_OPTIC(element))
      {
	if (IS_BEAMER_OLD(element))
	{
	  Tile[x][y] = EL_BEAMER_BLUE_START + (element - EL_BEAMER_START);
	  element = Tile[x][y];
	}

	if (!IS_FIBRE_OPTIC(element))
	{
	  static int steps_grid_auto = 0;

	  if (game_mm.num_cycle == 0)	// initialize cycle steps for grids
	    steps_grid_auto = RND(16) * (RND(2) ? -1 : +1);

	  if (IS_GRID_STEEL_AUTO(element) ||
	      IS_GRID_WOOD_AUTO(element))
	    game_mm.cycle[game_mm.num_cycle].steps = steps_grid_auto;
	  else
	    game_mm.cycle[game_mm.num_cycle].steps = RND(16) * (RND(2) ? -1 : +1);

	  game_mm.cycle[game_mm.num_cycle].x = x;
	  game_mm.cycle[game_mm.num_cycle].y = y;
	  game_mm.num_cycle++;
	}

	if (IS_BEAMER(element) || IS_FIBRE_OPTIC(element))
	{
	  int beamer_nr = BEAMER_NR(element);
	  int nr = laser.beamer[beamer_nr][0].num;

	  laser.beamer[beamer_nr][nr].x = x;
	  laser.beamer[beamer_nr][nr].y = y;
	  laser.beamer[beamer_nr][nr].num = 1;
	}
      }
      else if (IS_PACMAN(element))
      {
	InitMovDir_MM(x, y);
      }
      else if (IS_MCDUFFIN(element) || IS_LASER(element))
      {
	if (init_game)
	{
	  laser.start_edge.x = x;
	  laser.start_edge.y = y;
	  laser.start_angle = get_element_angle(element);
	}

        if (IS_MCDUFFIN(element))
        {
          game_mm.laser_red   = native_mm_level.mm_laser_red;
          game_mm.laser_green = native_mm_level.mm_laser_green;
          game_mm.laser_blue  = native_mm_level.mm_laser_blue;
        }
        else
        {
          game_mm.laser_red   = native_mm_level.df_laser_red;
          game_mm.laser_green = native_mm_level.df_laser_green;
          game_mm.laser_blue  = native_mm_level.df_laser_blue;
        }

	game_mm.has_mcduffin = (IS_MCDUFFIN(element));
      }

      break;
  }
}

static void InitCycleElements_RotateSingleStep(void)
{
  int i;

  if (game_mm.num_cycle == 0)	// no elements to cycle
    return;

  for (i = 0; i < game_mm.num_cycle; i++)
  {
    int x = game_mm.cycle[i].x;
    int y = game_mm.cycle[i].y;
    int step = SIGN(game_mm.cycle[i].steps);
    int last_element = Tile[x][y];
    int next_element = get_rotated_element(last_element, step);

    if (!game_mm.cycle[i].steps)
      continue;

    Tile[x][y] = next_element;

    game_mm.cycle[i].steps -= step;
  }
}

static void InitLaser(void)
{
  int start_element = Tile[laser.start_edge.x][laser.start_edge.y];
  int step = (IS_LASER(start_element) ? 4 : 0);

  LX = laser.start_edge.x * TILEX;
  if (laser.start_angle == ANG_RAY_UP || laser.start_angle == ANG_RAY_DOWN)
    LX += 14;
  else
    LX += (laser.start_angle == ANG_RAY_RIGHT ? 28 + step : 0 - step);

  LY = laser.start_edge.y * TILEY;
  if (laser.start_angle == ANG_RAY_UP || laser.start_angle == ANG_RAY_DOWN)
    LY += (laser.start_angle == ANG_RAY_DOWN ? 28 + step : 0 - step);
  else
    LY += 14;

  XS = 2 * Step[laser.start_angle].x;
  YS = 2 * Step[laser.start_angle].y;

  laser.current_angle = laser.start_angle;

  laser.num_damages = 0;
  laser.num_edges = 0;
  laser.num_beamers = 0;
  laser.beamer_edge[0] = 0;

  laser.dest_element = EL_EMPTY;
  laser.wall_mask = 0;

  AddLaserEdge(LX, LY);		// set laser starting edge

  SetLaserColor(0xFF);
}

void InitGameEngine_MM(void)
{
  int i, x, y;

  BEGIN_NO_HEADLESS
  {
    // initialize laser bitmap to current playfield (screen) size
    ReCreateBitmap(&laser_bitmap, drawto_mm->width, drawto_mm->height);
    ClearRectangle(laser_bitmap, 0, 0, drawto_mm->width, drawto_mm->height);
  }
  END_NO_HEADLESS

  // set global game control values
  game_mm.num_cycle = 0;
  game_mm.num_pacman = 0;

  game_mm.score = 0;
  game_mm.energy_left = 0;	// later set to "native_mm_level.time"
  game_mm.kettles_still_needed =
    (native_mm_level.auto_count_kettles ? 0 : native_mm_level.kettles_needed);
  game_mm.lights_still_needed = 0;
  game_mm.num_keys = 0;
  game_mm.ball_choice_pos = 0;

  game_mm.laser_red = FALSE;
  game_mm.laser_green = FALSE;
  game_mm.laser_blue = TRUE;
  game_mm.has_mcduffin = TRUE;

  game_mm.level_solved = FALSE;
  game_mm.game_over = FALSE;
  game_mm.game_over_cause = 0;
  game_mm.game_over_message = NULL;

  game_mm.laser_overload_value = 0;
  game_mm.laser_enabled = FALSE;

  // set global laser control values (must be set before "InitLaser()")
  laser.start_edge.x = 0;
  laser.start_edge.y = 0;
  laser.start_angle = 0;

  for (i = 0; i < MAX_NUM_BEAMERS; i++)
    laser.beamer[i][0].num = laser.beamer[i][1].num = 0;

  laser.overloaded = FALSE;
  laser.overload_value = 0;
  laser.fuse_off = FALSE;
  laser.fuse_x = laser.fuse_y = -1;

  laser.dest_element = EL_EMPTY;
  laser.dest_element_last = EL_EMPTY;
  laser.dest_element_last_x = -1;
  laser.dest_element_last_y = -1;
  laser.wall_mask = 0;

  last_LX = 0;
  last_LY = 0;
  last_hit_mask = 0;

  hold_x = -1;
  hold_y = -1;

  pacman_nr = -1;

  CT = Ct = 0;

  rotate_delay.count = 0;
  pacman_delay.count = 0;
  energy_delay.count = 0;
  overload_delay.count = 0;

  ClickElement(-1, -1, -1);

  for (x = 0; x < lev_fieldx; x++)
  {
    for (y = 0; y < lev_fieldy; y++)
    {
      Tile[x][y] = Ur[x][y];
      Hit[x][y] = Box[x][y] = 0;
      Angle[x][y] = 0;
      MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
      Store[x][y] = Store2[x][y] = 0;
      Stop[x][y] = FALSE;

      InitField(x, y, TRUE);
    }
  }

  DrawLevel_MM();
}

void InitGameActions_MM(void)
{
  int num_init_game_frames = INIT_GAME_ACTIONS_DELAY;
  int cycle_steps_done = 0;
  int i;

  InitLaser();

  for (i = 0; i <= num_init_game_frames; i++)
  {
    if (i == num_init_game_frames)
      StopSound_MM(SND_MM_GAME_LEVELTIME_CHARGING);
    else if (setup.sound_loops)
      PlaySoundLoop_MM(SND_MM_GAME_LEVELTIME_CHARGING);
    else
      PlaySound_MM(SND_MM_GAME_LEVELTIME_CHARGING);

    game_mm.energy_left = native_mm_level.time * i / num_init_game_frames;

    UpdateAndDisplayGameControlValues();

    while (cycle_steps_done < NUM_INIT_CYCLE_STEPS * i / num_init_game_frames)
    {
      InitCycleElements_RotateSingleStep();

      cycle_steps_done++;
    }

    AdvanceFrameCounter();
    AdvanceGfxFrame();

    if (PendingEscapeKeyEvent())
      continue;

#ifdef DEBUG
    if (setup.quick_doors)
      continue;
#endif

    DrawLevel_MM();

    BackToFront_MM();
  }

#ifdef DEBUG
  if (setup.quick_doors)
    DrawLevel_MM();
#endif

  ScanLaser();

  if (game_mm.kettles_still_needed == 0)
    CheckExitMM();

  SetTileCursorXY(laser.start_edge.x, laser.start_edge.y);
  SetTileCursorActive(TRUE);

  // restart all delay counters after initially cycling game elements
  ResetFrameCounter(&rotate_delay);
  ResetFrameCounter(&pacman_delay);
  ResetFrameCounter(&energy_delay);
  ResetFrameCounter(&overload_delay);
}

static void FadeOutLaser(void)
{
  int i;

  for (i = 15; i >= 0; i--)
  {
    SetLaserColor(0x11 * i);

    DrawLaser(0, DL_LASER_ENABLED);

    BackToFront_MM();
    Delay_WithScreenUpdates(50);
  }

  DrawLaser(0, DL_LASER_DISABLED);

  StopSound_MM(SND_MM_GAME_HEALTH_CHARGING);
}

static void GameOver_MM(int game_over_cause)
{
  game_mm.game_over = TRUE;
  game_mm.game_over_cause = game_over_cause;
  game_mm.game_over_message = (game_mm.has_mcduffin ?
			       (game_over_cause == GAME_OVER_BOMB ?
				"Bomb killed Mc Duffin!" :
				game_over_cause == GAME_OVER_NO_ENERGY ?
				"Out of magic energy!" :
				game_over_cause == GAME_OVER_OVERLOADED ?
				"Magic spell hit Mc Duffin!" :
				NULL) :
			       (game_over_cause == GAME_OVER_BOMB ?
				"Bomb destroyed laser cannon!" :
				game_over_cause == GAME_OVER_NO_ENERGY ?
				"Out of laser energy!" :
				game_over_cause == GAME_OVER_OVERLOADED ?
				"Laser beam hit laser cannon!" :
				NULL));

  SetTileCursorActive(FALSE);
}

static void AddLaserEdge(int lx, int ly)
{
  int full_sxsize = MAX(FULL_SXSIZE, lev_fieldx * TILEX);
  int full_sysize = MAX(FULL_SYSIZE, lev_fieldy * TILEY);

  // check if laser is still inside visible playfield area (or inside level)
  if (cSX + lx < REAL_SX || cSX + lx >= REAL_SX + full_sxsize ||
      cSY + ly < REAL_SY || cSY + ly >= REAL_SY + full_sysize)
  {
    Warn("AddLaserEdge: out of bounds: %d, %d", lx, ly);

    return;
  }

  laser.edge[laser.num_edges].x = cSX2 + lx;
  laser.edge[laser.num_edges].y = cSY2 + ly;
  laser.num_edges++;

  laser.redraw = TRUE;
}

static void AddDamagedField(int ex, int ey)
{
  // prevent adding the same field position again
  if (laser.num_damages > 0 &&
      laser.damage[laser.num_damages - 1].x == ex &&
      laser.damage[laser.num_damages - 1].y == ey &&
      laser.damage[laser.num_damages - 1].edge == laser.num_edges)
    return;

  laser.damage[laser.num_damages].is_mirror = FALSE;
  laser.damage[laser.num_damages].angle = laser.current_angle;
  laser.damage[laser.num_damages].edge = laser.num_edges;
  laser.damage[laser.num_damages].x = ex;
  laser.damage[laser.num_damages].y = ey;
  laser.num_damages++;
}

static boolean StepBehind(void)
{
  if (laser.num_edges)
  {
    int x = LX - XS;
    int y = LY - YS;
    int last_x = laser.edge[laser.num_edges - 1].x - cSX2;
    int last_y = laser.edge[laser.num_edges - 1].y - cSY2;

    return ((x - last_x) * XS < 0 || (y - last_y) * YS < 0);
  }

  return FALSE;
}

static int getMaskFromElement(int element)
{
  if (IS_MCDUFFIN(element))
    return MM_MASK_MCDUFFIN_RIGHT + get_element_phase(element);
  else if (IS_GRID(element))
    return MM_MASK_GRID_1 + get_element_phase(element);
  else if (IS_DF_GRID(element))
    return MM_MASK_RECTANGLE;
  else if (IS_DF_SLOPE(element))
    return MM_MASK_SLOPE_1 + get_element_phase(element);
  else if (IS_RECTANGLE(element))
    return MM_MASK_RECTANGLE;
  else
    return MM_MASK_CIRCLE;
}

static int getPixelFromMask(int pos, int dx, int dy)
{
  return (mm_masks[pos][dy / 2][dx / 2] == 'X' ? 1 : 0);
}

static int getLevelFromLaserX(int x)
{
  return x / TILEX - (x < 0 ? 1 : 0);		// correct negative values
}

static int getLevelFromLaserY(int y)
{
  return y / TILEY - (y < 0 ? 1 : 0);		// correct negative values
}

static int ScanPixel(void)
{
  int hit_mask = 0;

#if 0
  Debug("game:mm:ScanPixel", "start scanning at (%d, %d) [%d, %d] [%d, %d]",
	LX, LY, LX / TILEX, LY / TILEY, LX % TILEX, LY % TILEY);
#endif

  // follow laser beam until it hits something (at least the screen border)
  while (hit_mask == HIT_MASK_NO_HIT)
  {
    int i;

#if 0
    // for safety
    if (SX + LX < REAL_SX || SX + LX >= REAL_SX + FULL_SXSIZE ||
	SY + LY < REAL_SY || SY + LY >= REAL_SY + FULL_SYSIZE)
    {
      Debug("game:mm:ScanPixel", "touched screen border!");

      return HIT_MASK_ALL;
    }
#endif

    // check if laser scan has crossed element boundaries (not just mini tiles)
    boolean cross_x = (LX / TILEX != (LX + 2) / TILEX);
    boolean cross_y = (LY / TILEY != (LY + 2) / TILEY);

    if (cross_x && cross_y)
    {
      int elx1 = (LX - XS) / TILEX;
      int ely1 = (LY + YS) / TILEY;
      int elx2 = (LX + XS) / TILEX;
      int ely2 = (LY - YS) / TILEY;

      // add element corners left and right from the laser beam to damage list

      if (IN_LEV_FIELD(elx1, ely1) && Tile[elx1][ely1] != EL_EMPTY)
	AddDamagedField(elx1, ely1);

      if (IN_LEV_FIELD(elx2, ely2) && Tile[elx2][ely2] != EL_EMPTY)
	AddDamagedField(elx2, ely2);
    }

    for (i = 0; i < 4; i++)
    {
      int px = LX + (i % 2) * 2;
      int py = LY + (i / 2) * 2;
      int dx = px % TILEX;
      int dy = py % TILEY;
      int lx = getLevelFromLaserX(px);
      int ly = getLevelFromLaserY(py);
      Pixel pixel;

      if (IN_LEV_FIELD(lx, ly))
      {
	int element = Tile[lx][ly];

	if (element == EL_EMPTY || element == EL_EXPLODING_TRANSP)
	{
	  pixel = 0;
	}
	else if (IS_WALL(element) || IS_WALL_CHANGING(element))
	{
	  int pos = dy / MINI_TILEY * 2 + dx / MINI_TILEX;

	  pixel = ((element & (1 << pos)) ? 1 : 0);
	}
	else
	{
	  int pos = getMaskFromElement(element);

	  pixel = getPixelFromMask(pos, dx, dy);
	}
      }
      else
      {
	// check if laser is still inside visible playfield area
	pixel = (cSX + px < REAL_SX || cSX + px >= REAL_SX + FULL_SXSIZE ||
		 cSY + py < REAL_SY || cSY + py >= REAL_SY + FULL_SYSIZE);
      }

      if ((Sign[laser.current_angle] & (1 << i)) && pixel)
	hit_mask |= (1 << i);
    }

    if (hit_mask == HIT_MASK_NO_HIT)
    {
      // hit nothing -- go on with another step
      LX += XS;
      LY += YS;
    }
  }

  return hit_mask;
}

static void DeactivateLaserTargetElement(void)
{
  if (laser.dest_element_last == EL_BOMB_ACTIVE ||
      laser.dest_element_last == EL_MINE_ACTIVE ||
      laser.dest_element_last == EL_GRAY_BALL_ACTIVE ||
      laser.dest_element_last == EL_GRAY_BALL_OPENING)
  {
    int x = laser.dest_element_last_x;
    int y = laser.dest_element_last_y;
    int element = laser.dest_element_last;

    if (Tile[x][y] == element)
      Tile[x][y] = (element == EL_BOMB_ACTIVE ? EL_BOMB :
		    element == EL_MINE_ACTIVE ? EL_MINE : EL_GRAY_BALL);

    if (Tile[x][y] == EL_GRAY_BALL)
      MovDelay[x][y] = 0;

    laser.dest_element_last = EL_EMPTY;
    laser.dest_element_last_x = -1;
    laser.dest_element_last_y = -1;
  }
}

static void ScanLaser(void)
{
  int element = EL_EMPTY;
  int last_element = EL_EMPTY;
  int end = 0, rf = laser.num_edges;

  // do not scan laser again after the game was lost for whatever reason
  if (game_mm.game_over)
    return;

  // do not scan laser if fuse is off
  if (laser.fuse_off)
    return;

  DeactivateLaserTargetElement();

  laser.overloaded = FALSE;
  laser.stops_inside_element = FALSE;

  DrawLaser(0, DL_LASER_ENABLED);

#if 0
  Debug("game:mm:ScanLaser",
	"Start scanning with LX == %d, LY == %d, XS == %d, YS == %d",
	LX, LY, XS, YS);
#endif

  while (1)
  {
    int hit_mask;

    if (laser.num_edges > MAX_LASER_LEN || laser.num_damages > MAX_LASER_LEN)
    {
      end = 1;
      laser.overloaded = TRUE;

      break;
    }

    hit_mask = ScanPixel();

#if 0
    Debug("game:mm:ScanLaser",
	  "Hit something at LX == %d, LY == %d, XS == %d, YS == %d",
	  LX, LY, XS, YS);
#endif

    // check if laser scan has hit two diagonally adjacent element corners
    boolean diag_1 = ((hit_mask & HIT_MASK_DIAGONAL_1) == HIT_MASK_DIAGONAL_1);
    boolean diag_2 = ((hit_mask & HIT_MASK_DIAGONAL_2) == HIT_MASK_DIAGONAL_2);

    // check if laser scan has crossed element boundaries (not just mini tiles)
    boolean cross_x = (getLevelFromLaserX(LX) != getLevelFromLaserX(LX + 2));
    boolean cross_y = (getLevelFromLaserY(LY) != getLevelFromLaserY(LY + 2));

    if (cross_x || cross_y)
    {
      // hit something at next tile -- check out what it was
      ELX = getLevelFromLaserX(LX + XS);
      ELY = getLevelFromLaserY(LY + YS);
    }
    else
    {
      // hit something at same tile -- check out what it was
      ELX = getLevelFromLaserX(LX);
      ELY = getLevelFromLaserY(LY);
    }

#if 0
    Debug("game:mm:ScanLaser", "hit_mask (1) == '%x' (%d, %d) (%d, %d)",
	  hit_mask, LX, LY, ELX, ELY);
#endif

    if (!IN_LEV_FIELD(ELX, ELY))
    {
      // laser next step position
      int x = cSX + LX + XS;
      int y = cSY + LY + YS;

      // check if next step of laser is still inside visible playfield area
      if (x >= REAL_SX && x < REAL_SX + FULL_SXSIZE &&
	  y >= REAL_SY && y < REAL_SY + FULL_SYSIZE)
      {
	// go on with another step
	LX += XS;
	LY += YS;

	continue;
      }

      element = EL_EMPTY;
      laser.dest_element = element;

      break;
    }

    // handle special case of laser hitting two diagonally adjacent elements
    // (with or without a third corner element behind these two elements)
    if ((diag_1 || diag_2) && cross_x && cross_y)
    {
      // compare the two diagonally adjacent elements
      int xoffset = 2;
      int yoffset = 2 * (diag_1 ? -1 : +1);
      int elx1 = (LX - xoffset) / TILEX;
      int ely1 = (LY + yoffset) / TILEY;
      int elx2 = (LX + xoffset) / TILEX;
      int ely2 = (LY - yoffset) / TILEY;
      int e1 = Tile[elx1][ely1];
      int e2 = Tile[elx2][ely2];
      boolean use_element_1 = FALSE;

      if (IS_WALL_ICE(e1) || IS_WALL_ICE(e2))
      {
	if (IS_WALL_ICE(e1) && IS_WALL_ICE(e2))
	  use_element_1 = (RND(2) ? TRUE : FALSE);
	else if (IS_WALL_ICE(e1))
	  use_element_1 = TRUE;
      }
      else if (IS_WALL_AMOEBA(e1) || IS_WALL_AMOEBA(e2))
      {
	// if both tiles match, we can just select the first one
	if (IS_WALL_AMOEBA(e1))
	  use_element_1 = TRUE;
      }
      else if (IS_ABSORBING_BLOCK(e1) || IS_ABSORBING_BLOCK(e2))
      {
	// if both tiles match, we can just select the first one
	if (IS_ABSORBING_BLOCK(e1))
	  use_element_1 = TRUE;
      }

      ELX = (use_element_1 ? elx1 : elx2);
      ELY = (use_element_1 ? ely1 : ely2);
    }

#if 0
    Debug("game:mm:ScanLaser", "hit_mask (2) == '%x' (%d, %d) (%d, %d)",
	  hit_mask, LX, LY, ELX, ELY);
#endif

    last_element = element;

    element = Tile[ELX][ELY];
    laser.dest_element = element;

#if 0
    Debug("game:mm:ScanLaser",
	  "Hit element %d at (%d, %d) [%d, %d] [%d, %d] [%d]",
	  element, ELX, ELY,
	  LX, LY,
	  LX % TILEX, LY % TILEY,
	  hit_mask);
#endif

#if 0
    if (!IN_LEV_FIELD(ELX, ELY))
      Debug("game:mm:ScanLaser", "WARNING! (1) %d, %d (%d)",
	    ELX, ELY, element);
#endif

    // special case: leaving fixed MM steel grid (upwards) with non-90° angle
    if (element == EL_EMPTY &&
	IS_GRID_STEEL(last_element) &&
	laser.current_angle % 4)		// angle is not 90°
      element = last_element;

    if (element == EL_EMPTY)
    {
      if (!HitOnlyAnEdge(hit_mask))
	break;
    }
    else if (element == EL_FUSE_ON)
    {
      if (HitPolarizer(element, hit_mask))
	break;
    }
    else if (IS_GRID(element) || IS_DF_GRID(element))
    {
      if (HitPolarizer(element, hit_mask))
	break;
    }
    else if (element == EL_BLOCK_STONE || element == EL_BLOCK_WOOD ||
	     element == EL_GATE_STONE || element == EL_GATE_WOOD)
    {
      if (HitBlock(element, hit_mask))
      {
	rf = 1;

	break;
      }
    }
    else if (IS_MCDUFFIN(element))
    {
      if (HitLaserSource(element, hit_mask))
	break;
    }
    else if ((element >= EL_EXIT_CLOSED && element <= EL_EXIT_OPEN) ||
	     IS_RECEIVER(element))
    {
      if (HitLaserDestination(element, hit_mask))
	break;
    }
    else if (IS_WALL(element))
    {
      if (IS_WALL_STEEL(element) || IS_DF_WALL_STEEL(element))
      {
	if (HitReflectingWalls(element, hit_mask))
	  break;
      }
      else
      {
	if (HitAbsorbingWalls(element, hit_mask))
	  break;
      }
    }
    else
    {
      if (HitElement(element, hit_mask))
	break;
    }

    if (rf)
      DrawLaser(rf - 1, DL_LASER_ENABLED);
    rf = laser.num_edges;

    if (!IS_DF_WALL_STEEL(element))
    {
      // only used for scanning DF steel walls; reset for all other elements
      last_LX = 0;
      last_LY = 0;
      last_hit_mask = 0;
    }
  }

#if 0
  if (laser.dest_element != Tile[ELX][ELY])
  {
    Debug("game:mm:ScanLaser",
	  "ALARM: laser.dest_element == %d, Tile[ELX][ELY] == %d",
	  laser.dest_element, Tile[ELX][ELY]);
  }
#endif

  if (!end && !laser.stops_inside_element && !StepBehind())
  {
#if 0
    Debug("game:mm:ScanLaser", "Go one step back");
#endif

    LX -= XS;
    LY -= YS;

    AddLaserEdge(LX, LY);
  }

  if (rf)
    DrawLaser(rf - 1, DL_LASER_ENABLED);

  Ct = CT = FrameCounter;

#if 0
    if (!IN_LEV_FIELD(ELX, ELY))
      Debug("game:mm:ScanLaser", "WARNING! (2) %d, %d", ELX, ELY);
#endif
}

static void ScanLaser_FromLastMirror(void)
{
  int start_pos = (laser.num_damages > 0 ? laser.num_damages - 1 : 0);
  int i;

  for (i = start_pos; i >= 0; i--)
    if (laser.damage[i].is_mirror)
      break;

  int start_edge = (i > 0 ? laser.damage[i].edge - 1 : 0);

  DrawLaser(start_edge, DL_LASER_DISABLED);

  ScanLaser();
}

static void DrawLaserExt(int start_edge, int num_edges, int mode)
{
  int element;
  int elx, ely;

#if 0
  Debug("game:mm:DrawLaserExt", "start_edge, num_edges, mode == %d, %d, %d",
	start_edge, num_edges, mode);
#endif

  if (start_edge < 0)
  {
    Warn("DrawLaserExt: start_edge < 0");

    return;
  }

  if (num_edges < 0)
  {
    Warn("DrawLaserExt: num_edges < 0");

    return;
  }

#if 0
  if (mode == DL_LASER_DISABLED)
  {
    Debug("game:mm:DrawLaserExt", "Delete laser from edge %d", start_edge);
  }
#endif

  // now draw the laser to the backbuffer and (if enabled) to the screen
  DrawLaserLines(&laser.edge[start_edge], num_edges, mode);

  redraw_mask |= REDRAW_FIELD;

  if (mode == DL_LASER_ENABLED)
    return;

  // after the laser was deleted, the "damaged" graphics must be restored
  if (laser.num_damages)
  {
    int damage_start = 0;
    int i;

    // determine the starting edge, from which graphics need to be restored
    if (start_edge > 0)
    {
      for (i = 0; i < laser.num_damages; i++)
      {
	if (laser.damage[i].edge == start_edge + 1)
	{
	  damage_start = i;

	  break;
	}
      }
    }

    // restore graphics from this starting edge to the end of damage list
    for (i = damage_start; i < laser.num_damages; i++)
    {
      int lx = laser.damage[i].x;
      int ly = laser.damage[i].y;
      int element = Tile[lx][ly];

      if (Hit[lx][ly] == laser.damage[i].edge)
	if (!((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
	       i == damage_start))
	  Hit[lx][ly] = 0;
      if (Box[lx][ly] == laser.damage[i].edge)
	Box[lx][ly] = 0;

      if (IS_DRAWABLE(element))
	DrawField_MM(lx, ly);
    }

    elx = laser.damage[damage_start].x;
    ely = laser.damage[damage_start].y;
    element = Tile[elx][ely];

#if 0
    if (IS_BEAMER(element))
    {
      int i;

      for (i = 0; i < laser.num_beamers; i++)
	Debug("game:mm:DrawLaserExt", "-> %d", laser.beamer_edge[i]);

      Debug("game:mm:DrawLaserExt", "IS_BEAMER: [%d]: Hit[%d][%d] == %d [%d]",
	    mode, elx, ely, Hit[elx][ely], start_edge);
      Debug("game:mm:DrawLaserExt", "IS_BEAMER: %d / %d",
	    get_element_angle(element), laser.damage[damage_start].angle);
    }
#endif

    if ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
	laser.num_beamers > 0 &&
	start_edge == laser.beamer_edge[laser.num_beamers - 1])
    {
      // element is outgoing beamer
      laser.num_damages = damage_start + 1;

      if (IS_BEAMER(element))
	laser.current_angle = get_element_angle(element);
    }
    else
    {
      // element is incoming beamer or other element
      laser.num_damages = damage_start;
      laser.current_angle = laser.damage[laser.num_damages].angle;
    }
  }
  else
  {
    // no damages but McDuffin himself (who needs to be redrawn anyway)

    elx = laser.start_edge.x;
    ely = laser.start_edge.y;
    element = Tile[elx][ely];
  }

  laser.num_edges = start_edge + 1;
  if (start_edge == 0)
    laser.current_angle = laser.start_angle;

  LX = laser.edge[start_edge].x - cSX2;
  LY = laser.edge[start_edge].y - cSY2;
  XS = 2 * Step[laser.current_angle].x;
  YS = 2 * Step[laser.current_angle].y;

#if 0
  Debug("game:mm:DrawLaserExt", "Set (LX, LY) to (%d, %d) [%d]",
	LX, LY, element);
#endif

  if (start_edge > 0)
  {
    if (IS_BEAMER(element) ||
	IS_FIBRE_OPTIC(element) ||
	IS_PACMAN(element) ||
	IS_POLAR(element) ||
	IS_POLAR_CROSS(element) ||
	element == EL_FUSE_ON)
    {
      int step_size;

#if 0
      Debug("game:mm:DrawLaserExt", "element == %d", element);
#endif

      if (IS_22_5_ANGLE(laser.current_angle))	// neither 90° nor 45° angle
	step_size = ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) ? 4 : 3);
      else
	step_size = 8;

      if (IS_POLAR(element) || IS_POLAR_CROSS(element) ||
	  ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
	   (laser.num_beamers == 0 ||
	    start_edge != laser.beamer_edge[laser.num_beamers - 1])))
      {
	// element is incoming beamer or other element
	step_size = -step_size;
	laser.num_edges--;
      }

#if 0
      if (IS_BEAMER(element))
	Debug("game:mm:DrawLaserExt",
	      "start_edge == %d, laser.beamer_edge == %d",
	      start_edge, laser.beamer_edge);
#endif

      LX += step_size * XS;
      LY += step_size * YS;
    }
    else if (element != EL_EMPTY)
    {
      LX -= 3 * XS;
      LY -= 3 * YS;
      laser.num_edges--;
    }
  }

#if 0
  Debug("game:mm:DrawLaserExt", "Finally: (LX, LY) to (%d, %d) [%d]",
	LX, LY, element);
#endif
}

void DrawLaser(int start_edge, int mode)
{
  // do not draw laser if fuse is off
  if (laser.fuse_off && mode == DL_LASER_ENABLED)
    return;

  if (mode == DL_LASER_DISABLED)
    DeactivateLaserTargetElement();

  if (laser.num_edges - start_edge < 0)
  {
    Warn("DrawLaser: laser.num_edges - start_edge < 0");

    return;
  }

  // check if laser is interrupted by beamer element
  if (laser.num_beamers > 0 &&
      start_edge < laser.beamer_edge[laser.num_beamers - 1])
  {
    if (mode == DL_LASER_ENABLED)
    {
      int i;
      int tmp_start_edge = start_edge;

      // draw laser segments forward from the start to the last beamer
      for (i = 0; i < laser.num_beamers; i++)
      {
	int tmp_num_edges = laser.beamer_edge[i] - tmp_start_edge;

	if (tmp_num_edges <= 0)
	  continue;

#if 0
	Debug("game:mm:DrawLaser", "DL_LASER_ENABLED: i==%d: %d, %d",
	      i, laser.beamer_edge[i], tmp_start_edge);
#endif

	DrawLaserExt(tmp_start_edge, tmp_num_edges, DL_LASER_ENABLED);

	tmp_start_edge = laser.beamer_edge[i];
      }

      // draw last segment from last beamer to the end
      DrawLaserExt(tmp_start_edge, laser.num_edges - tmp_start_edge,
		   DL_LASER_ENABLED);
    }
    else
    {
      int i;
      int last_num_edges = laser.num_edges;
      int num_beamers = laser.num_beamers;

      // delete laser segments backward from the end to the first beamer
      for (i = num_beamers - 1; i >= 0; i--)
      {
	int tmp_num_edges = last_num_edges - laser.beamer_edge[i];

	if (laser.beamer_edge[i] - start_edge <= 0)
	  break;

	DrawLaserExt(laser.beamer_edge[i], tmp_num_edges, DL_LASER_DISABLED);

	last_num_edges = laser.beamer_edge[i];
	laser.num_beamers--;
      }

#if 0
      if (last_num_edges - start_edge <= 0)
	Debug("game:mm:DrawLaser", "DL_LASER_DISABLED: %d, %d",
	      last_num_edges, start_edge);
#endif

      // special case when rotating first beamer: delete laser edge on beamer
      // (but do not start scanning on previous edge to prevent mirror sound)
      if (last_num_edges - start_edge == 1 && start_edge > 0)
	DrawLaserLines(&laser.edge[start_edge - 1], 2, DL_LASER_DISABLED);

      // delete first segment from start to the first beamer
      DrawLaserExt(start_edge, last_num_edges - start_edge, DL_LASER_DISABLED);
    }
  }
  else
  {
    DrawLaserExt(start_edge, laser.num_edges - start_edge, mode);
  }

  game_mm.laser_enabled = mode;
}

void DrawLaser_MM(void)
{
  DrawLaser(0, game_mm.laser_enabled);
}

static boolean HitElement(int element, int hit_mask)
{
  if (IS_DF_SLOPE(element))
  {
    // check if laser scan has crossed element boundaries (not just mini tiles)
    boolean cross_x = (getLevelFromLaserX(LX) != getLevelFromLaserX(LX + 2));
    boolean cross_y = (getLevelFromLaserY(LY) != getLevelFromLaserY(LY + 2));
    int element_angle = get_element_angle(element);
    int mirrored_angle = get_mirrored_angle(laser.current_angle, element_angle);
    int opposite_angle = get_opposite_angle(laser.current_angle);

    // check if wall (horizontal or vertical) side of slope was hit
    if (hit_mask == HIT_MASK_LEFT ||
	hit_mask == HIT_MASK_RIGHT ||
	hit_mask == HIT_MASK_TOP ||
	hit_mask == HIT_MASK_BOTTOM)
    {
      boolean hit_slope_corner_in_laser_direction =
	((hit_mask == HIT_MASK_LEFT   && (element == EL_DF_SLOPE_01 ||
					  element == EL_DF_SLOPE_02)) ||
	 (hit_mask == HIT_MASK_RIGHT  && (element == EL_DF_SLOPE_00 ||
					  element == EL_DF_SLOPE_03)) ||
	 (hit_mask == HIT_MASK_TOP    && (element == EL_DF_SLOPE_02 ||
					  element == EL_DF_SLOPE_03)) ||
	 (hit_mask == HIT_MASK_BOTTOM && (element == EL_DF_SLOPE_00 ||
					  element == EL_DF_SLOPE_01)));

      boolean hit_slope_corner_in_laser_direction_double_checked =
	(cross_x && cross_y &&
	 laser.current_angle == mirrored_angle &&
	 hit_slope_corner_in_laser_direction);

      // check special case of laser hitting the corner of a slope and another
      // element (either wall or another slope), following the diagonal side
      // of the slope which has the same angle as the direction of the laser
      if (!hit_slope_corner_in_laser_direction_double_checked)
	return HitReflectingWalls(element, hit_mask);
    }

    // check if an edge was hit while crossing element borders
    if (cross_x && cross_y && get_number_of_bits(hit_mask) == 1)
    {
      // check both sides of potentially diagonal side of slope
      int dx1 = (LX + XS) % TILEX;
      int dy1 = (LY + YS) % TILEY;
      int dx2 = (LX + XS + 2) % TILEX;
      int dy2 = (LY + YS + 2) % TILEY;
      int pos = getMaskFromElement(element);

      // check if we are entering empty space area after hitting edge
      if (!getPixelFromMask(pos, dx1, dy1) &&
	  !getPixelFromMask(pos, dx2, dy2))
      {
	// we already know that we hit an edge, but use this function to go on
	if (HitOnlyAnEdge(hit_mask))
	  return FALSE;
      }
    }

    // check if laser is reflected by slope by 180°
    if (mirrored_angle == opposite_angle)
    {
      AddDamagedField(LX / TILEX, LY / TILEY);

      laser.overloaded = TRUE;

      return TRUE;
    }
  }
  else
  {
    if (HitOnlyAnEdge(hit_mask))
      return FALSE;
  }

  if (IS_MOVING(ELX, ELY) || IS_BLOCKED(ELX, ELY))
    element = MovingOrBlocked2Element_MM(ELX, ELY);

#if 0
  Debug("game:mm:HitElement", "(1): element == %d", element);
#endif

#if 0
  if ((ELX * TILEX + 14 - LX) * YS == (ELY * TILEY + 14 - LY) * XS)
    Debug("game:mm:HitElement", "(%d): EXACT MATCH @ (%d, %d)",
	  element, ELX, ELY);
  else
    Debug("game:mm:HitElement", "(%d): FUZZY MATCH @ (%d, %d)",
	  element, ELX, ELY);
#endif

  AddDamagedField(ELX, ELY);

  boolean through_center = ((ELX * TILEX + 14 - LX) * YS ==
			    (ELY * TILEY + 14 - LY) * XS);

  // this is more precise: check if laser would go through the center
  if (!IS_DF_SLOPE(element) && !through_center)
  {
    int skip_count = 0;

    // prevent cutting through laser emitter with laser beam
    if (IS_LASER(element))
      return TRUE;

    // skip the whole element before continuing the scan
    do
    {
      LX += XS;
      LY += YS;

      skip_count++;
    }
    while (ELX == LX/TILEX && ELY == LY/TILEY && LX > 0 && LY > 0);

    if ((LX/TILEX > ELX || LY/TILEY > ELY) && skip_count > 1)
    {
      /* skipping scan positions to the right and down skips one scan
	 position too much, because this is only the top left scan position
	 of totally four scan positions (plus one to the right, one to the
	 bottom and one to the bottom right) */
      /* ... but only roll back scan position if more than one step done */

      LX -= XS;
      LY -= YS;
    }

    return FALSE;
  }

#if 0
  Debug("game:mm:HitElement", "(2): element == %d", element);
#endif

  if (LX + 5 * XS < 0 ||
      LY + 5 * YS < 0)
  {
    LX += 2 * XS;
    LY += 2 * YS;

    return FALSE;
  }

#if 0
  Debug("game:mm:HitElement", "(3): element == %d", element);
#endif

  if (IS_POLAR(element) &&
      ((element - EL_POLAR_START) % 2 ||
       (element - EL_POLAR_START) / 2 != laser.current_angle % 8))
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    laser.num_damages--;

    return TRUE;
  }

  if (IS_POLAR_CROSS(element) &&
      (element - EL_POLAR_CROSS_START) != laser.current_angle % 4)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    laser.num_damages--;

    return TRUE;
  }

  if (IS_DF_SLOPE(element) && !through_center)
  {
    int correction = 2;

    if (hit_mask == HIT_MASK_ALL)
    {
      // laser already inside slope -- go back half step
      LX -= XS / 2;
      LY -= YS / 2;

      correction = 1;
    }

    AddLaserEdge(LX, LY);

    LX -= (ABS(XS) < ABS(YS) ? correction * SIGN(XS) : 0);
    LY -= (ABS(YS) < ABS(XS) ? correction * SIGN(YS) : 0);
  }
  else if (!IS_BEAMER(element) &&
	   !IS_FIBRE_OPTIC(element) &&
	   !IS_GRID_WOOD(element) &&
	   element != EL_FUEL_EMPTY)
  {
#if 0
    if ((ELX * TILEX + 14 - LX) * YS == (ELY * TILEY + 14 - LY) * XS)
      Debug("game:mm:HitElement", "EXACT MATCH @ (%d, %d)", ELX, ELY);
    else
      Debug("game:mm:HitElement", "FUZZY MATCH @ (%d, %d)", ELX, ELY);
#endif

    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;

    AddLaserEdge(LX, LY);
  }

  if (IS_MIRROR(element) ||
      IS_MIRROR_FIXED(element) ||
      IS_POLAR(element) ||
      IS_POLAR_CROSS(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element) ||
      IS_DF_MIRROR_FIXED(element) ||
      IS_DF_SLOPE(element) ||
      element == EL_PRISM ||
      element == EL_REFRACTOR)
  {
    int current_angle = laser.current_angle;
    int step_size;

    laser.num_damages--;

    AddDamagedField(ELX, ELY);

    laser.damage[laser.num_damages - 1].is_mirror = TRUE;

    if (!Hit[ELX][ELY])
      Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

    if (IS_MIRROR(element) ||
	IS_MIRROR_FIXED(element) ||
	IS_DF_MIRROR(element) ||
	IS_DF_MIRROR_AUTO(element) ||
	IS_DF_MIRROR_FIXED(element) ||
	IS_DF_SLOPE(element))
      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       get_element_angle(element));

    if (element == EL_PRISM || element == EL_REFRACTOR)
      laser.current_angle = RND(16);

    XS = 2 * Step[laser.current_angle].x;
    YS = 2 * Step[laser.current_angle].y;

    if (through_center)
    {
      // start from center position for all game elements but slope
      if (!IS_22_5_ANGLE(laser.current_angle))	// 90° or 45° angle
	step_size = 8;
      else
	step_size = 4;

      LX += step_size * XS;
      LY += step_size * YS;
    }
    else
    {
      // advance laser position until reaching the next tile (slopes)
      while (LX / TILEX == ELX && (LX + 2) / TILEX == ELX &&
	     LY / TILEY == ELY && (LY + 2) / TILEY == ELY)
      {
	LX += XS;
	LY += YS;
      }
    }

    // draw sparkles on mirror
    if ((IS_MIRROR(element) ||
	 IS_MIRROR_FIXED(element) ||
	 element == EL_PRISM) &&
	current_angle != laser.current_angle)
    {
      MovDelay[ELX][ELY] = 11;		// start animation
    }

    if ((!IS_POLAR(element) && !IS_POLAR_CROSS(element)) &&
	current_angle != laser.current_angle)
      PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    laser.overloaded =
      (get_opposite_angle(laser.current_angle) ==
       laser.damage[laser.num_damages - 1].angle ? TRUE : FALSE);

    if (IS_DF_SLOPE(element))
    {
      // handle special cases for slope element

      if (IS_45_ANGLE(laser.current_angle))
      {
	int elx, ely;

	elx = getLevelFromLaserX(LX + XS);
	ely = getLevelFromLaserY(LY + YS);

	if (IN_LEV_FIELD(elx, ely))
	{
	  int element_next = Tile[elx][ely];

	  // check if slope is followed by slope with opposite orientation
	  if (IS_DF_SLOPE(element_next) && ABS(element - element_next) == 2)
	    laser.overloaded = TRUE;
	}

	int nr = element - EL_DF_SLOPE_START;
	int dx = (nr == 0 ? (XS > 0 ? TILEX - 1 : -1) :
		  nr == 1 ? (XS > 0 ? TILEX     :  0) :
		  nr == 2 ? (XS > 0 ? TILEX     :  0) :
		  nr == 3 ? (XS > 0 ? TILEX - 1 : -1) : 0);
	int dy = (nr == 0 ? (YS > 0 ? TILEY - 1 : -1) :
		  nr == 1 ? (YS > 0 ? TILEY - 1 : -1) :
		  nr == 2 ? (YS > 0 ? TILEY     :  0) :
		  nr == 3 ? (YS > 0 ? TILEY     :  0) : 0);

	int px = ELX * TILEX + dx;
	int py = ELY * TILEY + dy;

	dx = px % TILEX;
	dy = py % TILEY;

	elx = getLevelFromLaserX(px);
	ely = getLevelFromLaserY(py);

	if (IN_LEV_FIELD(elx, ely))
	{
	  int element_side = Tile[elx][ely];

	  // check if end of slope is blocked by other element
	  if (IS_WALL(element_side) || IS_WALL_CHANGING(element_side))
	  {
	    int pos = dy / MINI_TILEY * 2 + dx / MINI_TILEX;

	    if (element & (1 << pos))
	      laser.overloaded = TRUE;
	  }
	  else
	  {
	    int pos = getMaskFromElement(element_side);

	    if (getPixelFromMask(pos, dx, dy))
	      laser.overloaded = TRUE;
	  }
	}
      }
    }

    return (laser.overloaded ? TRUE : FALSE);
  }

  if (element == EL_FUEL_FULL)
  {
    laser.stops_inside_element = TRUE;

    return TRUE;
  }

  if (element == EL_BOMB || element == EL_MINE || element == EL_GRAY_BALL)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    Tile[ELX][ELY] = (element == EL_BOMB ? EL_BOMB_ACTIVE :
		      element == EL_MINE ? EL_MINE_ACTIVE :
		      EL_GRAY_BALL_ACTIVE);

    GfxFrame[ELX][ELY] = 0;		// restart animation

    laser.dest_element_last = Tile[ELX][ELY];
    laser.dest_element_last_x = ELX;
    laser.dest_element_last_y = ELY;

    if (element == EL_MINE)
      laser.overloaded = TRUE;
  }

  if (element == EL_KETTLE ||
      element == EL_CELL ||
      element == EL_KEY ||
      element == EL_LIGHTBALL ||
      element == EL_PACMAN ||
      IS_PACMAN(element) ||
      IS_ENVELOPE(element))
  {
    if (!IS_PACMAN(element) &&
	!IS_ENVELOPE(element))
      Bang_MM(ELX, ELY);

    if (element == EL_PACMAN)
      Bang_MM(ELX, ELY);

    if (element == EL_KETTLE || element == EL_CELL)
    {
      if (game_mm.kettles_still_needed > 0)
	game_mm.kettles_still_needed--;

      game.snapshot.collected_item = TRUE;

      if (game_mm.kettles_still_needed == 0)
      {
	CheckExitMM();

	DrawLaser(0, DL_LASER_ENABLED);
      }
    }
    else if (element == EL_KEY)
    {
      game_mm.num_keys++;
    }
    else if (IS_PACMAN(element))
    {
      DeletePacMan(ELX, ELY);
    }
    else if (IS_ENVELOPE(element))
    {
      Tile[ELX][ELY] = EL_ENVELOPE_1_OPENING + ENVELOPE_NR(Tile[ELX][ELY]);
    }

    RaiseScoreElement_MM(element);

    return FALSE;
  }

  if (element == EL_LIGHTBULB_OFF || element == EL_LIGHTBULB_ON)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    DrawLaser(0, DL_LASER_ENABLED);

    if (Tile[ELX][ELY] == EL_LIGHTBULB_OFF)
    {
      Tile[ELX][ELY] = EL_LIGHTBULB_ON;
      game_mm.lights_still_needed--;
    }
    else
    {
      Tile[ELX][ELY] = EL_LIGHTBULB_OFF;
      game_mm.lights_still_needed++;
    }

    DrawField_MM(ELX, ELY);
    DrawLaser(0, DL_LASER_ENABLED);

    /*
    BackToFront();
    */
    laser.stops_inside_element = TRUE;

    return TRUE;
  }

#if 0
  Debug("game:mm:HitElement", "(4): element == %d", element);
#endif

  if ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
      laser.num_beamers < MAX_NUM_BEAMERS &&
      laser.beamer[BEAMER_NR(element)][1].num)
  {
    int beamer_angle = get_element_angle(element);
    int beamer_nr = BEAMER_NR(element);
    int step_size;

#if 0
    Debug("game:mm:HitElement", "(BEAMER): element == %d", element);
#endif

    laser.num_damages--;

    if (IS_FIBRE_OPTIC(element) ||
	laser.current_angle == get_opposite_angle(beamer_angle))
    {
      int pos;

      LX = ELX * TILEX + 14;
      LY = ELY * TILEY + 14;

      AddLaserEdge(LX, LY);
      AddDamagedField(ELX, ELY);

      laser.damage[laser.num_damages - 1].is_mirror = TRUE;

      if (!Hit[ELX][ELY])
	Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

      pos = (ELX == laser.beamer[beamer_nr][0].x &&
	     ELY == laser.beamer[beamer_nr][0].y ? 1 : 0);
      ELX = laser.beamer[beamer_nr][pos].x;
      ELY = laser.beamer[beamer_nr][pos].y;
      LX = ELX * TILEX + 14;
      LY = ELY * TILEY + 14;

      if (IS_BEAMER(element))
      {
	laser.current_angle = get_element_angle(Tile[ELX][ELY]);
	XS = 2 * Step[laser.current_angle].x;
	YS = 2 * Step[laser.current_angle].y;
      }

      laser.beamer_edge[laser.num_beamers] = laser.num_edges;

      AddLaserEdge(LX, LY);
      AddDamagedField(ELX, ELY);

      laser.damage[laser.num_damages - 1].is_mirror = TRUE;

      if (!Hit[ELX][ELY])
	Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

      if (laser.current_angle == (laser.current_angle >> 1) << 1)
	step_size = 8;
      else
	step_size = 4;

      LX += step_size * XS;
      LY += step_size * YS;

      laser.num_beamers++;

      return FALSE;
    }
  }

  return TRUE;
}

static boolean HitOnlyAnEdge(int hit_mask)
{
  // check if the laser hit only the edge of an element and, if so, go on

#if 0
  Debug("game:mm:HitOnlyAnEdge", "LX, LY, hit_mask == %d, %d, %d",
	LX, LY, hit_mask);
#endif

  if ((hit_mask == HIT_MASK_TOPLEFT ||
       hit_mask == HIT_MASK_TOPRIGHT ||
       hit_mask == HIT_MASK_BOTTOMLEFT ||
       hit_mask == HIT_MASK_BOTTOMRIGHT) &&
      laser.current_angle % 4)			// angle is not 90°
  {
    int dx, dy;

    if (hit_mask == HIT_MASK_TOPLEFT)
    {
      dx = -1;
      dy = -1;
    }
    else if (hit_mask == HIT_MASK_TOPRIGHT)
    {
      dx = +1;
      dy = -1;
    }
    else if (hit_mask == HIT_MASK_BOTTOMLEFT)
    {
      dx = -1;
      dy = +1;
    }
    else // (hit_mask == HIT_MASK_BOTTOMRIGHT)
    {
      dx = +1;
      dy = +1;
    }

    AddDamagedField((LX + 2 * dx) / TILEX, (LY + 2 * dy) / TILEY);

    LX += XS;
    LY += YS;

#if 0
    Debug("game:mm:HitOnlyAnEdge", "[HitOnlyAnEdge() == TRUE]");
#endif

    return TRUE;
  }

#if 0
  Debug("game:mm:HitOnlyAnEdge", "[HitOnlyAnEdge() == FALSE]");
#endif

  return FALSE;
}

static boolean HitPolarizer(int element, int hit_mask)
{
  if (HitOnlyAnEdge(hit_mask))
    return FALSE;

  if (IS_DF_GRID(element))
  {
    int grid_angle = get_element_angle(element);

#if 0
    Debug("game:mm:HitPolarizer", "angle: grid == %d, laser == %d",
	  grid_angle, laser.current_angle);
#endif

    AddLaserEdge(LX, LY);
    AddDamagedField(ELX, ELY);

    if (!Hit[ELX][ELY])
      Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

    if (laser.current_angle == grid_angle ||
	laser.current_angle == get_opposite_angle(grid_angle))
    {
      // skip the whole element before continuing the scan
      do
      {
	LX += XS;
	LY += YS;
      }
      while (ELX == LX/TILEX && ELY == LY/TILEY && LX > 0 && LY > 0);

      if (LX/TILEX > ELX || LY/TILEY > ELY)
      {
	/* skipping scan positions to the right and down skips one scan
	   position too much, because this is only the top left scan position
	   of totally four scan positions (plus one to the right, one to the
	   bottom and one to the bottom right) */

	LX -= XS;
	LY -= YS;
      }

      AddLaserEdge(LX, LY);

      LX += XS;
      LY += YS;

#if 0
      Debug("game:mm:HitPolarizer", "LX, LY == %d, %d [%d, %d] [%d, %d]",
	    LX, LY,
	    LX / TILEX, LY / TILEY,
	    LX % TILEX, LY % TILEY);
#endif

      return FALSE;
    }
    else if (IS_GRID_STEEL_FIXED(element) || IS_GRID_STEEL_AUTO(element))
    {
      return HitReflectingWalls(element, hit_mask);
    }
    else
    {
      return HitAbsorbingWalls(element, hit_mask);
    }
  }
  else if (IS_GRID_STEEL(element))
  {
    // may be required if graphics for steel grid redefined
    AddDamagedField(ELX, ELY);

    return HitReflectingWalls(element, hit_mask);
  }
  else	// IS_GRID_WOOD
  {
    // may be required if graphics for wooden grid redefined
    AddDamagedField(ELX, ELY);

    return HitAbsorbingWalls(element, hit_mask);
  }

  return TRUE;
}

static boolean HitBlock(int element, int hit_mask)
{
  boolean check = FALSE;

  if ((element == EL_GATE_STONE || element == EL_GATE_WOOD) &&
      game_mm.num_keys == 0)
    check = TRUE;

  if (element == EL_BLOCK_STONE || element == EL_BLOCK_WOOD)
  {
    int i, x, y;
    int ex = ELX * TILEX + 14;
    int ey = ELY * TILEY + 14;

    check = TRUE;

    for (i = 1; i < 32; i++)
    {
      x = LX + i * XS;
      y = LY + i * YS;

      if ((x == ex || x == ex + 1) && (y == ey || y == ey + 1))
	check = FALSE;
    }
  }

  if (check && (element == EL_BLOCK_WOOD || element == EL_GATE_WOOD))
    return HitAbsorbingWalls(element, hit_mask);

  if (check)
  {
    AddLaserEdge(LX - XS, LY - YS);
    AddDamagedField(ELX, ELY);

    if (!Box[ELX][ELY])
      Box[ELX][ELY] = laser.num_edges;

    return HitReflectingWalls(element, hit_mask);
  }

  if (element == EL_GATE_STONE || element == EL_GATE_WOOD)
  {
    int xs = XS / 2, ys = YS / 2;

    if ((hit_mask & HIT_MASK_DIAGONAL_1) == HIT_MASK_DIAGONAL_1 ||
	(hit_mask & HIT_MASK_DIAGONAL_2) == HIT_MASK_DIAGONAL_2)
    {
      laser.overloaded = (element == EL_GATE_STONE);

      return TRUE;
    }

    if (ABS(xs) == 1 && ABS(ys) == 1 &&
	(hit_mask == HIT_MASK_TOP ||
	 hit_mask == HIT_MASK_LEFT ||
	 hit_mask == HIT_MASK_RIGHT ||
	 hit_mask == HIT_MASK_BOTTOM))
      AddDamagedField(ELX - xs * (hit_mask == HIT_MASK_TOP ||
				  hit_mask == HIT_MASK_BOTTOM),
		      ELY - ys * (hit_mask == HIT_MASK_LEFT ||
				  hit_mask == HIT_MASK_RIGHT));
    AddLaserEdge(LX, LY);

    Bang_MM(ELX, ELY);

    game_mm.num_keys--;

    if (element == EL_GATE_STONE && Box[ELX][ELY])
    {
      DrawLaser(Box[ELX][ELY] - 1, DL_LASER_DISABLED);
      /*
      BackToFront();
      */
      ScanLaser();

      return TRUE;
    }

    return FALSE;
  }

  if (element == EL_BLOCK_STONE || element == EL_BLOCK_WOOD)
  {
    int xs = XS / 2, ys = YS / 2;

    if ((hit_mask & HIT_MASK_DIAGONAL_1) == HIT_MASK_DIAGONAL_1 ||
	(hit_mask & HIT_MASK_DIAGONAL_2) == HIT_MASK_DIAGONAL_2)
    {
      laser.overloaded = (element == EL_BLOCK_STONE);

      return TRUE;
    }

    if (ABS(xs) == 1 && ABS(ys) == 1 &&
	(hit_mask == HIT_MASK_TOP ||
	 hit_mask == HIT_MASK_LEFT ||
	 hit_mask == HIT_MASK_RIGHT ||
	 hit_mask == HIT_MASK_BOTTOM))
      AddDamagedField(ELX - xs * (hit_mask == HIT_MASK_TOP ||
				  hit_mask == HIT_MASK_BOTTOM),
		      ELY - ys * (hit_mask == HIT_MASK_LEFT ||
				  hit_mask == HIT_MASK_RIGHT));
    AddDamagedField(ELX, ELY);

    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;

    AddLaserEdge(LX, LY);

    laser.stops_inside_element = TRUE;

    return TRUE;
  }

  return TRUE;
}

static boolean HitLaserSource(int element, int hit_mask)
{
  if (HitOnlyAnEdge(hit_mask))
    return FALSE;

  PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

  laser.overloaded = TRUE;

  return TRUE;
}

static boolean HitLaserDestination(int element, int hit_mask)
{
  if (HitOnlyAnEdge(hit_mask))
    return FALSE;

  if (element != EL_EXIT_OPEN &&
      !(IS_RECEIVER(element) &&
	game_mm.kettles_still_needed == 0 &&
	laser.current_angle == get_opposite_angle(get_element_angle(element))))
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    return TRUE;
  }

  if (IS_RECEIVER(element) ||
      (IS_22_5_ANGLE(laser.current_angle) &&
       (ELX != (LX + 6 * XS) / TILEX ||
	ELY != (LY + 6 * YS) / TILEY ||
	LX + 6 * XS < 0 ||
	LY + 6 * YS < 0)))
  {
    LX -= XS;
    LY -= YS;
  }
  else
  {
    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;

    laser.stops_inside_element = TRUE;
  }

  AddLaserEdge(LX, LY);
  AddDamagedField(ELX, ELY);

  if (game_mm.lights_still_needed == 0)
  {
    game_mm.level_solved = TRUE;

    SetTileCursorActive(FALSE);
  }

  return TRUE;
}

static boolean HitReflectingWalls(int element, int hit_mask)
{
  // check if laser hits side of a wall with an angle that is not 90°
  if (!IS_90_ANGLE(laser.current_angle) && (hit_mask == HIT_MASK_TOP ||
					    hit_mask == HIT_MASK_LEFT ||
					    hit_mask == HIT_MASK_RIGHT ||
					    hit_mask == HIT_MASK_BOTTOM))
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    LX -= XS;
    LY -= YS;

    if (!IS_DF_GRID(element))
      AddLaserEdge(LX, LY);

    // check if laser hits wall with an angle of 45°
    if (!IS_22_5_ANGLE(laser.current_angle))
    {
      if (hit_mask == HIT_MASK_TOP || hit_mask == HIT_MASK_BOTTOM)
      {
	LX += 2 * XS;
	laser.current_angle = get_mirrored_angle(laser.current_angle,
						 ANG_MIRROR_0);
      }
      else	// hit_mask == HIT_MASK_LEFT || hit_mask == HIT_MASK_RIGHT
      {
	LY += 2 * YS;
	laser.current_angle = get_mirrored_angle(laser.current_angle,
						 ANG_MIRROR_90);
      }

      AddLaserEdge(LX, LY);

      XS = 2 * Step[laser.current_angle].x;
      YS = 2 * Step[laser.current_angle].y;

      return FALSE;
    }
    else if (hit_mask == HIT_MASK_TOP || hit_mask == HIT_MASK_BOTTOM)
    {
      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       ANG_MIRROR_0);
      if (ABS(XS) == 4)
      {
	LX += 2 * XS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }
      else
      {
	LX += XS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY + YS / 2);

	LX += XS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }

      YS = 2 * Step[laser.current_angle].y;

      return FALSE;
    }
    else	// hit_mask == HIT_MASK_LEFT || hit_mask == HIT_MASK_RIGHT
    {
      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       ANG_MIRROR_90);
      if (ABS(YS) == 4)
      {
	LY += 2 * YS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }
      else
      {
	LY += YS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX + XS / 2, LY);

	LY += YS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }

      XS = 2 * Step[laser.current_angle].x;

      return FALSE;
    }
  }

  // reflection at the edge of reflecting DF style wall
  if (IS_DF_WALL_STEEL(element) && IS_22_5_ANGLE(laser.current_angle))
  {
    if (((laser.current_angle == 1 || laser.current_angle == 3) &&
	 hit_mask == HIT_MASK_TOPRIGHT) ||
	((laser.current_angle == 5 || laser.current_angle == 7) &&
	 hit_mask == HIT_MASK_TOPLEFT) ||
	((laser.current_angle == 9 || laser.current_angle == 11) &&
	 hit_mask == HIT_MASK_BOTTOMLEFT) ||
	((laser.current_angle == 13 || laser.current_angle == 15) &&
	 hit_mask == HIT_MASK_BOTTOMRIGHT))
    {
      int mirror_angle =
	(hit_mask == HIT_MASK_TOPRIGHT || hit_mask == HIT_MASK_BOTTOMLEFT ?
	 ANG_MIRROR_135 : ANG_MIRROR_45);

      PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

      AddDamagedField(ELX, ELY);
      AddLaserEdge(LX, LY);

      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       mirror_angle);
      XS = 8 / -XS;
      YS = 8 / -YS;

      LX += XS;
      LY += YS;

      AddLaserEdge(LX, LY);

      return FALSE;
    }
  }

  // reflection inside an edge of reflecting DF style wall
  if (IS_DF_WALL_STEEL(element) && IS_22_5_ANGLE(laser.current_angle))
  {
    if (((laser.current_angle == 1 || laser.current_angle == 3) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_BOTTOMLEFT)) ||
	((laser.current_angle == 5 || laser.current_angle == 7) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_BOTTOMRIGHT)) ||
	((laser.current_angle == 9 || laser.current_angle == 11) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_TOPRIGHT)) ||
	((laser.current_angle == 13 || laser.current_angle == 15) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_TOPLEFT)))
    {
      int mirror_angle =
	(hit_mask == (HIT_MASK_ALL ^ HIT_MASK_BOTTOMLEFT) ||
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_TOPRIGHT) ?
	 ANG_MIRROR_135 : ANG_MIRROR_45);

      PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

      /*
      AddDamagedField(ELX, ELY);
      */

      AddLaserEdge(LX - XS, LY - YS);
      AddLaserEdge(LX - XS + (ABS(XS) == 4 ? XS/2 : 0),
		   LY - YS + (ABS(YS) == 4 ? YS/2 : 0));

      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       mirror_angle);
      XS = 8 / -XS;
      YS = 8 / -YS;

      LX += XS;
      LY += YS;

      AddLaserEdge(LX, LY);

      return FALSE;
    }
  }

  // check if laser hits DF style wall with an angle of 90°
  if (IS_DF_WALL(element) && IS_90_ANGLE(laser.current_angle))
  {
    if ((IS_HORIZ_ANGLE(laser.current_angle) &&
	 (!(hit_mask & HIT_MASK_TOP) || !(hit_mask & HIT_MASK_BOTTOM))) ||
	(IS_VERT_ANGLE(laser.current_angle) &&
	 (!(hit_mask & HIT_MASK_LEFT) || !(hit_mask & HIT_MASK_RIGHT))))
    {
      // laser at last step touched nothing or the same side of the wall
      if (LX != last_LX || LY != last_LY || hit_mask == last_hit_mask)
      {
	AddDamagedField(ELX, ELY);

	LX += 8 * XS;
	LY += 8 * YS;

	last_LX = LX;
	last_LY = LY;
	last_hit_mask = hit_mask;

	return FALSE;
      }
    }
  }

  if (!HitOnlyAnEdge(hit_mask))
  {
    laser.overloaded = TRUE;

    return TRUE;
  }

  return FALSE;
}

static boolean HitAbsorbingWalls(int element, int hit_mask)
{
  if (HitOnlyAnEdge(hit_mask))
    return FALSE;

  if (ABS(XS) == 4 &&
      (hit_mask == HIT_MASK_LEFT || hit_mask == HIT_MASK_RIGHT))
  {
    AddLaserEdge(LX - XS, LY - YS);

    LX = LX + XS / 2;
    LY = LY + YS;
  }

  if (ABS(YS) == 4 &&
      (hit_mask == HIT_MASK_TOP || hit_mask == HIT_MASK_BOTTOM))
  {
    AddLaserEdge(LX - XS, LY - YS);

    LX = LX + XS;
    LY = LY + YS / 2;
  }

  if (IS_WALL_WOOD(element) ||
      IS_DF_WALL_WOOD(element) ||
      IS_GRID_WOOD(element) ||
      IS_GRID_WOOD_FIXED(element) ||
      IS_GRID_WOOD_AUTO(element) ||
      element == EL_FUSE_ON ||
      element == EL_BLOCK_WOOD ||
      element == EL_GATE_WOOD)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    return TRUE;
  }

  if (IS_WALL_ICE(element))
  {
    int lx = LX + XS;
    int ly = LY + YS;
    int mask;

    // check if laser hit adjacent edges of two diagonal tiles
    if (ELX != lx / TILEX)
      lx = LX - XS;
    if (ELY != ly / TILEY)
      ly = LY - YS;

    mask =     lx / MINI_TILEX - ELX * 2 + 1;    // Quadrant (horizontal)
    mask <<= ((ly / MINI_TILEY - ELY * 2) > 0 ? 2 : 0);  // || (vertical)

    // check if laser hits wall with an angle of 90°
    if (IS_90_ANGLE(laser.current_angle))
      mask += mask * (2 + IS_HORIZ_ANGLE(laser.current_angle) * 2);

    if (mask == 1 || mask == 2 || mask == 4 || mask == 8)
    {
      int i;

      for (i = 0; i < 4; i++)
      {
	if (mask == (1 << i) && (XS > 0) == (i % 2) && (YS > 0) == (i / 2))
	  mask = 15 - (8 >> i);
	else if (ABS(XS) == 4 &&
		 mask == (1 << i) &&
		 (XS > 0) == (i % 2) &&
		 (YS < 0) == (i / 2))
	  mask = 3 + (i / 2) * 9;
	else if (ABS(YS) == 4 &&
		 mask == (1 << i) &&
		 (XS < 0) == (i % 2) &&
		 (YS > 0) == (i / 2))
	  mask = 5 + (i % 2) * 5;
      }
    }

    laser.wall_mask = mask;
  }
  else if (IS_WALL_AMOEBA(element))
  {
    int elx = (LX - 2 * XS) / TILEX;
    int ely = (LY - 2 * YS) / TILEY;
    int element2 = Tile[elx][ely];
    int mask;

    if (element2 != EL_EMPTY && !IS_WALL_AMOEBA(element2))
    {
      laser.dest_element = EL_EMPTY;

      return TRUE;
    }

    ELX = elx;
    ELY = ely;

    mask = (LX - 2 * XS) / 16 - ELX * 2 + 1;
    mask <<= ((LY - 2 * YS) / 16 - ELY * 2) * 2;

    if (IS_90_ANGLE(laser.current_angle))
      mask += mask * (2 + IS_HORIZ_ANGLE(laser.current_angle) * 2);

    laser.dest_element = element2 | EL_WALL_AMOEBA_BASE;

    laser.wall_mask = mask;
  }

  return TRUE;
}

static void OpenExit(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		// next animation frame
    MovDelay[x][y] = 4 * delay;

  if (MovDelay[x][y])		// wait some time before next frame
  {
    int phase;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;

    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
      DrawGraphicAnimation_MM(x, y, IMG_MM_EXIT_OPENING, 3 - phase);

    if (!MovDelay[x][y])
    {
      Tile[x][y] = EL_EXIT_OPEN;
      DrawField_MM(x, y);
    }
  }
}

static void OpenGrayBall(int x, int y)
{
  int delay = 2;

  if (!MovDelay[x][y])		// next animation frame
  {
    if (IS_WALL(Store[x][y]))
    {
      DrawWalls_MM(x, y, Store[x][y]);

      // copy wall tile to spare bitmap for "melting" animation
      BlitBitmap(drawto_mm, bitmap_db_field, cSX + x * TILEX, cSY + y * TILEY,
		 TILEX, TILEY, x * TILEX, y * TILEY);

      DrawElement_MM(x, y, EL_GRAY_BALL);
    }

    MovDelay[x][y] = 50 * delay;
  }

  if (MovDelay[x][y])		// wait some time before next frame
  {
    MovDelay[x][y]--;

    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
    {
      Bitmap *bitmap;
      int gx, gy;
      int dx = RND(26), dy = RND(26);

      if (IS_WALL(Store[x][y]))
      {
	// copy wall tile from spare bitmap for "melting" animation
	bitmap = bitmap_db_field;
	gx = x * TILEX;
	gy = y * TILEY;
      }
      else
      {
	int graphic = el2gfx(Store[x][y]);

	getGraphicSource(graphic, 0, &bitmap, &gx, &gy);
      }

      BlitBitmap(bitmap, drawto_mm, gx + dx, gy + dy, 6, 6,
		 cSX + x * TILEX + dx, cSY + y * TILEY + dy);

      laser.redraw = TRUE;

      MarkTileDirty(x, y);
    }

    if (!MovDelay[x][y])
    {
      Tile[x][y] = Store[x][y];
      Store[x][y] = Store2[x][y] = 0;
      MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;

      InitField(x, y, FALSE);
      DrawField_MM(x, y);

      ScanLaser_FromLastMirror();
    }
  }
}

static void OpenEnvelope(int x, int y)
{
  int num_frames = 8;		// seven frames plus final empty space

  if (!MovDelay[x][y])		// next animation frame
    MovDelay[x][y] = num_frames;

  if (MovDelay[x][y])		// wait some time before next frame
  {
    int nr = ENVELOPE_OPENING_NR(Tile[x][y]);

    MovDelay[x][y]--;

    if (MovDelay[x][y] > 0 && IN_SCR_FIELD(x, y))
    {
      int graphic = el_act2gfx(EL_ENVELOPE_1 + nr, MM_ACTION_COLLECTING);
      int frame = num_frames - MovDelay[x][y] - 1;

      DrawGraphicAnimation_MM(x, y, graphic, frame);

      laser.redraw = TRUE;
    }

    if (MovDelay[x][y] == 0)
    {
      Tile[x][y] = EL_EMPTY;

      DrawField_MM(x, y);

      ScanLaser();

      ShowEnvelope(nr);
    }
  }
}

static void MeltIce(int x, int y)
{
  int frames = 5;
  int delay = 5;

  if (!MovDelay[x][y])		// next animation frame
    MovDelay[x][y] = frames * delay;

  if (MovDelay[x][y])		// wait some time before next frame
  {
    int phase;
    int wall_mask = Store2[x][y];
    int real_element = Tile[x][y] - EL_WALL_CHANGING_BASE + EL_WALL_ICE_BASE;

    MovDelay[x][y]--;
    phase = frames - MovDelay[x][y] / delay - 1;

    if (!MovDelay[x][y])
    {
      Tile[x][y] = real_element & (wall_mask ^ 0xFF);
      Store[x][y] = Store2[x][y] = 0;

      DrawWalls_MM(x, y, Tile[x][y]);

      if (Tile[x][y] == EL_WALL_ICE_BASE)
	Tile[x][y] = EL_EMPTY;

      ScanLaser_FromLastMirror();
    }
    else if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
    {
      DrawWallsAnimation_MM(x, y, real_element, phase, wall_mask);

      laser.redraw = TRUE;
    }
  }
}

static void GrowAmoeba(int x, int y)
{
  int frames = 5;
  int delay = 1;

  if (!MovDelay[x][y])		// next animation frame
    MovDelay[x][y] = frames * delay;

  if (MovDelay[x][y])		// wait some time before next frame
  {
    int phase;
    int wall_mask = Store2[x][y];
    int real_element = Tile[x][y] - EL_WALL_CHANGING_BASE + EL_WALL_AMOEBA_BASE;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;

    if (!MovDelay[x][y])
    {
      Tile[x][y] = real_element;
      Store[x][y] = Store2[x][y] = 0;

      DrawWalls_MM(x, y, Tile[x][y]);
      DrawLaser(0, DL_LASER_ENABLED);
    }
    else if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
    {
      DrawWallsAnimation_MM(x, y, real_element, phase, wall_mask);
    }
  }
}

static void DrawFieldAnimated_MM(int x, int y)
{
  DrawField_MM(x, y);

  laser.redraw = TRUE;
}

static void DrawFieldAnimatedIfNeeded_MM(int x, int y)
{
  int element = Tile[x][y];
  int graphic = el2gfx(element);

  if (!getGraphicInfo_NewFrame(x, y, graphic))
    return;

  DrawField_MM(x, y);

  laser.redraw = TRUE;
}

static void DrawFieldTwinkle(int x, int y)
{
  if (MovDelay[x][y] != 0)	// wait some time before next frame
  {
    MovDelay[x][y]--;

    DrawField_MM(x, y);

    if (MovDelay[x][y] != 0)
    {
      int graphic = IMG_TWINKLE_WHITE;
      int frame = getGraphicAnimationFrame(graphic, 10 - MovDelay[x][y]);

      DrawGraphicThruMask_MM(SCREENX(x), SCREENY(y), graphic, frame);
    }

    laser.redraw = TRUE;
  }
}

static void Explode_MM(int x, int y, int phase, int mode)
{
  int num_phase = 9, delay = 2;
  int last_phase = num_phase * delay;
  int half_phase = (num_phase / 2) * delay;
  int center_element;

  laser.redraw = TRUE;

  if (phase == EX_PHASE_START)		// initialize 'Store[][]' field
  {
    center_element = Tile[x][y];

    if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
    {
      // put moving element to center field (and let it explode there)
      center_element = MovingOrBlocked2Element_MM(x, y);
      RemoveMovingField_MM(x, y);

      Tile[x][y] = center_element;
    }

    if (center_element != EL_GRAY_BALL_ACTIVE)
      Store[x][y] = EL_EMPTY;
    Store2[x][y] = center_element;

    Tile[x][y] = EL_EXPLODING_OPAQUE;

    GfxElement[x][y] = (center_element == EL_BOMB_ACTIVE ? EL_BOMB :
			center_element == EL_GRAY_BALL_ACTIVE ? EL_GRAY_BALL :
			IS_MCDUFFIN(center_element) ? EL_MCDUFFIN :
			center_element);

    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;

    ExplodePhase[x][y] = 1;

    return;
  }

  if (phase == 1)
    GfxFrame[x][y] = 0;		// restart explosion animation

  ExplodePhase[x][y] = (phase < last_phase ? phase + 1 : 0);

  center_element = Store2[x][y];

  if (phase == half_phase && Store[x][y] == EL_EMPTY)
  {
    Tile[x][y] = EL_EXPLODING_TRANSP;

    if (x == ELX && y == ELY)
      ScanLaser();
  }

  if (phase == last_phase)
  {
    if (center_element == EL_BOMB_ACTIVE)
    {
      DrawLaser(0, DL_LASER_DISABLED);
      InitLaser();

      Bang_MM(laser.start_edge.x, laser.start_edge.y);

      laser.overloaded = FALSE;
    }
    else if (IS_MCDUFFIN(center_element) || IS_LASER(center_element))
    {
      GameOver_MM(GAME_OVER_BOMB);
    }

    Tile[x][y] = Store[x][y];

    Store[x][y] = Store2[x][y] = 0;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;

    InitField(x, y, FALSE);
    DrawField_MM(x, y);

    if (center_element == EL_GRAY_BALL_ACTIVE)
      ScanLaser_FromLastMirror();
  }
  else if (!(phase % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    int graphic = el_act2gfx(GfxElement[x][y], MM_ACTION_EXPLODING);
    int frame = getGraphicAnimationFrameXY(graphic, x, y);

    DrawGraphicAnimation_MM(x, y, graphic, frame);

    MarkTileDirty(x, y);
  }
}

static void Bang_MM(int x, int y)
{
  int element = Tile[x][y];

  if (IS_PACMAN(element))
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);
  else if (element == EL_BOMB_ACTIVE || IS_MCDUFFIN(element))
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);
  else if (element == EL_KEY)
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);
  else
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);

  Explode_MM(x, y, EX_PHASE_START, EX_TYPE_NORMAL);
}

static void TurnRound(int x, int y)
{
  static struct
  {
    int x, y;
  } move_xy[] =
  {
    {  0,  0 },
    { -1,  0 },
    { +1,  0 },
    {  0,  0 },
    {  0, -1 },
    {  0,  0 }, { 0, 0 }, { 0, 0 },
    {  0, +1 }
  };
  static struct
  {
    int left, right, back;
  } turn[] =
  {
    { 0,	0,		0	 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT	 },
    { 0,	0,		0	 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN	 },
    { 0,	0,		0	 },
    { 0,	0,		0	 },
    { 0,	0,		0	 },
    { MV_RIGHT,	MV_LEFT,	MV_UP	 }
  };

  int element = Tile[x][y];
  int old_move_dir = MovDir[x][y];
  int right_dir = turn[old_move_dir].right;
  int back_dir = turn[old_move_dir].back;
  int right_dx = move_xy[right_dir].x, right_dy = move_xy[right_dir].y;
  int right_x = x + right_dx, right_y = y + right_dy;

  if (element == EL_PACMAN)
  {
    boolean can_turn_right = FALSE;

    if (IN_LEV_FIELD(right_x, right_y) &&
	IS_EATABLE4PACMAN(Tile[right_x][right_y]))
      can_turn_right = TRUE;

    if (can_turn_right)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 0;
  }
}

static void StartMoving_MM(int x, int y)
{
  int element = Tile[x][y];

  if (Stop[x][y])
    return;

  if (CAN_MOVE(element))
  {
    int newx, newy;

    if (MovDelay[x][y])		// wait some time before next movement
    {
      MovDelay[x][y]--;

      if (MovDelay[x][y])
	return;
    }

    // now make next step

    Moving2Blocked(x, y, &newx, &newy);	// get next screen position

    if (element == EL_PACMAN &&
	IN_LEV_FIELD(newx, newy) && IS_EATABLE4PACMAN(Tile[newx][newy]) &&
	!ObjHit(newx, newy, HIT_POS_CENTER))
    {
      Store[newx][newy] = Tile[newx][newy];
      Tile[newx][newy] = EL_EMPTY;

      DrawField_MM(newx, newy);
    }
    else if (!IN_LEV_FIELD(newx, newy) || !IS_FREE(newx, newy) ||
	     ObjHit(newx, newy, HIT_POS_CENTER))
    {
      // object was running against a wall

      TurnRound(x, y);

      return;
    }

    InitMovingField_MM(x, y, MovDir[x][y]);
  }

  if (MovDir[x][y])
    ContinueMoving_MM(x, y);
}

static void ContinueMoving_MM(int x, int y)
{
  int element = Tile[x][y];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int horiz_move = (dx!=0);
  int newx = x + dx, newy = y + dy;
  int step = (horiz_move ? dx : dy) * TILEX / 8;

  MovPos[x][y] += step;

  if (ABS(MovPos[x][y]) >= TILEX)	// object reached its destination
  {
    Tile[x][y] = EL_EMPTY;
    Tile[newx][newy] = element;

    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    MovDelay[newx][newy] = 0;

    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;

    DrawField_MM(x, y);
    DrawField_MM(newx, newy);

    Stop[newx][newy] = TRUE;

    if (element == EL_PACMAN)
    {
      if (Store[newx][newy] == EL_BOMB)
	Bang_MM(newx, newy);

      if (IS_WALL_AMOEBA(Store[newx][newy]) &&
	  (LX + 2 * XS) / TILEX == newx &&
	  (LY + 2 * YS) / TILEY == newy)
      {
	laser.num_edges--;
	ScanLaser();
      }
    }
  }
  else				// still moving on
  {
    DrawField_MM(x, y);
  }

  laser.redraw = TRUE;
}

boolean ClickElement(int x, int y, int button)
{
  static DelayCounter click_delay = { CLICK_DELAY };
  static boolean new_button = TRUE;
  boolean element_clicked = FALSE;
  int element;

  if (button == -1)
  {
    // initialize static variables
    click_delay.count = 0;
    click_delay.value = CLICK_DELAY;
    new_button = TRUE;

    return FALSE;
  }

  // do not rotate objects hit by the laser after the game was solved
  if (game_mm.level_solved && Hit[x][y])
    return FALSE;

  if (button == MB_RELEASED)
  {
    new_button = TRUE;
    click_delay.value = CLICK_DELAY;

    // release eventually hold auto-rotating mirror
    RotateMirror(x, y, MB_RELEASED);

    return FALSE;
  }

  if (!FrameReached(&click_delay) && !new_button)
    return FALSE;

  if (button == MB_MIDDLEBUTTON)	// middle button has no function
    return FALSE;

  if (!IN_LEV_FIELD(x, y))
    return FALSE;

  if (Tile[x][y] == EL_EMPTY)
    return FALSE;

  element = Tile[x][y];

  if (IS_MIRROR(element) ||
      IS_BEAMER(element) ||
      IS_POLAR(element) ||
      IS_POLAR_CROSS(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element))
  {
    RotateMirror(x, y, button);

    element_clicked = TRUE;
  }
  else if (IS_MCDUFFIN(element))
  {
    boolean has_laser = (x == laser.start_edge.x && y == laser.start_edge.y);

    if (has_laser && !laser.fuse_off)
      DrawLaser(0, DL_LASER_DISABLED);

    element = get_rotated_element(element, BUTTON_ROTATION(button));

    Tile[x][y] = element;
    DrawField_MM(x, y);

    if (has_laser)
    {
      laser.start_angle = get_element_angle(element);

      InitLaser();

      if (!laser.fuse_off)
	ScanLaser();
    }

    element_clicked = TRUE;
  }
  else if (element == EL_FUSE_ON && laser.fuse_off)
  {
    if (x != laser.fuse_x || y != laser.fuse_y)
      return FALSE;

    laser.fuse_off = FALSE;
    laser.fuse_x = laser.fuse_y = -1;

    DrawGraphic_MM(x, y, IMG_MM_FUSE_ACTIVE);
    ScanLaser();

    element_clicked = TRUE;
  }
  else if (element == EL_FUSE_ON && !laser.fuse_off && new_button)
  {
    laser.fuse_off = TRUE;
    laser.fuse_x = x;
    laser.fuse_y = y;
    laser.overloaded = FALSE;

    DrawLaser(0, DL_LASER_DISABLED);
    DrawGraphic_MM(x, y, IMG_MM_FUSE);

    element_clicked = TRUE;
  }
  else if (element == EL_LIGHTBALL)
  {
    Bang_MM(x, y);
    RaiseScoreElement_MM(element);
    DrawLaser(0, DL_LASER_ENABLED);

    element_clicked = TRUE;
  }
  else if (IS_ENVELOPE(element))
  {
    Tile[x][y] = EL_ENVELOPE_1_OPENING + ENVELOPE_NR(element);

    element_clicked = TRUE;
  }

  click_delay.value = (new_button ? CLICK_DELAY_FIRST : CLICK_DELAY);
  new_button = FALSE;

  return element_clicked;
}

static void RotateMirror(int x, int y, int button)
{
  if (button == MB_RELEASED)
  {
    // release eventually hold auto-rotating mirror
    hold_x = -1;
    hold_y = -1;

    return;
  }

  if (IS_MIRROR(Tile[x][y]) ||
      IS_POLAR_CROSS(Tile[x][y]) ||
      IS_POLAR(Tile[x][y]) ||
      IS_BEAMER(Tile[x][y]) ||
      IS_DF_MIRROR(Tile[x][y]) ||
      IS_GRID_STEEL_AUTO(Tile[x][y]) ||
      IS_GRID_WOOD_AUTO(Tile[x][y]))
  {
    Tile[x][y] = get_rotated_element(Tile[x][y], BUTTON_ROTATION(button));
  }
  else if (IS_DF_MIRROR_AUTO(Tile[x][y]))
  {
    if (button == MB_LEFTBUTTON)
    {
      // left mouse button only for manual adjustment, no auto-rotating;
      // freeze mirror for until mouse button released
      hold_x = x;
      hold_y = y;
    }
    else if (button == MB_RIGHTBUTTON && (hold_x != x || hold_y != y))
    {
      Tile[x][y] = get_rotated_element(Tile[x][y], ROTATE_RIGHT);
    }
  }

  if (IS_GRID_STEEL_AUTO(Tile[x][y]) || IS_GRID_WOOD_AUTO(Tile[x][y]))
  {
    int edge = Hit[x][y];

    DrawField_MM(x, y);

    if (edge > 0)
    {
      DrawLaser(edge - 1, DL_LASER_DISABLED);
      ScanLaser();
    }
  }
  else if (ObjHit(x, y, HIT_POS_CENTER))
  {
    int edge = Hit[x][y];

    if (edge == 0)
    {
      Warn("RotateMirror: inconsistent field Hit[][]!\n");

      edge = 1;
    }

    DrawLaser(edge - 1, DL_LASER_DISABLED);
    ScanLaser();
  }
  else
  {
    int check = 1;

    if (ObjHit(x, y, HIT_POS_EDGE | HIT_POS_BETWEEN))
      check = 2;

    DrawField_MM(x, y);

    if ((IS_BEAMER(Tile[x][y]) ||
	 IS_POLAR(Tile[x][y]) ||
	 IS_POLAR_CROSS(Tile[x][y])) && x == ELX && y == ELY)
    {
      if (IS_BEAMER(Tile[x][y]))
      {
#if 0
	Debug("game:mm:RotateMirror", "TEST (%d, %d) [%d] [%d]",
	      LX, LY, laser.beamer_edge, laser.beamer[1].num);
#endif

	if (check == 1)
	  laser.num_edges--;
      }

      ScanLaser();

      check = 0;
    }

    if (check == 2)
      DrawLaser(0, DL_LASER_ENABLED);
  }
}

static void AutoRotateMirrors(void)
{
  int x, y;

  if (!FrameReached(&rotate_delay))
    return;

  for (x = 0; x < lev_fieldx; x++)
  {
    for (y = 0; y < lev_fieldy; y++)
    {
      int element = Tile[x][y];

      // do not rotate objects hit by the laser after the game was solved
      if (game_mm.level_solved && Hit[x][y])
	continue;

      if (IS_DF_MIRROR_AUTO(element) ||
	  IS_GRID_WOOD_AUTO(element) ||
	  IS_GRID_STEEL_AUTO(element) ||
	  element == EL_REFRACTOR)
      {
	RotateMirror(x, y, MB_RIGHTBUTTON);

	laser.redraw = TRUE;
      }
    }
  }
}

static boolean ObjHit(int obx, int oby, int bits)
{
  int i;

  obx *= TILEX;
  oby *= TILEY;

  if (bits & HIT_POS_CENTER)
  {
    if (CheckLaserPixel(cSX + obx + 15,
			cSY + oby + 15))
      return TRUE;
  }

  if (bits & HIT_POS_EDGE)
  {
    for (i = 0; i < 4; i++)
      if (CheckLaserPixel(cSX + obx + 31 * (i % 2),
			  cSY + oby + 31 * (i / 2)))
	return TRUE;
  }

  if (bits & HIT_POS_BETWEEN)
  {
    for (i = 0; i < 4; i++)
      if (CheckLaserPixel(cSX + 4 + obx + 22 * (i % 2),
			  cSY + 4 + oby + 22 * (i / 2)))
	return TRUE;
  }

  return FALSE;
}

static void DeletePacMan(int px, int py)
{
  int i, j;

  Bang_MM(px, py);

  if (game_mm.num_pacman <= 1)
  {
    game_mm.num_pacman = 0;
    return;
  }

  for (i = 0; i < game_mm.num_pacman; i++)
    if (game_mm.pacman[i].x == px && game_mm.pacman[i].y == py)
      break;

  game_mm.num_pacman--;

  for (j = i; j < game_mm.num_pacman; j++)
  {
    game_mm.pacman[j].x   = game_mm.pacman[j + 1].x;
    game_mm.pacman[j].y   = game_mm.pacman[j + 1].y;
    game_mm.pacman[j].dir = game_mm.pacman[j + 1].dir;
  }
}

static void GameActions_MM_Ext(void)
{
  int element;
  int x, y, i;

  int r, d;

  for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
    Stop[x][y] = FALSE;

  for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
  {
    element = Tile[x][y];

    if (!IS_MOVING(x, y) && CAN_MOVE(element))
      StartMoving_MM(x, y);
    else if (IS_MOVING(x, y))
      ContinueMoving_MM(x, y);
    else if (IS_EXPLODING(element))
      Explode_MM(x, y, ExplodePhase[x][y], EX_TYPE_NORMAL);
    else if (element == EL_EXIT_OPENING)
      OpenExit(x, y);
    else if (element == EL_GRAY_BALL_OPENING)
      OpenGrayBall(x, y);
    else if (IS_ENVELOPE_OPENING(element))
      OpenEnvelope(x, y);
    else if (IS_WALL_CHANGING(element) && Store[x][y] == EL_WALL_ICE_BASE)
      MeltIce(x, y);
    else if (IS_WALL_CHANGING(element) && Store[x][y] == EL_WALL_AMOEBA_BASE)
      GrowAmoeba(x, y);
    else if (IS_MIRROR(element) ||
	     IS_MIRROR_FIXED(element) ||
	     element == EL_PRISM)
      DrawFieldTwinkle(x, y);
    else if (element == EL_GRAY_BALL_ACTIVE ||
	     element == EL_BOMB_ACTIVE ||
	     element == EL_MINE_ACTIVE)
      DrawFieldAnimated_MM(x, y);
    else if (!IS_BLOCKED(x, y))
      DrawFieldAnimatedIfNeeded_MM(x, y);
  }

  AutoRotateMirrors();

#if 1
  // !!! CHANGE THIS: REDRAW ONLY WHEN NEEDED !!!

  // redraw after Explode_MM() ...
  if (laser.redraw)
    DrawLaser(0, DL_LASER_ENABLED);
  laser.redraw = FALSE;
#endif

  CT = FrameCounter;

  if (game_mm.num_pacman && FrameReached(&pacman_delay))
  {
    MovePacMen();

    if (laser.num_damages > MAX_LASER_LEN && !laser.fuse_off)
    {
      DrawLaser(0, DL_LASER_DISABLED);
      ScanLaser();
    }
  }

  // skip all following game actions if game is over
  if (game_mm.game_over)
    return;

  if (game_mm.energy_left == 0 && !game.no_level_time_limit && game.time_limit)
  {
    FadeOutLaser();

    GameOver_MM(GAME_OVER_NO_ENERGY);

    return;
  }

  if (FrameReached(&energy_delay))
  {
    if (game_mm.energy_left > 0)
      game_mm.energy_left--;

    // when out of energy, wait another frame to play "out of time" sound
  }

  element = laser.dest_element;

#if 0
  if (element != Tile[ELX][ELY])
  {
    Debug("game:mm:GameActions_MM_Ext", "element == %d, Tile[ELX][ELY] == %d",
	  element, Tile[ELX][ELY]);
  }
#endif

  if (!laser.overloaded && laser.overload_value == 0 &&
      element != EL_BOMB &&
      element != EL_BOMB_ACTIVE &&
      element != EL_MINE &&
      element != EL_MINE_ACTIVE &&
      element != EL_GRAY_BALL &&
      element != EL_GRAY_BALL_ACTIVE &&
      element != EL_BLOCK_STONE &&
      element != EL_BLOCK_WOOD &&
      element != EL_FUSE_ON &&
      element != EL_FUEL_FULL &&
      !IS_WALL_ICE(element) &&
      !IS_WALL_AMOEBA(element))
    return;

  overload_delay.value = HEALTH_DELAY(laser.overloaded);

  if (((laser.overloaded && laser.overload_value < MAX_LASER_OVERLOAD) ||
       (!laser.overloaded && laser.overload_value > 0)) &&
      FrameReached(&overload_delay))
  {
    if (laser.overloaded)
      laser.overload_value++;
    else
      laser.overload_value--;

    if (game_mm.cheat_no_overload)
    {
      laser.overloaded = FALSE;
      laser.overload_value = 0;
    }

    game_mm.laser_overload_value = laser.overload_value;

    if (laser.overload_value < MAX_LASER_OVERLOAD - 8)
    {
      SetLaserColor(0xFF);

      DrawLaser(0, DL_LASER_ENABLED);
    }

    if (!laser.overloaded)
      StopSound_MM(SND_MM_GAME_HEALTH_CHARGING);
    else if (setup.sound_loops)
      PlaySoundLoop_MM(SND_MM_GAME_HEALTH_CHARGING);
    else
      PlaySound_MM(SND_MM_GAME_HEALTH_CHARGING);

    if (laser.overload_value == MAX_LASER_OVERLOAD)
    {
      UpdateAndDisplayGameControlValues();

      FadeOutLaser();

      GameOver_MM(GAME_OVER_OVERLOADED);

      return;
    }
  }

  if (laser.fuse_off)
    return;

  CT -= Ct;

  if (element == EL_BOMB && CT > native_mm_level.time_bomb)
  {
    if (game_mm.cheat_no_explosion)
      return;

    Bang_MM(ELX, ELY);

    laser.dest_element = EL_EXPLODING_OPAQUE;

    return;
  }

  if (element == EL_FUSE_ON && CT > native_mm_level.time_fuse)
  {
    laser.fuse_off = TRUE;
    laser.fuse_x = ELX;
    laser.fuse_y = ELY;

    DrawLaser(0, DL_LASER_DISABLED);
    DrawGraphic_MM(ELX, ELY, IMG_MM_FUSE);
  }

  if (element == EL_GRAY_BALL && CT > native_mm_level.time_ball)
  {
    if (!Store2[ELX][ELY])	// check if content element not yet determined
    {
      int last_anim_random_frame = gfx.anim_random_frame;
      int element_pos;

      if (native_mm_level.ball_choice_mode == ANIM_RANDOM)
	gfx.anim_random_frame = RND(native_mm_level.num_ball_contents);

      element_pos = getAnimationFrame(native_mm_level.num_ball_contents, 1,
				      native_mm_level.ball_choice_mode, 0,
				      game_mm.ball_choice_pos);

      if (native_mm_level.ball_choice_mode == ANIM_RANDOM)
	gfx.anim_random_frame = last_anim_random_frame;

      game_mm.ball_choice_pos++;

      int new_element = native_mm_level.ball_content[element_pos];
      int new_element_base = map_wall_to_base_element(new_element);

      if (IS_WALL(new_element_base))
      {
	// always use completely filled wall element
	new_element = new_element_base | 0x000f;
      }
      else if (native_mm_level.rotate_ball_content &&
	       get_num_elements(new_element) > 1)
      {
	// randomly rotate newly created game element
	new_element = get_rotated_element(new_element, RND(16));
      }

      Store[ELX][ELY] = new_element;
      Store2[ELX][ELY] = TRUE;
    }

    if (native_mm_level.explode_ball)
      Bang_MM(ELX, ELY);
    else
      Tile[ELX][ELY] = EL_GRAY_BALL_OPENING;

    laser.dest_element = laser.dest_element_last = Tile[ELX][ELY];

    return;
  }

  if (IS_WALL_ICE(element) && CT > 50)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_SHRINKING);

    Tile[ELX][ELY] = Tile[ELX][ELY] - EL_WALL_ICE_BASE + EL_WALL_CHANGING_BASE;
    Store[ELX][ELY] = EL_WALL_ICE_BASE;
    Store2[ELX][ELY] = laser.wall_mask;

    laser.dest_element = Tile[ELX][ELY];

    return;
  }

  if (IS_WALL_AMOEBA(element) && CT > 60)
  {
    int k1, k2, k3;
    int element2 = Tile[ELX][ELY];

    if (element2 != EL_EMPTY && !IS_WALL_AMOEBA(element))
      return;

    for (i = laser.num_damages - 1; i >= 0; i--)
      if (laser.damage[i].is_mirror)
	break;

    r = laser.num_edges;
    d = laser.num_damages;
    k1 = i;

    if (k1 > 0)
    {
      int x, y;

      DrawLaser(laser.damage[k1].edge - 1, DL_LASER_DISABLED);

      laser.num_edges++;
      DrawLaser(0, DL_LASER_ENABLED);
      laser.num_edges--;

      x = laser.damage[k1].x;
      y = laser.damage[k1].y;

      DrawField_MM(x, y);
    }

    for (i = 0; i < 4; i++)
    {
      if (laser.wall_mask & (1 << i))
      {
	if (CheckLaserPixel(cSX + ELX * TILEX + 14 + (i % 2) * 2,
			    cSY + ELY * TILEY + 31 * (i / 2)))
	  break;

	if (CheckLaserPixel(cSX + ELX * TILEX + 31 * (i % 2),
			    cSY + ELY * TILEY + 14 + (i / 2) * 2))
	  break;
      }
    }

    k2 = i;

    for (i = 0; i < 4; i++)
    {
      if (laser.wall_mask & (1 << i))
      {
	if (CheckLaserPixel(cSX + ELX * TILEX + 31 * (i % 2),
			    cSY + ELY * TILEY + 31 * (i / 2)))
	  break;
      }
    }

    k3 = i;

    if (laser.num_beamers > 0 ||
	k1 < 1 || k2 < 4 || k3 < 4 ||
	CheckLaserPixel(cSX + ELX * TILEX + 14,
			cSY + ELY * TILEY + 14))
    {
      laser.num_edges = r;
      laser.num_damages = d;

      DrawLaser(0, DL_LASER_DISABLED);
    }

    Tile[ELX][ELY] = element | laser.wall_mask;

    int x = ELX, y = ELY;
    int wall_mask = laser.wall_mask;

    ScanLaser();
    DrawLaser(0, DL_LASER_ENABLED);

    PlayLevelSound_MM(x, y, element, MM_ACTION_GROWING);

    Tile[x][y] = Tile[x][y] - EL_WALL_AMOEBA_BASE + EL_WALL_CHANGING_BASE;
    Store[x][y] = EL_WALL_AMOEBA_BASE;
    Store2[x][y] = wall_mask;

    return;
  }

  if ((element == EL_BLOCK_WOOD || element == EL_BLOCK_STONE) &&
      laser.stops_inside_element && CT > native_mm_level.time_block)
  {
    int x, y;
    int k;

    if (ABS(XS) > ABS(YS))
      k = 0;
    else
      k = 1;
    if (XS < YS)
      k += 2;

    for (i = 0; i < 4; i++)
    {
      if (i)
	k++;
      if (k > 3)
	k = 0;

      x = ELX + Step[k * 4].x;
      y = ELY + Step[k * 4].y;

      if (!IN_LEV_FIELD(x, y) || Tile[x][y] != EL_EMPTY)
	continue;

      if (ObjHit(x, y, HIT_POS_CENTER | HIT_POS_EDGE | HIT_POS_BETWEEN))
	continue;

      break;
    }

    if (i > 3)
    {
      laser.overloaded = (element == EL_BLOCK_STONE);

      return;
    }

    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_PUSHING);

    Tile[ELX][ELY] = 0;
    Tile[x][y] = element;

    DrawGraphic_MM(ELX, ELY, IMG_EMPTY);
    DrawField_MM(x, y);

    if (element == EL_BLOCK_STONE && Box[ELX][ELY])
    {
      DrawLaser(Box[ELX][ELY] - 1, DL_LASER_DISABLED);
      DrawLaser(laser.num_edges - 1, DL_LASER_ENABLED);
    }

    ScanLaser();

    return;
  }

  if (element == EL_FUEL_FULL && CT > 10)
  {
    int num_init_game_frames = INIT_GAME_ACTIONS_DELAY;
    int start = num_init_game_frames * game_mm.energy_left / native_mm_level.time;

    for (i = start; i <= num_init_game_frames; i++)
    {
      if (i == num_init_game_frames)
	StopSound_MM(SND_MM_GAME_LEVELTIME_CHARGING);
      else if (setup.sound_loops)
	PlaySoundLoop_MM(SND_MM_GAME_LEVELTIME_CHARGING);
      else
	PlaySound_MM(SND_MM_GAME_LEVELTIME_CHARGING);

      game_mm.energy_left = native_mm_level.time * i / num_init_game_frames;

      UpdateAndDisplayGameControlValues();

      BackToFront_MM();
    }

    Tile[ELX][ELY] = laser.dest_element = EL_FUEL_EMPTY;

    DrawField_MM(ELX, ELY);

    DrawLaser(0, DL_LASER_ENABLED);

    return;
  }
}

void GameActions_MM(struct MouseActionInfo action)
{
  boolean element_clicked = ClickElement(action.lx, action.ly, action.button);
  boolean button_released = (action.button == MB_RELEASED);

  GameActions_MM_Ext();

  CheckSingleStepMode_MM(element_clicked, button_released);
}

static void MovePacMen(void)
{
  int mx, my, ox, oy, nx, ny;
  int element;
  int l;

  if (++pacman_nr >= game_mm.num_pacman)
    pacman_nr = 0;

  game_mm.pacman[pacman_nr].dir--;

  for (l = 1; l < 5; l++)
  {
    game_mm.pacman[pacman_nr].dir++;

    if (game_mm.pacman[pacman_nr].dir > 4)
      game_mm.pacman[pacman_nr].dir = 1;

    if (game_mm.pacman[pacman_nr].dir % 2)
    {
      mx = 0;
      my = game_mm.pacman[pacman_nr].dir - 2;
    }
    else
    {
      my = 0;
      mx = 3 - game_mm.pacman[pacman_nr].dir;
    }

    ox = game_mm.pacman[pacman_nr].x;
    oy = game_mm.pacman[pacman_nr].y;
    nx = ox + mx;
    ny = oy + my;
    element = Tile[nx][ny];

    if (nx < 0 || nx > 15 || ny < 0 || ny > 11)
      continue;

    if (!IS_EATABLE4PACMAN(element))
      continue;

    if (ObjHit(nx, ny, HIT_POS_CENTER))
      continue;

    Tile[ox][oy] = EL_EMPTY;
    Tile[nx][ny] =
      EL_PACMAN_RIGHT - 1 +
      (game_mm.pacman[pacman_nr].dir - 1 +
       (game_mm.pacman[pacman_nr].dir % 2) * 2);

    game_mm.pacman[pacman_nr].x = nx;
    game_mm.pacman[pacman_nr].y = ny;

    DrawGraphic_MM(ox, oy, IMG_EMPTY);

    if (element != EL_EMPTY)
    {
      int graphic = el2gfx(Tile[nx][ny]);
      Bitmap *bitmap;
      int src_x, src_y;
      int i;

      getGraphicSource(graphic, 0, &bitmap, &src_x, &src_y);

      CT = FrameCounter;
      ox = cSX + ox * TILEX;
      oy = cSY + oy * TILEY;

      for (i = 1; i < 33; i += 2)
	BlitBitmap(bitmap, window,
		   src_x, src_y, TILEX, TILEY,
		   ox + i * mx, oy + i * my);
      Ct = Ct + FrameCounter - CT;
    }

    DrawField_MM(nx, ny);
    BackToFront_MM();

    if (!laser.fuse_off)
    {
      DrawLaser(0, DL_LASER_ENABLED);

      if (ObjHit(nx, ny, HIT_POS_BETWEEN))
      {
	AddDamagedField(nx, ny);

	laser.damage[laser.num_damages - 1].edge = 0;
      }
    }

    if (element == EL_BOMB)
      DeletePacMan(nx, ny);

    if (IS_WALL_AMOEBA(element) &&
	(LX + 2 * XS) / TILEX == nx &&
	(LY + 2 * YS) / TILEY == ny)
    {
      laser.num_edges--;
      ScanLaser();
    }

    break;
  }
}

static void InitMovingField_MM(int x, int y, int direction)
{
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  MovDir[x][y] = direction;
  MovDir[newx][newy] = direction;

  if (Tile[newx][newy] == EL_EMPTY)
    Tile[newx][newy] = EL_BLOCKED;
}

static int MovingOrBlocked2Element_MM(int x, int y)
{
  int element = Tile[x][y];

  if (element == EL_BLOCKED)
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);

    return Tile[oldx][oldy];
  }

  return element;
}

static void RemoveMovingField_MM(int x, int y)
{
  int oldx = x, oldy = y, newx = x, newy = y;

  if (Tile[x][y] != EL_BLOCKED && !IS_MOVING(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    Moving2Blocked(x, y, &newx, &newy);
    if (Tile[newx][newy] != EL_BLOCKED)
      return;
  }
  else if (Tile[x][y] == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &oldx, &oldy);
    if (!IS_MOVING(oldx, oldy))
      return;
  }

  Tile[oldx][oldy] = EL_EMPTY;
  Tile[newx][newy] = EL_EMPTY;
  MovPos[oldx][oldy] = MovDir[oldx][oldy] = MovDelay[oldx][oldy] = 0;
  MovPos[newx][newy] = MovDir[newx][newy] = MovDelay[newx][newy] = 0;

  DrawLevelField_MM(oldx, oldy);
  DrawLevelField_MM(newx, newy);
}

static void RaiseScore_MM(int value)
{
  game_mm.score += value;
}

void RaiseScoreElement_MM(int element)
{
  switch (element)
  {
    case EL_PACMAN:
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      RaiseScore_MM(native_mm_level.score[SC_PACMAN]);
      break;

    case EL_KEY:
      RaiseScore_MM(native_mm_level.score[SC_KEY]);
      break;

    case EL_KETTLE:
    case EL_CELL:
      RaiseScore_MM(native_mm_level.score[SC_COLLECTIBLE]);
      break;

    case EL_LIGHTBALL:
      RaiseScore_MM(native_mm_level.score[SC_LIGHTBALL]);
      break;

    default:
      break;
  }
}


// ----------------------------------------------------------------------------
// Mirror Magic game engine snapshot handling functions
// ----------------------------------------------------------------------------

void SaveEngineSnapshotValues_MM(void)
{
  int x, y;

  engine_snapshot_mm.game_mm = game_mm;
  engine_snapshot_mm.laser = laser;

  for (x = 0; x < MAX_PLAYFIELD_WIDTH; x++)
  {
    for (y = 0; y < MAX_PLAYFIELD_HEIGHT; y++)
    {
      engine_snapshot_mm.Ur[x][y]    = Ur[x][y];
      engine_snapshot_mm.Hit[x][y]   = Hit[x][y];
      engine_snapshot_mm.Box[x][y]   = Box[x][y];
      engine_snapshot_mm.Angle[x][y] = Angle[x][y];
    }
  }

  engine_snapshot_mm.LX = LX;
  engine_snapshot_mm.LY = LY;
  engine_snapshot_mm.XS = XS;
  engine_snapshot_mm.YS = YS;
  engine_snapshot_mm.ELX = ELX;
  engine_snapshot_mm.ELY = ELY;
  engine_snapshot_mm.CT = CT;
  engine_snapshot_mm.Ct = Ct;

  engine_snapshot_mm.last_LX = last_LX;
  engine_snapshot_mm.last_LY = last_LY;
  engine_snapshot_mm.last_hit_mask = last_hit_mask;
  engine_snapshot_mm.hold_x = hold_x;
  engine_snapshot_mm.hold_y = hold_y;
  engine_snapshot_mm.pacman_nr = pacman_nr;

  engine_snapshot_mm.rotate_delay = rotate_delay;
  engine_snapshot_mm.pacman_delay = pacman_delay;
  engine_snapshot_mm.energy_delay = energy_delay;
  engine_snapshot_mm.overload_delay = overload_delay;
}

void LoadEngineSnapshotValues_MM(void)
{
  int x, y;

  // stored engine snapshot buffers already restored at this point

  game_mm = engine_snapshot_mm.game_mm;
  laser   = engine_snapshot_mm.laser;

  for (x = 0; x < MAX_PLAYFIELD_WIDTH; x++)
  {
    for (y = 0; y < MAX_PLAYFIELD_HEIGHT; y++)
    {
      Ur[x][y]    = engine_snapshot_mm.Ur[x][y];
      Hit[x][y]   = engine_snapshot_mm.Hit[x][y];
      Box[x][y]   = engine_snapshot_mm.Box[x][y];
      Angle[x][y] = engine_snapshot_mm.Angle[x][y];
    }
  }

  LX  = engine_snapshot_mm.LX;
  LY  = engine_snapshot_mm.LY;
  XS  = engine_snapshot_mm.XS;
  YS  = engine_snapshot_mm.YS;
  ELX = engine_snapshot_mm.ELX;
  ELY = engine_snapshot_mm.ELY;
  CT  = engine_snapshot_mm.CT;
  Ct  = engine_snapshot_mm.Ct;

  last_LX       = engine_snapshot_mm.last_LX;
  last_LY       = engine_snapshot_mm.last_LY;
  last_hit_mask = engine_snapshot_mm.last_hit_mask;
  hold_x        = engine_snapshot_mm.hold_x;
  hold_y        = engine_snapshot_mm.hold_y;
  pacman_nr     = engine_snapshot_mm.pacman_nr;

  rotate_delay   = engine_snapshot_mm.rotate_delay;
  pacman_delay   = engine_snapshot_mm.pacman_delay;
  energy_delay   = engine_snapshot_mm.energy_delay;
  overload_delay = engine_snapshot_mm.overload_delay;

  RedrawPlayfield_MM();
}

static int getAngleFromTouchDelta(int dx, int dy,  int base)
{
  double pi = 3.141592653;
  double rad = atan2((double)-dy, (double)dx);
  double rad2 = (rad < 0 ? rad + 2 * pi : rad);
  double deg = rad2 * 180.0 / pi;

  return (int)(deg * base / 360.0 + 0.5) % base;
}

int getButtonFromTouchPosition(int x, int y, int dst_mx, int dst_my)
{
  // calculate start (source) position to be at the middle of the tile
  int src_mx = cSX + x * TILESIZE_VAR + TILESIZE_VAR / 2;
  int src_my = cSY + y * TILESIZE_VAR + TILESIZE_VAR / 2;
  int dx = dst_mx - src_mx;
  int dy = dst_my - src_my;
  int element;
  int base = 16;
  int phases = 16;
  int angle_old = -1;
  int angle_new = -1;
  int button = 0;
  int i;

  if (!IN_LEV_FIELD(x, y))
    return 0;

  element = Tile[x][y];

  if (!IS_MCDUFFIN(element) &&
      !IS_MIRROR(element) &&
      !IS_BEAMER(element) &&
      !IS_POLAR(element) &&
      !IS_POLAR_CROSS(element) &&
      !IS_DF_MIRROR(element))
    return 0;

  angle_old = get_element_angle(element);

  if (IS_MCDUFFIN(element))
  {
    angle_new = (dx > 0 && ABS(dy) < ABS(dx) ? ANG_RAY_RIGHT :
		 dy < 0 && ABS(dx) < ABS(dy) ? ANG_RAY_UP :
		 dx < 0 && ABS(dy) < ABS(dx) ? ANG_RAY_LEFT :
		 dy > 0 && ABS(dx) < ABS(dy) ? ANG_RAY_DOWN :
		 -1);
  }
  else if (IS_MIRROR(element) ||
	   IS_DF_MIRROR(element))
  {
    for (i = 0; i < laser.num_damages; i++)
    {
      if (laser.damage[i].x == x &&
	  laser.damage[i].y == y &&
	  ObjHit(x, y, HIT_POS_CENTER))
      {
	angle_old = get_mirrored_angle(laser.damage[i].angle, angle_old);
	angle_new = getAngleFromTouchDelta(dx, dy, base) % phases;

	break;
      }
    }
  }

  if (angle_new == -1)
  {
    if (IS_MIRROR(element) ||
	IS_DF_MIRROR(element) ||
	IS_POLAR(element))
      base = 32;

    if (IS_POLAR_CROSS(element))
      phases = 4;

    angle_new = getAngleFromTouchDelta(dx, dy, base) % phases;
  }

  button = (angle_new == angle_old ? 0 :
	    (angle_new - angle_old + phases) % phases < (phases / 2) ?
	    MB_LEFTBUTTON : MB_RIGHTBUTTON);

  return button;
}
