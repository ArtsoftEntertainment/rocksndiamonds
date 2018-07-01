// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//                  Holger Schemel
//                  info@artsoft.org
//                  http://www.artsoft.org/
// ----------------------------------------------------------------------------
// conf_g2m.c
// ============================================================================

/* ----- this file was automatically generated -- do not edit by hand ----- */

#ifndef CONF_G2M_C
#define CONF_G2M_C

/* values for gamemode/music mapping configuration */

static struct
{
  int gamemode;

  int music;
}
gamemode_to_music[] =
{
  {
    -1,
    MUS_BACKGROUND
  },
  {
    GFX_SPECIAL_ARG_TITLE_INITIAL,
    MUS_BACKGROUND_TITLE_INITIAL
  },
  {
    GFX_SPECIAL_ARG_TITLE,
    MUS_BACKGROUND_TITLE
  },
  {
    GFX_SPECIAL_ARG_MAIN,
    MUS_BACKGROUND_MAIN
  },
  {
    GFX_SPECIAL_ARG_LEVELS,
    MUS_BACKGROUND_LEVELS
  },
  {
    GFX_SPECIAL_ARG_LEVELNR,
    MUS_BACKGROUND_LEVELNR
  },
  {
    GFX_SPECIAL_ARG_SCORES,
    MUS_BACKGROUND_SCORES
  },
  {
    GFX_SPECIAL_ARG_EDITOR,
    MUS_BACKGROUND_EDITOR
  },
  {
    GFX_SPECIAL_ARG_INFO,
    MUS_BACKGROUND_INFO
  },
  {
    GFX_SPECIAL_ARG_SETUP,
    MUS_BACKGROUND_SETUP
  },
  {
    -1,
    -1
  },
};

#endif	/* CONF_G2M_C */
