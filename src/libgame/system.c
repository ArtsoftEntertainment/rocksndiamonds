/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* system.c                                                 *
***********************************************************/

#include <string.h>
#include <signal.h>

#include "platform.h"

#if defined(PLATFORM_MSDOS)
#include <fcntl.h>
#endif

#include "system.h"
#include "sound.h"
#include "misc.h"


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

struct ProgramInfo	program;
struct OptionInfo	options;
struct VideoSystemInfo	video;
struct AudioSystemInfo	audio;
struct GfxInfo		gfx;

struct LevelDirInfo    *leveldir_first = NULL;
struct LevelDirInfo    *leveldir_current = NULL;

Display        *display = NULL;
Visual	       *visual = NULL;
int		screen = 0;
Colormap	cmap = None;

DrawWindow     *window = NULL;
DrawBuffer     *backbuffer = NULL;
DrawBuffer     *drawto = NULL;

int		button_status = MB_NOT_PRESSED;
boolean		motion_status = FALSE;

int		redraw_mask = REDRAW_NONE;
int		redraw_tiles = 0;

int		FrameCounter = 0;


/* ========================================================================= */
/* init/close functions                                                      */
/* ========================================================================= */

void InitCommandName(char *argv0)
{
  program.command_basename =
    (strrchr(argv0, '/') ? strrchr(argv0, '/') + 1 : argv0);
}

void InitExitFunction(void (*exit_function)(int))
{
  program.exit_function = exit_function;

  /* set signal handlers to custom exit function */
  signal(SIGINT, exit_function);
  signal(SIGTERM, exit_function);

#if defined(TARGET_SDL)
  /* set exit function to automatically cleanup SDL stuff after exit() */
  atexit(SDL_Quit);
#endif
}

void InitPlatformDependantStuff(void)
{
#if defined(PLATFORM_MSDOS)
  _fmode = O_BINARY;
#endif

#if !defined(PLATFORM_UNIX)
  program.userdata_directory = "userdata";
  initErrorFile();
#endif

#if defined(TARGET_SDL)
  if (SDL_Init(SDL_INIT_EVENTTHREAD | SDL_INIT_NOPARACHUTE) < 0)
    Error(ERR_EXIT, "SDL_Init() failed: %s", SDL_GetError());
#endif
}

void ClosePlatformDependantStuff(void)
{
#if !defined(PLATFORM_UNIX)
  dumpErrorFile();
#endif
}

void InitProgramInfo(char *unix_userdata_directory, char *program_title,
		     char *window_title, char *icon_title,
		     char *x11_icon_basename, char *x11_iconmask_basename,
		     char *msdos_pointer_basename)
{
  char *gfx_dir = getPath2(options.ro_base_directory, GRAPHICS_DIRECTORY);
  char *x11_icon_filename = getPath2(gfx_dir, x11_icon_basename);
  char *x11_iconmask_filename = getPath2(gfx_dir, x11_iconmask_basename);
  char *msdos_pointer_filename = getPath2(gfx_dir, msdos_pointer_basename);

  free(gfx_dir);

#if defined(PLATFORM_UNIX)
  program.userdata_directory = unix_userdata_directory;
#else
  program.userdata_directory = "userdata";
#endif

  program.program_title = program_title;
  program.window_title = window_title;
  program.icon_title = icon_title;
  program.x11_icon_filename = x11_icon_filename;
  program.x11_iconmask_filename = x11_iconmask_filename;
  program.msdos_pointer_filename = msdos_pointer_filename;
}

void InitGfxFieldInfo(int sx, int sy, int sxsize, int sysize,
		      int real_sx, int real_sy,
		      int full_sxsize, int full_sysize)
{
  gfx.sx = sx;
  gfx.sy = sy;
  gfx.sxsize = sxsize;
  gfx.sysize = sysize;
  gfx.real_sx = real_sx;
  gfx.real_sy = real_sy;
  gfx.full_sxsize = full_sxsize;
  gfx.full_sysize = full_sysize;
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

void InitGfxScrollbufferInfo(int scrollbuffer_width, int scrollbuffer_height)
{
  /* currently only used by MSDOS code to alloc VRAM buffer, if available */
  gfx.scrollbuffer_width = scrollbuffer_width;
  gfx.scrollbuffer_height = scrollbuffer_height;
}


/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

inline static int GetRealDepth(int depth)
{
  return (depth == DEFAULT_DEPTH ? video.default_depth : depth);
}

inline void InitVideoDisplay(void)
{
#if defined(TARGET_SDL)
  SDLInitVideoDisplay();
#else
  X11InitVideoDisplay();
#endif
}

inline void CloseVideoDisplay(void)
{
  KeyboardAutoRepeatOn();

#if defined(TARGET_SDL)
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
#else
  if (display)
    XCloseDisplay(display);
#endif
}

inline void InitVideoBuffer(DrawBuffer **backbuffer, DrawWindow **window,
			    int width, int height, int depth,
			    boolean fullscreen)
{
  video.width = width;
  video.height = height;
  video.depth = GetRealDepth(depth);
  video.fullscreen_available = FULLSCREEN_STATUS;
  video.fullscreen_enabled = FALSE;

#ifdef TARGET_SDL
  SDLInitVideoBuffer(backbuffer, window, fullscreen);
#else
  X11InitVideoBuffer(backbuffer, window);
#endif
}

inline Bitmap *CreateBitmapStruct(void)
{
#ifdef TARGET_SDL
  return checked_calloc(sizeof(struct SDLSurfaceInfo));
#else
  return checked_calloc(sizeof(struct X11DrawableInfo));
#endif
}

inline Bitmap *CreateBitmap(int width, int height, int depth)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  int real_depth = GetRealDepth(depth);

#ifdef TARGET_SDL
  SDL_Surface *surface_tmp, *surface_native;

  if ((surface_tmp = SDL_CreateRGBSurface(SURFACE_FLAGS, width, height,
					  real_depth, 0, 0, 0, 0))
      == NULL)
    Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

  if ((surface_native = SDL_DisplayFormat(surface_tmp)) == NULL)
    Error(ERR_EXIT, "SDL_DisplayFormat() failed: %s", SDL_GetError());

  SDL_FreeSurface(surface_tmp);

  new_bitmap->surface = surface_native;
#else
  Pixmap pixmap;

  if ((pixmap = XCreatePixmap(display, window->drawable,
			      width, height, real_depth))
      == None)
    Error(ERR_EXIT, "cannot create pixmap");

  new_bitmap->drawable = pixmap;

  if (window == NULL)
    Error(ERR_EXIT, "Window GC needed for Bitmap -- create Window first");

  new_bitmap->gc = window->gc;

  new_bitmap->line_gc[0] = window->line_gc[0];
  new_bitmap->line_gc[1] = window->line_gc[1];
#endif

  return new_bitmap;
}

inline void FreeBitmap(Bitmap *bitmap)
{
  if (bitmap == NULL)
    return;

#ifdef TARGET_SDL
  if (bitmap->surface)
    SDL_FreeSurface(bitmap->surface);
  if (bitmap->surface_masked)
    SDL_FreeSurface(bitmap->surface_masked);
#else
  if (bitmap->drawable)
    XFreePixmap(display, bitmap->drawable);
  if (bitmap->clip_mask)
    XFreePixmap(display, bitmap->clip_mask);
  if (bitmap->stored_clip_gc)
    XFreeGC(display, bitmap->stored_clip_gc);
#endif

  free(bitmap);
}

inline void CloseWindow(DrawWindow *window)
{
#ifdef TARGET_X11
  if (window->drawable)
  {
    XUnmapWindow(display, window->drawable);
    XDestroyWindow(display, window->drawable);
  }
  if (window->gc)
    XFreeGC(display, window->gc);
#endif
}

inline void BlitBitmap(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		       int src_x, int src_y,
		       int width, int height,
		       int dst_x, int dst_y)
{
#ifdef TARGET_SDL
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y, SDLCOPYAREA_OPAQUE);
#else
  XCopyArea(display, src_bitmap->drawable, dst_bitmap->drawable,
	    dst_bitmap->gc, src_x, src_y, width, height, dst_x, dst_y);
#endif
}

inline void ClearRectangle(Bitmap *bitmap, int x, int y, int width, int height)
{
#ifdef TARGET_SDL
  SDLFillRectangle(bitmap, x, y, width, height, 0x000000);
#else
  XFillRectangle(display, bitmap->drawable, bitmap->gc, x, y, width, height);
#endif
}

#if 0
#ifndef TARGET_SDL
static GC last_clip_gc = 0;	/* needed for XCopyArea() through clip mask */
#endif
#endif

inline void SetClipMask(Bitmap *bitmap, GC clip_gc, Pixmap clip_pixmap)
{
#ifdef TARGET_X11
  if (clip_gc)
  {
    bitmap->clip_gc = clip_gc;
    XSetClipMask(display, bitmap->clip_gc, clip_pixmap);
  }
#if 0
  last_clip_gc = clip_gc;
#endif
#endif
}

inline void SetClipOrigin(Bitmap *bitmap, GC clip_gc, int clip_x, int clip_y)
{
#ifdef TARGET_X11
  if (clip_gc)
  {
    bitmap->clip_gc = clip_gc;
    XSetClipOrigin(display, bitmap->clip_gc, clip_x, clip_y);
  }
#if 0
  last_clip_gc = clip_gc;
#endif
#endif
}

inline void BlitBitmapMasked(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			     int src_x, int src_y,
			     int width, int height,
			     int dst_x, int dst_y)
{
#ifdef TARGET_SDL
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y, SDLCOPYAREA_MASKED);
#else
  XCopyArea(display, src_bitmap->drawable, dst_bitmap->drawable,
	    src_bitmap->clip_gc, src_x, src_y, width, height, dst_x, dst_y);
#endif
}

inline void DrawSimpleWhiteLine(Bitmap *bitmap, int from_x, int from_y,
				int to_x, int to_y)
{
#ifdef TARGET_SDL
  SDLDrawSimpleLine(bitmap->surface, from_x, from_y, to_x, to_y, 0xffffff);
#else
  XSetForeground(display, bitmap->gc, WhitePixel(display, screen));
  XDrawLine(display, bitmap->drawable, bitmap->gc, from_x, from_y, to_x, to_y);
  XSetForeground(display, bitmap->gc, BlackPixel(display, screen));
#endif
}

#if !defined(TARGET_X11_NATIVE)
inline void DrawLine(Bitmap *bitmap, int from_x, int from_y,
		     int to_x, int to_y, Pixel pixel, int line_width)
{
  int x, y;

  for (x=0; x<line_width; x++)
  {
    for (y=0; y<line_width; y++)
    {
      int dx = x - line_width / 2;
      int dy = y - line_width / 2;

      if ((x == 0 && y == 0) ||
	  (x == 0 && y == line_width - 1) ||
	  (x == line_width - 1 && y == 0) ||
	  (x == line_width - 1 && y == line_width - 1))
	continue;

#if defined(TARGET_SDL)
      sge_Line(bitmap->surface,
	       from_x + dx, from_y + dy, to_x + dx, to_y + dy, pixel);
#elif defined(TARGET_ALLEGRO)
      AllegroDrawLine(bitmap->drawable, from_x + dx, from_y + dy,
		      to_x + dx, to_y + dy, pixel);
#endif
    }
  }
}
#endif

inline void DrawLines(Bitmap *bitmap, struct XY *points, int num_points,
		      Pixel pixel)
{
#if !defined(TARGET_X11_NATIVE)
  int line_width = 4;
  int i;

  for (i=0; i<num_points - 1; i++)
    DrawLine(bitmap, points[i].x, points[i].y,
	     points[i + 1].x, points[i + 1].y, pixel, line_width);

  /*
  SDLDrawLines(bitmap->surface, points, num_points, pixel);
  */
#else
  XSetForeground(display, bitmap->line_gc[1], pixel);
  XDrawLines(display, bitmap->drawable, bitmap->line_gc[1],
	     (XPoint *)points, num_points, CoordModeOrigin);
  /*
  XSetForeground(display, gc, BlackPixel(display, screen));
  */
#endif
}

inline Pixel GetPixelFromRGB(Bitmap *bitmap, unsigned int color_r,
			     unsigned int color_g, unsigned int color_b)
{
  Pixel pixel;

#if defined(TARGET_SDL)
  pixel = SDL_MapRGB(bitmap->surface->format, color_r, color_g, color_b);
#elif defined(TARGET_ALLEGRO)
  pixel = AllegroAllocColorCell(color_r << 8, color_g << 8, color_b << 8);
#elif defined(TARGET_X11_NATIVE)
  XColor xcolor;

  xcolor.flags = DoRed | DoGreen | DoBlue;
  xcolor.red = (color_r << 8);
  xcolor.green = (color_g << 8);
  xcolor.blue = (color_b << 8);
  XAllocColor(display, cmap, &xcolor);
  pixel = xcolor.pixel;
#endif

  return pixel;
}

inline Pixel GetPixelFromRGBcompact(Bitmap *bitmap, unsigned int color)
{
  unsigned int color_r = (color >> 16) & 0xff;
  unsigned int color_g = (color >>  8) & 0xff;
  unsigned int color_b = (color >>  0) & 0xff;

  return GetPixelFromRGB(bitmap, color_r, color_g, color_b);
}

/* execute all pending screen drawing operations */
inline void FlushDisplay(void)
{
#ifndef TARGET_SDL
  XFlush(display);
#endif
}

/* execute and wait for all pending screen drawing operations */
inline void SyncDisplay(void)
{
#ifndef TARGET_SDL
  XSync(display, FALSE);
#endif
}

inline void KeyboardAutoRepeatOn(void)
{
#ifdef TARGET_SDL
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2,
		      SDL_DEFAULT_REPEAT_INTERVAL / 2);
  SDL_EnableUNICODE(1);
#else
  if (display)
    XAutoRepeatOn(display);
#endif
}

inline void KeyboardAutoRepeatOff(void)
{
#ifdef TARGET_SDL
  SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(0);
#else
  if (display)
    XAutoRepeatOff(display);
#endif
}

inline boolean PointerInWindow(DrawWindow *window)
{
#ifdef TARGET_SDL
  return TRUE;
#else
  Window root, child;
  int root_x, root_y;
  unsigned int mask;
  int win_x, win_y;

  /* if XQueryPointer() returns False, the pointer
     is not on the same screen as the specified window */
  return XQueryPointer(display, window->drawable, &root, &child,
		       &root_x, &root_y, &win_x, &win_y, &mask);
#endif
}

inline boolean SetVideoMode(boolean fullscreen)
{
#ifdef TARGET_SDL
  return SDLSetVideoMode(&backbuffer, fullscreen);
#else
  boolean success = TRUE;

  if (fullscreen && video.fullscreen_available)
  {
    Error(ERR_WARN, "fullscreen not available in X11 version");

    /* display error message only once */
    video.fullscreen_available = FALSE;

    success = FALSE;
  }

  return success;
#endif
}

inline boolean ChangeVideoModeIfNeeded(boolean fullscreen)
{
#ifdef TARGET_SDL
  if ((fullscreen && !video.fullscreen_enabled && video.fullscreen_available)||
      (!fullscreen && video.fullscreen_enabled))
    fullscreen = SetVideoMode(fullscreen);
#endif

  return fullscreen;
}

Bitmap *LoadImage(char *basename)
{
  Bitmap *new_bitmap;
  char *filename = getPath3(options.ro_base_directory, GRAPHICS_DIRECTORY,
			    basename);

#if defined(TARGET_SDL)
  new_bitmap = SDLLoadImage(filename);
#else
  new_bitmap = X11LoadImage(filename);
#endif

  free(filename);

  return new_bitmap;
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

inline void OpenAudio(void)
{
  /* always start with reliable default values */
  audio.sound_available = FALSE;
  audio.music_available = FALSE;
  audio.loops_available = FALSE;
  audio.mods_available = FALSE;
  audio.sound_enabled = FALSE;

  audio.soundserver_pipe[0] = audio.soundserver_pipe[1] = 0;
  audio.soundserver_pid = 0;
  audio.device_name = NULL;
  audio.device_fd = 0;

  audio.channels = 0;
  audio.music_channel = 0;
  audio.music_nr = 0;

#if defined(TARGET_SDL)
  SDLOpenAudio();
#elif defined(PLATFORM_MSDOS)
  MSDOSOpenAudio();
#elif defined(PLATFORM_UNIX)
  UnixOpenAudio();
#endif
}

inline void CloseAudio(void)
{
#if defined(TARGET_SDL)
  SDLCloseAudio();
#elif defined(PLATFORM_MSDOS)
  MSDOSCloseAudio();
#elif defined(PLATFORM_UNIX)
  UnixCloseAudio();
#endif

  audio.sound_enabled = FALSE;
}

inline void SetAudioMode(boolean enabled)
{
  if (!audio.sound_available)
    return;

  audio.sound_enabled = enabled;
}


/* ========================================================================= */
/* event functions                                                           */
/* ========================================================================= */

inline void InitEventFilter(EventFilter filter_function)
{
#ifdef TARGET_SDL
  /* set event filter to filter out certain events */
  SDL_SetEventFilter(filter_function);
#endif
}

inline boolean PendingEvent(void)
{
#ifdef TARGET_SDL
  return (SDL_PollEvent(NULL) ? TRUE : FALSE);
#else
  return (XPending(display) ? TRUE : FALSE);
#endif
}

inline void NextEvent(Event *event)
{
#ifdef TARGET_SDL
  SDL_WaitEvent(event);
#else
  XNextEvent(display, event);
#endif
}

inline Key GetEventKey(KeyEvent *event, boolean with_modifiers)
{
#ifdef TARGET_SDL
#if 0
  printf("unicode == '%d', sym == '%d', mod == '0x%04x'\n",
	 (int)event->keysym.unicode,
	 (int)event->keysym.sym,
	 (int)SDL_GetModState());
#endif

  if (with_modifiers && event->keysym.unicode != 0)
    return event->keysym.unicode;
  else
    return event->keysym.sym;
#else
#if 0
  printf("with modifiers == '0x%04x', without modifiers == '0x%04x'\n",
	 (int)XLookupKeysym(event, event->state),
	 (int)XLookupKeysym(event, 0));
#endif

  if (with_modifiers)
    return XLookupKeysym(event, event->state);
  else
    return XLookupKeysym(event, 0);
#endif
}

inline boolean CheckCloseWindowEvent(ClientMessageEvent *event)
{
  if (event->type != EVENT_CLIENTMESSAGE)
    return FALSE;

#if defined(TARGET_SDL)
  return TRUE;		/* the only possible message here is SDL_QUIT */
#elif defined(PLATFORM_UNIX)
  if ((event->window == window->drawable) &&
      (event->data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", FALSE)))
    return TRUE;
#endif

  return FALSE;
}


inline void dummy(void)
{
#ifdef TARGET_SDL
#else
#endif
}
