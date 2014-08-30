// ----------------------------------------------------------------------------
// Globals.c
// ----------------------------------------------------------------------------

#include "Globals.h"


boolean LevelLoaded;

boolean DemoAvailable;
boolean menBorder;

int FieldWidth;		// standard size = 60
int FieldHeight;	// standard size = 24
int HeaderSize;		// standard size = 96
int FieldMax, LevelMax;
long FileMax;

#if 1
int PlayField16[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
byte PlayField8[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
byte DisPlayField[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
#else
int *PlayField16;
byte *PlayField8;
byte *DisPlayField;
#endif

int TimerVar;
#if 1
short RandomSeed;
#else
int RandomSeed;
#endif

int FreezeZonks;

LevelInfoType LInfo;

int ScrollMinX, ScrollMaxX, ScrollMinY, ScrollMaxY;
int ScrollX, ScrollY;

int MurphyPosIndex, MurphyXPos, MurphyYPos;
int MurphyScreenXPos, MurphyScreenYPos;
int MurphyExplodePos, SplitMoveFlag, RedDiskReleaseMurphyPos;
int KillMurphyFlag, MurphyMoveCounter;
long YawnSleepCounter;
int MurphyVarFaceLeft;
int ScratchGravity, GravityFlag;
int RedDiskReleaseFlag, MovingPictureSequencePhase;

int YellowDisksExploded;
int AllowRedDiskCheat, AllowEatRightRedDiskBug;

int GameBusyFlag;
int InfotronsNeeded, TotalInfotronsNeeded;
int RedDiskCount;
int SnikSnaksElectronsFrozen;

int DemoKeyCode;

int RedDiskReleasePhase;

int fiGraphic[] =
{
  aniSpace,
  aniZonk,
  aniBase,
  aniMurphy,
  aniInfotron,
  aniRAM,
  aniHardWare,
  aniExit,
  aniOrangeDisk,
  aniPortRight,
  aniPortDown,
  aniPortLeft,
  aniPortUp,
  aniSpPortRight,
  aniSpPortDown,
  aniSpPortLeft,
  aniSpPortUp,
  aniSnikSnak,
  aniYellowDisk,
  aniTerminal,
  aniRedDisk,
  aniPortUpAndDown,
  aniPortLeftAndRight,
  aniPortAllDirections,
  aniElectron,
  aniBug,
  aniRAMLeft,
  aniRAMRight,
  aniHW0,
  aniHW1,
  aniHW2,
  aniHW3,
  aniHW4,
  aniHW5,
  aniHW6,
  aniHW7,
  aniHW8,
  aniHW9,
  aniRAMTop,
  aniRAMBottom,
  aniWallSpace
};

int aniSnikSnakTurningLeft[] =
{
  aniSnikSnakTurnUpToLeft,
  aniSnikSnakTurnLeftToDown,
  aniSnikSnakTurnDownToRight,
  aniSnikSnakTurnRightToUp
};

int aniSnikSnakTurningRight[] =
{
  aniSnikSnakTurnUpToRight,
  aniSnikSnakTurnRightToDown,
  aniSnikSnakTurnDownToLeft,
  aniSnikSnakTurnLeftToUp
};


int getSequenceLength(int sequence)
{
  switch (sequence)
  {
    case aniBug:
      return 14;

    case aniElectron:
    case aniExplosion:
      return 9;

    case aniTouchInfotron:
      return 7;

    case aniMurphyExit:
      return 40;

    case aniRedDisk:
      return 1;

    default:
      return 8;
  }
}

boolean isSnappingSequence(int sequence)
{
  switch (sequence)
  {
    case aniTouchBase:
    case aniTouchInfotron:
    case aniTouchRedDisk:
      return TRUE;

    default:
      return FALSE;
  }
}

void InitGlobals()
{
  AutoScrollFlag = True;
  FreezeZonks = 0;
  LevelLoaded = False;
  FieldWidth = 60;
  FieldHeight = 24;
  HeaderSize = 96;
  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;
  bPlaying = False;
  menBorder = False;

#if 0
  /* these defaults will be changed after reading a Supaplex level file */
  PlayField8 = REDIM_1D(sizeof(byte), 0, FieldMax);
  DisPlayField = REDIM_1D(sizeof(byte), 0, FieldMax);
  PlayField16 = REDIM_1D(sizeof(int), 0, FieldMax);

  AnimationPosTable = REDIM_1D(sizeof(int), 0, LevelMax);
  AnimationSubTable = REDIM_1D(sizeof(byte), 0, LevelMax);
  TerminalState = REDIM_1D(sizeof(byte), 0, FieldMax);
#endif
}

int GetSI(int X, int Y)
{
  return Y * FieldWidth + X;
}

int GetX(int si)
{
  return si % FieldWidth;
}

int GetY(int si)
{
  return si / FieldWidth;
}

int GetStretchX(int si)
{
  return StretchWidth * (si % FieldWidth);
}

int GetStretchY(int si)
{
  return StretchWidth * (si / FieldWidth);
}

void PrepareLevel()
{
  copyInternalEngineVars_SP();

#if 1
  SetDisplayRegion();
  SetScrollEdges();
#endif

  LevelLoaded = True;
}

#if 0
void Trace(char *Source, char *Message)
{
  printf("::: Trace: Source == '%s', Message == '%s'\n", Source, Message);
}

void ReportError(char *Source, char *Message)
{
  printf("::: ReportError: Source == '%s', Message == '%s'\n", Source, Message);
}
#endif

int Min(int A, int B)
{
  return (A < B ? A : B);
}

int Max(int A, int B)
{
  return (A < B ? B : A);
}
