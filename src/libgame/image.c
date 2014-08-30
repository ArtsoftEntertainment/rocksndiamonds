/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* image.c                                                  *
***********************************************************/

#include "image.h"
#include "pcx.h"
#include "misc.h"


/* ========================================================================= */
/* PLATFORM SPECIFIC IMAGE FUNCTIONS                                         */
/* ========================================================================= */

#if defined(TARGET_X11)

/* for MS-DOS/Allegro, exclude all except newImage() and freeImage() */

Image *newImage(unsigned int width, unsigned int height, unsigned int depth)
{
  Image *image;
  unsigned int bytes_per_pixel = (depth + 7) / 8;
  int i;

#if 0
  if (depth > 8)
    Error(ERR_EXIT, "images with more than 256 colors are not supported");

  depth = 8;
#endif

  image = checked_malloc(sizeof(Image));
  image->data = checked_malloc(width * height * bytes_per_pixel);
  image->width = width;
  image->height = height;
  image->depth = depth;
  image->bytes_per_pixel = bytes_per_pixel;
  image->bytes_per_row = width * bytes_per_pixel;

  image->rgb.used = 0;
  for (i=0; i<MAX_COLORS; i++)
    image->rgb.color_used[i] = FALSE;

  image->type = (depth < 8 ? IMAGETYPE_BITMAP :
		 depth > 8 ? IMAGETYPE_TRUECOLOR : IMAGETYPE_RGB);

  return image;
}

void freeImage(Image *image)
{
  free(image->data);
  free(image);
}

#if defined(PLATFORM_UNIX)

/* extra colors to try allocating in private color maps to minimize flashing */
#define NOFLASH_COLORS 256

/* architecture independent value <-> memory conversions;
   note: the internal format is big endian */

#define memory_to_value(ptr, len) (					    \
(len) == 1 ? (unsigned long)(                 *( (byte *)(ptr))         ) : \
(len) == 2 ? (unsigned long)(((unsigned long)(*( (byte *)(ptr))   ))<< 8)   \
			  + (                 *(((byte *)(ptr))+1)      ) : \
(len) == 3 ? (unsigned long)(((unsigned long)(*( (byte *)(ptr))   ))<<16)   \
			  + (((unsigned long)(*(((byte *)(ptr))+1)))<< 8)   \
			  + (                 *(((byte *)(ptr))+2)      ) : \
	     (unsigned long)(((unsigned long)(*( (byte *)(ptr))   ))<<24)   \
			  + (((unsigned long)(*(((byte *)(ptr))+1)))<<16)   \
			  + (((unsigned long)(*(((byte *)(ptr))+2)))<< 8)   \
			  + (                 *(((byte *)(ptr))+3)      ) )


#define value_to_memory(value, ptr, len) (				\
(len) == 1 ? (*( (byte *)(ptr)   ) = ( value     ) ) :			\
(len) == 2 ? (*( (byte *)(ptr)   ) = (((unsigned long)(value))>> 8),	\
	      *(((byte *)(ptr))+1) = ( value     ) ) :			\
(len) == 3 ? (*( (byte *)(ptr)   ) = (((unsigned long)(value))>>16),	\
	      *(((byte *)(ptr))+1) = (((unsigned long)(value))>> 8),	\
	      *(((byte *)(ptr))+2) = ( value     ) ) :			\
             (*( (byte *)(ptr)   ) = (((unsigned long)(value))>>24),	\
	      *(((byte *)(ptr))+1) = (((unsigned long)(value))>>16),	\
	      *(((byte *)(ptr))+2) = (((unsigned long)(value))>> 8),	\
	      *(((byte *)(ptr))+3) = ( value     ) ))

static Pixmap Image_to_Mask(Image *image, Display *display, Window window)
{
  byte *src_ptr, *dst_ptr, *dst_ptr2;
  unsigned int bytes_per_row;
  unsigned int x, y, i;
  byte bitmask;
  byte *mask_data;
  Pixmap mask_pixmap;

  bytes_per_row = (image->width + 7) / 8;
  mask_data = checked_calloc(bytes_per_row * image->height);

  src_ptr = image->data;
  dst_ptr = mask_data;

  /* create bitmap data which can be used by 'XCreateBitmapFromData()'
   * directly to create a pixmap of depth 1 for use as a clip mask for
   * the corresponding image pixmap
   */

  for (y=0; y<image->height; y++)
  {
    bitmask = 0x01;		/* start with leftmost bit in the byte     */
    dst_ptr2 = dst_ptr;		/* start with leftmost byte in the row     */

    for (x=0; x<image->width; x++)
    {
      for (i=0; i<image->bytes_per_pixel; i++)
	if (*src_ptr++)		/* source pixel solid? (pixel index != 0)  */
	  *dst_ptr2 |= bitmask;	/* then write a bit into the image mask    */

      if ((bitmask <<= 1) == 0)	/* bit at rightmost byte position reached? */
      {
	bitmask = 0x01;		/* start again with leftmost bit position  */
	dst_ptr2++;		/* continue with next byte in image mask   */
      }
    }

    dst_ptr += bytes_per_row;	/* continue with leftmost byte of next row */
  }

  mask_pixmap = XCreateBitmapFromData(display, window, (char *)mask_data,
				      image->width, image->height);
  free(mask_data);

  return mask_pixmap;
}

static int bitsPerPixelAtDepth(Display *display, int screen, int depth)
{
  XPixmapFormatValues *pixmap_format;
  int i, num_pixmap_formats, bits_per_pixel = -1;

  /* get Pixmap formats supported by the X server */
  pixmap_format = XListPixmapFormats(display, &num_pixmap_formats);

  /* find format that matches the given depth */
  for (i=0; i<num_pixmap_formats; i++)
    if (pixmap_format[i].depth == depth)
      bits_per_pixel = pixmap_format[i].bits_per_pixel;

  XFree(pixmap_format);

  if (bits_per_pixel == -1)
    Error(ERR_EXIT, "cannot find pixmap format for depth %d", depth);

  return bits_per_pixel;
}

XImageInfo *Image_to_Pixmap(Display *display, int screen, Visual *visual,
			    Window window, GC gc, int depth, Image *image)
{
  static XColor xcolor_private[NOFLASH_COLORS];
  static int colorcell_used[NOFLASH_COLORS];
  static Colormap global_cmap = 0;
  static Pixel *global_cmap_index;
  static int num_cmap_entries, free_cmap_entries;
  static boolean private_cmap = FALSE;
  Pixel *redvalue, *greenvalue, *bluevalue;
  unsigned int display_bytes_per_pixel, display_bits_per_pixel;
  unsigned int a, c = 0, x, y;
  XColor xcolor;
  XImage *ximage;
  XImageInfo *ximageinfo;
  byte *src_ptr, *dst_ptr;
  char *error = "Image_to_Pixmap(): %s";

  if (image->type == IMAGETYPE_TRUECOLOR && depth == 8)
  {
    SetError(error, "cannot handle true-color images on 8-bit display");
    return NULL;
  }

  if (!global_cmap)
  {
    if (visual == DefaultVisual(display, screen))
      global_cmap = DefaultColormap(display, screen);
    else
    {
      global_cmap = XCreateColormap(display, RootWindow(display, screen),
				    visual, AllocNone);
      private_cmap = TRUE;
    }
  }

  xcolor.flags = DoRed | DoGreen | DoBlue;
  redvalue = greenvalue = bluevalue = NULL;
  ximageinfo = checked_malloc(sizeof(XImageInfo));
  ximageinfo->display = display;
  ximageinfo->depth = depth;

  switch (visual->class)
  {
    case TrueColor:
    case DirectColor:
    {
      Pixel pixval;
      unsigned int redcolors, greencolors, bluecolors;
      unsigned int redstep, greenstep, bluestep;
      unsigned int redbottom, greenbottom, bluebottom;
      unsigned int redtop, greentop, bluetop;

      redvalue = (Pixel *)checked_malloc(sizeof(Pixel) * 256);
      greenvalue = (Pixel *)checked_malloc(sizeof(Pixel) * 256);
      bluevalue = (Pixel *)checked_malloc(sizeof(Pixel) * 256);

      ximageinfo->cmap = global_cmap;

      retry_direct: /* tag we hit if a DirectColor allocation fails on
		     * default colormap */

      /* calculate number of distinct colors in each band */

      redcolors = greencolors = bluecolors = 1;
      for (pixval=1; pixval; pixval <<= 1)
      {
	if (pixval & visual->red_mask)
	  redcolors <<= 1;
	if (pixval & visual->green_mask)
	  greencolors <<= 1;
	if (pixval & visual->blue_mask)
	  bluecolors <<= 1;
      }
      
      /* consistency check */
      if (redcolors > visual->map_entries ||
	  greencolors > visual->map_entries ||
	  bluecolors > visual->map_entries)
	Error(ERR_WARN, "inconsistency in color information");

      redstep = 256 / redcolors;
      greenstep = 256 / greencolors;
      bluestep = 256 / bluecolors;
      redbottom = greenbottom = bluebottom = 0;
      redtop = greentop = bluetop = 0;

      for (a=0; a<visual->map_entries; a++)
      {
	if (redbottom < 256)
	  redtop = redbottom + redstep;
	if (greenbottom < 256)
	  greentop = greenbottom + greenstep;
	if (bluebottom < 256)
	  bluetop = bluebottom + bluestep;

	xcolor.red = (redtop - 1) << 8;
	xcolor.green = (greentop - 1) << 8;
	xcolor.blue = (bluetop - 1) << 8;
	if (!XAllocColor(display, ximageinfo->cmap, &xcolor))
	{
	  /* if an allocation fails for a DirectColor default visual then
	     we should create a private colormap and try again. */

	  if ((visual->class == DirectColor) &&
	      (visual == DefaultVisual(display, screen)))
	  {
	    global_cmap = XCopyColormapAndFree(display, global_cmap);
	    ximageinfo->cmap = global_cmap;
	    private_cmap = TRUE;

	    goto retry_direct;
	  }

	  /* something completely unexpected happened */

	  fprintf(stderr, "Image_to_Pixmap: XAllocColor failed on a TrueColor/Directcolor visual\n");

          free(redvalue);
          free(greenvalue);
          free(bluevalue);
          free(ximageinfo);

	  return NULL;
	}

	/* fill in pixel values for each band at this intensity */

	while ((redbottom < 256) && (redbottom < redtop))
	  redvalue[redbottom++] = xcolor.pixel & visual->red_mask;
	while ((greenbottom < 256) && (greenbottom < greentop))
	  greenvalue[greenbottom++] = xcolor.pixel & visual->green_mask;
	while ((bluebottom < 256) && (bluebottom < bluetop))
	  bluevalue[bluebottom++] = xcolor.pixel & visual->blue_mask;
      }

      break;
    }

    case PseudoColor:

      ximageinfo->cmap = global_cmap;

      for (a=0; a<MAX_COLORS; a++)
      {
	XColor xcolor2;
	unsigned short mask;
	int color_found;
  	int i;

	if (!image->rgb.color_used[a])
	  continue;

  	xcolor.red = *(image->rgb.red + a);
  	xcolor.green = *(image->rgb.green + a);
  	xcolor.blue = *(image->rgb.blue + a);
  
  	/* look if this color already exists in our colormap */
	if (!XAllocColor(display, ximageinfo->cmap, &xcolor))
	{
	  if (!private_cmap)
	  {
	    if (options.verbose)
	      Error(ERR_RETURN, "switching to private colormap");

	    /* we just filled up the default colormap -- get a private one
	       which contains all already allocated colors */

	    global_cmap = XCopyColormapAndFree(display, global_cmap);
	    ximageinfo->cmap = global_cmap;
	    private_cmap = TRUE;

	    /* allocate the rest of the color cells read/write */
	    global_cmap_index =
	      (Pixel *)checked_malloc(sizeof(Pixel) * NOFLASH_COLORS);
	    for (i=0; i<NOFLASH_COLORS; i++)
	      if (!XAllocColorCells(display, global_cmap, FALSE, NULL, 0,
				    global_cmap_index + i, 1))
		break;
	    num_cmap_entries = free_cmap_entries = i;

	    /*
	    printf("We've got %d free colormap entries.\n", free_cmap_entries);
	    */

	    /* to minimize colormap flashing, copy default colors and try
	       to keep them as near as possible to the old values */

	    for(i=0; i<num_cmap_entries; i++)
	    {
	      xcolor2.pixel = *(global_cmap_index + i);
	      XQueryColor(display, DefaultColormap(display, screen), &xcolor2);
	      XStoreColor(display, global_cmap, &xcolor2);
	      xcolor_private[xcolor2.pixel] = xcolor2;
	      colorcell_used[xcolor2.pixel] = FALSE;
	    }

	    /* now we have the default colormap private: all colors we
	       successfully allocated so far are read-only, which is okay,
	       because we don't want to change them anymore -- if we need
	       an existing color again, we get it by XAllocColor; all other
	       colors are read/write and we can set them by XStoreColor,
	       but we will try to overwrite those color cells with our new
	       color which are as close as possible to our new color */
	  }

  	  /* look for an existing default color close the one we want */

	  mask = 0xf000;
	  color_found = FALSE;

	  while (!color_found)
	  {
	    for (i=num_cmap_entries-1; i>=0; i--)
	    {
	      xcolor2.pixel = *(global_cmap_index + i);
	      xcolor2 = xcolor_private[xcolor2.pixel];

	      if (colorcell_used[xcolor2.pixel])
		continue;

	      if ((xcolor.red & mask) == (xcolor2.red & mask) &&
		  (xcolor.green & mask) == (xcolor2.green & mask) &&
		  (xcolor.blue & mask) == (xcolor2.blue & mask))
	      {
		/*
		printf("replacing color cell %ld with a close color\n",
		       xcolor2.pixel);
		       */
		color_found = TRUE;
		break;
	      }
	    }

	    if (mask == 0x0000)
	      break;

	    mask = (mask << 1) & 0xffff;
	  }

	  if (!color_found)		/* no more free color cells */
	  {
	    SetError(error, "cannot allocate enough color cells");
	    return NULL;
	  }

	  xcolor.pixel = xcolor2.pixel;
	  xcolor_private[xcolor.pixel] = xcolor;
	  colorcell_used[xcolor.pixel] = TRUE;
	  XStoreColor(display, ximageinfo->cmap, &xcolor);
	  free_cmap_entries--;
	}

	*(ximageinfo->index + a) = xcolor.pixel;
      }

      /*
      printf("still %d free colormap entries\n", free_cmap_entries);
      */

      ximageinfo->no = a;	/* number of pixels allocated for this image */
      break;
  
    default:
      Error(ERR_RETURN,"DirectColor, TrueColor or PseudoColor display needed");
      SetError(error, "display class not supported");

      return NULL;
  }

#if DEBUG_TIMING
  debug_print_timestamp(2, "   ALLOCATING IMAGE COLORS:   ");
#endif

  /* create XImage from internal image structure and convert it to Pixmap */

  display_bits_per_pixel = bitsPerPixelAtDepth(display, screen, depth);
  display_bytes_per_pixel = (display_bits_per_pixel + 7) / 8;

  ximage = XCreateImage(display, visual, depth, ZPixmap, 0,
			NULL, image->width, image->height,
			8, image->width * display_bytes_per_pixel);
  ximage->data =
    checked_malloc(image->width * image->height * display_bytes_per_pixel);
  ximage->byte_order = MSBFirst;

  src_ptr = image->data;
  dst_ptr = (byte *)ximage->data;

  switch (visual->class)
  {
    case DirectColor:
    case TrueColor:
    {
      Pixel pixval;

      switch (image->type)
      {
        case IMAGETYPE_RGB:
	{
	  for (y=0; y<image->height; y++)		/* general case */
	  {
	    for (x=0; x<image->width; x++)
	    {
	      pixval = *src_ptr++;
	      pixval =
		redvalue[image->rgb.red[pixval] >> 8] |
		greenvalue[image->rgb.green[pixval] >> 8] |
		bluevalue[image->rgb.blue[pixval] >> 8];
	      value_to_memory(pixval, dst_ptr, display_bytes_per_pixel);
	      dst_ptr += display_bytes_per_pixel;
	    }
	  }

	  break;
	}

        case IMAGETYPE_TRUECOLOR:
	{
	  for (y=0; y<image->height; y++)		/* general case */
	  {
	    for (x=0; x<image->width; x++)
	    {
	      pixval = memory_to_value(src_ptr, image->bytes_per_pixel);
	      pixval =
		redvalue[TRUECOLOR_RED(pixval)] |
		greenvalue[TRUECOLOR_GREEN(pixval)] |
		bluevalue[TRUECOLOR_BLUE(pixval)];
	      value_to_memory(pixval, dst_ptr, display_bytes_per_pixel);
	      src_ptr += image->bytes_per_pixel;
	      dst_ptr += display_bytes_per_pixel;
	    }
	  }

	  break;
	}

        default:
	  Error(ERR_RETURN, "RGB or TrueColor image needed");
	  SetError(error, "image type not supported");

	  return NULL;
      }

      break;
    }

    case PseudoColor:
    {
      if (display_bytes_per_pixel == 1)		/* special case */
      {
	for (y=0; y<image->height; y++)
	  for (x=0; x<image->width; x++)
	    *dst_ptr++ = ximageinfo->index[c + *src_ptr++];
      }
      else					/* general case */
      {
	for (y=0; y<image->height; y++)
	{
	  for (x=0; x<image->width; x++)
	  {
	    value_to_memory(ximageinfo->index[c + *src_ptr++],
			    dst_ptr, display_bytes_per_pixel);
	    dst_ptr += display_bytes_per_pixel;
	  }
	}
      }

      break;
    }

    default:
      Error(ERR_RETURN,"DirectColor, TrueColor or PseudoColor display needed");
      SetError(error, "display class not supported");

      return NULL;
  }

  if (redvalue)
  {
    free((byte *)redvalue);
    free((byte *)greenvalue);
    free((byte *)bluevalue);
  }

#if DEBUG_TIMING
  debug_print_timestamp(2, "   CONVERTING IMAGE TO XIMAGE:");
#endif

  ximageinfo->pixmap = XCreatePixmap(display, window,
				     ximage->width, ximage->height,
				     ximageinfo->depth);

  XPutImage(ximageinfo->display, ximageinfo->pixmap, gc,
	    ximage, 0, 0, 0, 0, ximage->width, ximage->height);

  XDestroyImage(ximage);

  return ximageinfo;
}

void freeXImage(Image *image, XImageInfo *ximageinfo)
{
  if (ximageinfo->index != NULL && ximageinfo->no > 0)
    XFreeColors(ximageinfo->display, ximageinfo->cmap, ximageinfo->index,
		ximageinfo->no, 0);
  /* this       ^^^^^^^^^^^^^^ is wrong, because the used color cells
   * are somewhere between 0 and MAX_COLORS; there are indeed 'ximageinfo->no'
   * used color cells, but they are not at array position 0 - 'ximageinfo->no'
   */

  free(ximageinfo);
}

int Read_PCX_to_Pixmap(Display *display, Window window, GC gc, char *filename,
		       Pixmap *pixmap, Pixmap *pixmap_mask)
{
  Image *image;
  XImageInfo *ximageinfo;
  int screen;
  Visual *visual;
  int depth;

#if DEBUG_TIMING
  debug_print_timestamp(2, NULL);	/* initialize timestamp function */
#endif

  /* read the graphic file in PCX format to image structure */
  if ((image = Read_PCX_to_Image(filename)) == NULL)
    return errno_pcx;

#if DEBUG_TIMING
  printf("%s:\n", filename);
  debug_print_timestamp(2, "   READING PCX FILE TO IMAGE: ");
#endif

  screen = DefaultScreen(display);
  visual = DefaultVisual(display, screen);
  depth = DefaultDepth(display, screen);

  /* convert image structure to X11 Pixmap */
  if (!(ximageinfo = Image_to_Pixmap(display, screen, visual,
				     window, gc, depth, image)))
  {
    freeImage(image);

    return PCX_OtherError;
  }

  /* if a private colormap has been created, install it */
  if (ximageinfo->cmap != DefaultColormap(display, screen))
    XSetWindowColormap(display, window, ximageinfo->cmap);

#if DEBUG_TIMING
  debug_print_timestamp(2, "   CONVERTING IMAGE TO PIXMAP:");
#endif

  /* create clip mask for the image */
  ximageinfo->pixmap_mask = Image_to_Mask(image, display, window);

#if DEBUG_TIMING
  debug_print_timestamp(2, "   CONVERTING IMAGE TO MASK:  ");
#endif

  *pixmap = ximageinfo->pixmap;
  *pixmap_mask = ximageinfo->pixmap_mask;

  /* free generic image and ximageinfo after native Pixmap has been created */
  free(ximageinfo);
  freeImage(image);

  return PCX_Success;
}

#endif	/* PLATFORM_UNIX */
#endif	/* TARGET_X11 */


/* ========================================================================= */
/* PLATFORM INDEPENDANT IMAGE FUNCTIONS                                      */
/* ========================================================================= */

struct ImageInfo
{
  char *source_filename;
  int num_references;

  Bitmap *bitmap;
};
typedef struct ImageInfo ImageInfo;

static struct ArtworkListInfo *image_info = NULL;

static void *Load_PCX(char *filename)
{
  ImageInfo *img_info;

#if 0
  printf("loading PCX file '%s'\n", filename);
#endif

  img_info = checked_calloc(sizeof(ImageInfo));

  if ((img_info->bitmap = LoadImage(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read image file '%s': LoadImage() failed: %s",
	  filename, GetError());
    free(img_info);
    return NULL;
  }

  img_info->source_filename = getStringCopy(filename);

  return img_info;
}

static void FreeImage(void *ptr)
{
  ImageInfo *image = (ImageInfo *)ptr;

  if (image == NULL)
    return;

  if (image->bitmap)
    FreeBitmap(image->bitmap);

  if (image->source_filename)
    free(image->source_filename);

  free(image);
}

void InitImageList(struct FileInfo *file_list, int num_list_entries)
{
  if (image_info == NULL)
    image_info = checked_calloc(sizeof(struct ArtworkListInfo));

  if (image_info->artwork_list == NULL)
    image_info->artwork_list =
      checked_calloc(num_list_entries * sizeof(ImageInfo *));

  image_info->type = ARTWORK_TYPE_GRAPHICS;
  image_info->num_list_entries = num_list_entries;
  image_info->file_list = file_list;
  image_info->content_list = NULL;
  image_info->load_artwork = Load_PCX;
  image_info->free_artwork = FreeImage;
}

void ReloadCustomImages()
{
#if 0
  printf("DEBUG: reloading images '%s' ...\n", artwork.gfx_current_identifier);
#endif

  ReloadCustomArtworkList(image_info);
}

void FreeAllImages()
{
  FreeCustomArtworkList(image_info);
}
