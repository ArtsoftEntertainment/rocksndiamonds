// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// text.h
// ============================================================================

#ifndef TEXT_H
#define TEXT_H

#include "system.h"


// default fonts
#define MAIN_FONT_INITIAL_1	0
#define MAIN_FONT_INITIAL_2	1
#define MAIN_FONT_INITIAL_3	2
#define MAIN_FONT_INITIAL_4	3

// font colors
#define FC_RED			MAIN_FONT_INITIAL_1
#define FC_BLUE			MAIN_FONT_INITIAL_2
#define FC_GREEN		MAIN_FONT_INITIAL_3
#define FC_YELLOW		MAIN_FONT_INITIAL_4

// text output definitions
#define MAX_OUTPUT_LINESIZE	1024
#define MAX_OUTPUT_LINES	1024

// special constants for old ISO-8859-1 character byte values
#define CHAR_BYTE_UMLAUT_A	((char)0xc4)
#define CHAR_BYTE_UMLAUT_O	((char)0xd6)
#define CHAR_BYTE_UMLAUT_U	((char)0xdc)
#define CHAR_BYTE_UMLAUT_a	((char)0xe4)
#define CHAR_BYTE_UMLAUT_o	((char)0xf6)
#define CHAR_BYTE_UMLAUT_u	((char)0xfc)
#define CHAR_BYTE_SHARP_S	((char)0xdf)
#define CHAR_BYTE_COPYRIGHT	((char)0xa9)
#define CHAR_BYTE_REGISTERED	((char)0xae)
#define CHAR_BYTE_DEGREE	((char)0xb0)
#define CHAR_BYTE_CURSOR	((char)0xa0)

// special character mapping for default fonts
#define FONT_ASCII_CURSOR	((char)160)
#define FONT_ASCII_BUTTON	((char)128)
#define FONT_ASCII_UP		((char)129)
#define FONT_ASCII_DOWN		((char)130)
#define FONT_ASCII_LEFT		((char)'<')
#define FONT_ASCII_RIGHT	((char)'>')

#define MAP_FONT_ASCII(c)	((c) >= 'a' && (c) <= 'z' ? 'A' + (c) - 'a' : \
				 (c) == CHAR_BYTE_COPYRIGHT  ?  96 :	\
				 (c) == CHAR_BYTE_UMLAUT_a   ?  97 :	\
				 (c) == CHAR_BYTE_UMLAUT_A   ?  97 :	\
				 (c) == CHAR_BYTE_UMLAUT_o   ?  98 :	\
				 (c) == CHAR_BYTE_UMLAUT_O   ?  98 :	\
				 (c) == CHAR_BYTE_UMLAUT_u   ?  99 :	\
				 (c) == CHAR_BYTE_UMLAUT_U   ?  99 :	\
				 (c) == CHAR_BYTE_DEGREE     ? 100 :	\
				 (c) == CHAR_BYTE_REGISTERED ? 101 :	\
				 (c) == FONT_ASCII_CURSOR    ? 102 :	\
				 (c) == FONT_ASCII_BUTTON    ? 109 :	\
				 (c) == FONT_ASCII_UP	     ? 110 :	\
				 (c) == FONT_ASCII_DOWN	     ? 111 :	\
				 (c))

#define MAP_FONT_ASCII_EXT(c)	((c) == CHAR_BYTE_COPYRIGHT  ? 128 :	\
				 (c) == CHAR_BYTE_UMLAUT_A   ? 129 :	\
				 (c) == CHAR_BYTE_UMLAUT_O   ? 130 :	\
				 (c) == CHAR_BYTE_UMLAUT_U   ? 131 :	\
				 (c) == CHAR_BYTE_DEGREE     ? 132 :	\
				 (c) == CHAR_BYTE_REGISTERED ? 133 :	\
				 (c) == FONT_ASCII_CURSOR    ? 134 :	\
				 (c) == CHAR_BYTE_UMLAUT_a   ? 135 :	\
				 (c) == CHAR_BYTE_UMLAUT_o   ? 136 :	\
				 (c) == CHAR_BYTE_UMLAUT_u   ? 137 :	\
				 (c) == CHAR_BYTE_SHARP_S    ? 138 :	\
				 (c) == FONT_ASCII_BUTTON    ? 141 :	\
				 (c) == FONT_ASCII_UP	     ? 142 :	\
				 (c) == FONT_ASCII_DOWN	     ? 143 :	\
				 (c))

// 64 regular ordered ASCII characters, 6 special characters, 1 cursor char.
#define MIN_NUM_CHARS_PER_FONT			64
#define NUM_CHARS_PER_FONT_EXT			112
#define DEFAULT_NUM_CHARS_PER_FONT		(MIN_NUM_CHARS_PER_FONT + 6 +1)
#define DEFAULT_NUM_CHARS_PER_LINE		16


// structure definitions

struct WrappedTextInfo
{
  struct
  {
    char *text;
    int font_nr;
    boolean centered;
  } line[MAX_OUTPUT_LINES];

  // number of wrapped lines
  int num_lines;

  // total height of all lines
  int total_height;

  // internal info for processing lines
  int line_width, cut_length, max_height;
  int line_spacing, mask_mode;
  int line_visible_first;
  int line_visible_last;
};


// function definitions

void EnableDrawingText(void);
void DisableDrawingText(void);

void InitFontInfo(struct FontBitmapInfo *, int,
		  int (*function1)(int),
                  int (*function2)(char *),
                  char * (*function3)(int));
void FreeFontInfo(struct FontBitmapInfo *);

struct FontBitmapInfo *getFontBitmapInfo(int);

int getFontWidth(int);
int getFontHeight(int);
int getFontDrawOffsetX(int);
int getFontDrawOffsetY(int);
int getTextWidth(char *, int);

void getFontCharSource(int, char, Bitmap **, int *, int *);

int maxWordLengthInRequestString(char *);

void DrawInitText(char *, int, int);
void DrawInitTextHead(char *);
void DrawInitTextItem(char *);

void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawTextS(int, int, int, char *);
void DrawTextSCentered(int, int, char *);
void DrawTextSAligned(int, int, char *, int, int);
void DrawText(int, int, char *, int);
void DrawTextExt(DrawBuffer *, int, int, char *, int, int);

char *GetTextBufferFromFile(char *, int);
struct WrappedTextInfo *GetWrappedTextBuffer(char *, int, int, int, int, int, int, int, int, int,
                                             boolean, boolean, boolean);
struct WrappedTextInfo *GetWrappedTextFile(char *, int, int, int, int, int, int, int, int, int,
                                           boolean, boolean, boolean);
int DrawWrappedText(int, int, struct WrappedTextInfo *, int);
void FreeWrappedText(struct WrappedTextInfo *);

int DrawTextArea(int, int, char *, int, int, int, int, int, int, int, int, int,
		 boolean, boolean, boolean);
int DrawTextBuffer(int, int, char *, int, int, int, int, int, int, int, int, int,
		   boolean, boolean, boolean);
int DrawTextBufferS(int, int, char *, int, int, int, int, int, int, int, int, int,
		    boolean, boolean, boolean);
int DrawTextBufferVA(int, int, char *, va_list, int, int, int, int, int, int, int, int, int,
		     boolean, boolean, boolean);
int DrawTextFile(int, int, char *, int, int, int, int, int, int, int, int, int,
		 boolean, boolean, boolean);

#endif	// TEXT_H
