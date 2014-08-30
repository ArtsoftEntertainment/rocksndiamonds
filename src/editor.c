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
*  editor.c                                                *
***********************************************************/

#include "editor.h"
#include "screens.h"
#include "tools.h"
#include "misc.h"
#include "buttons.h"
#include "files.h"

/* positions in the level editor */
#define ED_WIN_MB_LEFT_XPOS	7
#define ED_WIN_MB_LEFT_YPOS	6
#define ED_WIN_LEVELNR_XPOS	77
#define ED_WIN_LEVELNR_YPOS	7
#define ED_WIN_MB_MIDDLE_XPOS	7
#define ED_WIN_MB_MIDDLE_YPOS	258
#define ED_WIN_MB_RIGHT_XPOS	77
#define ED_WIN_MB_RIGHT_YPOS	258

/* other constants for the editor */
#define ED_SCROLL_NO		0
#define ED_SCROLL_LEFT		1
#define ED_SCROLL_RIGHT		2
#define ED_SCROLL_UP		4
#define ED_SCROLL_DOWN		8

/* delay value to avoid too fast scrolling etc. */
#define CHOICE_DELAY_VALUE	100

static int level_xpos,level_ypos;
static boolean edit_mode;
static boolean name_typing;
static int new_element1 = EL_MAUERWERK;
static int new_element2 = EL_LEERRAUM;
static int new_element3 = EL_ERDREICH;

int element_shift;
int editor_element[] =
{
  EL_CHAR_A + ('B' - 'A'),
  EL_CHAR_A + ('O' - 'A'),
  EL_CHAR_A + ('U' - 'A'),
  EL_CHAR_A + ('L' - 'A'),

  EL_CHAR_MINUS,
  EL_CHAR_A + ('D' - 'A'),
  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_A + ('R' - 'A'),

  EL_CHAR_A + ('D' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('S' - 'A'),
  EL_CHAR_A + ('H' - 'A'),

  EL_SPIELFIGUR,
  EL_LEERRAUM,
  EL_ERDREICH,
  EL_BETON,

  EL_FELSBODEN,
  EL_SIEB2_LEER,
  EL_AUSGANG_ZU,
  EL_AUSGANG_AUF,

  EL_EDELSTEIN_BD,
  EL_BUTTERFLY_O,
  EL_FIREFLY_O,
  EL_FELSBROCKEN,

  EL_BUTTERFLY_L,
  EL_FIREFLY_L,
  EL_BUTTERFLY_R,
  EL_FIREFLY_R,

  EL_AMOEBE_BD,
  EL_BUTTERFLY_U,
  EL_FIREFLY_U,
  EL_LEERRAUM,

  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_A + ('M' - 'A'),
  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_MINUS,

  EL_CHAR_A + ('R' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('L' - 'A'),
  EL_CHAR_A + ('D' - 'A'),

  EL_CHAR_A + ('M' - 'A'),
  EL_CHAR_A + ('I' - 'A'),
  EL_CHAR_A + ('N' - 'A'),
  EL_CHAR_A + ('E' - 'A'),

  EL_SPIELER1,
  EL_SPIELER2,
  EL_SPIELER3,
  EL_SPIELER4,

  EL_SPIELFIGUR,
  EL_LEERRAUM,
  EL_ERDREICH,
  EL_FELSBROCKEN,

  EL_BETON,
  EL_MAUERWERK,
  EL_FELSBODEN,
  EL_SIEB_LEER,

  EL_EDELSTEIN,
  EL_DIAMANT,
  EL_KOKOSNUSS,
  EL_BOMBE,

  EL_ERZ_EDEL,
  EL_ERZ_DIAM,
  EL_MORAST_LEER,
  EL_MORAST_VOLL,

  EL_DYNAMIT_AUS,
  EL_DYNAMIT,
  EL_AUSGANG_ZU,
  EL_AUSGANG_AUF,

  EL_MAMPFER,
  EL_KAEFER_O,
  EL_FLIEGER_O,
  EL_ROBOT,

  EL_KAEFER_L,
  EL_FLIEGER_L,
  EL_KAEFER_R,
  EL_FLIEGER_R,

  EL_ABLENK_AUS,
  EL_KAEFER_U,
  EL_FLIEGER_U,
  EL_UNSICHTBAR,

  EL_BADEWANNE1,
  EL_SALZSAEURE,
  EL_BADEWANNE2,
  EL_LEERRAUM,

  EL_BADEWANNE3,
  EL_BADEWANNE4,
  EL_BADEWANNE5,
  EL_LEERRAUM,

  EL_TROPFEN,
  EL_AMOEBE_TOT,
  EL_AMOEBE_NASS,
  EL_AMOEBE_NORM,

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

  EL_CHAR_A + ('M' - 'A'),
  EL_CHAR_A + ('O' - 'A'),
  EL_CHAR_A + ('R' - 'A'),
  EL_CHAR_A + ('E' - 'A'),

  EL_PFEIL_L,
  EL_PFEIL_R,
  EL_PFEIL_O,
  EL_PFEIL_U,

  EL_AMOEBE_VOLL,
  EL_EDELSTEIN_GELB,
  EL_EDELSTEIN_ROT,
  EL_EDELSTEIN_LILA,

  EL_ERZ_EDEL_BD,
  EL_ERZ_EDEL_GELB,
  EL_ERZ_EDEL_ROT,
  EL_ERZ_EDEL_LILA,

  EL_LIFE,
  EL_PACMAN_O,
  EL_ZEIT_VOLL,
  EL_ZEIT_LEER,

  EL_PACMAN_L,
  EL_MAMPFER2,
  EL_PACMAN_R,
  EL_MAUER_LEBT,

  EL_LIFE_ASYNC,
  EL_PACMAN_U,
  EL_BIRNE_AUS,
  EL_BIRNE_EIN,

  EL_DYNABOMB_NR,
  EL_DYNABOMB_SZ,
  EL_DYNABOMB_XL,
  EL_BADEWANNE,

  EL_MAULWURF,
  EL_PINGUIN,
  EL_SCHWEIN,
  EL_DRACHE,

  EL_SONDE,
  EL_MAUER_X,
  EL_MAUER_Y,
  EL_MAUER_XY,

  EL_CHAR_A + ('S' - 'A'),
  EL_CHAR_A + ('O' - 'A'),
  EL_CHAR_A + ('K' - 'A'),
  EL_CHAR_A + ('O' - 'A'),

  EL_CHAR_MINUS,
  EL_CHAR_A + ('B' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('N' - 'A'),

  EL_SOKOBAN_OBJEKT,
  EL_SOKOBAN_FELD_LEER,
  EL_SOKOBAN_FELD_VOLL,
  EL_BETON,

/*
  EL_CHAR_A + ('D' - 'A'),
  EL_CHAR_A + ('Y' - 'A'),
  EL_CHAR_A + ('N' - 'A'),
  EL_CHAR_A + ('A' - 'A'),

  EL_CHAR_A + ('B' - 'A'),
  EL_CHAR_A + ('L' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('S' - 'A'),

  EL_CHAR_MINUS,
  EL_CHAR_A + ('T' - 'A'),
  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_A + ('R' - 'A'),
*/

  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,

  EL_CHAR_AUSRUF,
  EL_CHAR_ZOLL,
  EL_CHAR_DOLLAR,
  EL_CHAR_PROZ,

  EL_CHAR_APOSTR,
  EL_CHAR_KLAMM1,
  EL_CHAR_KLAMM2,
  EL_CHAR_PLUS,

  EL_CHAR_KOMMA,
  EL_CHAR_MINUS,
  EL_CHAR_PUNKT,
  EL_CHAR_SLASH,

  EL_CHAR_0 + 0,
  EL_CHAR_0 + 1,
  EL_CHAR_0 + 2,
  EL_CHAR_0 + 3,

  EL_CHAR_0 + 4,
  EL_CHAR_0 + 5,
  EL_CHAR_0 + 6,
  EL_CHAR_0 + 7,

  EL_CHAR_0 + 8,
  EL_CHAR_0 + 9,
  EL_CHAR_DOPPEL,
  EL_CHAR_SEMIKL,

  EL_CHAR_LT,
  EL_CHAR_GLEICH,
  EL_CHAR_GT,
  EL_CHAR_FRAGE,

  EL_CHAR_AT,
  EL_CHAR_A + 0,
  EL_CHAR_A + 1,
  EL_CHAR_A + 2,

  EL_CHAR_A + 3,
  EL_CHAR_A + 4,
  EL_CHAR_A + 5,
  EL_CHAR_A + 6,

  EL_CHAR_A + 7,
  EL_CHAR_A + 8,
  EL_CHAR_A + 9,
  EL_CHAR_A + 10,

  EL_CHAR_A + 11,
  EL_CHAR_A + 12,
  EL_CHAR_A + 13,
  EL_CHAR_A + 14,

  EL_CHAR_A + 15,
  EL_CHAR_A + 16,
  EL_CHAR_A + 17,
  EL_CHAR_A + 18,

  EL_CHAR_A + 19,
  EL_CHAR_A + 20,
  EL_CHAR_A + 21,
  EL_CHAR_A + 22,

  EL_CHAR_A + 23,
  EL_CHAR_A + 24,
  EL_CHAR_A + 25,
  EL_CHAR_AE,

  EL_CHAR_OE,
  EL_CHAR_UE,
  EL_CHAR_COPY
};
int elements_in_list = sizeof(editor_element)/sizeof(int);

void DrawLevelEd()
{
  int i, graphic;

  level_xpos=-1;
  level_ypos=-1;
  edit_mode = TRUE;
  name_typing = FALSE;
  element_shift = 0;

  CloseDoor(DOOR_CLOSE_2);

  DrawMiniLevel(level_xpos,level_ypos);
  FadeToFront();

  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX6,DOOR_GFX_PAGEY1,
	    DXSIZE,DYSIZE,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX6+ED_BUTTON_ELEM_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_ELEM_YPOS,
	    4*ED_BUTTON_ELEM_XSIZE,5*ED_BUTTON_ELEM_YSIZE,
	    DOOR_GFX_PAGEX1+ED_BUTTON_ELEM_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_EUP_Y2POS);

  for(i=0;i<MAX_ELEM_X*MAX_ELEM_Y;i++)
  {
    if (i < elements_in_list)
      graphic = el2gfx(editor_element[i]);
    else
      graphic = GFX_LEERRAUM;

    DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		       DOOR_GFX_PAGEX1+ED_BUTTON_ELEM_XPOS+3 + 
		       (i%MAX_ELEM_X)*ED_BUTTON_ELEM_XSIZE,
		       DOOR_GFX_PAGEY1+ED_BUTTON_ELEM_YPOS+3 +
		       (i/MAX_ELEM_X)*ED_BUTTON_ELEM_YSIZE,
		       graphic);
  }

  DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		     DOOR_GFX_PAGEX1+ED_WIN_MB_LEFT_XPOS,
		     DOOR_GFX_PAGEY1+ED_WIN_MB_LEFT_YPOS,
		     el2gfx(new_element1));
  DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		     DOOR_GFX_PAGEX1+ED_WIN_MB_MIDDLE_XPOS,
		     DOOR_GFX_PAGEY1+ED_WIN_MB_MIDDLE_YPOS,
		     el2gfx(new_element2));
  DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		     DOOR_GFX_PAGEX1+ED_WIN_MB_RIGHT_XPOS,
		     DOOR_GFX_PAGEY1+ED_WIN_MB_RIGHT_YPOS,
		     el2gfx(new_element3));
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX2+ED_WIN_LEVELNR_XPOS,
	      DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS,
	      int2str(level_nr,2),FS_SMALL,FC_SPECIAL1);
  XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX2+ED_WIN_LEVELNR_XPOS+3,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS,
	    7,FONT3_YSIZE,
	    DOOR_GFX_PAGEX1+ED_WIN_LEVELNR_XPOS,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS);
  XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX2+ED_WIN_LEVELNR_XPOS+14,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS,
	    7,FONT3_YSIZE,
	    DOOR_GFX_PAGEX1+ED_WIN_LEVELNR_XPOS+9,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS);

  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX6,DOOR_GFX_PAGEY2,
	    VXSIZE,VYSIZE,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);

  OpenDoor(DOOR_OPEN_1 | DOOR_OPEN_2);
}



void test_func(struct GadgetInfo *gi)
{
  if (gi->event == GD_EVENT_PRESSED)
    printf("test_func: GD_EVENT_PRESSED\n");
  else if (gi->event == GD_EVENT_RELEASED)
    printf("test_func: GD_EVENT_RELEASED\n");
  else 
    printf("test_func: ?\n");
}





void DrawControlWindow()
{
  int i,x,y;

  ClearWindow();

  /* Inhalt der Mampfer */
  DrawText(ED_COUNT_GADGET_XPOS+1,SY+6,
	   "Contents of a smashed cruncher:",FS_SMALL,FC_YELLOW);
  for(i=0;i<4;i++) for(y=0;y<4;y++) for(x=0;x<4;x++)
  {
    DrawMiniElement(1+5*i+x,2+y,EL_ERDREICH);
    XFillRectangle(display,drawto,gc,
		   SX+(1+5*i)*MINI_TILEX+MINI_TILEX/2-1,
		   SY+(2)*MINI_TILEY+MINI_TILEY/2-1,
		   3*MINI_TILEX+2,3*MINI_TILEY+2);
  }
  XCopyArea(display,drawto,drawto,gc,
	    SX+1*MINI_TILEX,SY+2*MINI_TILEY,
	    4*5*MINI_TILEX,5*MINI_TILEY,
	    SX+1*MINI_TILEX-MINI_TILEX/2,SY+2*MINI_TILEY-MINI_TILEY/2);
  for(i=0;i<4;i++)
  {
    for(y=0;y<3;y++) for(x=0;x<3;x++)
      DrawMiniElement(1+5*i+x,2+y,level.mampfer_inhalt[i][x][y]);

    DrawText(SX+MINI_TILEX+(5*i+1)*MINI_TILEX+1,
	     SY+2*MINI_TILEY+(4)*MINI_TILEY-4,
	     int2str(i+1,1),FS_SMALL,FC_YELLOW);
  }

  /* Inhalt der Am�be */
  for(y=0;y<2;y++) for(x=0;x<2;x++)
  {
    DrawMiniElement(29+x,26+y,EL_ERDREICH);
    XFillRectangle(display,drawto,gc,
		   SX+29*MINI_TILEX+MINI_TILEX/2-1,
		   SY+26*MINI_TILEY+MINI_TILEY/2-1,
		   MINI_TILEX+2,MINI_TILEY+2);
  }
  XCopyArea(display,drawto,drawto,gc,
	    SX+29*MINI_TILEX,SY+26*MINI_TILEY,
	    3*MINI_TILEX,3*MINI_TILEY,
	    SX+29*MINI_TILEX-MINI_TILEX/2,SY+26*MINI_TILEY-MINI_TILEY/2);
  DrawMiniElement(29,26,level.amoebe_inhalt);

  for(i=0;i<11+3+2;i++)
  {
    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      DOOR_GFX_PAGEX4+ED_BUTTON_MINUS_XPOS,
	      DOOR_GFX_PAGEY1+ED_BUTTON_MINUS_YPOS,
	      DXSIZE-4,ED_BUTTON_MINUS_YSIZE,
	      ED_COUNT_GADGET_XPOS,
	      ED_COUNT_GADGET_YPOS+i*ED_COUNT_GADGET_YSIZE);

    if (i<11)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+i*ED_COUNT_GADGET_YSIZE,
	       int2str(level.score[i],3),FS_SMALL,FC_YELLOW);
    else if (i==11)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+11*ED_COUNT_GADGET_YSIZE,
	       int2str(level.tempo_amoebe,3),FS_SMALL,FC_YELLOW);
    else if (i==12)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+12*ED_COUNT_GADGET_YSIZE,
	       int2str(level.dauer_sieb,3),FS_SMALL,FC_YELLOW);
    else if (i==13)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+13*ED_COUNT_GADGET_YSIZE,
	       int2str(level.dauer_ablenk,3),FS_SMALL,FC_YELLOW);
    else if (i==14)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+14*ED_COUNT_GADGET_YSIZE,
	       int2str(level.edelsteine,3),FS_SMALL,FC_YELLOW);
    else if (i==15)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+15*ED_COUNT_GADGET_YSIZE,
	       int2str(level.time,3),FS_SMALL,FC_YELLOW);
  }

  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+0*ED_COUNT_TEXT_YSIZE,
	   "Score for Emerald",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+1*ED_COUNT_TEXT_YSIZE,
	   "Score for Diamond",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+2*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Bug",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+3*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Spaceship",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+4*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Cruncher",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+5*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing an Alien",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+6*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Pacman",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+7*ED_COUNT_TEXT_YSIZE,
	   "Score for cracking a nut",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+8*ED_COUNT_TEXT_YSIZE,
	   "Score for dynamite",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+9*ED_COUNT_TEXT_YSIZE,
	   "Score for key",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+10*ED_COUNT_TEXT_YSIZE,
	   "Score for each 10 seconds left",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+11*ED_COUNT_TEXT_YSIZE,
	   "Speed of the amoeba / Content",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+12*ED_COUNT_TEXT_YSIZE,
	   "Time for magic wall",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+13*ED_COUNT_TEXT_YSIZE,
	   "Time for wheel",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+14*ED_COUNT_TEXT_YSIZE,
	   "Emeralds needed in this level",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+15*ED_COUNT_TEXT_YSIZE,
	   "Time available for this level",FS_SMALL,FC_YELLOW);

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+ED_WIN_COUNT_XPOS,
	    DOOR_GFX_PAGEY1+ED_WIN_COUNT_YPOS,
	    ED_WIN_COUNT_XSIZE,ED_WIN_COUNT_YSIZE,
	    ED_COUNT_GADGET_XPOS,
	    ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE);
  for(i=1;i<31;i++)
    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      DOOR_GFX_PAGEX4+ED_WIN_COUNT_XPOS+3+2*FONT2_XSIZE,
	      DOOR_GFX_PAGEY1+ED_WIN_COUNT_YPOS,
	      ED_WIN_COUNT_XSIZE-3-2*FONT2_XSIZE,ED_WIN_COUNT_YSIZE,
	      ED_COUNT_GADGET_XPOS+3+i*FONT2_XSIZE,
	      ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE);
  DrawText(ED_COUNT_GADGET_XPOS+5,
	   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
	   level.name,FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_GADGET_XPOS+(30+3)*FONT2_XSIZE-5,
	   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
	   "Title",FS_SMALL,FC_YELLOW);

  DrawText(ED_SIZE_GADGET_XPOS,ED_SIZE_GADGET_YPOS-18,
	   "Playfield size:",FS_SMALL,FC_YELLOW);
  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+ED_BUTTON_MINUS_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_MINUS_YPOS,
	    DXSIZE-4,ED_BUTTON_MINUS_YSIZE,
	    ED_SIZE_GADGET_XPOS,
	    ED_SIZE_GADGET_YPOS+0*ED_COUNT_GADGET_YSIZE);
  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+ED_BUTTON_MINUS_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_MINUS_YPOS,
	    DXSIZE-4,ED_BUTTON_MINUS_YSIZE,
	    ED_SIZE_GADGET_XPOS,
	    ED_SIZE_GADGET_YPOS+1*ED_COUNT_GADGET_YSIZE);
  DrawText(ED_SIZE_TEXT_XPOS,ED_SIZE_TEXT_YPOS+0*ED_SIZE_TEXT_YSIZE,
	   "Width",FS_SMALL,FC_YELLOW);
  DrawText(ED_SIZE_TEXT_XPOS,ED_SIZE_TEXT_YPOS+1*ED_SIZE_TEXT_YSIZE,
	   "Height",FS_SMALL,FC_YELLOW);
  DrawText(ED_SIZE_VALUE_XPOS,ED_SIZE_VALUE_YPOS+0*ED_SIZE_GADGET_YSIZE,
	   int2str(level.fieldx,3),FS_SMALL,FC_YELLOW);
  DrawText(ED_SIZE_VALUE_XPOS,ED_SIZE_VALUE_YPOS+1*ED_SIZE_GADGET_YSIZE,
	   int2str(level.fieldy,3),FS_SMALL,FC_YELLOW);

  {
    Pixmap gd_pixmap = pix[PIX_DOOR];
    int gd_x1 = DOOR_GFX_PAGEX4 + ED_BUTTON_MINUS_XPOS;
    int gd_x2 = DOOR_GFX_PAGEX3 + ED_BUTTON_MINUS_XPOS;
    int gd_y = DOOR_GFX_PAGEY1 + ED_BUTTON_MINUS_YPOS;
    struct GadgetInfo *gi;

    gi = CreateGadget(GDI_X, 100,
		      GDI_Y, 100,
		      GDI_WIDTH, ED_BUTTON_MINUS_XSIZE,
		      GDI_HEIGHT, ED_BUTTON_MINUS_YSIZE,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
		      GDI_CALLBACK, test_func,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    MapGadget(gi);
  }


}

void ScrollMiniLevel(int from_x, int from_y, int scroll)
{
  int x,y;
  int dx = (scroll==ED_SCROLL_LEFT ? -1 : scroll==ED_SCROLL_RIGHT ? 1 : 0);
  int dy = (scroll==ED_SCROLL_UP   ? -1 : scroll==ED_SCROLL_DOWN  ? 1 : 0);

  XCopyArea(display,drawto,drawto,gc,
	    SX+MINI_TILEX*(dx==-1),SY+MINI_TILEY*(dy==-1),
	    SXSIZE-MINI_TILEX*ABS(dx),SYSIZE-MINI_TILEY*ABS(dy),
	    SX+MINI_TILEX*(dx==+1),SY+MINI_TILEY*(dy==+1));
  if (dx)
  {
    x = (dx==1 ? 0 : 2*SCR_FIELDX-1);
    for(y=0;y<2*SCR_FIELDY;y++)
      DrawMiniElementOrWall(x,y,from_x,from_y);
  }
  else if (dy)
  {
    y = (dy==1 ? 0 : 2*SCR_FIELDY-1);
    for(x=0;x<2*SCR_FIELDX;x++)
      DrawMiniElementOrWall(x,y,from_x,from_y);
  }

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
}

void AdjustLevelScrollPosition()
{
  if (level_xpos<-1)
    level_xpos = -1;
  if (level_xpos>lev_fieldx-2*SCR_FIELDX+1)
    level_xpos = lev_fieldx-2*SCR_FIELDX+1;
  if (lev_fieldx<2*SCR_FIELDX-2)
    level_xpos = -1;

  if (level_ypos<-1)
    level_ypos = -1;
  if (level_ypos>lev_fieldy-2*SCR_FIELDY+1)
    level_ypos = lev_fieldy-2*SCR_FIELDY+1;
  if (lev_fieldy<2*SCR_FIELDY-2)
    level_ypos = -1;
}

void FloodFill(int from_x, int from_y, int fill_element)
{
  int i,x,y;
  int old_element;
  static int check[4][2] = { {-1,0}, {0,-1}, {1,0}, {0,1} };
  static int safety = 0;

  /* check if starting field still has the desired content */
  if (Feld[from_x][from_y] == fill_element)
    return;

  safety++;

  if (safety > lev_fieldx*lev_fieldy)
    Error(ERR_EXIT, "Something went wrong in 'FloodFill()'. Please debug.");

  old_element = Feld[from_x][from_y];
  Feld[from_x][from_y] = fill_element;

  for(i=0;i<4;i++)
  {
    x = from_x + check[i][0];
    y = from_y + check[i][1];

    if (IN_LEV_FIELD(x,y) && Feld[x][y] == old_element)
      FloodFill(x, y, fill_element);
  }

  safety--;
}

void LevelEd(int mx, int my, int button)
{
  static int last_button = 0;
  static int in_field_pressed = FALSE;
  static boolean use_floodfill = FALSE;
  int x = (mx-SX)/MINI_TILEX; 
  int y = (my-SY)/MINI_TILEY; 

  if (use_floodfill)		/********** FLOOD FILL **********/
  {
    if (button)
    {
      if (mx>=SX && mx<SX+SXSIZE && my>=SY && my<SY+SYSIZE)
      {
	int from_x, from_y;
	int fill_element;

	if (x>lev_fieldx || y>lev_fieldy ||
	    (x==0 && level_xpos<0) ||
	    (x==2*SCR_FIELDX-1 && level_xpos>lev_fieldx-2*SCR_FIELDX) ||
	    (y==0 && level_ypos<0) ||
	    (y==2*SCR_FIELDY-1 && level_ypos>lev_fieldy-2*SCR_FIELDY))
	  return;

	from_x = x+level_xpos;
	from_y = y+level_ypos;
	fill_element = (button==1 ? new_element1 :
			button==2 ? new_element2 :
			button==3 ? new_element3 : 0);

	FloodFill(from_x,from_y,fill_element);
	DrawMiniLevel(level_xpos,level_ypos);
      }

      use_floodfill = FALSE;
      CloseDoor(DOOR_CLOSE_1);
      OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
    }
    return;
  }
  else				/********** EDIT/CTRL-FENSTER **********/
  {
    static unsigned long choice_delay = 0;
    int choice = CheckElemButtons(mx,my,button);
    int elem_pos = choice-ED_BUTTON_ELEM;

    if (((choice == ED_BUTTON_EUP && element_shift>0) ||
	 (choice == ED_BUTTON_EDOWN &&
	  element_shift<elements_in_list-MAX_ELEM_X*MAX_ELEM_Y)) &&
	DelayReached(&choice_delay, CHOICE_DELAY_VALUE))
    {
      int i, step;

      step = (button==1 ? MAX_ELEM_X : button==2 ? 5*MAX_ELEM_X :
	      elements_in_list);
      element_shift += (choice==ED_BUTTON_EUP ? -step : step);
      if (element_shift<0)
	element_shift = 0;
      if (element_shift>elements_in_list-MAX_ELEM_X*MAX_ELEM_Y)
	element_shift = elements_in_list-MAX_ELEM_X*MAX_ELEM_Y;
      if (element_shift % MAX_ELEM_X)
	element_shift += MAX_ELEM_X-(element_shift % MAX_ELEM_X);

      for(i=0;i<MAX_ELEM_X*MAX_ELEM_Y;i++)
	DrawElemButton(i+2,ED_BUTTON_RELEASED);
    }
    else if (elem_pos>=0 && elem_pos<MAX_ELEM_X*MAX_ELEM_Y)
    {
      int new_element;

      if (elem_pos+element_shift < elements_in_list)
	new_element = editor_element[elem_pos+element_shift];
      else
	new_element = EL_LEERRAUM;

      if (last_button==1)
	new_element1 = new_element;
      else if (last_button==2)
	new_element2 = new_element;
      else if (last_button==3)
	new_element3 = new_element;

      DrawMiniGraphicExt(drawto,gc,
			 DX+ED_WIN_MB_LEFT_XPOS,
			 DY+ED_WIN_MB_LEFT_YPOS,
			 el2gfx(new_element1));
      DrawMiniGraphicExt(drawto,gc,
			 DX+ED_WIN_MB_MIDDLE_XPOS,
			 DY+ED_WIN_MB_MIDDLE_YPOS,
			 el2gfx(new_element2));
      DrawMiniGraphicExt(drawto,gc,
			 DX+ED_WIN_MB_RIGHT_XPOS,
			 DY+ED_WIN_MB_RIGHT_YPOS,
			 el2gfx(new_element3));
      redraw_mask |= REDRAW_DOOR_1;
    }
  
    if (edit_mode)		/********** EDIT-FENSTER **********/
    {
      switch(CheckEditButtons(mx,my,button))
      {
	case ED_BUTTON_CTRL:
	  CloseDoor(DOOR_CLOSE_2);
	  DrawControlWindow();
	  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
		    DOOR_GFX_PAGEX4,DOOR_GFX_PAGEY1+80,
		    VXSIZE,VYSIZE,
		    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);
	  OpenDoor(DOOR_OPEN_2);
	  edit_mode = FALSE;
	  break;
	case ED_BUTTON_FILL:
	  Request("Caution ! Flood fill mode ! Choose area !",REQ_OPEN);
	  use_floodfill = TRUE;
	  return;
	  break;
	case ED_BUTTON_LEFT:
	  if (level_xpos>=0)
	  {
	    if (!DelayReached(&choice_delay, CHOICE_DELAY_VALUE))
	      break;
	    if (lev_fieldx<2*SCR_FIELDX-2)
	      break;

	    level_xpos -= (button==1 ? 1 : button==2 ? 5 : lev_fieldx);
	    if (level_xpos<-1)
	      level_xpos = -1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_RIGHT);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_RIGHT:
	  if (level_xpos<=lev_fieldx-2*SCR_FIELDX)
	  {
	    if (!DelayReached(&choice_delay, CHOICE_DELAY_VALUE))
	      break;
	    if (lev_fieldx<2*SCR_FIELDX-2)
	      break;

	    level_xpos += (button==1 ? 1 : button==2 ? 5 : lev_fieldx);
	    if (level_xpos>lev_fieldx-2*SCR_FIELDX+1)
	      level_xpos = lev_fieldx-2*SCR_FIELDX+1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_LEFT);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_UP:
	  if (level_ypos>=0)
	  {
	    if (!DelayReached(&choice_delay, CHOICE_DELAY_VALUE))
	      break;
	    if (lev_fieldy<2*SCR_FIELDY-2)
	      break;

	    level_ypos -= (button==1 ? 1 : button==2 ? 5 : lev_fieldy);
	    if (level_ypos<-1)
	      level_ypos = -1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_DOWN);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_DOWN:
	  if (level_ypos<=lev_fieldy-2*SCR_FIELDY)
	  {
	    if (!DelayReached(&choice_delay, CHOICE_DELAY_VALUE))
	      break;
	    if (lev_fieldy<2*SCR_FIELDY-2)
	      break;

	    level_ypos += (button==1 ? 1 : button==2 ? 5 : lev_fieldy);
	    if (level_ypos>lev_fieldy-2*SCR_FIELDY+1)
	      level_ypos = lev_fieldy-2*SCR_FIELDY+1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_UP);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	default:
	  break;
      }

      if (mx>=SX && mx<SX+SXSIZE && my>=SY && my<SY+SYSIZE)
      {
	int new_element;

	if (button && !motion_status)
	  in_field_pressed = TRUE;

	if (!button || !in_field_pressed || button<1 || button>3 ||
	    (y==0 && level_ypos<0) ||
	    (y==2*SCR_FIELDY-1 && level_ypos>lev_fieldy-2*SCR_FIELDY) ||
	    (x==0 && level_xpos<0) ||
	    (x==2*SCR_FIELDX-1 && level_xpos>lev_fieldx-2*SCR_FIELDX) ||
	    x>lev_fieldx || y>lev_fieldy)
	  return;

	new_element = (button==1 ? new_element1 :
		       button==2 ? new_element2 :
		       button==3 ? new_element3 : 0);

	if (new_element != Feld[x+level_xpos][y+level_ypos])
	{
	  if (new_element==EL_SPIELFIGUR) /* Jeder nur EINE Figur bitte... */
	  {
	    int x,y;

	    for(x=0;x<lev_fieldx;x++) for(y=0;y<lev_fieldy;y++)
	    {
	      if (Feld[x][y]==EL_SPIELFIGUR || Feld[x][y]==EL_SPIELER1)
	      {
		Feld[x][y] = EL_LEERRAUM;
		if (x-level_xpos>=0 && x-level_xpos<2*SCR_FIELDX &&
		    y-level_ypos>=0 && y-level_ypos<2*SCR_FIELDY)
		  DrawMiniElement(x-level_xpos,y-level_ypos,EL_LEERRAUM);
	      }
	    }
	  }

	  Feld[x+level_xpos][y+level_ypos] = new_element;
	  DrawMiniElement(x,y,new_element);
	}
      }
      else if (!motion_status)	/* Mauszeiger nicht im Level-Feld */
	in_field_pressed = FALSE;
    }
    else			/********** KONTROLL-FENSTER **********/
    {
      int choice = CheckCountButtons(mx,my,button);
      int step = (button==1 ? 1 : button==2 ? 5 : button==3 ? 10 : 0);

      if (choice >= 0 && choice < 36 &&
	  DelayReached(&choice_delay, CHOICE_DELAY_VALUE))
      {
	if (!(choice % 2))
	  step = -step;

	choice /= 2;

	if (choice<11)
	{
	  level.score[choice] += step;
	  if (level.score[choice]<0)
	    level.score[choice] = 0;
	  else if (level.score[choice]>255)
	    level.score[choice] = 255;
	}
	else if (choice==11)
	{
	  level.tempo_amoebe += step;
	  if (level.tempo_amoebe<0)
	    level.tempo_amoebe = 0;
	  else if (level.tempo_amoebe>255)
	    level.tempo_amoebe = 255;
	}
	else if (choice==12)
	{
	  level.dauer_sieb += step;
	  if (level.dauer_sieb<0)
	    level.dauer_sieb = 0;
	  else if (level.dauer_sieb>255)
	    level.dauer_sieb = 255;
	}
	else if (choice==13)
	{
	  level.dauer_ablenk += step;
	  if (level.dauer_ablenk<0)
	    level.dauer_ablenk = 0;
	  else if (level.dauer_ablenk>255)
	    level.dauer_ablenk = 255;
	}
	else if (choice==14)
	{
	  level.edelsteine += step;
	  if (level.edelsteine<0)
	    level.edelsteine = 0;
	  else if (level.edelsteine>999)
	    level.edelsteine = 999;
	}
	else if (choice==15)
	{
	  level.time += step;
	  if (level.time<0)
	    level.time = 0;
	  else if (level.time>999)
	    level.time = 999;
	}
	else if (choice==16)
	{
	  lev_fieldx += step;
	  if (lev_fieldx<MIN_LEV_FIELDX)
	    lev_fieldx = MIN_LEV_FIELDX;
	  else if (lev_fieldx>MAX_LEV_FIELDX)
	    lev_fieldx = MAX_LEV_FIELDX;
	  level.fieldx = lev_fieldx;
	}
	else if (choice==17)
	{
	  lev_fieldy += step;
	  if (lev_fieldy<MIN_LEV_FIELDY)
	    lev_fieldy = MIN_LEV_FIELDY;
	  else if (lev_fieldy>MAX_LEV_FIELDY)
	    lev_fieldy = MAX_LEV_FIELDY;
	  level.fieldy = lev_fieldy;
	}

	if (choice<11)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+choice*ED_COUNT_GADGET_YSIZE,
		   int2str(level.score[choice],3),FS_SMALL,FC_YELLOW);
	else if (choice==11)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+11*ED_COUNT_GADGET_YSIZE,
		   int2str(level.tempo_amoebe,3),FS_SMALL,FC_YELLOW);
	else if (choice==12)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+12*ED_COUNT_GADGET_YSIZE,
		   int2str(level.dauer_sieb,3),FS_SMALL,FC_YELLOW);
	else if (choice==13)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+13*ED_COUNT_GADGET_YSIZE,
		   int2str(level.dauer_ablenk,3),FS_SMALL,FC_YELLOW);
	else if (choice==14)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+14*ED_COUNT_GADGET_YSIZE,
		   int2str(level.edelsteine,3),FS_SMALL,FC_YELLOW);
	else if (choice==15)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+15*ED_COUNT_GADGET_YSIZE,
		   int2str(level.time,3),FS_SMALL,FC_YELLOW);
	else if (choice==16)
	  DrawText(ED_SIZE_VALUE_XPOS,
		   ED_SIZE_VALUE_YPOS+0*ED_SIZE_GADGET_YSIZE,
		   int2str(level.fieldx,3),FS_SMALL,FC_YELLOW);
	else if (choice==17)
	  DrawText(ED_SIZE_VALUE_XPOS,
		   ED_SIZE_VALUE_YPOS+1*ED_SIZE_GADGET_YSIZE,
		   int2str(level.fieldy,3),FS_SMALL,FC_YELLOW);

	redraw_mask &= ~REDRAW_FIELD;
	if (choice<16)
	  XCopyArea(display,drawto,window,gc,
		    ED_COUNT_VALUE_XPOS,
		    ED_COUNT_VALUE_YPOS+choice*ED_COUNT_GADGET_YSIZE,
		    3*FONT2_XSIZE,FONT2_YSIZE,
		    ED_COUNT_VALUE_XPOS,
		    ED_COUNT_VALUE_YPOS+choice*ED_COUNT_GADGET_YSIZE);
	else
	  XCopyArea(display,drawto,window,gc,
		    ED_SIZE_VALUE_XPOS,
		    ED_SIZE_VALUE_YPOS+(choice-16)*ED_SIZE_GADGET_YSIZE,
		    3*FONT2_XSIZE,FONT2_YSIZE,
		    ED_SIZE_VALUE_XPOS,
		    ED_SIZE_VALUE_YPOS+(choice-16)*ED_SIZE_GADGET_YSIZE);
	XFlush(display);
      }

      switch(CheckCtrlButtons(mx,my,button))
      {
	case ED_BUTTON_EDIT:
	  CloseDoor(DOOR_CLOSE_2);
	  AdjustLevelScrollPosition();
	  DrawMiniLevel(level_xpos,level_ypos);
	  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
		    DOOR_GFX_PAGEX6,DOOR_GFX_PAGEY2,
		    VXSIZE,VYSIZE,
		    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);
	  OpenDoor(DOOR_OPEN_2);
	  edit_mode = TRUE;
	  break;
	case ED_BUTTON_CLEAR:
	  if (Request("Are you sure to clear this level ?",REQ_ASK))
	  {
	    for(x=0;x<MAX_LEV_FIELDX;x++) 
	      for(y=0;y<MAX_LEV_FIELDY;y++) 
		Feld[x][y] = EL_ERDREICH;
	    DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_UNDO:
	  if (leveldir[leveldir_nr].readonly ||
	      Request("Exit without saving ?",REQ_ASK | REQ_STAY_OPEN))
	  {
	    CloseDoor(DOOR_CLOSE_BOTH);
	    game_status=MAINMENU;
	    DrawMainMenu();
	  }
	  else
	  {
	    CloseDoor(DOOR_CLOSE_1);
	    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
	  }
	  break;
	case ED_BUTTON_EXIT:
	  {
	    int figur_vorhanden = FALSE;

	    if (leveldir[leveldir_nr].readonly)
	    {
	      Request("This level is read only !",REQ_CONFIRM);
	      break;
	    }

	    for(y=0;y<lev_fieldy;y++) 
	      for(x=0;x<lev_fieldx;x++)
		if (Feld[x][y]==EL_SPIELFIGUR || Feld[x][y]==EL_SPIELER1) 
		  figur_vorhanden = TRUE;

	    if (!figur_vorhanden)
	      Request("No Level without Gregor Mc Duffin please !",
			 REQ_CONFIRM);
	    else
	    {
	      if (Request("Save this level and kill the old ?",
			     REQ_ASK | REQ_STAY_OPEN))
	      {
		for(x=0;x<lev_fieldx;x++)
		  for(y=0;y<lev_fieldy;y++) 
		    Ur[x][y]=Feld[x][y];
		SaveLevel(level_nr);
	      }
	      CloseDoor(DOOR_CLOSE_BOTH);
	      game_status=MAINMENU;
	      DrawMainMenu();
	    }
	  }
	  break;
	default:
	  break;
      }

      if (mx>=ED_COUNT_GADGET_XPOS &&
	  mx<ED_COUNT_GADGET_XPOS+31*FONT2_XSIZE+10 &&
	  my>=ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE &&
	  my<ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE+ED_WIN_COUNT_YSIZE)
      {
	if (!name_typing)
	{
	  name_typing = TRUE;
	  DrawText(ED_COUNT_GADGET_XPOS+5,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   level.name,FS_SMALL,FC_GREEN);
	  DrawText(ED_COUNT_GADGET_XPOS+5+strlen(level.name)*FONT2_XSIZE,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   "<",FS_SMALL,FC_RED);
	}
      }
      else
      {
	if (name_typing)
	{
	  name_typing = FALSE;
	  DrawText(ED_COUNT_GADGET_XPOS+5,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   level.name,FS_SMALL,FC_YELLOW);
	  DrawText(ED_COUNT_GADGET_XPOS+5+strlen(level.name)*FONT2_XSIZE,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   " ",FS_SMALL,FC_RED);
	}
      }

      if (mx>=SX+29*MINI_TILEX && mx<SX+30*MINI_TILEX &&
	  my>=SY+26*MINI_TILEY && my<SY+27*MINI_TILEY)
      {
	int new_element;

	if (!button || button<1 || button>3)
	  return;

	new_element = (button==1 ? new_element1 :
		       button==2 ? new_element2 :
		       button==3 ? new_element3 : 0);

	if (new_element != level.amoebe_inhalt)
	{
	  level.amoebe_inhalt = new_element;
	  DrawMiniElement(29,26,new_element);
	}
      }

      if (mx>=SX+1*MINI_TILEX && mx<SX+(1+4*5)*MINI_TILEX &&
	  my>=SY+2*MINI_TILEY && my<SY+(2+3)*MINI_TILEY)
      {
	int x = (mx-SX-1*MINI_TILEX)/MINI_TILEX;
	int y = (my-SY-2*MINI_TILEY)/MINI_TILEY;
	int i = x/5;
	int new_element;

	x = x-i*5;
	if (i>=0 && i<43 && x>=0 && x<3 && y>=0 && y<3)
	{
	  if (button && !motion_status)
	    in_field_pressed = TRUE;

	  if (!button || !in_field_pressed || button<1 || button>3)
	    return;

	  new_element = (button==1 ? new_element1 :
			 button==2 ? new_element2 :
			 button==3 ? new_element3 : 0);

	  if (new_element != level.mampfer_inhalt[i][x][y])
	  {
	    level.mampfer_inhalt[i][x][y] = new_element;
	    DrawMiniElement(1+5*i+x,2+y,new_element);
	  }
	}
	else if (!motion_status)/* Mauszeiger nicht im Cruncher-Feld */
	  in_field_pressed = FALSE;
      }
      else if (!motion_status)	/* Mauszeiger nicht im Cruncher-Feld */
	in_field_pressed = FALSE;
    }
  }

  last_button = button;

  BackToFront();
}

void LevelNameTyping(KeySym key)
{
  unsigned char ascii = 0;
  int len = strlen(level.name);

  if (!name_typing)
    return;

  if (key>=XK_A && key<=XK_Z)
    ascii = 'A'+(char)(key-XK_A);
  else if (key>=XK_a && key<=XK_z)
    ascii = 'a'+(char)(key-XK_a);
  else if (key>=XK_0 && key<=XK_9)
    ascii = '0'+(char)(key-XK_0);
#ifdef XK_LATIN1
  else if (key>=XK_space && key<=XK_at)
    ascii = ' '+(char)(key-XK_space);
  else if (key==XK_Adiaeresis)
    ascii = '�';
  else if (key==XK_Odiaeresis)
    ascii = '�';
  else if (key==XK_Udiaeresis)
    ascii = '�';
  else if (key==XK_adiaeresis)
    ascii = '�';
  else if (key==XK_odiaeresis)
    ascii = '�';
  else if (key==XK_udiaeresis)
    ascii = '�';
  else if (key==XK_underscore)
    ascii = '_';
#endif

  if (ascii && len<MAX_LEVNAMLEN-2)
  {
    level.name[len] = ascii;
    level.name[len+1] = 0;
    len++;

    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_GREEN);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_GREEN);
    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"<",FS_SMALL,FC_RED);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"<",FS_SMALL,FC_RED);
  }
  else if ((key==XK_Delete || key==XK_BackSpace) && len>0)
  {
    level.name[len-1] = 0;
    len--;

    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"< ",FS_SMALL,FC_GREEN);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"< ",FS_SMALL,FC_GREEN);
  }
  else if (key==XK_Return)
  {
    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_YELLOW);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_YELLOW);
    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		" ",FS_SMALL,FC_YELLOW);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		" ",FS_SMALL,FC_YELLOW);

    name_typing = FALSE;
  }
}
