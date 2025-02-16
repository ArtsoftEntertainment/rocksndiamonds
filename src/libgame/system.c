// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// system.c
// ============================================================================

#include <string.h>
#include <signal.h>

#include "platform.h"

#include "system.h"
#include "image.h"
#include "sound.h"
#include "setup.h"
#include "joystick.h"
#include "misc.h"

#define ENABLE_UNUSED_CODE	0	// currently unused functions


// ============================================================================
// exported variables
// ============================================================================

struct ProgramInfo	program;
struct NetworkInfo	network;
struct RuntimeInfo	runtime;
struct OptionInfo	options;
struct VideoSystemInfo	video;
struct AudioSystemInfo	audio;
struct GfxInfo		gfx;
struct TileCursorInfo	tile_cursor;
struct OverlayInfo	overlay;
struct ArtworkInfo	artwork;
struct JoystickInfo	joystick;
struct SetupInfo	setup;
struct UserInfo		user;
struct ZipFileInfo	zip_file;

LevelDirTree	       *leveldir_first_all = NULL;
LevelDirTree	       *leveldir_first = NULL;
LevelDirTree	       *leveldir_current = NULL;
int			level_nr;

struct LevelSetInfo	levelset;
struct LevelStats	level_stats[MAX_LEVELS];

DrawWindow	       *window = NULL;
DrawBuffer	       *backbuffer = NULL;
DrawBuffer	       *drawto = NULL;

int			button_status = MB_NOT_PRESSED;
boolean			motion_status = FALSE;
int			wheel_steps = DEFAULT_WHEEL_STEPS;
boolean			keyrepeat_status = TRUE;
boolean			textinput_status = FALSE;

int			redraw_mask = REDRAW_NONE;

int			FrameCounter = 0;


// ============================================================================
// init/close functions
// ============================================================================

void InitProgramInfo(char *command_filename,
		     char *config_filename, char *userdata_subdir,
		     char *program_basename, char *program_title,
		     char *icon_filename, char *cookie_prefix,
		     char *program_version_string, VersionType program_version)
{
  program.command_basepath = getBasePath(command_filename);
  program.command_basename = getBaseName(command_filename);

  program.config_filename = config_filename;

  program.userdata_subdir = userdata_subdir;
  program.userdata_path = getMainUserGameDataDir();

  program.program_basename = program_basename;
  program.program_title = program_title;
  program.window_title = "(undefined)";

  program.icon_filename = icon_filename;

  program.cookie_prefix = cookie_prefix;

  program.version_super = VERSION_SUPER(program_version);
  program.version_major = VERSION_MAJOR(program_version);
  program.version_minor = VERSION_MINOR(program_version);
  program.version_patch = VERSION_PATCH(program_version);
  program.version_ident = program_version;

  program.version_string = program_version_string;

  program.log_filename = getLogFilename(getLogBasename(program_basename));
  program.log_file = program.log_file_default = stdout;

  program.api_thread_count = 0;

  program.headless = FALSE;
}

void InitNetworkInfo(boolean enabled, boolean connected, boolean serveronly,
		     char *server_host, int server_port)
{
  network.enabled = enabled;
  network.connected = connected;
  network.serveronly = serveronly;

  network.server_host = server_host;
  network.server_port = server_port;

  network.server_thread = NULL;
  network.is_server_thread = FALSE;
}

void InitRuntimeInfo(void)
{
#if defined(HAS_TOUCH_DEVICE)
  runtime.uses_touch_device = TRUE;
#else
  runtime.uses_touch_device = FALSE;
#endif

  runtime.use_api_server = setup.use_api_server;
}

void SetWindowTitle(void)
{
  program.window_title = program.window_title_function();

  SDLSetWindowTitle();
}

void SetWindowResized(void)
{
  SDLSetWindowResized();
}

void InitWindowTitleFunction(char *(*window_title_function)(void))
{
  program.window_title_function = window_title_function;
}

void InitExitMessageFunction(void (*exit_message_function)(char *, va_list))
{
  program.exit_message_function = exit_message_function;
}

void InitExitFunction(void (*exit_function)(int))
{
  program.exit_function = exit_function;

  // set signal handlers to custom exit function
  // signal(SIGINT, exit_function);
  signal(SIGTERM, exit_function);

  // set exit function to automatically cleanup SDL stuff after exit()
  atexit(SDL_Quit);
}

void InitPlatformDependentStuff(void)
{
  InitEmscriptenFilesystem();

  // this is initialized in GetOptions(), but may already be used before
  options.verbose = TRUE;

  OpenLogFile();

  int sdl_init_flags = SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE;

  if (SDL_Init(sdl_init_flags) < 0)
    Fail("SDL_Init() failed: %s", SDL_GetError());

  SDLNet_Init();
}

void ClosePlatformDependentStuff(void)
{
  CloseLogFile();
}

void InitGfxFieldInfo(int sx, int sy, int sxsize, int sysize,
		      int real_sx, int real_sy,
		      int full_sxsize, int full_sysize,
		      Bitmap *field_save_buffer)
{
  gfx.sx = sx;
  gfx.sy = sy;
  gfx.sxsize = sxsize;
  gfx.sysize = sysize;
  gfx.real_sx = real_sx;
  gfx.real_sy = real_sy;
  gfx.full_sxsize = full_sxsize;
  gfx.full_sysize = full_sysize;

  gfx.field_save_buffer = field_save_buffer;

  SetDrawDeactivationMask(REDRAW_NONE);		// do not deactivate drawing
  SetDrawBackgroundMask(REDRAW_NONE);		// deactivate masked drawing
}

void InitGfxTileSizeInfo(int game_tile_size, int standard_tile_size)
{
  gfx.game_tile_size = game_tile_size;
  gfx.standard_tile_size = standard_tile_size;
}

void InitGfxDoor1Info(int dx, int dy, int dxsize, int dysize)
{
  gfx.dx = dx;
  gfx.dy = dy;
  gfx.dxsize = dxsize;
  gfx.dysize = dysize;
}

void InitGfxDoor2Info(int vx, int vy, int vxsize, int vysize)
{
  gfx.vx = vx;
  gfx.vy = vy;
  gfx.vxsize = vxsize;
  gfx.vysize = vysize;
}

void InitGfxDoor3Info(int ex, int ey, int exsize, int eysize)
{
  gfx.ex = ex;
  gfx.ey = ey;
  gfx.exsize = exsize;
  gfx.eysize = eysize;
}

void InitGfxWindowInfo(int win_xsize, int win_ysize)
{
  if (win_xsize != gfx.win_xsize || win_ysize != gfx.win_ysize)
  {
    ReCreateBitmap(&gfx.background_bitmap, win_xsize, win_ysize);

    ReCreateBitmap(&gfx.final_screen_bitmap, win_xsize, win_ysize);

    ReCreateBitmap(&gfx.fade_bitmap_backup, win_xsize, win_ysize);
    ReCreateBitmap(&gfx.fade_bitmap_source, win_xsize, win_ysize);
    ReCreateBitmap(&gfx.fade_bitmap_target, win_xsize, win_ysize);
    ReCreateBitmap(&gfx.fade_bitmap_black,  win_xsize, win_ysize);

    ClearRectangle(gfx.fade_bitmap_black, 0, 0, win_xsize, win_ysize);
  }

  gfx.win_xsize = win_xsize;
  gfx.win_ysize = win_ysize;

  gfx.background_bitmap_mask = REDRAW_NONE;
}

void InitGfxScrollbufferInfo(int scrollbuffer_width, int scrollbuffer_height)
{
  // currently only used by MSDOS code to alloc VRAM buffer, if available
  // 2009-03-24: also (temporarily?) used for overlapping blit workaround
  gfx.scrollbuffer_width = scrollbuffer_width;
  gfx.scrollbuffer_height = scrollbuffer_height;
}

void InitGfxClipRegion(boolean enabled, int x, int y, int width, int height)
{
  gfx.clipping_enabled = enabled;
  gfx.clip_x = x;
  gfx.clip_y = y;
  gfx.clip_width = width;
  gfx.clip_height = height;
}

void InitGfxDrawBusyAnimFunction(void (*draw_busy_anim_function)(boolean))
{
  gfx.draw_busy_anim_function = draw_busy_anim_function;
}

void InitGfxDrawGlobalAnimFunction(void (*draw_global_anim_function)(int, int))
{
  gfx.draw_global_anim_function = draw_global_anim_function;
}

void InitGfxDrawGlobalBorderFunction(void (*draw_global_border_function)(int))
{
  gfx.draw_global_border_function = draw_global_border_function;
}

void InitGfxDrawTileCursorFunction(void (*draw_tile_cursor_function)(int, int))
{
  gfx.draw_tile_cursor_function = draw_tile_cursor_function;
}

void InitGfxDrawEnvelopeRequestFunction(void (*draw_envelope_request_function)(int))
{
  gfx.draw_envelope_request_function = draw_envelope_request_function;
}

void InitGfxCustomArtworkInfo(void)
{
  gfx.override_level_graphics = FALSE;
  gfx.override_level_sounds = FALSE;
  gfx.override_level_music = FALSE;

  gfx.draw_init_text = TRUE;
}

void InitGfxOtherSettings(void)
{
  gfx.cursor_mode = CURSOR_DEFAULT;
  gfx.cursor_mode_override = CURSOR_UNDEFINED;
  gfx.cursor_mode_final = gfx.cursor_mode;

  // prevent initially displaying custom mouse cursor in upper left corner
  gfx.mouse_x = POS_OFFSCREEN;
  gfx.mouse_y = POS_OFFSCREEN;
}

void InitTileCursorInfo(void)
{
  tile_cursor.enabled = FALSE;
  tile_cursor.active = FALSE;
  tile_cursor.moving = FALSE;

  tile_cursor.xpos = 0;
  tile_cursor.ypos = 0;
  tile_cursor.x = 0;
  tile_cursor.y = 0;
  tile_cursor.target_x = 0;
  tile_cursor.target_y = 0;

  tile_cursor.sx = 0;
  tile_cursor.sy = 0;

  tile_cursor.xsn_debug = FALSE;
}

void InitOverlayInfo(void)
{
  overlay.enabled = FALSE;
  overlay.active = FALSE;

  overlay.show_grid = FALSE;

  overlay.grid_button_highlight = CHAR_GRID_BUTTON_NONE;
  overlay.grid_button_action = JOY_NO_ACTION;

  SetOverlayGridSizeAndButtons();

#if defined(USE_TOUCH_INPUT_OVERLAY)
  if (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS))
    overlay.enabled = TRUE;
#endif
}

void SetOverlayGridSizeAndButtons(void)
{
  int nr = GRID_ACTIVE_NR();
  int x, y;

  overlay.grid_xsize = setup.touch.grid_xsize[nr];
  overlay.grid_ysize = setup.touch.grid_ysize[nr];

  for (x = 0; x < MAX_GRID_XSIZE; x++)
    for (y = 0; y < MAX_GRID_YSIZE; y++)
      overlay.grid_button[x][y] = setup.touch.grid_button[nr][x][y];
}

void SetTileCursorEnabled(boolean enabled)
{
  tile_cursor.enabled = enabled;
}

void SetTileCursorActive(boolean active)
{
  tile_cursor.active = active;
}

void SetTileCursorTargetXY(int x, int y)
{
  // delayed placement of tile selection cursor at target position
  // (tile cursor will be moved to target position step by step)

  tile_cursor.xpos = x;
  tile_cursor.ypos = y;
  tile_cursor.target_x = tile_cursor.sx + x * gfx.game_tile_size;
  tile_cursor.target_y = tile_cursor.sy + y * gfx.game_tile_size;

  tile_cursor.moving = TRUE;
}

void SetTileCursorXY(int x, int y)
{
  // immediate placement of tile selection cursor at target position

  SetTileCursorTargetXY(x, y);

  tile_cursor.x = tile_cursor.target_x;
  tile_cursor.y = tile_cursor.target_y;

  tile_cursor.moving = FALSE;
}

void SetTileCursorSXSY(int sx, int sy)
{
  tile_cursor.sx = sx;
  tile_cursor.sy = sy;
}

void SetOverlayEnabled(boolean enabled)
{
  overlay.enabled = enabled;
}

void SetOverlayActive(boolean active)
{
  overlay.active = active;
}

void SetOverlayShowGrid(boolean show_grid)
{
  overlay.show_grid = show_grid;

  SetOverlayActive(show_grid);

  if (show_grid)
    SetOverlayEnabled(TRUE);
}

boolean GetOverlayEnabled(void)
{
  return overlay.enabled;
}

boolean GetOverlayActive(void)
{
  return overlay.active;
}

void SetDrawDeactivationMask(int draw_deactivation_mask)
{
  gfx.draw_deactivation_mask = draw_deactivation_mask;
}

int GetDrawDeactivationMask(void)
{
  return gfx.draw_deactivation_mask;
}

void SetDrawBackgroundMask(int draw_background_mask)
{
  gfx.draw_background_mask = draw_background_mask;
}

int GetDrawBackgroundMask(void)
{
  return gfx.draw_background_mask;
}

void SetBackgroundBitmap(Bitmap *background_bitmap_tile, int mask,
			 int x, int y, int width, int height)
{
  if (background_bitmap_tile != NULL)
    gfx.background_bitmap_mask |= mask;
  else
    gfx.background_bitmap_mask &= ~mask;

  if (background_bitmap_tile == NULL)	// empty background requested
    return;

  if (mask == REDRAW_ALL)
    BlitBitmapTiled(background_bitmap_tile, gfx.background_bitmap,
		    x, y, width, height,
		    0, 0, video.width, video.height);
  else if (mask == REDRAW_FIELD)
    BlitBitmapTiled(background_bitmap_tile, gfx.background_bitmap,
		    x, y, width, height,
		    gfx.real_sx, gfx.real_sy, gfx.full_sxsize, gfx.full_sysize);
  else if (mask == REDRAW_DOOR_1)
    BlitBitmapTiled(background_bitmap_tile, gfx.background_bitmap,
		    x, y, width, height,
		    gfx.dx, gfx.dy, gfx.dxsize, gfx.dysize);
}


// ============================================================================
// video functions
// ============================================================================

static int GetRealDepth(int depth)
{
  return (depth == DEFAULT_DEPTH ? video.default_depth : depth);
}

static void sysFillRectangle(Bitmap *bitmap, int x, int y,
			     int width, int height, Pixel color)
{
  SDLFillRectangle(bitmap, x, y, width, height, color);

  if (bitmap == backbuffer)
    SetRedrawMaskFromArea(x, y, width, height);
}

static void sysCopyArea(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			int src_x, int src_y, int width, int height,
			int dst_x, int dst_y, int mask_mode)
{
  SDLCopyArea(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	      dst_x, dst_y, mask_mode);

  if (dst_bitmap == backbuffer)
    SetRedrawMaskFromArea(dst_x, dst_y, width, height);
}

void LimitScreenUpdates(boolean enable)
{
  SDLLimitScreenUpdates(enable);
}

void InitVideoDefaults(void)
{
  video.default_depth = 32;
}

void InitVideoDisplay(void)
{
  if (program.headless)
    return;

  SDLInitVideoDisplay();
  SDLSetDisplaySize();
}

void CloseVideoDisplay(void)
{
  KeyboardAutoRepeatOn();

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void InitVideoBuffer(int width, int height, int depth, boolean fullscreen)
{
  video.width = width;
  video.height = height;
  video.depth = GetRealDepth(depth);

  video.screen_width = width;
  video.screen_height = height;
  video.screen_xoffset = 0;
  video.screen_yoffset = 0;

  video.fullscreen_available = FULLSCREEN_STATUS;
  video.fullscreen_enabled = FALSE;

  video.window_scaling_available = WINDOW_SCALING_STATUS;

  video.frame_counter = 0;
  video.frame_delay.count = 0;
  video.frame_delay.value = GAME_FRAME_DELAY;

  video.shifted_up = FALSE;
  video.shifted_up_pos = 0;
  video.shifted_up_pos_last = 0;
  video.shifted_up_delay.count = 0;
  video.shifted_up_delay.value = ONE_SECOND_DELAY / 4;

  SDLInitVideoBuffer(fullscreen);

  video.initialized = !program.headless;

  drawto = backbuffer;
}

static void FreeBitmapPointers(Bitmap *bitmap)
{
  if (bitmap == NULL)
    return;

  SDLFreeBitmapPointers(bitmap);

  checked_free(bitmap->source_filename);
  bitmap->source_filename = NULL;
}

static void TransferBitmapPointers(Bitmap *src_bitmap,
				   Bitmap *dst_bitmap)
{
  if (src_bitmap == NULL || dst_bitmap == NULL)
    return;

  FreeBitmapPointers(dst_bitmap);

  *dst_bitmap = *src_bitmap;
}

void FreeBitmap(Bitmap *bitmap)
{
  if (bitmap == NULL)
    return;

  FreeBitmapPointers(bitmap);

  free(bitmap);
}

void ResetBitmapAlpha(Bitmap *bitmap)
{
  bitmap->alpha[0][0] = -1;
  bitmap->alpha[0][1] = -1;
  bitmap->alpha[1][0] = -1;
  bitmap->alpha[1][1] = -1;
  bitmap->alpha_next_blit = -1;
}

Bitmap *CreateBitmapStruct(void)
{
  Bitmap *new_bitmap = checked_calloc(sizeof(Bitmap));

  ResetBitmapAlpha(new_bitmap);

  return new_bitmap;
}

Bitmap *CreateBitmap(int width, int height, int depth)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  int real_width  = MAX(1, width);	// prevent zero bitmap width
  int real_height = MAX(1, height);	// prevent zero bitmap height
  int real_depth  = GetRealDepth(depth);

  new_bitmap->surface = SDLCreateNativeSurface(real_width, real_height, real_depth);

  new_bitmap->width  = real_width;
  new_bitmap->height = real_height;

  return new_bitmap;
}

void ReCreateBitmap(Bitmap **bitmap, int width, int height)
{
  if (*bitmap != NULL)
  {
    // if new bitmap size fits into old one, no need to re-create it
    if (width  <= (*bitmap)->width &&
        height <= (*bitmap)->height)
      return;

    // else adjust size so that old and new bitmap size fit into it
    width  = MAX(width,  (*bitmap)->width);
    height = MAX(height, (*bitmap)->height);
  }

  Bitmap *new_bitmap = CreateBitmap(width, height, DEFAULT_DEPTH);

  if (*bitmap == NULL)
  {
    *bitmap = new_bitmap;
  }
  else
  {
    TransferBitmapPointers(new_bitmap, *bitmap);
    free(new_bitmap);
  }
}

#if 0
static void CloseWindow(DrawWindow *window)
{
}
#endif

void SetRedrawMaskFromArea(int x, int y, int width, int height)
{
  int x1 = x;
  int y1 = y;
  int x2 = x + width - 1;
  int y2 = y + height - 1;

  if (width == 0 || height == 0)
    return;

  if (IN_GFX_FIELD_FULL(x1, y1) && IN_GFX_FIELD_FULL(x2, y2))
    redraw_mask |= REDRAW_FIELD;
  else if (IN_GFX_DOOR_1(x1, y1) && IN_GFX_DOOR_1(x2, y2))
    redraw_mask |= REDRAW_DOOR_1;
  else if (IN_GFX_DOOR_2(x1, y1) && IN_GFX_DOOR_2(x2, y2))
    redraw_mask |= REDRAW_DOOR_2;
  else if (IN_GFX_DOOR_3(x1, y1) && IN_GFX_DOOR_3(x2, y2))
    redraw_mask |= REDRAW_DOOR_3;
  else
    redraw_mask = REDRAW_ALL;
}

static boolean CheckDrawingArea(int x, int y, int draw_mask)
{
  if (draw_mask == REDRAW_NONE)
    return FALSE;

  if (draw_mask & REDRAW_ALL)
    return TRUE;

  if ((draw_mask & REDRAW_FIELD) && IN_GFX_FIELD_FULL(x, y))
    return TRUE;

  if ((draw_mask & REDRAW_DOOR_1) && IN_GFX_DOOR_1(x, y))
    return TRUE;

  if ((draw_mask & REDRAW_DOOR_2) && IN_GFX_DOOR_2(x, y))
    return TRUE;

  if ((draw_mask & REDRAW_DOOR_3) && IN_GFX_DOOR_3(x, y))
    return TRUE;

  return FALSE;
}

boolean DrawingDeactivatedField(void)
{
  if (program.headless)
    return TRUE;

  if (gfx.draw_deactivation_mask & REDRAW_FIELD)
    return TRUE;

  return FALSE;
}

boolean DrawingDeactivated(int x, int y)
{
  return CheckDrawingArea(x, y, gfx.draw_deactivation_mask);
}

boolean DrawingOnBackground(int x, int y)
{
  return (CheckDrawingArea(x, y, gfx.background_bitmap_mask) &&
	  CheckDrawingArea(x, y, gfx.draw_background_mask));
}

static boolean InClippedRectangle(Bitmap *bitmap, int *x, int *y,
				  int *width, int *height, boolean is_dest)
{
  int clip_x, clip_y, clip_width, clip_height;

  if (gfx.clipping_enabled && is_dest)	// only clip destination bitmap
  {
    clip_x = MIN(MAX(0, gfx.clip_x), bitmap->width);
    clip_y = MIN(MAX(0, gfx.clip_y), bitmap->height);
    clip_width = MIN(MAX(0, gfx.clip_width), bitmap->width - clip_x);
    clip_height = MIN(MAX(0, gfx.clip_height), bitmap->height - clip_y);
  }
  else
  {
    clip_x = 0;
    clip_y = 0;
    clip_width = bitmap->width;
    clip_height = bitmap->height;
  }

  // skip if rectangle completely outside bitmap

  if (*x + *width  <= clip_x ||
      *y + *height <= clip_y ||
      *x >= clip_x + clip_width ||
      *y >= clip_y + clip_height)
    return FALSE;

  // clip if rectangle overlaps bitmap

  if (*x < clip_x)
  {
    *width -= clip_x - *x;
    *x = clip_x;
  }
  else if (*x + *width > clip_x + clip_width)
  {
    *width = clip_x + clip_width - *x;
  }

  if (*y < clip_y)
  {
    *height -= clip_y - *y;
    *y = clip_y;
  }
  else if (*y + *height > clip_y + clip_height)
  {
    *height = clip_y + clip_height - *y;
  }

  return TRUE;
}

void SetBitmapAlphaNextBlit(Bitmap *bitmap, int alpha)
{
  // set alpha value for next blitting of bitmap
  bitmap->alpha_next_blit = alpha;
}

void BlitBitmap(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		int src_x, int src_y, int width, int height,
		int dst_x, int dst_y)
{
  int dst_x_unclipped = dst_x;
  int dst_y_unclipped = dst_y;

  if (program.headless)
    return;

  if (src_bitmap == NULL || dst_bitmap == NULL)
    return;

  if (DrawingDeactivated(dst_x, dst_y))
    return;

  if (!InClippedRectangle(src_bitmap, &src_x, &src_y, &width, &height, FALSE) ||
      !InClippedRectangle(dst_bitmap, &dst_x, &dst_y, &width, &height, TRUE))
    return;

  // source x/y might need adjustment if destination x/y was clipped top/left
  src_x += dst_x - dst_x_unclipped;
  src_y += dst_y - dst_y_unclipped;

  // !!! 2013-12-11: An "old friend" is back. Same bug in SDL2 2.0.1 !!!
  // !!! 2009-03-30: Fixed by using self-compiled, patched SDL.dll !!!
  /* (This bug still exists in the actual (as of 2009-06-15) version 1.2.13,
     but is already fixed in SVN and should therefore finally be fixed with
     the next official SDL release, which is probably version 1.2.14.) */
  // !!! 2009-03-24: It seems that this problem still exists in 1.2.12 !!!

  if (src_bitmap == dst_bitmap)
  {
    // needed when blitting directly to same bitmap -- should not be needed with
    // recent SDL libraries, but apparently does not work in 1.2.11 directly

    static Bitmap *tmp_bitmap = NULL;
    static int tmp_bitmap_xsize = 0;
    static int tmp_bitmap_ysize = 0;

    // start with largest static bitmaps for initial bitmap size ...
    if (tmp_bitmap_xsize == 0 && tmp_bitmap_ysize == 0)
    {
      tmp_bitmap_xsize = MAX(gfx.win_xsize, gfx.scrollbuffer_width);
      tmp_bitmap_ysize = MAX(gfx.win_ysize, gfx.scrollbuffer_height);
    }

    // ... and allow for later re-adjustments due to custom artwork bitmaps
    if (src_bitmap->width > tmp_bitmap_xsize ||
	src_bitmap->height > tmp_bitmap_ysize)
    {
      tmp_bitmap_xsize = MAX(tmp_bitmap_xsize, src_bitmap->width);
      tmp_bitmap_ysize = MAX(tmp_bitmap_ysize, src_bitmap->height);

      FreeBitmap(tmp_bitmap);

      tmp_bitmap = NULL;
    }

    if (tmp_bitmap == NULL)
      tmp_bitmap = CreateBitmap(tmp_bitmap_xsize, tmp_bitmap_ysize,
				DEFAULT_DEPTH);

    sysCopyArea(src_bitmap, tmp_bitmap,
		src_x, src_y, width, height, dst_x, dst_y, BLIT_OPAQUE);
    sysCopyArea(tmp_bitmap, dst_bitmap,
		dst_x, dst_y, width, height, dst_x, dst_y, BLIT_OPAQUE);

    return;
  }

  sysCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y, BLIT_OPAQUE);
}

void BlitBitmapTiled(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		     int src_x, int src_y, int src_width, int src_height,
		     int dst_x, int dst_y, int dst_width, int dst_height)
{
  int src_xsize = (src_width  == 0 ? src_bitmap->width  : src_width);
  int src_ysize = (src_height == 0 ? src_bitmap->height : src_height);
  int dst_xsize = dst_width;
  int dst_ysize = dst_height;
  int src_xsteps = (dst_xsize + src_xsize - 1) / src_xsize;
  int src_ysteps = (dst_ysize + src_ysize - 1) / src_ysize;
  int x, y;

  for (y = 0; y < src_ysteps; y++)
  {
    for (x = 0; x < src_xsteps; x++)
    {
      int draw_x = dst_x + x * src_xsize;
      int draw_y = dst_y + y * src_ysize;
      int draw_xsize = MIN(src_xsize, dst_xsize - x * src_xsize);
      int draw_ysize = MIN(src_ysize, dst_ysize - y * src_ysize);

      BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, draw_xsize, draw_ysize,
		 draw_x, draw_y);
    }
  }
}

void FadeRectangle(int x, int y, int width, int height,
		   int fade_mode, int fade_delay, int post_delay,
		   void (*draw_border_function)(void))
{
  // (use destination bitmap "backbuffer" -- "bitmap_cross" may be undefined)
  if (!InClippedRectangle(backbuffer, &x, &y, &width, &height, TRUE))
    return;

  SDLFadeRectangle(x, y, width, height,
		   fade_mode, fade_delay, post_delay, draw_border_function);
}

void FillRectangle(Bitmap *bitmap, int x, int y, int width, int height,
		   Pixel color)
{
  if (program.headless)
    return;

  if (DrawingDeactivated(x, y))
    return;

  if (!InClippedRectangle(bitmap, &x, &y, &width, &height, TRUE))
    return;

  sysFillRectangle(bitmap, x, y, width, height, color);
}

void ClearRectangle(Bitmap *bitmap, int x, int y, int width, int height)
{
  FillRectangle(bitmap, x, y, width, height, BLACK_PIXEL);
}

void ClearRectangleOnBackground(Bitmap *bitmap, int x, int y,
				int width, int height)
{
  if (DrawingOnBackground(x, y))
    BlitBitmap(gfx.background_bitmap, bitmap, x, y, width, height, x, y);
  else
    ClearRectangle(bitmap, x, y, width, height);
}

void BlitBitmapMasked(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		      int src_x, int src_y, int width, int height,
		      int dst_x, int dst_y)
{
  if (program.headless)
    return;

  if (src_bitmap == NULL || dst_bitmap == NULL)
    return;

  if (DrawingDeactivated(dst_x, dst_y))
    return;

  sysCopyArea(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	      dst_x, dst_y, BLIT_MASKED);
}

void BlitBitmapOnBackground(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			    int src_x, int src_y, int width, int height,
			    int dst_x, int dst_y)
{
  if (DrawingOnBackground(dst_x, dst_y))
  {
    // draw background
    BlitBitmap(gfx.background_bitmap, dst_bitmap, dst_x, dst_y, width, height,
	       dst_x, dst_y);

    // draw foreground
    BlitBitmapMasked(src_bitmap, dst_bitmap, src_x, src_y, width, height,
		     dst_x, dst_y);
  }
  else
    BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	       dst_x, dst_y);
}

void BlitTexture(Bitmap *bitmap,
		int src_x, int src_y, int width, int height,
		int dst_x, int dst_y)
{
  if (bitmap == NULL)
    return;

  SDLBlitTexture(bitmap, src_x, src_y, width, height, dst_x, dst_y,
		 BLIT_OPAQUE);
}

void BlitTextureMasked(Bitmap *bitmap,
		       int src_x, int src_y, int width, int height,
		       int dst_x, int dst_y)
{
  if (bitmap == NULL)
    return;

  SDLBlitTexture(bitmap, src_x, src_y, width, height, dst_x, dst_y,
		 BLIT_MASKED);
}

void BlitToScreen(Bitmap *bitmap,
		  int src_x, int src_y, int width, int height,
		  int dst_x, int dst_y)
{
  if (bitmap == NULL)
    return;

  if (video.screen_rendering_mode == SPECIAL_RENDERING_BITMAP)
    BlitBitmap(bitmap, gfx.final_screen_bitmap, src_x, src_y,
	       width, height, dst_x, dst_y);
  else
    BlitTexture(bitmap, src_x, src_y, width, height, dst_x, dst_y);
}

void BlitToScreenMasked(Bitmap *bitmap,
			int src_x, int src_y, int width, int height,
			int dst_x, int dst_y)
{
  if (bitmap == NULL)
    return;

  if (video.screen_rendering_mode == SPECIAL_RENDERING_BITMAP)
    BlitBitmapMasked(bitmap, gfx.final_screen_bitmap, src_x, src_y,
		     width, height, dst_x, dst_y);
  else
    BlitTextureMasked(bitmap, src_x, src_y, width, height, dst_x, dst_y);
}

void DrawSimpleWhiteLine(Bitmap *bitmap, int from_x, int from_y,
			 int to_x, int to_y)
{
  SDLDrawSimpleLine(bitmap, from_x, from_y, to_x, to_y, WHITE_PIXEL);
}

static void DrawLine(Bitmap *bitmap, int from_x, int from_y,
		     int to_x, int to_y, Pixel pixel, int line_width)
{
  int x, y;

  if (program.headless)
    return;

  for (x = 0; x < line_width; x++)
  {
    for (y = 0; y < line_width; y++)
    {
      int dx = x - line_width / 2;
      int dy = y - line_width / 2;

      if ((x == 0 && y == 0) ||
	  (x == 0 && y == line_width - 1) ||
	  (x == line_width - 1 && y == 0) ||
	  (x == line_width - 1 && y == line_width - 1))
	continue;

      SDLDrawLine(bitmap,
		  from_x + dx, from_y + dy, to_x + dx, to_y + dy, pixel);
    }
  }
}

void DrawLines(Bitmap *bitmap, struct XY *points, int num_points, Pixel pixel)
{
  int line_width = 4;
  int i;

  for (i = 0; i < num_points - 1; i++)
    DrawLine(bitmap, points[i].x, points[i].y,
	     points[i + 1].x, points[i + 1].y, pixel, line_width);

  /*
  SDLDrawLines(bitmap->surface, points, num_points, pixel);
  */
}

Pixel GetPixel(Bitmap *bitmap, int x, int y)
{
  if (program.headless)
    return BLACK_PIXEL;

  if (x < 0 || x >= bitmap->width ||
      y < 0 || y >= bitmap->height)
    return BLACK_PIXEL;

  return SDLGetPixel(bitmap, x, y);
}

Pixel GetPixelFromRGB(Bitmap *bitmap, unsigned int color_r,
		      unsigned int color_g, unsigned int color_b)
{
  if (program.headless)
    return BLACK_PIXEL;

  return SDL_MapRGB(bitmap->surface->format, color_r, color_g, color_b);
}

void KeyboardAutoRepeatOn(void)
{
  keyrepeat_status = TRUE;
}

void KeyboardAutoRepeatOff(void)
{
  keyrepeat_status = FALSE;
}

boolean SetVideoMode(boolean fullscreen)
{
  return SDLSetVideoMode(fullscreen);
}

void SetVideoFrameDelay(unsigned int frame_delay_value)
{
  video.frame_delay.value = frame_delay_value;
}

unsigned int GetVideoFrameDelay(void)
{
  return video.frame_delay.value;
}

boolean ChangeVideoModeIfNeeded(boolean fullscreen)
{
  if ((fullscreen && !video.fullscreen_enabled && video.fullscreen_available)||
      (!fullscreen && video.fullscreen_enabled))
    fullscreen = SetVideoMode(fullscreen);

  return fullscreen;
}

Bitmap *LoadImage(char *filename)
{
  Bitmap *new_bitmap;

  new_bitmap = SDLLoadImage(filename);

  if (new_bitmap)
    new_bitmap->source_filename = getStringCopy(filename);

  return new_bitmap;
}

Bitmap *LoadCustomImage(char *basename)
{
  char *filename = getCustomImageFilename(basename);
  Bitmap *new_bitmap;

  if (filename == NULL)
    Fail("LoadCustomImage(): cannot find file '%s'", basename);

  if ((new_bitmap = LoadImage(filename)) == NULL)
    Fail("LoadImage('%s') failed", basename);

  return new_bitmap;
}

void ReloadCustomImage(Bitmap *bitmap, char *basename)
{
  char *filename = getCustomImageFilename(basename);
  Bitmap *new_bitmap;

  if (filename == NULL)		// (should never happen)
  {
    Warn("ReloadCustomImage(): cannot find file '%s'", basename);

    return;
  }

  if (strEqual(filename, bitmap->source_filename))
  {
    // The old and new image are the same (have the same filename and path).
    // This usually means that this image does not exist in this graphic set
    // and a fallback to the existing image is done.

    return;
  }

  if ((new_bitmap = LoadImage(filename)) == NULL)
  {
    Warn("LoadImage('%s') failed", basename);

    return;
  }

  if (bitmap->width != new_bitmap->width ||
      bitmap->height != new_bitmap->height)
  {
    Warn("ReloadCustomImage: new image '%s' has wrong dimensions",
	  filename);

    FreeBitmap(new_bitmap);

    return;
  }

  TransferBitmapPointers(new_bitmap, bitmap);
  free(new_bitmap);
}

Bitmap *ZoomBitmap(Bitmap *src_bitmap, int zoom_width, int zoom_height)
{
  return SDLZoomBitmap(src_bitmap, zoom_width, zoom_height);
}

void ReCreateGameTileSizeBitmap(Bitmap **bitmaps)
{
  if (bitmaps[IMG_BITMAP_CUSTOM])
  {
    // check if original sized bitmap points to custom sized bitmap
    if (bitmaps[IMG_BITMAP_PTR_ORIGINAL] == bitmaps[IMG_BITMAP_CUSTOM])
    {
      SDLFreeBitmapTextures(bitmaps[IMG_BITMAP_PTR_ORIGINAL]);

      // keep pointer of previous custom size bitmap
      bitmaps[IMG_BITMAP_OTHER] = bitmaps[IMG_BITMAP_CUSTOM];

      // set original bitmap pointer to scaled original bitmap of other size
      bitmaps[IMG_BITMAP_PTR_ORIGINAL] = bitmaps[IMG_BITMAP_OTHER];

      SDLCreateBitmapTextures(bitmaps[IMG_BITMAP_PTR_ORIGINAL]);
    }
    else
    {
      FreeBitmap(bitmaps[IMG_BITMAP_CUSTOM]);
    }

    bitmaps[IMG_BITMAP_CUSTOM] = NULL;
  }

  if (gfx.game_tile_size == gfx.standard_tile_size)
  {
    // set game bitmap pointer to standard sized bitmap (already existing)
    bitmaps[IMG_BITMAP_PTR_GAME] = bitmaps[IMG_BITMAP_STANDARD];

    return;
  }

  Bitmap *bitmap = bitmaps[IMG_BITMAP_STANDARD];
  int width  = bitmap->width  * gfx.game_tile_size / gfx.standard_tile_size;;
  int height = bitmap->height * gfx.game_tile_size / gfx.standard_tile_size;;

  bitmaps[IMG_BITMAP_CUSTOM] = ZoomBitmap(bitmap, width, height);

  // set game bitmap pointer to custom sized bitmap (newly created)
  bitmaps[IMG_BITMAP_PTR_GAME] = bitmaps[IMG_BITMAP_CUSTOM];
}

static void CreateScaledBitmaps(Bitmap **bitmaps, int zoom_factor,
				int tile_size, boolean create_small_bitmaps)
{
  Bitmap *old_bitmap = bitmaps[IMG_BITMAP_STANDARD];
  Bitmap *tmp_bitmap_final = NULL;
  Bitmap *tmp_bitmap_0 = NULL;
  Bitmap *tmp_bitmap_1 = NULL;
  Bitmap *tmp_bitmap_2 = NULL;
  Bitmap *tmp_bitmap_4 = NULL;
  Bitmap *tmp_bitmap_8 = NULL;
  Bitmap *tmp_bitmap_16 = NULL;
  Bitmap *tmp_bitmap_32 = NULL;
  int width_final, height_final;
  int width_0, height_0;
  int width_1, height_1;
  int width_2, height_2;
  int width_4, height_4;
  int width_8, height_8;
  int width_16, height_16;
  int width_32, height_32;
  int old_width, old_height;
  int i;

  print_timestamp_init("CreateScaledBitmaps");

  old_width  = old_bitmap->width;
  old_height = old_bitmap->height;

  // calculate new image dimensions for final image size
  width_final  = old_width  * zoom_factor;
  height_final = old_height * zoom_factor;

  // get image with final size (this might require scaling up)
  // ("final" size may result in non-standard tile size image)
  if (zoom_factor != 1)
    tmp_bitmap_final = ZoomBitmap(old_bitmap, width_final, height_final);
  else
    tmp_bitmap_final = old_bitmap;

  UPDATE_BUSY_STATE();

  width_0  = width_1  = width_final;
  height_0 = height_1 = height_final;

  tmp_bitmap_0 = tmp_bitmap_1 = tmp_bitmap_final;

  if (create_small_bitmaps)
  {
    // check if we have a non-gameplay tile size image
    if (tile_size != gfx.game_tile_size)
    {
      // get image with gameplay tile size
      width_0  = width_final  * gfx.game_tile_size / tile_size;
      height_0 = height_final * gfx.game_tile_size / tile_size;

      if (width_0 == old_width)
	tmp_bitmap_0 = old_bitmap;
      else if (width_0 == width_final)
	tmp_bitmap_0 = tmp_bitmap_final;
      else
	tmp_bitmap_0 = ZoomBitmap(old_bitmap, width_0, height_0);

      UPDATE_BUSY_STATE();
    }

    // check if we have a non-standard tile size image
    if (tile_size != gfx.standard_tile_size)
    {
      // get image with standard tile size
      width_1  = width_final  * gfx.standard_tile_size / tile_size;
      height_1 = height_final * gfx.standard_tile_size / tile_size;

      if (width_1 == old_width)
	tmp_bitmap_1 = old_bitmap;
      else if (width_1 == width_final)
	tmp_bitmap_1 = tmp_bitmap_final;
      else if (width_1 == width_0)
	tmp_bitmap_1 = tmp_bitmap_0;
      else
	tmp_bitmap_1 = ZoomBitmap(old_bitmap, width_1, height_1);

      UPDATE_BUSY_STATE();
    }

    // calculate new image dimensions for small images
    width_2  = width_1  / 2;
    height_2 = height_1 / 2;
    width_4  = width_1  / 4;
    height_4 = height_1 / 4;
    width_8  = width_1  / 8;
    height_8 = height_1 / 8;
    width_16  = width_1  / 16;
    height_16 = height_1 / 16;
    width_32  = width_1  / 32;
    height_32 = height_1 / 32;

    // get image with 1/2 of normal size (for use in the level editor)
    if (width_2 == old_width)
      tmp_bitmap_2 = old_bitmap;
    else
      tmp_bitmap_2 = ZoomBitmap(tmp_bitmap_1, width_2, height_2);

    UPDATE_BUSY_STATE();

    // get image with 1/4 of normal size (for use in the level editor)
    if (width_4 == old_width)
      tmp_bitmap_4 = old_bitmap;
    else
      tmp_bitmap_4 = ZoomBitmap(tmp_bitmap_2, width_4, height_4);

    UPDATE_BUSY_STATE();

    // get image with 1/8 of normal size (for use on the preview screen)
    if (width_8 == old_width)
      tmp_bitmap_8 = old_bitmap;
    else
      tmp_bitmap_8 = ZoomBitmap(tmp_bitmap_4, width_8, height_8);

    UPDATE_BUSY_STATE();

    // get image with 1/16 of normal size (for use on the preview screen)
    if (width_16 == old_width)
      tmp_bitmap_16 = old_bitmap;
    else
      tmp_bitmap_16 = ZoomBitmap(tmp_bitmap_8, width_16, height_16);

    UPDATE_BUSY_STATE();

    // get image with 1/32 of normal size (for use on the preview screen)
    if (width_32 == old_width)
      tmp_bitmap_32 = old_bitmap;
    else
      tmp_bitmap_32 = ZoomBitmap(tmp_bitmap_16, width_32, height_32);

    UPDATE_BUSY_STATE();

    bitmaps[IMG_BITMAP_32x32] = tmp_bitmap_1;
    bitmaps[IMG_BITMAP_16x16] = tmp_bitmap_2;
    bitmaps[IMG_BITMAP_8x8]   = tmp_bitmap_4;
    bitmaps[IMG_BITMAP_4x4]   = tmp_bitmap_8;
    bitmaps[IMG_BITMAP_2x2]   = tmp_bitmap_16;
    bitmaps[IMG_BITMAP_1x1]   = tmp_bitmap_32;

    if (width_0 != width_1)
      bitmaps[IMG_BITMAP_CUSTOM] = tmp_bitmap_0;

    if (bitmaps[IMG_BITMAP_CUSTOM])
      bitmaps[IMG_BITMAP_PTR_GAME] = bitmaps[IMG_BITMAP_CUSTOM];
    else
      bitmaps[IMG_BITMAP_PTR_GAME] = bitmaps[IMG_BITMAP_STANDARD];

    // store the "final" (up-scaled) original bitmap, if not already stored

    int tmp_bitmap_final_nr = -1;

    for (i = 0; i < NUM_IMG_BITMAPS; i++)
      if (bitmaps[i] == tmp_bitmap_final)
	tmp_bitmap_final_nr = i;

    if (tmp_bitmap_final_nr == -1)	// scaled original bitmap not stored
    {
      // store pointer of scaled original bitmap (not used for any other size)
      bitmaps[IMG_BITMAP_OTHER] = tmp_bitmap_final;

      // set original bitmap pointer to scaled original bitmap of other size
      bitmaps[IMG_BITMAP_PTR_ORIGINAL] = bitmaps[IMG_BITMAP_OTHER];
    }
    else
    {
      // set original bitmap pointer to corresponding sized bitmap
      bitmaps[IMG_BITMAP_PTR_ORIGINAL] = bitmaps[tmp_bitmap_final_nr];
    }

    // free the "old" (unscaled) original bitmap, if not already stored

    boolean free_old_bitmap = TRUE;

    for (i = 0; i < NUM_IMG_BITMAPS; i++)
      if (bitmaps[i] == old_bitmap)
	free_old_bitmap = FALSE;

    if (free_old_bitmap)
    {
      // copy image filename from old to new standard sized bitmap
      bitmaps[IMG_BITMAP_STANDARD]->source_filename =
	getStringCopy(old_bitmap->source_filename);

      FreeBitmap(old_bitmap);
    }
  }
  else
  {
    bitmaps[IMG_BITMAP_32x32] = tmp_bitmap_1;

    // set original bitmap pointer to corresponding sized bitmap
    bitmaps[IMG_BITMAP_PTR_ORIGINAL] = bitmaps[IMG_BITMAP_32x32];

    if (old_bitmap != tmp_bitmap_1)
      FreeBitmap(old_bitmap);
  }

  UPDATE_BUSY_STATE();

  print_timestamp_done("CreateScaledBitmaps");
}

void CreateBitmapWithSmallBitmaps(Bitmap **bitmaps, int zoom_factor,
				  int tile_size)
{
  CreateScaledBitmaps(bitmaps, zoom_factor, tile_size, TRUE);
}

void CreateBitmapTextures(Bitmap **bitmaps)
{
  if (bitmaps[IMG_BITMAP_PTR_ORIGINAL] != NULL)
    SDLCreateBitmapTextures(bitmaps[IMG_BITMAP_PTR_ORIGINAL]);
  else
    SDLCreateBitmapTextures(bitmaps[IMG_BITMAP_STANDARD]);
}

void FreeBitmapTextures(Bitmap **bitmaps)
{
  if (bitmaps[IMG_BITMAP_PTR_ORIGINAL] != NULL)
    SDLFreeBitmapTextures(bitmaps[IMG_BITMAP_PTR_ORIGINAL]);
  else
    SDLFreeBitmapTextures(bitmaps[IMG_BITMAP_STANDARD]);
}

void ScaleBitmap(Bitmap **bitmaps, int zoom_factor)
{
  CreateScaledBitmaps(bitmaps, zoom_factor, 0, FALSE);
}


// ----------------------------------------------------------------------------
// mouse pointer functions
// ----------------------------------------------------------------------------

#define USE_ONE_PIXEL_PLAYFIELD_MOUSEPOINTER		0

// XPM image definitions
static const char *cursor_image_none[] =
{
  // width height num_colors chars_per_pixel
  "    16    16        3            1",

  // colors
  "X c #000000",
  ". c #ffffff",
  "  c None",

  // pixels
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",

  // hot spot
  "0,0"
};

#if USE_ONE_PIXEL_PLAYFIELD_MOUSEPOINTER
static const char *cursor_image_dot[] =
{
  // width height num_colors chars_per_pixel
  "    16    16        3            1",

  // colors
  "X c #000000",
  ". c #ffffff",
  "  c None",

  // pixels
  " X              ",
  "X.X             ",
  " X              ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",

  // hot spot
  "1,1"
};
static const char **cursor_image_playfield = cursor_image_dot;
#else
// some people complained about a "white dot" on the screen and thought it
// was a graphical error... OK, let's just remove the whole pointer :-)
static const char **cursor_image_playfield = cursor_image_none;
#endif

static const int cursor_bit_order = BIT_ORDER_MSB;

static struct MouseCursorInfo *get_cursor_from_image(const char **image)
{
  struct MouseCursorInfo *cursor;
  boolean bit_order_msb = (cursor_bit_order == BIT_ORDER_MSB);
  int header_lines = 4;
  int x, y, i;

  cursor = checked_calloc(sizeof(struct MouseCursorInfo));

  sscanf(image[0], " %d %d ", &cursor->width, &cursor->height);

  i = -1;
  for (y = 0; y < cursor->width; y++)
  {
    for (x = 0; x < cursor->height; x++)
    {
      int bit_nr = x % 8;
      int bit_mask = 0x01 << (bit_order_msb ? 7 - bit_nr : bit_nr );

      if (bit_nr == 0)
      {
        i++;
        cursor->data[i] = cursor->mask[i] = 0;
      }

      switch (image[header_lines + y][x])
      {
        case 'X':
	  cursor->data[i] |= bit_mask;
	  cursor->mask[i] |= bit_mask;
	  break;

        case '.':
	  cursor->mask[i] |= bit_mask;
	  break;

        case ' ':
	  break;
      }
    }
  }

  sscanf(image[header_lines + y], "%d,%d", &cursor->hot_x, &cursor->hot_y);

  return cursor;
}

void SetMouseCursor(int mode)
{
  static struct MouseCursorInfo *cursor_none = NULL;
  static struct MouseCursorInfo *cursor_playfield = NULL;
  struct MouseCursorInfo *cursor_new;
  int mode_final = mode;

  if (cursor_none == NULL)
    cursor_none = get_cursor_from_image(cursor_image_none);

  if (cursor_playfield == NULL)
    cursor_playfield = get_cursor_from_image(cursor_image_playfield);

  if (gfx.cursor_mode_override != CURSOR_UNDEFINED)
    mode_final = gfx.cursor_mode_override;

  cursor_new = (mode_final == CURSOR_DEFAULT   ? NULL :
		mode_final == CURSOR_NONE      ? cursor_none :
		mode_final == CURSOR_PLAYFIELD ? cursor_playfield : NULL);

  SDLSetMouseCursor(cursor_new);

  gfx.cursor_mode = mode;
  gfx.cursor_mode_final = mode_final;
}

void UpdateRawMousePosition(int mouse_x, int mouse_y)
{
  // mouse events do not contain logical screen size corrections yet
  SDLCorrectRawMousePosition(&mouse_x, &mouse_y);

  mouse_x -= video.screen_xoffset;
  mouse_y -= video.screen_yoffset;

  gfx.mouse_x = mouse_x;
  gfx.mouse_y = mouse_y;
}

void UpdateMousePosition(void)
{
  int mouse_x, mouse_y;

  SDL_PumpEvents();
  SDL_GetMouseState(&mouse_x, &mouse_y);

  UpdateRawMousePosition(mouse_x, mouse_y);
}


// ============================================================================
// audio functions
// ============================================================================

void OpenAudio(void)
{
  // always start with reliable default values
  audio.sound_available = FALSE;
  audio.music_available = FALSE;
  audio.loops_available = FALSE;

  audio.sound_enabled = FALSE;
  audio.sound_deactivated = FALSE;

  audio.mixer_pipe[0] = audio.mixer_pipe[1] = 0;
  audio.mixer_pid = 0;
  audio.device_name = NULL;
  audio.device_fd = -1;

  audio.sample_rate = DEFAULT_AUDIO_SAMPLE_RATE;

  audio.num_channels = 0;
  audio.music_channel = 0;
  audio.first_sound_channel = 0;

  SDLOpenAudio();
}

void CloseAudio(void)
{
  SDLCloseAudio();

  audio.sound_enabled = FALSE;
}

void SetAudioMode(boolean enabled)
{
  if (!audio.sound_available)
    return;

  audio.sound_enabled = enabled;
}


// ============================================================================
// event functions
// ============================================================================

void InitEventFilter(EventFilter filter_function)
{
  SDL_SetEventFilter(filter_function, NULL);
}

boolean PendingEvent(void)
{
  return (SDL_PollEvent(NULL) ? TRUE : FALSE);
}

void WaitEvent(Event *event)
{
  SDLWaitEvent(event);
}

void PeekEvent(Event *event)
{
  SDL_PeepEvents(event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
}

void PumpEvents(void)
{
  SDL_PumpEvents();
}

void CheckQuitEvent(void)
{
  if (SDL_QuitRequested())
    program.exit_function(0);
}

Key GetEventKey(KeyEvent *event)
{
  // key up/down events in SDL2 do not return text characters anymore
  return event->keysym.sym;
}

KeyMod HandleKeyModState(Key key, int key_status)
{
  static KeyMod current_modifiers = KMOD_None;

  if (key != KSYM_UNDEFINED)	// new key => check for modifier key change
  {
    KeyMod new_modifier = KMOD_None;

    switch (key)
    {
      case KSYM_Shift_L:
	new_modifier = KMOD_Shift_L;
	break;
      case KSYM_Shift_R:
	new_modifier = KMOD_Shift_R;
	break;
      case KSYM_Control_L:
	new_modifier = KMOD_Control_L;
	break;
      case KSYM_Control_R:
	new_modifier = KMOD_Control_R;
	break;
      case KSYM_Meta_L:
	new_modifier = KMOD_Meta_L;
	break;
      case KSYM_Meta_R:
	new_modifier = KMOD_Meta_R;
	break;
      case KSYM_Alt_L:
	new_modifier = KMOD_Alt_L;
	break;
      case KSYM_Alt_R:
	new_modifier = KMOD_Alt_R;
	break;
      default:
	break;
    }

    if (key_status == KEY_PRESSED)
      current_modifiers |= new_modifier;
    else
      current_modifiers &= ~new_modifier;
  }

  return current_modifiers;
}

KeyMod GetKeyModState(void)
{
  return (KeyMod)SDL_GetModState();
}

KeyMod GetKeyModStateFromEvents(void)
{
  /* always use key modifier state as tracked from key events (this is needed
     if the modifier key event was injected into the event queue, but the key
     was not really pressed on keyboard -- SDL_GetModState() seems to directly
     query the keys as held pressed on the keyboard) -- this case is currently
     only used to filter out clipboard insert events from "True X-Mouse" tool */

  return HandleKeyModState(KSYM_UNDEFINED, 0);
}

void StartTextInput(int x, int y, int width, int height)
{
  textinput_status = TRUE;

#if defined(HAS_SCREEN_KEYBOARD)
  SDL_StartTextInput();

  if (y + height > SCREEN_KEYBOARD_POS(video.height))
  {
    video.shifted_up_pos = y + height - SCREEN_KEYBOARD_POS(video.height);
    video.shifted_up_delay.count = SDL_GetTicks();
    video.shifted_up = TRUE;
  }
#endif
}

void StopTextInput(void)
{
  textinput_status = FALSE;

#if defined(HAS_SCREEN_KEYBOARD)
  SDL_StopTextInput();

  if (video.shifted_up)
  {
    video.shifted_up_pos = 0;
    video.shifted_up_delay.count = SDL_GetTicks();
    video.shifted_up = FALSE;
  }
#endif
}

void PushUserEvent(int code, int value1, int value2)
{
  UserEvent event;

  SDL_memset(&event, 0, sizeof(event));

  event.type = EVENT_USER;
  event.code = code;
  event.value1 = value1;
  event.value2 = value2;

  SDL_PushEvent((SDL_Event *)&event);
}

void PushDropEvent(char *file)
{
  SDL_DropEvent event;

  SDL_memset(&event, 0, sizeof(event));

  event.type = SDL_DROPBEGIN;
  event.file = NULL;

  SDL_PushEvent((SDL_Event *)&event);

  event.type = SDL_DROPFILE;
  event.file = getStringCopy(file);

  SDL_PushEvent((SDL_Event *)&event);

  event.type = SDL_DROPCOMPLETE;
  event.file = NULL;

  SDL_PushEvent((SDL_Event *)&event);
}

boolean PendingEscapeKeyEvent(void)
{
  if (PendingEvent())
  {
    Event event;

    // check if any key press event is pending
    if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN) != 1)
      return FALSE;

    // check if pressed key is "Escape" key
    if (event.key.keysym.sym == KSYM_Escape)
      return TRUE;
  }

  return FALSE;
}


// ============================================================================
// joystick functions
// ============================================================================

void InitJoysticks(void)
{
  int i;

#if defined(NO_JOYSTICK)
  return;	// joysticks generally deactivated by compile-time directive
#endif

  // always start with reliable default values
  joystick.status = JOYSTICK_NOT_AVAILABLE;
  for (i = 0; i < MAX_PLAYERS; i++)
    joystick.nr[i] = -1;		// no joystick configured

  SDLInitJoysticks();
}

boolean ReadJoystick(int nr, int *x, int *y, boolean *b1, boolean *b2)
{
  return SDLReadJoystick(nr, x, y, b1, b2);
}

boolean CheckJoystickOpened(int nr)
{
  return SDLCheckJoystickOpened(nr);
}

void ClearJoystickState(void)
{
  SDLClearJoystickState();
}


// ============================================================================
// Emscripten functions
// ============================================================================

void InitEmscriptenFilesystem(void)
{
#if defined(PLATFORM_EMSCRIPTEN)
  EM_ASM
  ({
    dir = UTF8ToString($0);

    Module.sync_done = 0;

    FS.mkdir(dir);			// create persistent data directory
    FS.mount(IDBFS, {}, dir);		// mount with IDBFS filesystem type
    FS.syncfs(true, function(err)	// sync persistent data into memory
    {
      assert(!err);
      Module.sync_done = 1;
    });
  }, PERSISTENT_DIRECTORY);

  // wait for persistent data to be synchronized to memory
  while (emscripten_run_script_int("Module.sync_done") == 0)
    Delay(20);
#endif
}

void SyncEmscriptenFilesystem(void)
{
#if defined(PLATFORM_EMSCRIPTEN)
  EM_ASM
  (
    FS.syncfs(function(err)
    {
      assert(!err);
    });
  );
#endif
}
