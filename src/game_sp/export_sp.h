// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// export_sp.h
// ============================================================================

#ifndef EXPORT_SP_H
#define EXPORT_SP_H

// ============================================================================
// functions and definitions exported from game_sp to main program
// ============================================================================

// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

#define SP_MAX_PLAYFIELD_WIDTH		MAX_PLAYFIELD_WIDTH
#define SP_MAX_PLAYFIELD_HEIGHT		MAX_PLAYFIELD_HEIGHT

#define SP_NUM_LEVELS_PER_PACKAGE	111

#define SP_STD_PLAYFIELD_WIDTH		60
#define SP_STD_PLAYFIELD_HEIGHT		24
#define SP_LEVEL_NAME_LEN		23
#define SP_MAX_SPECIAL_PORTS		10

#define SP_HEADER_SIZE			96
#define SP_STD_PLAYFIELD_SIZE		(SP_STD_PLAYFIELD_WIDTH *	\
					 SP_STD_PLAYFIELD_HEIGHT)
#define SP_MAX_PLAYFIELD_SIZE		(SP_MAX_PLAYFIELD_WIDTH *	\
					 SP_MAX_PLAYFIELD_HEIGHT)
#define SP_STD_LEVEL_SIZE		(SP_HEADER_SIZE + SP_STD_PLAYFIELD_SIZE)

#define SP_FRAMES_PER_SECOND		35

// use a much higher value to be able to load ultra-long MPX demo files
// (like for level collection 78, level 88 ("WAITING FOR GODOT AGAIN"))
// #define SP_MAX_TAPE_LEN			500000
#define SP_MAX_TAPE_LEN			64010	// (see "spfix63.doc")


// sound actions

#define actActive			0
#define actImpact			1
#define actExploding			2
#define actDigging			3
#define actSnapping			4
#define actCollecting			5
#define actPassing			6
#define actPushing			7
#define actDropping			8


// ----------------------------------------------------------------------------
// data structure definitions
// ----------------------------------------------------------------------------

#ifndef HAS_SpecialPortType
typedef struct
{
  short PortLocation; // = 2*(x+(y*60))		// big endian format
  byte Gravity; // 1 = turn on, anything else (0) = turn off
  byte FreezeZonks; // 2 = turn on, anything else (0) = turn off  (1=off!)
  byte FreezeEnemies; // 1 = turn on, anything else (0) = turn off
  byte UnUsed;
} SpecialPortType;
#define HAS_SpecialPortType
#endif

#ifndef HAS_LevelInfoType
typedef struct
{
  byte UnUsed[4];
  byte InitialGravity; // 1=on, anything else (0) = off
  byte Version; // SpeedFixVersion XOR &H20
  char LevelTitle[23];
  byte InitialFreezeZonks; // 2=on, anything else (0) = off.  (1=off too!)
  byte InfotronsNeeded;

  // Number of Infotrons needed. 0 means that Supaplex will count the total
  // amount of Infotrons in the level, and use the low byte of that number.
  // (A multiple of 256 Infotrons will then result in 0-to-eat, etc.!)
  byte SpecialPortCount; // Maximum 10 allowed!
  SpecialPortType SpecialPort[10];
  byte SpeedByte; // = Speed XOR Highbyte(RandomSeed)
  byte CheckSumByte; // = CheckSum XOR SpeedByte
  short DemoRandomSeed;				// little endian format
} LevelInfoType;
#define HAS_LevelInfoType
#endif

struct GameInfo_SP
{
  boolean level_solved;
  boolean game_over;

  // needed for updating panel
  int time_played;
  int infotrons_still_needed;
  int red_disk_count;
  int score;

  // needed for engine snapshots
  char **preceding_buffer;
  int preceding_buffer_size;

  int scroll_xoffset, scroll_yoffset;
};

struct DemoInfo_SP
{
  boolean is_available;		// structure contains valid demo

  int level_nr;			// number of corresponding level

  int length;			// number of demo entries
  byte data[SP_MAX_TAPE_LEN];	// array of demo entries
};

struct LevelInfo_SP
{
  LevelInfoType header;
  byte header_raw_bytes[SP_HEADER_SIZE];

  int width, height;

  byte playfield[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];

  struct DemoInfo_SP demo;
};

struct GraphicInfo_SP
{
  Bitmap *bitmap;
  int src_x, src_y;
  int src_offset_x, src_offset_y;
  int dst_offset_x, dst_offset_y;
  int width, height;

  Bitmap *crumbled_bitmap;
  int crumbled_src_x, crumbled_src_y;
  int crumbled_border_size;

  boolean has_crumbled_graphics;
  boolean preserve_background;

  int unique_identifier;	// used to identify needed screen updates
};

struct EngineSnapshotInfo_SP
{
  struct GameInfo_SP game_sp;

  int PlayField16[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
  byte PlayField8[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
  byte DisPlayField[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];

  int AnimationPosTable[SP_MAX_PLAYFIELD_SIZE];
  byte AnimationSubTable[SP_MAX_PLAYFIELD_SIZE];
  byte TerminalState[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GameInfo_SP game_sp;
extern struct LevelInfo_SP native_sp_level;
extern struct EngineSnapshotInfo_SP engine_snapshot_sp;

void sp_open_all(void);
void sp_close_all(void);

void InitPrecedingPlayfieldMemory(void);
void InitGfxBuffers_SP(void);

void InitGameEngine_SP(void);
void GameActions_SP(byte[MAX_PLAYERS]);

unsigned int InitEngineRandom_SP(int);

void setLevelInfoToDefaults_SP(void);
void copyInternalEngineVars_SP(void);
boolean LoadNativeLevel_SP(char *, int, boolean);
boolean SaveNativeLevel_SP(char *);

int getFieldbufferOffsetX_SP(void);
int getFieldbufferOffsetY_SP(void);

void BlitScreenToBitmap_SP(Bitmap *);
void RedrawPlayfield_SP(boolean);

void LoadEngineSnapshotValues_SP(void);
void SaveEngineSnapshotValues_SP(ListNode **);

int map_key_RND_to_SP(int);
int map_key_SP_to_RND(int);

int getRedDiskReleaseFlag_SP(void);

#endif	// EXPORT_SP_H
