// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_main.h
// ============================================================================

#ifndef MM_MAIN_H
#define MM_MAIN_H

#define STD_LEV_FIELDX  MM_STD_PLAYFIELD_WIDTH
#define STD_LEV_FIELDY  MM_STD_PLAYFIELD_HEIGHT
#define MAX_LEV_FIELDX	MM_MAX_PLAYFIELD_WIDTH
#define MAX_LEV_FIELDY	MM_MAX_PLAYFIELD_HEIGHT

#define SCREENX(a)      (a)
#define SCREENY(a)      (a)
#define LEVELX(a)       (a)
#define LEVELY(a)       (a)

#define IN_FIELD(x, y, xsize, ysize)	((x) >= 0 && (x) < (xsize) &&	   \
					 (y) >= 0 && (y) < (ysize))
#define IN_FIELD_MINMAX(x, y, xmin, ymin, xmax, ymax)			   \
					((x) >= (xmin) && (x) <= (xmax) && \
					 (y) >= (ymin) && (y) <= (ymax))

#define IN_PIX_FIELD(x, y)	IN_FIELD(x, y, SXSIZE, SYSIZE)
#define IN_VIS_FIELD(x, y)	IN_FIELD(x, y, SCR_FIELDX, SCR_FIELDY)
#define IN_LEV_FIELD(x, y)	IN_FIELD(x, y, lev_fieldx, lev_fieldy)
#define IN_SCR_FIELD(x, y)	IN_FIELD_MINMAX(x, y, BX1, BY1, BX2, BY2)

// values for 'Elementeigenschaften'
#define EP_BIT_GRID		(1 << 0)
#define EP_BIT_MCDUFFIN		(1 << 1)
#define EP_BIT_RECTANGLE	(1 << 2)
#define EP_BIT_MIRROR		(1 << 3)
#define EP_BIT_MIRROR_FIXED	(1 << 4)
#define EP_BIT_POLAR		(1 << 5)
#define EP_BIT_POLAR_CROSS	(1 << 6)
#define EP_BIT_BEAMER		(1 << 7)
#define EP_BIT_CHAR		(1 << 8)
#define EP_BIT_REFLECTING	(1 << 9)
#define EP_BIT_ABSORBING	(1 << 10)
#define EP_BIT_INACTIVE		(1 << 11)
#define EP_BIT_WALL		(1 << 12)
#define EP_BIT_PACMAN		(1 << 13)
#define EP_BIT_ENVELOPE		(1 << 14)

#define IS_GRID(e)		(Elementeigenschaften[e] & EP_BIT_GRID)
#define IS_MCDUFFIN(e)		(Elementeigenschaften[e] & EP_BIT_MCDUFFIN)
#define IS_RECTANGLE(e)		(Elementeigenschaften[e] & EP_BIT_RECTANGLE)
#define IS_MIRROR(e)		(Elementeigenschaften[e] & EP_BIT_MIRROR)
#define IS_MIRROR_FIXED(e)	(Elementeigenschaften[e] & EP_BIT_MIRROR_FIXED)
#define IS_POLAR(e)		(Elementeigenschaften[e] & EP_BIT_POLAR)
#define IS_POLAR_CROSS(e)	(Elementeigenschaften[e] & EP_BIT_POLAR_CROSS)
#define IS_BEAMER_OLD(e)	(Elementeigenschaften[e] & EP_BIT_BEAMER)
#define IS_CHAR(e)		(Elementeigenschaften[e] & EP_BIT_CHAR)
#define IS_REFLECTING(e)	(Elementeigenschaften[e] & EP_BIT_REFLECTING)
#define IS_ABSORBING(e)		(Elementeigenschaften[e] & EP_BIT_ABSORBING)
#define IS_INACTIVE(e)		(Elementeigenschaften[e] & EP_BIT_INACTIVE)
#define IS_MM_WALL(e)		(Elementeigenschaften[e] & EP_BIT_WALL)
#define IS_PACMAN(e)		(Elementeigenschaften[e] & EP_BIT_PACMAN)
#define IS_ENVELOPE(e)		(Elementeigenschaften[e] & EP_BIT_ENVELOPE)

#define IS_WALL_STEEL(e)	((e) >= EL_WALL_STEEL_START &&		\
				 (e) <= EL_WALL_STEEL_END)
#define IS_WALL_WOOD(e)		((e) >= EL_WALL_WOOD_START &&		\
				 (e) <= EL_WALL_WOOD_END)
#define IS_WALL_ICE(e)		((e) >= EL_WALL_ICE_START &&		\
				 (e) <= EL_WALL_ICE_END)
#define IS_WALL_AMOEBA(e)	((e) >= EL_WALL_AMOEBA_START &&		\
				 (e) <= EL_WALL_AMOEBA_END)
#define IS_DF_WALL_STEEL(e)	((e) >= EL_DF_WALL_STEEL_START &&	\
				 (e) <= EL_DF_WALL_STEEL_END)
#define IS_DF_WALL_WOOD(e)	((e) >= EL_DF_WALL_WOOD_START &&	\
				 (e) <= EL_DF_WALL_WOOD_END)
#define IS_DF_WALL(e)		((e) >= EL_DF_WALL_START &&		\
				 (e) <= EL_DF_WALL_END)
#define IS_WALL(e)		(IS_MM_WALL(e) || IS_DF_WALL(e))
#define IS_WALL_CHANGING(e)	((e) >= EL_WALL_CHANGING_START &&	\
				 (e) <= EL_WALL_CHANGING_END)
#define IS_GRID_STEEL(e)	((e) >= EL_GRID_STEEL_START &&		\
				 (e) <= EL_GRID_STEEL_END)
#define IS_GRID_WOOD(e)		((e) >= EL_GRID_WOOD_START &&		\
				 (e) <= EL_GRID_WOOD_END)
#define IS_GRID_WOOD_FIXED(e)	((e) >= EL_GRID_WOOD_FIXED_START &&	\
				 (e) <= EL_GRID_WOOD_FIXED_END)
#define IS_GRID_STEEL_FIXED(e)	((e) >= EL_GRID_STEEL_FIXED_START &&	\
				 (e) <= EL_GRID_STEEL_FIXED_END)
#define IS_GRID_WOOD_AUTO(e)	((e) >= EL_GRID_WOOD_AUTO_START &&	\
				 (e) <= EL_GRID_WOOD_AUTO_END)
#define IS_GRID_STEEL_AUTO(e)	((e) >= EL_GRID_STEEL_AUTO_START &&	\
				 (e) <= EL_GRID_STEEL_AUTO_END)
#define IS_DF_GRID(e)		(IS_GRID_WOOD_FIXED(e) ||		\
				 IS_GRID_STEEL_FIXED(e) ||		\
				 IS_GRID_WOOD_AUTO(e) ||		\
				 IS_GRID_STEEL_AUTO(e))
#define IS_DF_MIRROR(e)		((e) >= EL_DF_MIRROR_START &&		\
				 (e) <= EL_DF_MIRROR_END)
#define IS_DF_MIRROR_AUTO(e)	((e) >= EL_DF_MIRROR_AUTO_START &&	\
				 (e) <= EL_DF_MIRROR_AUTO_END)
#define IS_DF_MIRROR_FIXED(e)	((e) >= EL_DF_MIRROR_FIXED_START &&	\
				 (e) <= EL_DF_MIRROR_FIXED_END)
#define IS_DF_SLOPE(e)		((e) >= EL_DF_SLOPE_START &&		\
				 (e) <= EL_DF_SLOPE_END)
#define IS_LASER(e)		((e) >= EL_LASER_START &&		\
				 (e) <= EL_LASER_END)
#define IS_RECEIVER(e)		((e) >= EL_RECEIVER_START &&		\
				 (e) <= EL_RECEIVER_END)
#define IS_FIBRE_OPTIC(e)	((e) >= EL_FIBRE_OPTIC_START &&		\
				 (e) <= EL_FIBRE_OPTIC_END)
#define FIBRE_OPTIC_NR(e)	(((e) - EL_FIBRE_OPTIC_START) / 2)
#define IS_BEAMER(e)		((e) >= EL_BEAMER_RED_START &&		\
				 (e) <= EL_BEAMER_BLUE_END)
#define BEAMER_CLASSIC_NR(e)	(((e) - EL_BEAMER_RED_START) / 16)
#define BEAMER_NR(e)		(IS_BEAMER(e) ? BEAMER_CLASSIC_NR(e) :	\
				 IS_FIBRE_OPTIC(e) ? (FIBRE_OPTIC_NR(e)+4) : 0)
#define IS_EXPLODING(e)		((e) == EL_EXPLODING_OPAQUE ||		\
				 (e) == EL_EXPLODING_TRANSP)

#define IS_EATABLE4PACMAN(e)	((e) == EL_EMPTY ||			\
				 (e) == EL_KETTLE ||			\
				 (e) == EL_CELL ||			\
				 (e) == EL_BOMB ||			\
				 IS_WALL_AMOEBA(e))

#define IS_ABSORBING_BLOCK(e)	(IS_WALL_WOOD(e) ||			\
				 IS_DF_WALL_WOOD(e) ||			\
				 (e) == EL_BLOCK_WOOD ||		\
				 (e) == EL_GATE_WOOD ||			\
				 (e) == EL_EXIT_CLOSED ||		\
				 (e) == EL_EXIT_OPEN)

#define IS_ENVELOPE_OPENING(e)	((e) >= EL_ENVELOPE_OPENING_START &&	\
				 (e) <= EL_ENVELOPE_OPENING_END)

#define ENVELOPE_NR(e)		((e) - EL_ENVELOPE_1)
#define ENVELOPE_OPENING_NR(e)	((e) - EL_ENVELOPE_1_OPENING)

#define CAN_MOVE(e)		((e) == EL_PACMAN)
#define IS_FREE(x, y)		(Tile[x][y] == EL_EMPTY)

#define IS_MOVING(x, y)		(MovPos[x][y] != 0)
#define IS_BLOCKED(x, y)	(Tile[x][y] == EL_BLOCKED)
#define IS_DRAWABLE(e)		((e) < EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)	((e) >= EL_BLOCKED)

#define WALL_BASE(e)		((e) & 0xfff0)
#define WALL_BITS(e)		((e) & 0x000f)

// boundaries of arrays etc.
#define MAX_PLAYER_NAME_LEN	10
#define MAX_LEVEL_NAME_LEN	32
#define MAX_LEVEL_AUTHOR_LEN	32
#define MAX_SCORE_ENTRIES	100
#define MAX_ELEMENTS		700		// 500 static + 200 runtime

#define MICROLEVEL_SCROLL_DELAY	50	// delay for scrolling micro level
#define MICROLEVEL_LABEL_DELAY	250	// delay for micro level label

// score for elements
#define SC_COLLECTIBLE		0
#define SC_UNUSED_1		1
#define SC_UNUSED_2		2
#define SC_UNUSED_3		3
#define SC_UNUSED_4		4
#define SC_UNUSED_5		5
#define SC_PACMAN		6
#define SC_UNUSED_7		7
#define SC_UNUSED_8		8
#define SC_KEY			9
#define SC_TIME_BONUS		10
#define SC_UNUSED_11		11
#define SC_UNUSED_12		12
#define SC_UNUSED_13		13
#define SC_LIGHTBALL		14
#define SC_UNUSED_15		15

#define LEVEL_SCORE_ELEMENTS	16	// level elements with score


extern DrawBuffer      *drawto_mm;
extern DrawBuffer      *bitmap_db_field;

extern int		game_status;
extern boolean		level_editor_test_game;

extern boolean		redraw[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		redraw_x1, redraw_y1;

extern short		Tile[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Hit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Box[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Angle[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern short		MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		AmoebaCnt[MAX_NUM_AMOEBA];
extern short		AmoebaCnt2[MAX_NUM_AMOEBA];
extern short		ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern int		GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern unsigned int	Elementeigenschaften[MAX_ELEMENTS];

extern int		level_nr;
extern int		lev_fieldx, lev_fieldy, scroll_x, scroll_y;

extern int		FX, FY, ScrollStepSize;
extern int		ScreenMovDir, ScreenMovPos, ScreenGfxPos;
extern int		GameFrameDelay;
extern int		FfwdFrameDelay;
extern int		BX1, BY1, BX2, BY2;
extern int		SBX_Left, SBX_Right;
extern int		SBY_Upper, SBY_Lower;
extern int		TimeFrames, TimePlayed, TimeLeft;

extern struct LevelInfo_MM	native_mm_level;
extern struct GameInfo_MM	game_mm;
extern struct LaserInfo		laser;

extern short 		LX, LY, XS, YS, ELX, ELY;
extern short 		CT, Ct;

extern int		dSX, dSY;
extern int		cSX, cSY;
extern int		cSX2, cSY2;
extern int		cFX, cFY;

extern Pixel		pen_fg, pen_bg, pen_ray, pen_magicolor[2];
extern int		color_status;

extern struct XY	Step[];
extern short		Sign[16];

extern char	       *element_info[];
extern int		num_element_info;

// wall positions (that can be OR'ed together)
#define WALL_TOPLEFT		1
#define WALL_TOPRIGHT		2
#define WALL_BOTTOMLEFT		4
#define WALL_BOTTOMRIGHT	8
#define WALL_LEFT		(WALL_TOPLEFT    | WALL_BOTTOMLEFT)
#define WALL_RIGHT		(WALL_TOPRIGHT   | WALL_BOTTOMRIGHT)
#define WALL_TOP		(WALL_TOPLEFT    | WALL_TOPRIGHT)
#define WALL_BOTTOM		(WALL_BOTTOMLEFT | WALL_BOTTOMRIGHT)

// game elements:
//	  0 - 499: real elements, stored in level file
//      500 - 699: flag elements, only used at runtime

// "real" level elements
#define EL_MM_START		0
#define EL_MM_START_1		EL_MM_START

#define EL_EMPTY		0
#define EL_MIRROR_START		1
#define EL_MIRROR_00		(EL_MIRROR_START + 0)
#define EL_MIRROR_01		(EL_MIRROR_START + 1)
#define EL_MIRROR_02		(EL_MIRROR_START + 2)
#define EL_MIRROR_03		(EL_MIRROR_START + 3)
#define EL_MIRROR_04		(EL_MIRROR_START + 4)
#define EL_MIRROR_05		(EL_MIRROR_START + 5)
#define EL_MIRROR_06		(EL_MIRROR_START + 6)
#define EL_MIRROR_07		(EL_MIRROR_START + 7)
#define EL_MIRROR_08		(EL_MIRROR_START + 8)
#define EL_MIRROR_09		(EL_MIRROR_START + 9)
#define EL_MIRROR_10		(EL_MIRROR_START + 10)
#define EL_MIRROR_11		(EL_MIRROR_START + 11)
#define EL_MIRROR_12		(EL_MIRROR_START + 12)
#define EL_MIRROR_13		(EL_MIRROR_START + 13)
#define EL_MIRROR_14		(EL_MIRROR_START + 14)
#define EL_MIRROR_15		(EL_MIRROR_START + 15)
#define EL_MIRROR_END		EL_MIRROR_15
#define EL_GRID_STEEL_START	17
#define EL_GRID_STEEL_00	(EL_GRID_STEEL_START + 0)
#define EL_GRID_STEEL_01	(EL_GRID_STEEL_START + 1)
#define EL_GRID_STEEL_02	(EL_GRID_STEEL_START + 2)
#define EL_GRID_STEEL_03	(EL_GRID_STEEL_START + 3)
#define EL_GRID_STEEL_END	EL_GRID_STEEL_03
#define EL_MCDUFFIN_START	21
#define EL_MCDUFFIN_RIGHT	(EL_MCDUFFIN_START + 0)
#define EL_MCDUFFIN_UP		(EL_MCDUFFIN_START + 1)
#define EL_MCDUFFIN_LEFT	(EL_MCDUFFIN_START + 2)
#define EL_MCDUFFIN_DOWN	(EL_MCDUFFIN_START + 3)
#define EL_MCDUFFIN_END		EL_MCDUFFIN_DOWN
#define EL_EXIT_CLOSED		25
#define EL_EXIT_OPENING_1	26
#define EL_EXIT_OPENING_2	27
#define EL_EXIT_OPEN		28
#define EL_KETTLE		29
#define EL_BOMB			30
#define EL_PRISM		31
#define EL_WALL_START		EL_WALL_STEEL_START
#define EL_WALL_STEEL_BASE	32
#define EL_WALL_STEEL_START	(EL_WALL_STEEL_BASE + 0)
#define EL_WALL_STEEL_END	(EL_WALL_STEEL_BASE + 15)
#define EL_WALL_WOOD_BASE	48
#define EL_WALL_WOOD_START	(EL_WALL_WOOD_BASE + 0)
#define EL_WALL_WOOD_END	(EL_WALL_WOOD_BASE + 15)
#define EL_WALL_ICE_BASE	64
#define EL_WALL_ICE_START	(EL_WALL_ICE_BASE + 0)
#define EL_WALL_ICE_END		(EL_WALL_ICE_BASE + 15)
#define EL_WALL_AMOEBA_BASE	80
#define EL_WALL_AMOEBA_START	(EL_WALL_AMOEBA_BASE + 0)
#define EL_WALL_AMOEBA_END	(EL_WALL_AMOEBA_BASE + 15)
#define EL_WALL_END		EL_WALL_AMOEBA_END
#define EL_BLOCK_WOOD		96
#define EL_GRAY_BALL		97
#define EL_BEAMER_START		98
#define EL_BEAMER_00		(EL_BEAMER_START + 0)
#define EL_BEAMER_01		(EL_BEAMER_START + 1)
#define EL_BEAMER_02		(EL_BEAMER_START + 2)
#define EL_BEAMER_03		(EL_BEAMER_START + 3)
#define EL_BEAMER_04		(EL_BEAMER_START + 4)
#define EL_BEAMER_05		(EL_BEAMER_START + 5)
#define EL_BEAMER_06		(EL_BEAMER_START + 6)
#define EL_BEAMER_07		(EL_BEAMER_START + 7)
#define EL_BEAMER_08		(EL_BEAMER_START + 8)
#define EL_BEAMER_09		(EL_BEAMER_START + 9)
#define EL_BEAMER_10		(EL_BEAMER_START + 10)
#define EL_BEAMER_11		(EL_BEAMER_START + 11)
#define EL_BEAMER_12		(EL_BEAMER_START + 12)
#define EL_BEAMER_13		(EL_BEAMER_START + 13)
#define EL_BEAMER_14		(EL_BEAMER_START + 14)
#define EL_BEAMER_15		(EL_BEAMER_START + 15)
#define EL_BEAMER_END		EL_BEAMER_15
#define EL_FUSE_ON		114
#define EL_PACMAN_START		115
#define EL_PACMAN_RIGHT		(EL_PACMAN_START + 0)
#define EL_PACMAN_UP		(EL_PACMAN_START + 1)
#define EL_PACMAN_LEFT		(EL_PACMAN_START + 2)
#define EL_PACMAN_DOWN		(EL_PACMAN_START + 3)
#define EL_PACMAN_END		EL_PACMAN_DOWN
#define EL_POLAR_START		119
#define EL_POLAR_00		(EL_POLAR_START + 0)
#define EL_POLAR_01		(EL_POLAR_START + 1)
#define EL_POLAR_02		(EL_POLAR_START + 2)
#define EL_POLAR_03		(EL_POLAR_START + 3)
#define EL_POLAR_04		(EL_POLAR_START + 4)
#define EL_POLAR_05		(EL_POLAR_START + 5)
#define EL_POLAR_06		(EL_POLAR_START + 6)
#define EL_POLAR_07		(EL_POLAR_START + 7)
#define EL_POLAR_08		(EL_POLAR_START + 8)
#define EL_POLAR_09		(EL_POLAR_START + 9)
#define EL_POLAR_10		(EL_POLAR_START + 10)
#define EL_POLAR_11		(EL_POLAR_START + 11)
#define EL_POLAR_12		(EL_POLAR_START + 12)
#define EL_POLAR_13		(EL_POLAR_START + 13)
#define EL_POLAR_14		(EL_POLAR_START + 14)
#define EL_POLAR_15		(EL_POLAR_START + 15)
#define EL_POLAR_END		EL_POLAR_15
#define EL_POLAR_CROSS_START	135
#define EL_POLAR_CROSS_00	(EL_POLAR_CROSS_START + 0)
#define EL_POLAR_CROSS_01	(EL_POLAR_CROSS_START + 1)
#define EL_POLAR_CROSS_02	(EL_POLAR_CROSS_START + 2)
#define EL_POLAR_CROSS_03	(EL_POLAR_CROSS_START + 3)
#define EL_POLAR_CROSS_END	EL_POLAR_CROSS_03
#define EL_MIRROR_FIXED_START	139
#define EL_MIRROR_FIXED_00	(EL_MIRROR_FIXED_START + 0)
#define EL_MIRROR_FIXED_01	(EL_MIRROR_FIXED_START + 1)
#define EL_MIRROR_FIXED_02	(EL_MIRROR_FIXED_START + 2)
#define EL_MIRROR_FIXED_03	(EL_MIRROR_FIXED_START + 3)
#define EL_MIRROR_FIXED_END	EL_MIRROR_FIXED_03
#define EL_GATE_STONE		143
#define EL_KEY			144
#define EL_LIGHTBULB_OFF	145
#define EL_LIGHTBULB_ON		146
#define EL_LIGHTBALL		147
#define EL_BLOCK_STONE		148
#define EL_GATE_WOOD		149
#define EL_FUEL_FULL		150
#define EL_GRID_WOOD_START	151
#define EL_GRID_WOOD_00		(EL_GRID_WOOD_START + 0)
#define EL_GRID_WOOD_01		(EL_GRID_WOOD_START + 1)
#define EL_GRID_WOOD_02		(EL_GRID_WOOD_START + 2)
#define EL_GRID_WOOD_03		(EL_GRID_WOOD_START + 3)
#define EL_GRID_WOOD_END	EL_GRID_WOOD_03
#define EL_FUEL_EMPTY		155
#define EL_ENVELOPE_1		156
#define EL_ENVELOPE_2		157
#define EL_ENVELOPE_3		158
#define EL_ENVELOPE_4		159

#define EL_MM_END_1		159

#define EL_CHAR_START		160
#define EL_CHAR_ASCII0		(EL_CHAR_START - 32)
#define EL_CHAR_AUSRUF		(EL_CHAR_ASCII0 + 33)
#define EL_CHAR_ZOLL		(EL_CHAR_ASCII0 + 34)
#define EL_CHAR_RAUTE		(EL_CHAR_ASCII0 + 35)
#define EL_CHAR_DOLLAR		(EL_CHAR_ASCII0 + 36)
#define EL_CHAR_PROZ		(EL_CHAR_ASCII0 + 37)
#define EL_CHAR_AMPERSAND	(EL_CHAR_ASCII0 + 38)
#define EL_CHAR_APOSTR		(EL_CHAR_ASCII0 + 39)
#define EL_CHAR_KLAMM1		(EL_CHAR_ASCII0 + 40)
#define EL_CHAR_KLAMM2		(EL_CHAR_ASCII0 + 41)
#define EL_CHAR_MULT		(EL_CHAR_ASCII0 + 42)
#define EL_CHAR_PLUS		(EL_CHAR_ASCII0 + 43)
#define EL_CHAR_KOMMA		(EL_CHAR_ASCII0 + 44)
#define EL_CHAR_MINUS		(EL_CHAR_ASCII0 + 45)
#define EL_CHAR_PUNKT		(EL_CHAR_ASCII0 + 46)
#define EL_CHAR_SLASH		(EL_CHAR_ASCII0 + 47)
#define EL_CHAR_0		(EL_CHAR_ASCII0 + 48)
#define EL_CHAR_9		(EL_CHAR_ASCII0 + 57)
#define EL_CHAR_DOPPEL		(EL_CHAR_ASCII0 + 58)
#define EL_CHAR_SEMIKL		(EL_CHAR_ASCII0 + 59)
#define EL_CHAR_LT		(EL_CHAR_ASCII0 + 60)
#define EL_CHAR_GLEICH		(EL_CHAR_ASCII0 + 61)
#define EL_CHAR_GT		(EL_CHAR_ASCII0 + 62)
#define EL_CHAR_FRAGE		(EL_CHAR_ASCII0 + 63)
#define EL_CHAR_AT		(EL_CHAR_ASCII0 + 64)
#define EL_CHAR_A		(EL_CHAR_ASCII0 + 65)
#define EL_CHAR_Z		(EL_CHAR_ASCII0 + 90)
#define EL_CHAR_AE		(EL_CHAR_ASCII0 + 91)
#define EL_CHAR_OE		(EL_CHAR_ASCII0 + 92)
#define EL_CHAR_UE		(EL_CHAR_ASCII0 + 93)
#define EL_CHAR_COPY		(EL_CHAR_ASCII0 + 94)
#define EL_CHAR_END		(EL_CHAR_START + 79)

#define EL_CHAR(x)		((x) == CHAR_BYTE_UMLAUT_A ? EL_CHAR_AE : \
				 (x) == CHAR_BYTE_UMLAUT_O ? EL_CHAR_OE : \
				 (x) == CHAR_BYTE_UMLAUT_U ? EL_CHAR_UE : \
				 EL_CHAR_A + (x) - 'A')

#define EL_MM_START_2		240

// elements for "Deflektor" style levels
#define EL_DF_START		EL_MM_START_2

#define EL_DF_MIRROR_START	EL_DF_START
#define EL_DF_MIRROR_00		(EL_DF_MIRROR_START + 0)
#define EL_DF_MIRROR_01		(EL_DF_MIRROR_START + 1)
#define EL_DF_MIRROR_02		(EL_DF_MIRROR_START + 2)
#define EL_DF_MIRROR_03		(EL_DF_MIRROR_START + 3)
#define EL_DF_MIRROR_04		(EL_DF_MIRROR_START + 4)
#define EL_DF_MIRROR_05		(EL_DF_MIRROR_START + 5)
#define EL_DF_MIRROR_06		(EL_DF_MIRROR_START + 6)
#define EL_DF_MIRROR_07		(EL_DF_MIRROR_START + 7)
#define EL_DF_MIRROR_08		(EL_DF_MIRROR_START + 8)
#define EL_DF_MIRROR_09		(EL_DF_MIRROR_START + 9)
#define EL_DF_MIRROR_10		(EL_DF_MIRROR_START + 10)
#define EL_DF_MIRROR_11		(EL_DF_MIRROR_START + 11)
#define EL_DF_MIRROR_12		(EL_DF_MIRROR_START + 12)
#define EL_DF_MIRROR_13		(EL_DF_MIRROR_START + 13)
#define EL_DF_MIRROR_14		(EL_DF_MIRROR_START + 14)
#define EL_DF_MIRROR_15		(EL_DF_MIRROR_START + 15)
#define EL_DF_MIRROR_END	EL_DF_MIRROR_15

#define EL_GRID_WOOD_FIXED_START 256
#define EL_GRID_WOOD_FIXED_00	(EL_GRID_WOOD_FIXED_START + 0)	//   0.0°
#define EL_GRID_WOOD_FIXED_01	(EL_GRID_WOOD_FIXED_START + 1)	//  22.5°
#define EL_GRID_WOOD_FIXED_02	(EL_GRID_WOOD_FIXED_START + 2)	//  45.0°
#define EL_GRID_WOOD_FIXED_03	(EL_GRID_WOOD_FIXED_START + 3)	//  67.5°
#define EL_GRID_WOOD_FIXED_04	(EL_GRID_WOOD_FIXED_START + 4)	//  90.0°
#define EL_GRID_WOOD_FIXED_05	(EL_GRID_WOOD_FIXED_START + 5)	// 112.5°
#define EL_GRID_WOOD_FIXED_06	(EL_GRID_WOOD_FIXED_START + 6)	// 135.0°
#define EL_GRID_WOOD_FIXED_07	(EL_GRID_WOOD_FIXED_START + 7)	// 157.5°
#define EL_GRID_WOOD_FIXED_END	EL_GRID_WOOD_FIXED_07

#define EL_GRID_STEEL_FIXED_START 264
#define EL_GRID_STEEL_FIXED_00	(EL_GRID_STEEL_FIXED_START + 0)	//   0.0°
#define EL_GRID_STEEL_FIXED_01	(EL_GRID_STEEL_FIXED_START + 1)	//  22.5°
#define EL_GRID_STEEL_FIXED_02	(EL_GRID_STEEL_FIXED_START + 2)	//  45.0°
#define EL_GRID_STEEL_FIXED_03	(EL_GRID_STEEL_FIXED_START + 3)	//  67.5°
#define EL_GRID_STEEL_FIXED_04	(EL_GRID_STEEL_FIXED_START + 4)	//  90.0°
#define EL_GRID_STEEL_FIXED_05	(EL_GRID_STEEL_FIXED_START + 5)	// 112.5°
#define EL_GRID_STEEL_FIXED_06	(EL_GRID_STEEL_FIXED_START + 6)	// 135.0°
#define EL_GRID_STEEL_FIXED_07	(EL_GRID_STEEL_FIXED_START + 7)	// 157.5°
#define EL_GRID_STEEL_FIXED_END	EL_GRID_STEEL_FIXED_07

#define EL_DF_WALL_WOOD_BASE	272
#define EL_DF_WALL_WOOD_START	(EL_DF_WALL_WOOD_BASE + 0)
#define EL_DF_WALL_WOOD_END	(EL_DF_WALL_WOOD_BASE + 15)

#define EL_DF_WALL_STEEL_BASE	288
#define EL_DF_WALL_STEEL_START	(EL_DF_WALL_STEEL_BASE + 0)
#define EL_DF_WALL_STEEL_END	(EL_DF_WALL_STEEL_BASE + 15)

#define EL_DF_WALL_START	EL_DF_WALL_WOOD_START
#define EL_DF_WALL_END		EL_DF_WALL_STEEL_END

#define EL_DF_EMPTY		304
#define EL_CELL			305
#define EL_MINE			306
#define EL_REFRACTOR		307

#define EL_LASER_START		308
#define EL_LASER_RIGHT		(EL_LASER_START + 0)
#define EL_LASER_UP		(EL_LASER_START + 1)
#define EL_LASER_LEFT		(EL_LASER_START + 2)
#define EL_LASER_DOWN		(EL_LASER_START + 3)
#define EL_LASER_END		EL_LASER_DOWN

#define EL_RECEIVER_START	312
#define EL_RECEIVER_RIGHT	(EL_RECEIVER_START + 0)
#define EL_RECEIVER_UP		(EL_RECEIVER_START + 1)
#define EL_RECEIVER_LEFT	(EL_RECEIVER_START + 2)
#define EL_RECEIVER_DOWN	(EL_RECEIVER_START + 3)
#define EL_RECEIVER_END		EL_RECEIVER_DOWN

#define EL_FIBRE_OPTIC_START	316
#define EL_FIBRE_OPTIC_00	(EL_FIBRE_OPTIC_START + 0)
#define EL_FIBRE_OPTIC_01	(EL_FIBRE_OPTIC_START + 1)
#define EL_FIBRE_OPTIC_02	(EL_FIBRE_OPTIC_START + 2)
#define EL_FIBRE_OPTIC_03	(EL_FIBRE_OPTIC_START + 3)
#define EL_FIBRE_OPTIC_04	(EL_FIBRE_OPTIC_START + 4)
#define EL_FIBRE_OPTIC_05	(EL_FIBRE_OPTIC_START + 5)
#define EL_FIBRE_OPTIC_06	(EL_FIBRE_OPTIC_START + 6)
#define EL_FIBRE_OPTIC_07	(EL_FIBRE_OPTIC_START + 7)
#define EL_FIBRE_OPTIC_END	EL_FIBRE_OPTIC_07

#define EL_DF_MIRROR_AUTO_START	324
#define EL_DF_MIRROR_AUTO_00	(EL_DF_MIRROR_AUTO_START + 0)
#define EL_DF_MIRROR_AUTO_01	(EL_DF_MIRROR_AUTO_START + 1)
#define EL_DF_MIRROR_AUTO_02	(EL_DF_MIRROR_AUTO_START + 2)
#define EL_DF_MIRROR_AUTO_03	(EL_DF_MIRROR_AUTO_START + 3)
#define EL_DF_MIRROR_AUTO_04	(EL_DF_MIRROR_AUTO_START + 4)
#define EL_DF_MIRROR_AUTO_05	(EL_DF_MIRROR_AUTO_START + 5)
#define EL_DF_MIRROR_AUTO_06	(EL_DF_MIRROR_AUTO_START + 6)
#define EL_DF_MIRROR_AUTO_07	(EL_DF_MIRROR_AUTO_START + 7)
#define EL_DF_MIRROR_AUTO_08	(EL_DF_MIRROR_AUTO_START + 8)
#define EL_DF_MIRROR_AUTO_09	(EL_DF_MIRROR_AUTO_START + 9)
#define EL_DF_MIRROR_AUTO_10	(EL_DF_MIRROR_AUTO_START + 10)
#define EL_DF_MIRROR_AUTO_11	(EL_DF_MIRROR_AUTO_START + 11)
#define EL_DF_MIRROR_AUTO_12	(EL_DF_MIRROR_AUTO_START + 12)
#define EL_DF_MIRROR_AUTO_13	(EL_DF_MIRROR_AUTO_START + 13)
#define EL_DF_MIRROR_AUTO_14	(EL_DF_MIRROR_AUTO_START + 14)
#define EL_DF_MIRROR_AUTO_15	(EL_DF_MIRROR_AUTO_START + 15)
#define EL_DF_MIRROR_AUTO_END	EL_DF_MIRROR_AUTO_15

#define EL_GRID_WOOD_AUTO_START 340
#define EL_GRID_WOOD_AUTO_00	(EL_GRID_WOOD_AUTO_START + 0)
#define EL_GRID_WOOD_AUTO_01	(EL_GRID_WOOD_AUTO_START + 1)
#define EL_GRID_WOOD_AUTO_02	(EL_GRID_WOOD_AUTO_START + 2)
#define EL_GRID_WOOD_AUTO_03	(EL_GRID_WOOD_AUTO_START + 3)
#define EL_GRID_WOOD_AUTO_04	(EL_GRID_WOOD_AUTO_START + 4)
#define EL_GRID_WOOD_AUTO_05	(EL_GRID_WOOD_AUTO_START + 5)
#define EL_GRID_WOOD_AUTO_06	(EL_GRID_WOOD_AUTO_START + 6)
#define EL_GRID_WOOD_AUTO_07	(EL_GRID_WOOD_AUTO_START + 7)
#define EL_GRID_WOOD_AUTO_END	EL_GRID_WOOD_AUTO_07

#define EL_GRID_STEEL_AUTO_START 348
#define EL_GRID_STEEL_AUTO_00	(EL_GRID_STEEL_AUTO_START + 0)
#define EL_GRID_STEEL_AUTO_01	(EL_GRID_STEEL_AUTO_START + 1)
#define EL_GRID_STEEL_AUTO_02	(EL_GRID_STEEL_AUTO_START + 2)
#define EL_GRID_STEEL_AUTO_03	(EL_GRID_STEEL_AUTO_START + 3)
#define EL_GRID_STEEL_AUTO_04	(EL_GRID_STEEL_AUTO_START + 4)
#define EL_GRID_STEEL_AUTO_05	(EL_GRID_STEEL_AUTO_START + 5)
#define EL_GRID_STEEL_AUTO_06	(EL_GRID_STEEL_AUTO_START + 6)
#define EL_GRID_STEEL_AUTO_07	(EL_GRID_STEEL_AUTO_START + 7)
#define EL_GRID_STEEL_AUTO_END	EL_GRID_STEEL_AUTO_07

#define EL_DF_END		355

#define EL_BEAMER_RED_START	356
#define EL_BEAMER_RED_END	(EL_BEAMER_RED_START + 15)
#define EL_BEAMER_YELLOW_START	372
#define EL_BEAMER_YELLOW_END	(EL_BEAMER_YELLOW_START + 15)
#define EL_BEAMER_GREEN_START	388
#define EL_BEAMER_GREEN_END	(EL_BEAMER_GREEN_START + 15)
#define EL_BEAMER_BLUE_START	404
#define EL_BEAMER_BLUE_END	(EL_BEAMER_BLUE_START + 15)

// element definitions partially used for drawing graphics
#define EL_MCDUFFIN		420
#define EL_PACMAN		421
#define EL_FUSE_OFF		422
#define EL_WALL_STEEL		423
#define EL_WALL_WOOD		424
#define EL_WALL_ICE		425
#define EL_WALL_AMOEBA		426
#define EL_LASER		427
#define EL_RECEIVER		428
#define EL_DF_WALL_STEEL	429
#define EL_DF_WALL_WOOD		430

#define EL_DF_MIRROR_FIXED_START 431
#define EL_DF_MIRROR_FIXED_00	(EL_DF_MIRROR_FIXED_START + 0)
#define EL_DF_MIRROR_FIXED_01	(EL_DF_MIRROR_FIXED_START + 1)
#define EL_DF_MIRROR_FIXED_02	(EL_DF_MIRROR_FIXED_START + 2)
#define EL_DF_MIRROR_FIXED_03	(EL_DF_MIRROR_FIXED_START + 3)
#define EL_DF_MIRROR_FIXED_04	(EL_DF_MIRROR_FIXED_START + 4)
#define EL_DF_MIRROR_FIXED_05	(EL_DF_MIRROR_FIXED_START + 5)
#define EL_DF_MIRROR_FIXED_06	(EL_DF_MIRROR_FIXED_START + 6)
#define EL_DF_MIRROR_FIXED_07	(EL_DF_MIRROR_FIXED_START + 7)
#define EL_DF_MIRROR_FIXED_08	(EL_DF_MIRROR_FIXED_START + 8)
#define EL_DF_MIRROR_FIXED_09	(EL_DF_MIRROR_FIXED_START + 9)
#define EL_DF_MIRROR_FIXED_10	(EL_DF_MIRROR_FIXED_START + 10)
#define EL_DF_MIRROR_FIXED_11	(EL_DF_MIRROR_FIXED_START + 11)
#define EL_DF_MIRROR_FIXED_12	(EL_DF_MIRROR_FIXED_START + 12)
#define EL_DF_MIRROR_FIXED_13	(EL_DF_MIRROR_FIXED_START + 13)
#define EL_DF_MIRROR_FIXED_14	(EL_DF_MIRROR_FIXED_START + 14)
#define EL_DF_MIRROR_FIXED_15	(EL_DF_MIRROR_FIXED_START + 15)
#define EL_DF_MIRROR_FIXED_END	EL_DF_MIRROR_FIXED_15

#define EL_DF_SLOPE_START	 447
#define EL_DF_SLOPE_00		(EL_DF_SLOPE_START + 0)
#define EL_DF_SLOPE_01		(EL_DF_SLOPE_START + 1)
#define EL_DF_SLOPE_02		(EL_DF_SLOPE_START + 2)
#define EL_DF_SLOPE_03		(EL_DF_SLOPE_START + 3)
#define EL_DF_SLOPE_END		EL_DF_SLOPE_03

#define EL_MM_END_2		450
#define EL_MM_END		EL_MM_END_2

// "real" (and therefore drawable) runtime elements
#define EL_EXIT_OPENING		500
#define EL_EXIT_CLOSING		501
#define EL_GRAY_BALL_ACTIVE	502
#define EL_GRAY_BALL_OPENING	503
#define EL_WALL_ICE_SHRINKING	504
#define EL_WALL_AMOEBA_GROWING	505
#define EL_BOMB_ACTIVE		506
#define EL_MINE_ACTIVE		507
#define EL_ENVELOPE_1_OPENING	508
#define EL_ENVELOPE_2_OPENING	509
#define EL_ENVELOPE_3_OPENING	510
#define EL_ENVELOPE_4_OPENING	511

#define EL_ENVELOPE_OPENING_START	EL_ENVELOPE_1_OPENING
#define EL_ENVELOPE_OPENING_END		EL_ENVELOPE_4_OPENING

#define EL_WALL_CHANGING_BASE	512
#define EL_WALL_CHANGING_START	(EL_WALL_CHANGING_BASE + 0)
#define EL_WALL_CHANGING_END	(EL_WALL_CHANGING_BASE + 15)

#define EL_FIRST_RUNTIME_EL	EL_EXIT_OPENING

// "unreal" (and therefore not drawable) runtime elements
#define EL_BLOCKED		600
#define EL_EXPLODING_OPAQUE	601
#define EL_EXPLODING_TRANSP	602


// game graphics:
//	  0 -  191: graphics from "MirrorScreen"
//	192 -  255: pseudo graphics mapped to "MirrorScreen"
//	256 -  511: graphics from "MirrorFont"
//	512 -  767: graphics from "MirrorDF"

#define IMG_EMPTY		IMG_EMPTY_SPACE

// values for graphics/sounds action types
#define MM_ACTION_DEFAULT	0
#define MM_ACTION_WAITING	1
#define MM_ACTION_FALLING	2
#define MM_ACTION_MOVING	3
#define MM_ACTION_DIGGING	4
#define MM_ACTION_SNAPPING	5
#define MM_ACTION_COLLECTING	6
#define MM_ACTION_DROPPING	7
#define MM_ACTION_PUSHING	8
#define MM_ACTION_WALKING	9
#define MM_ACTION_PASSING	10
#define MM_ACTION_IMPACT	11
#define MM_ACTION_BREAKING	12
#define MM_ACTION_ACTIVATING	13
#define MM_ACTION_DEACTIVATING	14
#define MM_ACTION_OPENING	15
#define MM_ACTION_CLOSING	16
#define MM_ACTION_ATTACKING	17
#define MM_ACTION_GROWING	18
#define MM_ACTION_SHRINKING	19
#define MM_ACTION_ACTIVE	20
#define MM_ACTION_FILLING	21
#define MM_ACTION_EMPTYING	22
#define MM_ACTION_CHANGING	23
#define MM_ACTION_EXPLODING	24
#define MM_ACTION_BORING	25
#define MM_ACTION_BORING_1	26
#define MM_ACTION_BORING_2	27
#define MM_ACTION_BORING_3	28
#define MM_ACTION_BORING_4	29
#define MM_ACTION_BORING_5	30
#define MM_ACTION_BORING_6	31
#define MM_ACTION_BORING_7	32
#define MM_ACTION_BORING_8	33
#define MM_ACTION_BORING_9	34
#define MM_ACTION_BORING_10	35
#define MM_ACTION_SLEEPING	36
#define MM_ACTION_SLEEPING_1	37
#define MM_ACTION_SLEEPING_2	38
#define MM_ACTION_SLEEPING_3	39
#define MM_ACTION_AWAKENING	40
#define MM_ACTION_DYING		41
#define MM_ACTION_TURNING		42
#define MM_ACTION_TURNING_FROM_LEFT	43
#define MM_ACTION_TURNING_FROM_RIGHT	44
#define MM_ACTION_TURNING_FROM_UP	45
#define MM_ACTION_TURNING_FROM_DOWN	46
#define MM_ACTION_SMASHED_BY_ROCK	47
#define MM_ACTION_SMASHED_BY_SPRING	48
#define MM_ACTION_EATING		49
#define MM_ACTION_TWINKLING		50
#define MM_ACTION_SPLASHING		51
#define MM_ACTION_HITTING		52

// laser angles (directions)
#define ANG_RAY_RIGHT		0
#define ANG_RAY_UP		4
#define ANG_RAY_LEFT		8
#define ANG_RAY_DOWN		12

// laser angles (degree)
#define ANG_RAY_0		0
#define ANG_RAY_90		4
#define ANG_RAY_180		8
#define ANG_RAY_270		12
#define IS_22_5_ANGLE(angle)	((angle) % 2)
#define IS_90_ANGLE(angle)	(!((angle) % 4))
#define IS_45_ANGLE(angle)	(!(((angle) + 2) % 4))
#define IS_HORIZ_ANGLE(angle)	(!((angle) % 8))
#define IS_VERT_ANGLE(angle)	(!(((angle) + 4) % 8))

// mirror angles
#define ANG_MIRROR_0		0
#define ANG_MIRROR_45		4
#define ANG_MIRROR_90		8
#define ANG_MIRROR_135		12

// positions for checking where laser already hits element
#define HIT_POS_CENTER		1
#define HIT_POS_EDGE		2
#define HIT_POS_BETWEEN		4

// masks for scanning elements
#define HIT_MASK_NO_HIT		0
#define HIT_MASK_TOPLEFT	1
#define HIT_MASK_TOPRIGHT	2
#define HIT_MASK_BOTTOMLEFT	4
#define HIT_MASK_BOTTOMRIGHT	8
#define HIT_MASK_LEFT		(HIT_MASK_TOPLEFT | HIT_MASK_BOTTOMLEFT)
#define HIT_MASK_RIGHT		(HIT_MASK_TOPRIGHT | HIT_MASK_BOTTOMRIGHT)
#define HIT_MASK_TOP		(HIT_MASK_TOPLEFT | HIT_MASK_TOPRIGHT)
#define HIT_MASK_BOTTOM		(HIT_MASK_BOTTOMLEFT | HIT_MASK_BOTTOMRIGHT)
#define HIT_MASK_DIAGONAL_1	(HIT_MASK_TOPLEFT  | HIT_MASK_BOTTOMRIGHT)
#define HIT_MASK_DIAGONAL_2	(HIT_MASK_TOPRIGHT | HIT_MASK_BOTTOMLEFT)
#define HIT_MASK_ALL		(HIT_MASK_LEFT | HIT_MASK_RIGHT)

// step values for rotating elements
#define ROTATE_NO_ROTATING	0
#define ROTATE_LEFT		(+1)
#define ROTATE_RIGHT		(-1)
#define BUTTON_ROTATION(button)	((button) == MB_LEFTBUTTON  ? ROTATE_LEFT :  \
				 (button) == MB_RIGHTBUTTON ? ROTATE_RIGHT : \
				 ROTATE_NO_ROTATING)

// game over values
#define GAME_OVER_NO_ENERGY	1
#define GAME_OVER_OVERLOADED	2
#define GAME_OVER_BOMB		3

#define PROGRAM_VERSION_MAJOR	2
#define PROGRAM_VERSION_MINOR	0
#define PROGRAM_VERSION_PATCH	2
#define PROGRAM_VERSION_STRING	"2.0.2"

// functions for version handling
#define MM_VERSION_IDENT(x,y,z)	VERSION_IDENT(x,y,z,0)
#define MM_VERSION_MAJOR(x)	VERSION_PART_1(x)
#define MM_VERSION_MINOR(x)	VERSION_PART_2(x)
#define MM_VERSION_PATCH(x)	VERSION_PART_3(x)

// file version numbers for resource files (levels, score, setup, etc.)
// currently supported/known file version numbers:
//	1.4 (still in use)
//	2.0 (actual)

#define MM_FILE_VERSION_1_4	MM_VERSION_IDENT(1,4,0)
#define MM_FILE_VERSION_2_0	MM_VERSION_IDENT(2,0,0)

// file version does not change for every program version, but is changed
// when new features are introduced that are incompatible with older file
// versions, so that they can be treated accordingly
#define MM_FILE_VERSION_ACTUAL	MM_FILE_VERSION_2_0

#define MM_GAME_VERSION_ACTUAL	MM_VERSION_IDENT(PROGRAM_VERSION_MAJOR, \
						 PROGRAM_VERSION_MINOR, \
						 PROGRAM_VERSION_PATCH)

#endif	// MM_MAIN_H
