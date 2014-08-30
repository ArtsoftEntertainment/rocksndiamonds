/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  init.c                                                  *
***********************************************************/

#include <signal.h>

#include "init.h"
#include "misc.h"
#include "sound.h"
#include "screens.h"
#include "tools.h"
#include "files.h"
#include "joystick.h"
#include "gfxload.h"
#include "gifload.h"
#include "network.h"

#ifdef DEBUG
/*
#define DEBUG_TIMING
*/
#endif

struct PictureFileInfo
{
  char *picture_filename;
  BOOL picture_with_mask;
};

struct IconFileInfo
{
  char *picture_filename;
  char *picturemask_filename;
};

static int sound_process_id = 0;

static void InitServer(void);
static void InitLevelAndPlayerInfo(void);
static void InitDisplay(void);
static void InitSound(void);
static void InitSoundProcess(void);
static void InitWindow(int, char **);
static void InitGfx(void);
static void LoadGfx(int, struct PictureFileInfo *);
static void InitElementProperties(void);

void OpenAll(int argc, char *argv[])
{
  InitLevelAndPlayerInfo();
  InitServer();

  InitCounter();
  InitSound();
  InitSoundProcess();
  InitJoystick();
  InitRND(NEW_RANDOMIZE);

  signal(SIGINT, CloseAllAndExit);
  signal(SIGTERM, CloseAllAndExit);

  InitDisplay();
  InitWindow(argc, argv);

  XMapWindow(display, window);
  XFlush(display);

  InitGfx();
  InitElementProperties();

  DrawMainMenu();
}

void InitLevelAndPlayerInfo()
{
  local_player = &stored_player[0];

  if (!LoadLevelInfo())			/* global level info */
    Error(ERR_EXIT, NULL);

  LoadPlayerInfo(PLAYER_SETUP);		/* global setup info */
  LoadPlayerInfo(PLAYER_LEVEL);		/* level specific info */
}

void InitServer()
{
  standalone = FALSE + TRUE;

  if (standalone)
    return;

  if (!ConnectToServer(server_host, server_port))
    Error(ERR_EXIT, "cannot connect to multiplayer server");

  SendNicknameToServer(local_player->alias_name);
  SendProtocolVersionToServer();
}

void InitSound()
{
  int i;

  if (sound_status==SOUND_OFF)
    return;

#ifndef MSDOS
  if (access(sound_device_name,W_OK)<0)
  {
    Error(ERR_RETURN, "cannot access sound device - no sounds");
    sound_status = SOUND_OFF;
    return;
  }

  if ((sound_device=open(sound_device_name,O_WRONLY))<0)
  {
    Error(ERR_RETURN, "cannot open sound device - no sounds");
    sound_status = SOUND_OFF;
    return;
  }

  close(sound_device);
  sound_status=SOUND_AVAILABLE;

#ifdef VOXWARE
  sound_loops_allowed = TRUE;
  sound_loops_on = TRUE;
#endif
#else
  sound_loops_allowed = TRUE;
  sound_loops_on = TRUE;
#endif

  for(i=0;i<NUM_SOUNDS;i++)
  {
#ifdef MSDOS
  sprintf(sound_name[i], "%d", i+1);
#endif
    Sound[i].name = sound_name[i];
    if (!LoadSound(&Sound[i]))
    {
      sound_status=SOUND_OFF;
      return;
    }
  }
}

void InitSoundProcess()
{
  if (sound_status==SOUND_OFF)
    return;

#ifndef MSDOS
  if (pipe(sound_pipe)<0)
  {
    Error(ERR_RETURN, "cannot create pipe - no sounds");
    sound_status=SOUND_OFF;
    return;
  }

  if ((sound_process_id=fork())<0)
  {       
    Error(ERR_RETURN, "cannot create child process - no sounds");
    sound_status=SOUND_OFF;
    return;
  }

  if (!sound_process_id)	/* we are child */
    SoundServer();
  else				/* we are parent */
    close(sound_pipe[0]);	/* no reading from pipe needed */
#else
  SoundServer();
#endif
}

void InitJoystick()
{
  if (global_joystick_status==JOYSTICK_OFF)
    return;

#ifndef MSDOS
  if (access(joystick_device_name[joystick_nr],R_OK)<0)
  {
    Error(ERR_RETURN, "cannot access joystick device '%s'",
	  joystick_device_name[joystick_nr]);
    joystick_status = JOYSTICK_OFF;
    return;
  }

  if ((joystick_device=open(joystick_device_name[joystick_nr],O_RDONLY))<0)
  {
    Error(ERR_RETURN, "cannot open joystick device '%s'",
	  joystick_device_name[joystick_nr]);
    joystick_status = JOYSTICK_OFF;
    return;
  }

  joystick_status = JOYSTICK_AVAILABLE;
  LoadJoystickData();
#else
  joystick_status = JOYSTICK_AVAILABLE;
#endif
}

void InitDisplay()
{
  XVisualInfo vinfo_template, *vinfo;
  int num_visuals;
  unsigned int depth;

  /* connect to X server */
  if (!(display = XOpenDisplay(display_name)))
    Error(ERR_EXIT,"cannot connect to X server %s",XDisplayName(display_name));

  screen = DefaultScreen(display);
  visual = DefaultVisual(display, screen);
  depth  = DefaultDepth(display, screen);
  cmap   = DefaultColormap(display, screen);

  /* look for good enough visual */
  vinfo_template.screen = screen;
  vinfo_template.class = (depth == 8 ? PseudoColor : TrueColor);
  vinfo_template.depth = depth;
  if ((vinfo = XGetVisualInfo(display, VisualScreenMask | VisualClassMask |
			      VisualDepthMask, &vinfo_template, &num_visuals)))
  {
    visual = vinfo->visual;
    XFree((void *)vinfo);
  }

  /* got appropriate visual? */
  if (depth < 8)
  {
    printf("Sorry, displays with less than 8 bits per pixel not supported.\n");
    exit(-1);
  }
  else if ((depth ==8 && visual->class != PseudoColor) ||
	   (depth > 8 && visual->class != TrueColor &&
	    visual->class != DirectColor))
  {
    printf("Sorry, cannot get appropriate visual.\n");
    exit(-1);
  }
}

void InitWindow(int argc, char *argv[])
{
  unsigned int border_width = 4;
  Pixmap icon_pixmap, iconmask_pixmap;
  unsigned int icon_width,icon_height;
  int icon_hot_x,icon_hot_y;
  char icon_filename[256];
  XSizeHints size_hints;
  XWMHints wm_hints;
  XClassHint class_hints;
  XTextProperty windowName, iconName;
  XGCValues gc_values;
  unsigned long gc_valuemask;
  char *window_name = WINDOWTITLE_STRING;
  char *icon_name = WINDOWTITLE_STRING;
  long window_event_mask;
  Atom proto_atom = None, delete_atom = None;
  int screen_width, screen_height;
  int win_xpos = WIN_XPOS, win_ypos = WIN_YPOS;
  unsigned long pen_fg = WhitePixel(display,screen);
  unsigned long pen_bg = BlackPixel(display,screen);

#ifndef MSDOS
  static struct IconFileInfo icon_pic =
  {
    "rocks_icon.xbm",
    "rocks_iconmask.xbm"
  };
#endif

  screen_width = XDisplayWidth(display, screen);
  screen_height = XDisplayHeight(display, screen);

  width = WIN_XSIZE;
  height = WIN_YSIZE;

  win_xpos = (screen_width - width) / 2;
  win_ypos = (screen_height - height) / 2;

  window = XCreateSimpleWindow(display, RootWindow(display, screen),
			       win_xpos, win_ypos, width, height, border_width,
			       pen_fg, pen_bg);

#ifndef MSDOS
  proto_atom = XInternAtom(display, "WM_PROTOCOLS", FALSE);
  delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
  if ((proto_atom != None) && (delete_atom != None))
    XChangeProperty(display, window, proto_atom, XA_ATOM, 32,
		    PropModePrepend, (unsigned char *) &delete_atom, 1);

  sprintf(icon_filename,"%s/%s",GFX_PATH,icon_pic.picture_filename);
  XReadBitmapFile(display,window,icon_filename,
		  &icon_width,&icon_height,
		  &icon_pixmap,&icon_hot_x,&icon_hot_y);
  if (!icon_pixmap)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'", icon_filename);

  sprintf(icon_filename,"%s/%s",GFX_PATH,icon_pic.picturemask_filename);
  XReadBitmapFile(display,window,icon_filename,
		  &icon_width,&icon_height,
		  &iconmask_pixmap,&icon_hot_x,&icon_hot_y);
  if (!iconmask_pixmap)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'", icon_filename);

  size_hints.width  = size_hints.min_width  = size_hints.max_width  = width;
  size_hints.height = size_hints.min_height = size_hints.max_height = height;
  size_hints.flags = PSize | PMinSize | PMaxSize;

  if (win_xpos || win_ypos)
  {
    size_hints.x = win_xpos;
    size_hints.y = win_ypos;
    size_hints.flags |= PPosition;
  }

  if (!XStringListToTextProperty(&window_name, 1, &windowName))
    Error(ERR_EXIT, "structure allocation for windowName failed");

  if (!XStringListToTextProperty(&icon_name, 1, &iconName))
    Error(ERR_EXIT, "structure allocation for iconName failed");

  wm_hints.initial_state = NormalState;
  wm_hints.input = True;
  wm_hints.icon_pixmap = icon_pixmap;
  wm_hints.icon_mask = iconmask_pixmap;
  wm_hints.flags = StateHint | IconPixmapHint | IconMaskHint | InputHint;

  class_hints.res_name = program_name;
  class_hints.res_class = "Rocks'n'Diamonds";

  XSetWMProperties(display, window, &windowName, &iconName, 
		   argv, argc, &size_hints, &wm_hints, 
		   &class_hints);

  XFree(windowName.value);
  XFree(iconName.value);

  /* Select event types wanted */
  window_event_mask = ExposureMask | StructureNotifyMask | FocusChangeMask |
                      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                      KeyPressMask | KeyReleaseMask;
  XSelectInput(display, window, window_event_mask);
#endif

  /* create GC for drawing with window depth */
  gc_values.graphics_exposures = False;
  gc_values.foreground = pen_bg;
  gc_values.background = pen_bg;
  gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground;
  gc = XCreateGC(display, window, gc_valuemask, &gc_values);
}

void DrawInitText(char *text, int ypos, int color)
{
  if (display && window && pix[PIX_SMALLFONT])
  {
    XFillRectangle(display,window,gc,0,ypos, WIN_XSIZE,FONT2_YSIZE);
    DrawTextExt(window,gc,(WIN_XSIZE-strlen(text)*FONT2_XSIZE)/2,
		ypos,text,FS_SMALL,color);
    XFlush(display);
  }
}

void InitGfx()
{
  int i,j;
  GC copy_clipmask_gc;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;

#ifdef MSDOS
  static struct PictureFileInfo pic[NUM_PICTURES] =
  {
    { "Screen",	TRUE },
    { "Door",	TRUE },
    { "Heroes",	TRUE },
    { "Toons",	TRUE },
    { "Font",	FALSE },
    { "Font2",	FALSE }
  }; 
#else
  static struct PictureFileInfo pic[NUM_PICTURES] =
  {
    { "RocksScreen",	TRUE },
    { "RocksDoor",	TRUE },
    { "RocksHeroes",	TRUE },
    { "RocksToons",	TRUE },
    { "RocksFont",	FALSE },
    { "RocksFont2",	FALSE }
  }; 
#endif

  static struct
  {
    int start;
    int count;
  }
  tile_needs_clipping[] =
  {
    { GFX_SPIELER1_UP, 4 },
    { GFX_SPIELER1_DOWN, 4 },
    { GFX_SPIELER1_LEFT, 4 },
    { GFX_SPIELER1_RIGHT, 4 },
    { GFX_SPIELER1_PUSH_LEFT, 4 },
    { GFX_SPIELER1_PUSH_RIGHT, 4 },
    { GFX_SPIELER2_UP, 4 },
    { GFX_SPIELER2_DOWN, 4 },
    { GFX_SPIELER2_LEFT, 4 },
    { GFX_SPIELER2_RIGHT, 4 },
    { GFX_SPIELER2_PUSH_LEFT, 4 },
    { GFX_SPIELER2_PUSH_RIGHT, 4 },
    { GFX_SPIELER3_UP, 4 },
    { GFX_SPIELER3_DOWN, 4 },
    { GFX_SPIELER3_LEFT, 4 },
    { GFX_SPIELER3_RIGHT, 4 },
    { GFX_SPIELER3_PUSH_LEFT, 4 },
    { GFX_SPIELER3_PUSH_RIGHT, 4 },
    { GFX_SPIELER4_UP, 4 },
    { GFX_SPIELER4_DOWN, 4 },
    { GFX_SPIELER4_LEFT, 4 },
    { GFX_SPIELER4_RIGHT, 4 },
    { GFX_SPIELER4_PUSH_LEFT, 4 },
    { GFX_SPIELER4_PUSH_RIGHT, 4 },
    { GFX_GEBLUBBER, 4 },
    { GFX_DYNAMIT, 7 },
    { GFX_DYNABOMB, 4 },
    { GFX_EXPLOSION, 8 },
    { GFX_SOKOBAN_OBJEKT, 1 },
    { GFX_FUNKELN_BLAU, 3 },
    { GFX_FUNKELN_WEISS, 3 },
    { -1, 0 }
  };

#ifdef DEBUG_TIMING
  long count1, count2;
  count1 = Counter();
#endif

  LoadGfx(PIX_SMALLFONT,&pic[PIX_SMALLFONT]);
  DrawInitText(WINDOWTITLE_STRING,20,FC_YELLOW);
  DrawInitText(COPYRIGHT_STRING,50,FC_RED);
#ifdef MSDOS
  DrawInitText("MSDOS version done by Guido Schulz",210,FC_BLUE);
  rest(200);
#endif MSDOS
  DrawInitText("Loading graphics:",120,FC_GREEN);

  for(i=0; i<NUM_PICTURES; i++)
    if (i != PIX_SMALLFONT)
      LoadGfx(i,&pic[i]);

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("SUMMARY: %.2f SECONDS LOADING TIME\n",(float)(count2-count1)/1000.0);
#endif


  pix[PIX_DB_BACK] = XCreatePixmap(display, window,
				   WIN_XSIZE,WIN_YSIZE,
				   XDefaultDepth(display,screen));
  pix[PIX_DB_DOOR] = XCreatePixmap(display, window,
				   3*DXSIZE,DYSIZE+VYSIZE,
				   XDefaultDepth(display,screen));
  pix[PIX_DB_FIELD] = XCreatePixmap(display, window,
				    FXSIZE,FYSIZE,
				    XDefaultDepth(display,screen));

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc =
    XCreateGC(display,clipmask[PIX_BACK],clip_gc_valuemask,&clip_gc_values);

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  tile_clip_gc =
    XCreateGC(display,window,clip_gc_valuemask,&clip_gc_values);

  /* initialize pixmap array to Pixmap 'None' */
  for(i=0; i<NUM_TILES; i++)
    tile_clipmask[i] = None;

  /* create only those clipping Pixmaps we really need */
  for(i=0; tile_needs_clipping[i].start>=0; i++)
  {
    for(j=0; j<tile_needs_clipping[i].count; j++)
    {
      int tile = tile_needs_clipping[i].start + j;
      int graphic = tile;
      int src_x, src_y;
      Pixmap src_pixmap;

      if (graphic >= GFX_START_ROCKSSCREEN &&
	  graphic <= GFX_END_ROCKSSCREEN)
      {
	src_pixmap = clipmask[PIX_BACK];
	graphic -= GFX_START_ROCKSSCREEN;
	src_x = SX + (graphic % GFX_PER_LINE) * TILEX;
	src_y = SY + (graphic / GFX_PER_LINE) * TILEY;
      }
      else if (graphic >= GFX_START_ROCKSHEROES &&
	       graphic <= GFX_END_ROCKSHEROES)
      {
	src_pixmap = clipmask[PIX_HEROES];
	graphic -= GFX_START_ROCKSHEROES;
	src_x = (graphic % HEROES_PER_LINE) * TILEX;
	src_y = (graphic / HEROES_PER_LINE) * TILEY;
      }
      else if (graphic >= GFX_START_ROCKSFONT &&
	       graphic <= GFX_END_ROCKSFONT)
      {
	src_pixmap = clipmask[PIX_BIGFONT];
	graphic -= GFX_START_ROCKSFONT;
	src_x = (graphic % FONT_CHARS_PER_LINE) * TILEX;
	src_y = (graphic / FONT_CHARS_PER_LINE) * TILEY +
	  FC_SPECIAL1 * FONT_LINES_PER_FONT * TILEY;
      }
      else
	break;

      tile_clipmask[tile] = XCreatePixmap(display, window, TILEX,TILEY, 1);

      XCopyArea(display,src_pixmap,tile_clipmask[tile],copy_clipmask_gc,
		src_x,src_y, TILEX,TILEY, 0,0);
    }
  }

  if (!pix[PIX_DB_BACK] || !pix[PIX_DB_DOOR])
    Error(ERR_EXIT, "cannot create additional pixmaps");

  for(i=0;i<NUM_PIXMAPS;i++)
  {
    if (clipmask[i])
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_values.clip_mask = clipmask[i];
      clip_gc_valuemask = GCGraphicsExposures | GCClipMask;
      clip_gc[i] = XCreateGC(display,window,clip_gc_valuemask,&clip_gc_values);
    }
  }

  drawto = backbuffer = pix[PIX_DB_BACK];
  fieldbuffer = pix[PIX_DB_FIELD];
  SetDrawtoField(DRAW_BACKBUFFER);

  XCopyArea(display,pix[PIX_BACK],backbuffer,gc,
	    0,0, WIN_XSIZE,WIN_YSIZE, 0,0);
  XFillRectangle(display,pix[PIX_DB_BACK],gc,
		 REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE);
  XFillRectangle(display,pix[PIX_DB_DOOR],gc,
		 0,0, 3*DXSIZE,DYSIZE+VYSIZE);

  for(i=0; i<MAX_BUF_XSIZE; i++)
    for(j=0; j<MAX_BUF_YSIZE; j++)
      redraw[i][j] = 0;
  redraw_tiles = 0;
  redraw_mask = REDRAW_ALL;
}

void LoadGfx(int pos, struct PictureFileInfo *pic)
{
  char basefilename[256];
  char filename[256];

#ifdef XPM_INCLUDE_FILE
  int xpm_err, xbm_err;
  unsigned int width,height;
  int hot_x,hot_y;
  Pixmap shapemask;
  char *picture_ext = ".xpm";
  char *picturemask_ext = "Mask.xbm";
#else
  int gif_err;
  char *picture_ext = ".gif";
#endif

#ifdef DEBUG_TIMING
  long count1, count2;
#endif

  /* Grafik laden */
  if (pic->picture_filename)
  {
    sprintf(basefilename,"%s%s",pic->picture_filename,picture_ext);
    DrawInitText(basefilename,150,FC_YELLOW);
    sprintf(filename,"%s/%s",GFX_PATH,basefilename);

#ifdef MSDOS
    rest(100);
#endif MSDOS

#ifdef DEBUG_TIMING
    count1 = Counter();
#endif

#ifdef XPM_INCLUDE_FILE

    xpm_att[pos].valuemask = XpmCloseness;
    xpm_att[pos].closeness = 20000;
    xpm_err = XpmReadFileToPixmap(display,window,filename,
				  &pix[pos],&shapemask,&xpm_att[pos]);

    switch(xpm_err)
    {
      case XpmOpenFailed:
	Error(ERR_EXIT, "cannot open XPM file '%s'", filename);
      case XpmFileInvalid:
	Error(ERR_EXIT, "invalid XPM file '%s'", filename);
      case XpmNoMemory:
	Error(ERR_EXIT, "not enough memory for XPM file '%s'", filename);
      case XpmColorFailed:
	Error(ERR_EXIT, "cannot get colors for XPM file '%s'", filename);
      default:
	break;
    }

#ifdef DEBUG_TIMING
    count2 = Counter();
    printf("XPM LOADING %s IN %.2f SECONDS\n",
	   filename,(float)(count2-count1)/1000.0);
#endif

#else 

    gif_err = Read_GIF_to_Pixmaps(display, window, filename,
				  &pix[pos], &clipmask[pos]);

    switch(gif_err)
    {
      case GIF_Success:
        break;
      case GIF_OpenFailed:
        Error(ERR_EXIT, "cannot open GIF file '%s'", filename);
      case GIF_ReadFailed:
        Error(ERR_EXIT, "cannot read GIF file '%s'", filename);
      case GIF_FileInvalid:
	Error(ERR_EXIT, "invalid GIF file '%s'", filename);
      case GIF_NoMemory:
	Error(ERR_EXIT, "not enough memory for GIF file '%s'", filename);
      case GIF_ColorFailed:
	Error(ERR_EXIT, "cannot get colors for GIF file '%s'", filename);
      default:
	break;
    }

#ifdef DEBUG_TIMING
    count2 = Counter();
    printf("GIF LOADING %s IN %.2f SECONDS\n",
	   filename,(float)(count2-count1)/1000.0);
#endif

#endif

    if (!pix[pos])
      Error(ERR_EXIT, "cannot get graphics for '%s'", pic->picture_filename);
  }

  /* zugeh�rige Maske laden (wenn vorhanden) */
  if (pic->picture_with_mask)
  {

#ifdef XPM_INCLUDE_FILE

    sprintf(basefilename,"%s%s",pic->picture_filename,picturemask_ext);
    DrawInitText(basefilename,150,FC_YELLOW);
    sprintf(filename,"%s/%s",GFX_PATH,basefilename);

#ifdef DEBUG_TIMING
    count1 = Counter();
#endif

    xbm_err = XReadBitmapFile(display,window,filename,
			      &width,&height,&clipmask[pos],&hot_x,&hot_y);

    switch(xbm_err)
    {
      case BitmapSuccess:
        break;
      case BitmapOpenFailed:
	Error(ERR_EXIT, "cannot open XBM file '%s'", filename);
      case BitmapFileInvalid:
	Error(ERR_EXIT, "invalid XBM file '%s'", filename);
      case BitmapNoMemory:
	Error(ERR_EXIT, "not enough memory for XBM file '%s'", filename);
	break;
      default:
	break;
    }

#ifdef DEBUG_TIMING
    count2 = Counter();
    printf("XBM LOADING %s IN %.2f SECONDS\n",
	   filename,(float)(count2-count1)/1000.0);
#endif

#endif

    if (!clipmask[pos])
      Error(ERR_EXIT, "cannot get clipmask for '%s'", pic->picture_filename);
  }
}

void InitElementProperties()
{
  int i,j;

  static int ep_amoebalive[] =
  {
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_amoebalive_num = sizeof(ep_amoebalive)/sizeof(int);

  static int ep_amoeboid[] =
  {
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_amoeboid_num = sizeof(ep_amoeboid)/sizeof(int);

  static int ep_schluessel[] =
  {
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4
  };
  static int ep_schluessel_num = sizeof(ep_schluessel)/sizeof(int);

  static int ep_pforte[] =
  {
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X
  };
  static int ep_pforte_num = sizeof(ep_pforte)/sizeof(int);

  static int ep_solid[] =
  {
    EL_BETON,
    EL_MAUERWERK,
    EL_MAUER_LEBT,
    EL_MAUER_X,
    EL_MAUER_Y,
    EL_MAUER_XY,
    EL_FELSBODEN,
    EL_AUSGANG_ZU,
    EL_AUSGANG_ACT,
    EL_AUSGANG_AUF,
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD,
    EL_MORAST_VOLL,
    EL_MORAST_LEER,
    EL_SIEB_VOLL,
    EL_SIEB_LEER,
    EL_SIEB_TOT,
    EL_SIEB2_VOLL,
    EL_SIEB2_LEER,
    EL_SIEB2_TOT,
    EL_LIFE,
    EL_LIFE_ASYNC,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5
  };
  static int ep_solid_num = sizeof(ep_solid)/sizeof(int);

  static int ep_massiv[] =
  {
    EL_BETON,
    EL_SALZSAEURE,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X
  };
  static int ep_massiv_num = sizeof(ep_massiv)/sizeof(int);

  static int ep_slippery[] =
  {
    EL_FELSBODEN,
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_ABLENK_EIN,
    EL_ABLENK_AUS,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER,
    EL_BIRNE_EIN,
    EL_BIRNE_AUS,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_SONDE
  };
  static int ep_slippery_num = sizeof(ep_slippery)/sizeof(int);

  static int ep_enemy[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_MAMPFER2,
    EL_ROBOT,
    EL_PACMAN
  };
  static int ep_enemy_num = sizeof(ep_enemy)/sizeof(int);

  static int ep_mauer[] =
  {
    EL_BETON,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X,
    EL_AUSGANG_ZU,
    EL_AUSGANG_ACT,
    EL_AUSGANG_AUF,
    EL_MAUERWERK,
    EL_FELSBODEN,
    EL_MAUER_LEBT,
    EL_MAUER_X,
    EL_MAUER_Y,
    EL_MAUER_XY,
    EL_MAUERND
  };
  static int ep_mauer_num = sizeof(ep_mauer)/sizeof(int);

  static int ep_can_fall[] =
  {
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_TROPFEN,
    EL_MORAST_VOLL,
    EL_SIEB_VOLL,
    EL_SIEB2_VOLL,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER
  };
  static int ep_can_fall_num = sizeof(ep_can_fall)/sizeof(int);

  static int ep_can_smash[] =
  {
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_TROPFEN,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER
  };
  static int ep_can_smash_num = sizeof(ep_can_smash)/sizeof(int);

  static int ep_can_change[] =
  {
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT
  };
  static int ep_can_change_num = sizeof(ep_can_change)/sizeof(int);

  static int ep_can_move[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_MAMPFER2,
    EL_ROBOT,
    EL_PACMAN,
    EL_MAULWURF,
    EL_PINGUIN,
    EL_SCHWEIN,
    EL_DRACHE,
    EL_SONDE
  };
  static int ep_can_move_num = sizeof(ep_can_move)/sizeof(int);

  static int ep_could_move[] =
  {
    EL_KAEFER_R,
    EL_KAEFER_O,
    EL_KAEFER_L,
    EL_KAEFER_U,
    EL_FLIEGER_R,
    EL_FLIEGER_O,
    EL_FLIEGER_L,
    EL_FLIEGER_U,
    EL_BUTTERFLY_R,
    EL_BUTTERFLY_O,
    EL_BUTTERFLY_L,
    EL_BUTTERFLY_U,
    EL_FIREFLY_R,
    EL_FIREFLY_O,
    EL_FIREFLY_L,
    EL_FIREFLY_U,
    EL_PACMAN_R,
    EL_PACMAN_O,
    EL_PACMAN_L,
    EL_PACMAN_U
  };
  static int ep_could_move_num = sizeof(ep_could_move)/sizeof(int);

  static int ep_dont_touch[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY
  };
  static int ep_dont_touch_num = sizeof(ep_dont_touch)/sizeof(int);

  static int ep_dont_go_to[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_MAMPFER2,
    EL_ROBOT,
    EL_PACMAN,
    EL_TROPFEN,
    EL_SALZSAEURE
  };
  static int ep_dont_go_to_num = sizeof(ep_dont_go_to)/sizeof(int);

  static int ep_mampf2[] =
  {
    EL_ERDREICH,
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_ROBOT,
    EL_PACMAN,
    EL_TROPFEN,
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT
  };
  static int ep_mampf2_num = sizeof(ep_mampf2)/sizeof(int);

  static int ep_bd_element[] =
  {
    EL_LEERRAUM,
    EL_ERDREICH,
    EL_FELSBODEN,
    EL_FELSBROCKEN,
    EL_EDELSTEIN_BD,
    EL_SIEB2_LEER,
    EL_AUSGANG_ZU,
    EL_AUSGANG_AUF,
    EL_BETON,
    EL_SPIELFIGUR,
    EL_FIREFLY,
    EL_FIREFLY_1,
    EL_FIREFLY_2,
    EL_FIREFLY_3,
    EL_FIREFLY_4,
    EL_BUTTERFLY,
    EL_BUTTERFLY_1,
    EL_BUTTERFLY_2,
    EL_BUTTERFLY_3,
    EL_BUTTERFLY_4,
    EL_AMOEBE_BD,
    EL_CHAR_FRAGE
  };
  static int ep_bd_element_num = sizeof(ep_bd_element)/sizeof(int);

  static int ep_sb_element[] =
  {
    EL_LEERRAUM,
    EL_BETON,
    EL_SOKOBAN_OBJEKT,
    EL_SOKOBAN_FELD_LEER,
    EL_SOKOBAN_FELD_VOLL,
    EL_SPIELFIGUR
  };
  static int ep_sb_element_num = sizeof(ep_sb_element)/sizeof(int);

  static int ep_gem[] =
  {
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT
  };
  static int ep_gem_num = sizeof(ep_gem)/sizeof(int);

  static int ep_inactive[] =
  {
    EL_LEERRAUM,
    EL_ERDREICH,
    EL_MAUERWERK,
    EL_FELSBODEN,
    EL_SCHLUESSEL,
    EL_BETON,
    EL_AMOEBE_TOT,
    EL_MORAST_LEER,
    EL_BADEWANNE,
    EL_ABLENK_AUS,
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X,
    EL_DYNAMIT_AUS,
    EL_UNSICHTBAR,
    EL_BIRNE_AUS,
    EL_BIRNE_EIN,
    EL_ERZ_EDEL,
    EL_ERZ_DIAM,
    EL_ERZ_EDEL_BD,
    EL_ERZ_EDEL_GELB,
    EL_DYNABOMB_NR,
    EL_DYNABOMB_SZ,
    EL_DYNABOMB_XL,
    EL_SOKOBAN_OBJEKT,
    EL_SOKOBAN_FELD_LEER,
    EL_SOKOBAN_FELD_VOLL,
    EL_ERZ_EDEL_ROT,
    EL_ERZ_EDEL_LILA,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
    EL_SIEB_TOT,
    EL_SIEB2_TOT,
    EL_AMOEBA2DIAM,
    EL_BLOCKED
  };
  static int ep_inactive_num = sizeof(ep_inactive)/sizeof(int);

  static int ep_explosive[] =
  {
    EL_BOMBE,
    EL_DYNAMIT,
    EL_DYNAMIT_AUS,
    EL_DYNABOMB,
    EL_DYNABOMB_NR,
    EL_DYNABOMB_SZ,
    EL_DYNABOMB_XL,
    EL_KAEFER,
    EL_MAULWURF,
    EL_PINGUIN,
    EL_SCHWEIN,
    EL_DRACHE,
    EL_SONDE
  };
  static int ep_explosive_num = sizeof(ep_explosive)/sizeof(int);

  static int ep_mampf3[] =
  {
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT
  };
  static int ep_mampf3_num = sizeof(ep_mampf3)/sizeof(int);

  static int ep_pushable[] =
  {
    EL_FELSBROCKEN,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_ZEIT_LEER,
    EL_SOKOBAN_FELD_VOLL,
    EL_SOKOBAN_OBJEKT,
    EL_SONDE
  };
  static int ep_pushable_num = sizeof(ep_pushable)/sizeof(int);

  static int ep_player[] =
  {
    EL_SPIELFIGUR,
    EL_SPIELER1,
    EL_SPIELER2,
    EL_SPIELER3,
    EL_SPIELER4
  };
  static int ep_player_num = sizeof(ep_player)/sizeof(int);

  static long ep_bit[] =
  {
    EP_BIT_AMOEBALIVE,
    EP_BIT_AMOEBOID,
    EP_BIT_SCHLUESSEL,
    EP_BIT_PFORTE,
    EP_BIT_SOLID,
    EP_BIT_MASSIV,
    EP_BIT_SLIPPERY,
    EP_BIT_ENEMY,
    EP_BIT_MAUER,
    EP_BIT_CAN_FALL,
    EP_BIT_CAN_SMASH,
    EP_BIT_CAN_CHANGE,
    EP_BIT_CAN_MOVE,
    EP_BIT_COULD_MOVE,
    EP_BIT_DONT_TOUCH,
    EP_BIT_DONT_GO_TO,
    EP_BIT_MAMPF2,
    EP_BIT_BD_ELEMENT,
    EP_BIT_SB_ELEMENT,
    EP_BIT_GEM,
    EP_BIT_INACTIVE,
    EP_BIT_EXPLOSIVE,
    EP_BIT_MAMPF3,
    EP_BIT_PUSHABLE,
    EP_BIT_PLAYER
  };
  static int *ep_array[] =
  {
    ep_amoebalive,
    ep_amoeboid,
    ep_schluessel,
    ep_pforte,
    ep_solid,
    ep_massiv,
    ep_slippery,
    ep_enemy,
    ep_mauer,
    ep_can_fall,
    ep_can_smash,
    ep_can_change,
    ep_can_move,
    ep_could_move,
    ep_dont_touch,
    ep_dont_go_to,
    ep_mampf2,
    ep_bd_element,
    ep_sb_element,
    ep_gem,
    ep_inactive,
    ep_explosive,
    ep_mampf3,
    ep_pushable,
    ep_player
  };
  static int *ep_num[] =
  {
    &ep_amoebalive_num,
    &ep_amoeboid_num,
    &ep_schluessel_num,
    &ep_pforte_num,
    &ep_solid_num,
    &ep_massiv_num,
    &ep_slippery_num,
    &ep_enemy_num,
    &ep_mauer_num,
    &ep_can_fall_num,
    &ep_can_smash_num,
    &ep_can_change_num,
    &ep_can_move_num,
    &ep_could_move_num,
    &ep_dont_touch_num,
    &ep_dont_go_to_num,
    &ep_mampf2_num,
    &ep_bd_element_num,
    &ep_sb_element_num,
    &ep_gem_num,
    &ep_inactive_num,
    &ep_explosive_num,
    &ep_mampf3_num,
    &ep_pushable_num,
    &ep_player_num
  };
  static int num_properties = sizeof(ep_num)/sizeof(int *);

  for(i=0;i<MAX_ELEMENTS;i++)
    Elementeigenschaften[i] = 0;

  for(i=0;i<num_properties;i++)
    for(j=0;j<*(ep_num[i]);j++)
      Elementeigenschaften[(ep_array[i])[j]] |= ep_bit[i];
  for(i=EL_CHAR_START;i<EL_CHAR_END;i++)
    Elementeigenschaften[i] |= (EP_BIT_CHAR | EP_BIT_INACTIVE);
}

void CloseAllAndExit(int exit_value)
{
  int i;

  if (sound_process_id)
  {
    StopSounds();
    kill(sound_process_id, SIGTERM);
    FreeSounds(NUM_SOUNDS);
  }

  for(i=0;i<NUM_PIXMAPS;i++)
  {
    if (pix[i])
    {
#ifdef XPM_INCLUDE_FILE
      if (i<NUM_PICTURES)	/* XPM pictures */
      {
	XFreeColors(display,DefaultColormap(display,screen),
		    xpm_att[i].pixels,xpm_att[i].npixels,0);
	XpmFreeAttributes(&xpm_att[i]);
      }
#endif
      XFreePixmap(display,pix[i]);
    }
    if (clipmask[i])
      XFreePixmap(display,clipmask[i]);
    if (clip_gc[i])
      XFreeGC(display, clip_gc[i]);
  }

  if (gc)
    XFreeGC(display, gc);

  if (display)
  {
    XAutoRepeatOn(display);
    XCloseDisplay(display);
  }

  exit(exit_value);
}
