// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// text.c
// ============================================================================

#include <stdio.h>
#include <stdarg.h>

#include "text.h"
#include "misc.h"


// ============================================================================
// static font variables
// ============================================================================

boolean text_drawing_enabled = TRUE;


// ============================================================================
// font functions
// ============================================================================

void EnableDrawingText(void)
{
  text_drawing_enabled = TRUE;
}

void DisableDrawingText(void)
{
  text_drawing_enabled = FALSE;
}

void InitFontInfo(struct FontBitmapInfo *font_bitmap_info, int num_fonts,
		  int (*select_font_function)(int),
		  int (*get_font_from_token_function)(char *),
		  char * (*get_token_from_font_function)(int))
{
  gfx.num_fonts = num_fonts;
  gfx.font_bitmap_info = font_bitmap_info;
  gfx.select_font_function = select_font_function;
  gfx.get_font_from_token_function = get_font_from_token_function;
  gfx.get_token_from_font_function = get_token_from_font_function;
}

void FreeFontInfo(struct FontBitmapInfo *font_bitmap_info)
{
  if (font_bitmap_info == NULL)
    return;

  free(font_bitmap_info);
}

struct FontBitmapInfo *getFontBitmapInfo(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return &gfx.font_bitmap_info[font_bitmap_id];
}

int getFontWidth(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return gfx.font_bitmap_info[font_bitmap_id].width;
}

int getFontHeight(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return gfx.font_bitmap_info[font_bitmap_id].height;
}

int getFontDrawOffsetX(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return gfx.font_bitmap_info[font_bitmap_id].draw_xoffset;
}

int getFontDrawOffsetY(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return gfx.font_bitmap_info[font_bitmap_id].draw_yoffset;
}

int getTextWidth(char *text, int font_nr)
{
  return (text != NULL ? strlen(text) * getFontWidth(font_nr) : 0);
}

static int getFontCharPosition(int font_nr, char c)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);
  struct FontBitmapInfo *font = &gfx.font_bitmap_info[font_bitmap_id];
  int font_pos = (unsigned char)c - 32;

  // map some special characters to their ascii values in default font
  if (font->num_chars == DEFAULT_NUM_CHARS_PER_FONT)
    font_pos = MAP_FONT_ASCII(c) - 32;
  else if (font->num_chars == NUM_CHARS_PER_FONT_EXT)
    font_pos = MAP_FONT_ASCII_EXT(c) - 32;

  // this allows dynamic special characters together with special font
  if (font_pos < 0 || font_pos >= font->num_chars)
    font_pos = 0;

  return font_pos;
}

void getFontCharSource(int font_nr, char c, Bitmap **bitmap, int *x, int *y)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);
  struct FontBitmapInfo *font = &gfx.font_bitmap_info[font_bitmap_id];
  int font_pos = getFontCharPosition(font_nr, c);
  int offset_x = (font->offset_x != 0 ? font->offset_x : font->width);
  int offset_y = (font->offset_y != 0 ? font->offset_y : font->height);

  *bitmap = font->bitmap;
  *x = font->src_x + (font_pos % font->num_chars_per_line) * offset_x;
  *y = font->src_y + (font_pos / font->num_chars_per_line) * offset_y;
}


// ============================================================================
// text string helper functions
// ============================================================================

int maxWordLengthInRequestString(char *text)
{
  char *text_ptr;
  int word_len = 0, max_word_len = 0;

  for (text_ptr = text; *text_ptr; text_ptr++)
  {
    word_len = (*text_ptr != ' ' &&
		*text_ptr != '?' &&
		*text_ptr != '!' ? word_len + 1 : 0);

    max_word_len = MAX(word_len, max_word_len);
  }

  return max_word_len;
}


// ============================================================================
// simple text drawing functions
// ============================================================================

static void DrawInitTextExt(char *text, int ypos, int font_nr, boolean update)
{
  LimitScreenUpdates(TRUE);

  UPDATE_BUSY_STATE();

  if (!text_drawing_enabled)
    return;

  if (window != NULL &&
      gfx.draw_init_text &&
      gfx.num_fonts > 0 &&
      gfx.font_bitmap_info[font_nr].bitmap != NULL)
  {
    int x = (video.width - getTextWidth(text, font_nr)) / 2;
    int y = ypos;
    int width = video.width;
    int height = getFontHeight(font_nr);

    ClearRectangleOnBackground(drawto, 0, y, width, height);
    DrawTextExt(drawto, x, y, text, font_nr, BLIT_MASKED);

    if (update)
      BlitBitmap(drawto, window, 0, 0, video.width, video.height, 0, 0);
  }
}

void DrawInitText(char *text, int ypos, int font_nr)
{
  DrawInitTextExt(text, ypos, font_nr, FALSE);
}

void DrawInitTextHead(char *text)
{
  DrawInitTextExt(text, 120, FC_GREEN, FALSE);
}

void DrawInitTextItem(char *text)
{
  DrawInitTextExt(text, 150, FC_YELLOW, TRUE);
}

void DrawTextF(int x, int y, int font_nr, char *format, ...)
{
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  if (strlen(buffer) > MAX_OUTPUT_LINESIZE)
    Fail("string too long in DrawTextF() -- aborting");

  DrawText(gfx.sx + x, gfx.sy + y, buffer, font_nr);
}

void DrawTextFCentered(int y, int font_nr, char *format, ...)
{
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  if (strlen(buffer) > MAX_OUTPUT_LINESIZE)
    Fail("string too long in DrawTextFCentered() -- aborting");

  DrawText(gfx.sx + (gfx.sxsize - getTextWidth(buffer, font_nr)) / 2,
	   gfx.sy + y, buffer, font_nr);
}

void DrawTextS(int x, int y, int font_nr, char *text)
{
  DrawText(gfx.sx + x, gfx.sy + y, text, font_nr);
}

void DrawTextSCentered(int y, int font_nr, char *text)
{
  DrawText(gfx.sx + (gfx.sxsize - getTextWidth(text, font_nr)) / 2,
	   gfx.sy + y, text, font_nr);
}

void DrawTextSAligned(int x, int y, char *text, int font_nr, int align)
{
  DrawText(gfx.sx + ALIGNED_XPOS(x, getTextWidth(text, font_nr), align),
	   gfx.sy + y, text, font_nr);
}

void DrawText(int x, int y, char *text, int font_nr)
{
  int mask_mode = BLIT_OPAQUE;

  if (DrawingOnBackground(x, y))
    mask_mode = BLIT_ON_BACKGROUND;

  DrawTextExt(drawto, x, y, text, font_nr, mask_mode);

  if (IN_GFX_FIELD_FULL(x, y))
    redraw_mask |= REDRAW_FIELD;
  else if (IN_GFX_DOOR_1(x, y))
    redraw_mask |= REDRAW_DOOR_1;
  else if (IN_GFX_DOOR_2(x, y))
    redraw_mask |= REDRAW_DOOR_2;
  else if (IN_GFX_DOOR_3(x, y))
    redraw_mask |= REDRAW_DOOR_3;
  else
    redraw_mask |= REDRAW_ALL;
}

void DrawTextExt(DrawBuffer *dst_bitmap, int dst_x, int dst_y, char *text,
		 int font_nr, int mask_mode)
{
  struct FontBitmapInfo *font = getFontBitmapInfo(font_nr);
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  Bitmap *src_bitmap;
  int src_x, src_y;
  char *text_ptr = text;

  if (!text_drawing_enabled)
    return;

#if DEBUG
  Debug("font:token", "'%s' / '%s'",
        gfx.get_token_from_font_function(font_nr), text);
#endif

  if (font->bitmap == NULL)
    return;

  // skip text to be printed outside the window (left/right will be clipped)
  if (dst_y < 0 || dst_y + font_height > video.height)
    return;

  // add offset for drawing font characters
  dst_x += font->draw_xoffset;
  dst_y += font->draw_yoffset;

  while (*text_ptr)
  {
    char c = *text_ptr++;

    if (c == '\n')
      c = ' ';		// print space instead of newline

    getFontCharSource(font_nr, c, &src_bitmap, &src_x, &src_y);

    // clip text at the left side of the window
    if (dst_x < 0)
    {
      dst_x += font_width;

      continue;
    }

    // clip text at the right side of the window
    if (dst_x + font_width > video.width)
      break;

    if (mask_mode == BLIT_INVERSE)	// special mode for text gadgets
    {
      // first step: draw solid colored rectangle (use "cursor" character)
      if (strlen(text) == 1)	// only one char inverted => draw cursor
      {
	Bitmap *cursor_bitmap;
	int cursor_x, cursor_y;

	getFontCharSource(font_nr, FONT_ASCII_CURSOR, &cursor_bitmap,
			  &cursor_x, &cursor_y);

	BlitBitmap(cursor_bitmap, dst_bitmap, cursor_x, cursor_y,
		   font_width, font_height, dst_x, dst_y);
      }

      // second step: draw masked inverted character
      SDLCopyInverseMasked(src_bitmap, dst_bitmap, src_x, src_y,
			   font_width, font_height, dst_x, dst_y);
    }
    else if (mask_mode == BLIT_MASKED || mask_mode == BLIT_ON_BACKGROUND)
    {
      if (mask_mode == BLIT_ON_BACKGROUND)
      {
	// clear font character background
	ClearRectangleOnBackground(dst_bitmap, dst_x, dst_y,
				   font_width, font_height);
      }

      BlitBitmapMasked(src_bitmap, dst_bitmap, src_x, src_y,
		       font_width, font_height, dst_x, dst_y);
    }
    else	// normal, non-masked font blitting
    {
      BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y,
		 font_width, font_height, dst_x, dst_y);
    }

    dst_x += font_width;
  }
}


// ============================================================================
// text buffer drawing functions
// ============================================================================

char *GetTextBufferFromFile(char *filename, int max_lines)
{
  File *file;
  char *buffer;
  int num_lines = 0;

  if (filename == NULL)
    return NULL;

  if (!(file = openFile(filename, MODE_READ)))
    return NULL;

  buffer = checked_calloc(1);	// start with valid, but empty text buffer

  while (!checkEndOfFile(file) && num_lines < max_lines)
  {
    char line[MAX_LINE_LEN];
    char *line_ptr;
    int line_len;

    // read next line of input file
    if (!getStringFromFile(file, line, MAX_LINE_LEN))
      break;

    line_len = strlen(line);

    // cut trailing line break (this can be newline and/or carriage return)
    for (line_ptr = &line[line_len]; line_ptr >= line; line_ptr--)
      if ((*line_ptr == '\n' || *line_ptr == '\r') && *(line_ptr + 1) == '\0')
	*line_ptr = '\0';

    // re-add newline (so the result is terminated by newline, but not CR/LF)
    if (strlen(line) != line_len)
      strcat(line, "\n");

    buffer = checked_realloc(buffer, strlen(buffer) + strlen(line) + 1);

    strcat(buffer, line);

    num_lines++;
  }

  closeFile(file);

  if (getTextEncoding(buffer) == TEXT_ENCODING_UTF_8)
  {
    char *body_latin1 = getLatin1FromUTF8(buffer);

    checked_free(buffer);

    buffer = body_latin1;
  }

  return buffer;
}

static boolean RenderLineToBuffer(char **src_buffer_ptr, char *dst_buffer,
				  int *dst_buffer_len, int line_length,
				  boolean last_line_was_empty)
{
  char *text_ptr = *src_buffer_ptr;
  char *buffer = dst_buffer;
  int buffer_len = *dst_buffer_len;
  boolean buffer_filled = FALSE;

  while (*text_ptr)
  {
    char *word_ptr;
    int word_len;

    if (strEqual(text_ptr, "  "))	// special case: force line break
      buffer_filled = TRUE;

    // skip leading whitespaces
    while (*text_ptr == ' ' || *text_ptr == '\t')
      text_ptr++;

    word_ptr = text_ptr;
    word_len = 0;

    // look for end of next word
    while (*word_ptr != ' ' && *word_ptr != '\t' && *word_ptr != '\0')
    {
      word_ptr++;
      word_len++;
    }

    if (word_len == 0)
    {
      continue;
    }
    else if (*text_ptr == '\n')		// special case: force empty line
    {
      if (buffer_len == 0)
	text_ptr++;

      // prevent printing of multiple empty lines
      if (buffer_len > 0 || !last_line_was_empty)
	buffer_filled = TRUE;
    }
    else if (word_len < line_length - buffer_len)
    {
      // word fits into text buffer -- add word

      if (buffer_len > 0)
	buffer[buffer_len++] = ' ';

      strncpy(&buffer[buffer_len], text_ptr, word_len);
      buffer_len += word_len;
      buffer[buffer_len] = '\0';
      text_ptr += word_len;
    }
    else if (buffer_len > 0)
    {
      // not enough space left for word in text buffer -- print buffer

      buffer_filled = TRUE;
    }
    else
    {
      // word does not fit at all into empty text buffer -- cut word

      strncpy(buffer, text_ptr, line_length);
      buffer[line_length] = '\0';
      text_ptr += line_length;
      buffer_filled = TRUE;
    }

    if (buffer_filled)
      break;
  }

  *src_buffer_ptr = text_ptr;
  *dst_buffer_len = buffer_len;

  return buffer_filled;
}

static boolean getCheckedTokenValueFromString(char *string, char **token,
					      char **value)
{
  char *ptr;

  if (!getTokenValueFromString(string, token, value))
    return FALSE;

  if (**token != '.')			// token should begin with dot
    return FALSE;

  for (ptr = *token; *ptr; ptr++)	// token should contain no whitespace
    if (*ptr == ' ' || *ptr == '\t')
      return FALSE;

  for (ptr = *value; *ptr; ptr++)	// value should contain no whitespace
    if (*ptr == ' ' || *ptr == '\t')
      return FALSE;

  return TRUE;
}

static void AddLineToWrappedText(struct WrappedTextInfo *wrapped_text,
                                 char *buffer, int font_nr, int centered)
{
  wrapped_text->line[wrapped_text->num_lines].text = getStringCopy(buffer);
  wrapped_text->line[wrapped_text->num_lines].font_nr = font_nr;
  wrapped_text->line[wrapped_text->num_lines].centered = centered;

  wrapped_text->total_height += getFontHeight(font_nr) + wrapped_text->line_spacing;
  wrapped_text->num_lines++;
}

static void DrawTextBuffer_Flush(int x, int y, char *buffer,
				 int font_nr, int line_width, int line_length, int cut_length,
				 int mask_mode, boolean centered,
				 int current_ypos)
{
  int buffer_len = strlen(buffer);
  int font_width = getFontWidth(font_nr);
  int offset_chars = (centered ? (line_length - buffer_len) / 2 : 0);
  int buffer_width = font_width * buffer_len;
  int offset_xsize = (centered ? (line_width - buffer_width) / 2 : 0);
  int final_cut_length = MIN(MAX(0, cut_length - offset_chars), buffer_len);
  int xx = x + offset_xsize;
  int yy = y + current_ypos;

  buffer[final_cut_length] = '\0';

  if (mask_mode != -1)
    DrawTextExt(drawto, xx, yy, buffer, font_nr, mask_mode);
  else
    DrawText(xx, yy, buffer, font_nr);
}

static struct WrappedTextInfo *GetWrappedText(char *text_buffer, int base_font_nr,
                                              int line_length, int cut_length, int max_lines,
                                              int line_width, int cut_width, int max_height,
                                              int line_spacing, int mask_mode, boolean autowrap,
                                              boolean centered, boolean parse_comments,
                                              boolean is_text_area)
{
  struct WrappedTextInfo *wrapped_text;
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  int buffer_len;
  int font_nr = base_font_nr;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int line_height = font_height + line_spacing;

  if (text_buffer == NULL || *text_buffer == '\0')
    return NULL;

  // if pixel width of line buffer not defined, set from maximum line size
  if (line_width == -1)
    line_width = line_length * font_width;
  else
    line_length = line_width / font_width;

  // if pixel height of line buffer not defined, set from maximum number of lines
  if (max_height == -1)
    max_height = max_lines * line_height;
  else
    max_lines = max_height / line_height;

  // limit line buffer length to maximum output line size
  line_length = MIN(line_length, MAX_OUTPUT_LINESIZE);

  // if length where to cut line not defined, set to maximum (to ignore for all font sizes)
  if (cut_length == -1)
    cut_length = MAX_OUTPUT_LINESIZE;

  // if pixel width where to cut line not defined, set to maximum (to ignore for all font sizes)
  if (cut_width == -1)
    cut_width = line_width;

  wrapped_text = checked_calloc(sizeof(struct WrappedTextInfo));
  wrapped_text->line_width = line_width;
  wrapped_text->cut_length = cut_length;
  wrapped_text->max_height = max_height;
  wrapped_text->line_spacing = line_spacing;
  wrapped_text->mask_mode = mask_mode;
  wrapped_text->total_height = 0;
  wrapped_text->num_lines = 0;

  buffer[0] = '\0';
  buffer_len = 0;

  while (*text_buffer && wrapped_text->num_lines < MAX_OUTPUT_LINES)
  {
    char line[MAX_LINE_LEN + 1];
    char *line_ptr;
    boolean last_line_was_empty = TRUE;
    int num_line_chars = MAX_LINE_LEN;
    int i;

    // copy next line from text buffer to line buffer (nearly fgets() style)
    for (i = 0; i < num_line_chars && *text_buffer; i++)
    {
      if ((line[i] = *text_buffer++) == '\n')
      {
	// in text areas, do not skip newline after text ending at last column
	if (i > 0 && i % line_length == 0 && is_text_area)
	  text_buffer--;

	break;
      }
    }
    line[i] = '\0';

    // prevent 'num_line_chars' sized lines to cause additional empty line
    if (i == num_line_chars && *text_buffer == '\n')
      text_buffer++;

    // skip comments (lines directly beginning with '#')
    if (line[0] == '#' && parse_comments)
    {
      char *token, *value;

      // try to read generic token/value pair definition after comment sign
      if (getCheckedTokenValueFromString(line + 1, &token, &value))
      {
	// if found, flush the current buffer, if non-empty
	if (buffer_len > 0 && wrapped_text->num_lines < MAX_OUTPUT_LINES)
	{
          AddLineToWrappedText(wrapped_text, buffer, font_nr, centered);

	  buffer[0] = '\0';
	  buffer_len = 0;
	}

	if (strEqual(token, ".font"))
	  font_nr = gfx.get_font_from_token_function(value);
	else if (strEqual(token, ".autowrap"))
	  autowrap = get_boolean_from_string(value);
	else if (strEqual(token, ".centered"))
	  centered = get_boolean_from_string(value);
	else if (strEqual(token, ".parse_comments"))
	  parse_comments = get_boolean_from_string(value);

        // update line buffer length from line width and font width
        line_length = MIN(line_width / getFontWidth(font_nr), MAX_OUTPUT_LINESIZE);
      }

      continue;
    }

    // cut trailing newline and carriage return from input line
    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr == '\n' || *line_ptr == '\r')
      {
	*line_ptr = '\0';
	break;
      }
    }

    if (strlen(line) == 0)		// special case: force empty line
      strcpy(line, "\n");

    line_ptr = line;

    while (*line_ptr && wrapped_text->num_lines < MAX_OUTPUT_LINES)
    {
      boolean buffer_filled;

      if (autowrap)
      {
	buffer_filled = RenderLineToBuffer(&line_ptr, buffer, &buffer_len,
					   line_length, last_line_was_empty);
      }
      else
      {
	if (strlen(line_ptr) <= line_length)
	{
	  buffer_len = strlen(line_ptr);
	  strcpy(buffer, line_ptr);
	}
	else
	{
	  buffer_len = line_length;
	  strncpy(buffer, line_ptr, line_length);
	}

	buffer[buffer_len] = '\0';
	line_ptr += buffer_len;

	buffer_filled = TRUE;
      }

      if (buffer_filled)
      {
        AddLineToWrappedText(wrapped_text, buffer, font_nr, centered);

	last_line_was_empty = (buffer_len == 0);

	buffer[0] = '\0';
	buffer_len = 0;
      }
    }
  }

  if (buffer_len > 0 && wrapped_text->num_lines < MAX_OUTPUT_LINES)
    AddLineToWrappedText(wrapped_text, buffer, font_nr, centered);

  return wrapped_text;
}

struct WrappedTextInfo *GetWrappedTextBuffer(char *text_buffer, int font_nr,
                                             int line_length, int cut_length, int max_lines,
                                             int line_width, int cut_width, int max_height,
                                             int line_spacing, int mask_mode, boolean autowrap,
                                             boolean centered, boolean parse_comments)
{
  return GetWrappedText(text_buffer, font_nr,
                        line_length, cut_length, max_lines,
                        line_width, cut_width, max_height,
                        line_spacing, mask_mode, autowrap,
                        centered, parse_comments, FALSE);
}

struct WrappedTextInfo *GetWrappedTextFile(char *filename, int font_nr,
                                           int line_length, int cut_length, int max_lines,
                                           int line_width, int cut_width, int max_height,
                                           int line_spacing, int mask_mode, boolean autowrap,
                                           boolean centered, boolean parse_comments)
{
  char *text_buffer = GetTextBufferFromFile(filename, MAX_OUTPUT_LINES);
  struct WrappedTextInfo *wrapped_text = GetWrappedText(text_buffer, font_nr,
                                                        line_length, cut_length, max_lines,
                                                        line_width, cut_width, max_height,
                                                        line_spacing, mask_mode, autowrap,
                                                        centered, parse_comments, FALSE);
  checked_free(text_buffer);

  return wrapped_text;
}

static int DrawWrappedTextExt(int x, int y, struct WrappedTextInfo *wrapped_text, int start_pos,
                              boolean init_only)
{
  int current_line = 0;
  int current_ypos = 0;
  int i;

  if (wrapped_text == NULL)
    return 0;

  if (start_pos < 0 || start_pos >= wrapped_text->num_lines)
    return 0;

  wrapped_text->line_visible_first = start_pos;

  for (i = start_pos; i < wrapped_text->num_lines; i++)
  {
    int font_nr = wrapped_text->line[i].font_nr;
    int font_height = getFontHeight(font_nr);
    int line_length = MIN(wrapped_text->line_width / getFontWidth(font_nr), MAX_OUTPUT_LINESIZE);
    int line_height = font_height + wrapped_text->line_spacing;

    if (current_ypos + font_height > wrapped_text->max_height)
      break;

    if (!init_only)
      DrawTextBuffer_Flush(x, y, wrapped_text->line[i].text, font_nr,
                           wrapped_text->line_width, line_length,
                           wrapped_text->cut_length, wrapped_text->mask_mode,
                           wrapped_text->line[i].centered, current_ypos);

    current_ypos += line_height;
    current_line++;
  }

  wrapped_text->line_visible_last = start_pos + current_line - 1;

  return current_line;
}

int InitWrappedText(int x, int y, struct WrappedTextInfo *wrapped_text, int start_pos)
{
  return DrawWrappedTextExt(x, y, wrapped_text, start_pos, TRUE);
}

int DrawWrappedText(int x, int y, struct WrappedTextInfo *wrapped_text, int start_pos)
{
  return DrawWrappedTextExt(x, y, wrapped_text, start_pos, FALSE);
}

void FreeWrappedText(struct WrappedTextInfo *wrapped_text)
{
  int i;

  if (wrapped_text == NULL)
    return;

  for (i = 0; i < wrapped_text->num_lines; i++)
    checked_free(wrapped_text->line[i].text);

  checked_free(wrapped_text);
}

static int DrawTextBufferExt(int x, int y, char *text_buffer, int font_nr,
			     int line_length, int cut_length, int max_lines,
			     int line_width, int cut_width, int max_height,
			     int line_spacing, int mask_mode, boolean autowrap,
			     boolean centered, boolean parse_comments,
			     boolean is_text_area)
{
  struct WrappedTextInfo *wrapped_text = GetWrappedText(text_buffer, font_nr,
                                                        line_length, cut_length, max_lines,
                                                        line_width, cut_width, max_height,
                                                        line_spacing, mask_mode, autowrap,
                                                        centered, parse_comments, is_text_area);
  int num_lines_printed = DrawWrappedText(x, y, wrapped_text, 0);

  FreeWrappedText(wrapped_text);

  return num_lines_printed;
}

int DrawTextArea(int x, int y, char *text_buffer, int font_nr,
		 int line_length, int cut_length, int max_lines,
		 int line_width, int cut_width, int max_height,
		 int line_spacing, int mask_mode, boolean autowrap,
		 boolean centered, boolean parse_comments)
{
  return DrawTextBufferExt(x, y, text_buffer, font_nr,
			   line_length, cut_length, max_lines,
			   line_width, cut_width, max_height,
			   line_spacing, mask_mode, autowrap,
			   centered, parse_comments, TRUE);
}

int DrawTextBuffer(int x, int y, char *text_buffer, int font_nr,
		   int line_length, int cut_length, int max_lines,
		   int line_width, int cut_width, int max_height,
		   int line_spacing, int mask_mode, boolean autowrap,
		   boolean centered, boolean parse_comments)
{
  return DrawTextBufferExt(x, y, text_buffer, font_nr,
			   line_length, cut_length, max_lines,
			   line_width, cut_width, max_height,
			   line_spacing, mask_mode, autowrap,
			   centered, parse_comments, FALSE);
}

int DrawTextBufferS(int x, int y, char *text_buffer, int font_nr,
		    int line_length, int cut_length, int max_lines,
		    int line_width, int cut_width, int max_height,
		    int line_spacing, int mask_mode, boolean autowrap,
		    boolean centered, boolean parse_comments)
{
  return DrawTextBuffer(gfx.sx + x, gfx.sy + y, text_buffer, font_nr,
			line_length, cut_length, max_lines,
			line_width, cut_width, max_height,
			line_spacing, mask_mode, autowrap,
			centered, parse_comments);
}

int DrawTextBufferVA(int x, int y, char *format, va_list ap, int font_nr,
		     int line_length, int cut_length, int max_lines,
		     int line_width, int cut_width, int max_height,
		     int line_spacing, int mask_mode, boolean autowrap,
		     boolean centered, boolean parse_comments)
{
  char text_buffer[MAX_OUTPUT_LINESIZE];
  int text_length = vsnprintf(text_buffer, MAX_OUTPUT_LINESIZE, format, ap);

  if (text_length >= MAX_OUTPUT_LINESIZE)
    Warn("string too long in DrawTextBufferVA() -- truncated");

  int num_lines_printed = DrawTextBuffer(x, y, text_buffer, font_nr,
					 line_length, cut_length, max_lines,
					 line_width, cut_width, max_height,
					 line_spacing, mask_mode, autowrap,
					 centered, parse_comments);
  return num_lines_printed;
}

int DrawTextFile(int x, int y, char *filename, int font_nr,
		 int line_length, int cut_length, int max_lines,
		 int line_width, int cut_width, int max_height,
		 int line_spacing, int mask_mode, boolean autowrap,
		 boolean centered, boolean parse_comments)
{
  char *text_buffer = GetTextBufferFromFile(filename, MAX_OUTPUT_LINES);
  int num_lines_printed = DrawTextBuffer(x, y, text_buffer, font_nr,
					 line_length, cut_length, max_lines,
					 line_width, cut_width, max_height,
					 line_spacing, mask_mode, autowrap,
					 centered, parse_comments);
  checked_free(text_buffer);

  return num_lines_printed;
}
