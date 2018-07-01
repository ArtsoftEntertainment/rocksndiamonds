// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//                  Holger Schemel
//                  info@artsoft.org
//                  http://www.artsoft.org/
// ----------------------------------------------------------------------------
// conf_fnt.c
// ============================================================================

/* ----- this file was automatically generated -- do not edit by hand ----- */

#ifndef CONF_FNT_C
#define CONF_FNT_C

/* values for font/graphics mapping configuration */

static struct
{
  int font_nr;
  int special;

  int graphic;
}
font_to_graphic[] =
{
  {
    FONT_INITIAL_1,				-1,
    IMG_FONT_INITIAL_1
  },
  {
    FONT_INITIAL_2,				-1,
    IMG_FONT_INITIAL_2
  },
  {
    FONT_INITIAL_3,				-1,
    IMG_FONT_INITIAL_3
  },
  {
    FONT_INITIAL_4,				-1,
    IMG_FONT_INITIAL_4
  },
  {
    FONT_TITLE_1,				-1,
    IMG_FONT_TITLE_1
  },
  {
    FONT_TITLE_2,				-1,
    IMG_FONT_TITLE_2
  },
  {
    FONT_TITLE_2,				GFX_SPECIAL_ARG_SETUP,
    IMG_FONT_TITLE_2_SETUP
  },
  {
    FONT_MENU_1,				-1,
    IMG_FONT_MENU_1
  },
  {
    FONT_MENU_1_ACTIVE,				-1,
    IMG_FONT_MENU_1_ACTIVE
  },
  {
    FONT_MENU_2,				-1,
    IMG_FONT_MENU_2
  },
  {
    FONT_MENU_2_ACTIVE,				-1,
    IMG_FONT_MENU_2_ACTIVE
  },
  {
    FONT_TEXT_1,				-1,
    IMG_FONT_TEXT_1
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_MAIN,
    IMG_FONT_TEXT_1_MAIN
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_LEVELS,
    IMG_FONT_TEXT_1_LEVELS
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_LEVELNR,
    IMG_FONT_TEXT_1_LEVELNR
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_SETUP,
    IMG_FONT_TEXT_1_SETUP
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_PREVIEW,
    IMG_FONT_TEXT_1_PREVIEW
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_1_SCORES
  },
  {
    FONT_TEXT_1_ACTIVE,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_1_ACTIVE_SCORES
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_PANEL,
    IMG_FONT_TEXT_1_PANEL
  },
  {
    FONT_TEXT_1,				GFX_SPECIAL_ARG_DOOR,
    IMG_FONT_TEXT_1_DOOR
  },
  {
    FONT_TEXT_2,				-1,
    IMG_FONT_TEXT_2
  },
  {
    FONT_TEXT_2,				GFX_SPECIAL_ARG_MAIN,
    IMG_FONT_TEXT_2_MAIN
  },
  {
    FONT_TEXT_2,				GFX_SPECIAL_ARG_LEVELS,
    IMG_FONT_TEXT_2_LEVELS
  },
  {
    FONT_TEXT_2,				GFX_SPECIAL_ARG_LEVELNR,
    IMG_FONT_TEXT_2_LEVELNR
  },
  {
    FONT_TEXT_2,				GFX_SPECIAL_ARG_SETUP,
    IMG_FONT_TEXT_2_SETUP
  },
  {
    FONT_TEXT_2,				GFX_SPECIAL_ARG_PREVIEW,
    IMG_FONT_TEXT_2_PREVIEW
  },
  {
    FONT_TEXT_2,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_2_SCORES
  },
  {
    FONT_TEXT_2_ACTIVE,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_2_ACTIVE_SCORES
  },
  {
    FONT_TEXT_3,				-1,
    IMG_FONT_TEXT_3
  },
  {
    FONT_TEXT_3,				GFX_SPECIAL_ARG_LEVELS,
    IMG_FONT_TEXT_3_LEVELS
  },
  {
    FONT_TEXT_3,				GFX_SPECIAL_ARG_LEVELNR,
    IMG_FONT_TEXT_3_LEVELNR
  },
  {
    FONT_TEXT_3,				GFX_SPECIAL_ARG_SETUP,
    IMG_FONT_TEXT_3_SETUP
  },
  {
    FONT_TEXT_3,				GFX_SPECIAL_ARG_PREVIEW,
    IMG_FONT_TEXT_3_PREVIEW
  },
  {
    FONT_TEXT_3,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_3_SCORES
  },
  {
    FONT_TEXT_3_ACTIVE,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_3_ACTIVE_SCORES
  },
  {
    FONT_TEXT_4,				-1,
    IMG_FONT_TEXT_4
  },
  {
    FONT_TEXT_4,				GFX_SPECIAL_ARG_MAIN,
    IMG_FONT_TEXT_4_MAIN
  },
  {
    FONT_TEXT_4,				GFX_SPECIAL_ARG_LEVELS,
    IMG_FONT_TEXT_4_LEVELS
  },
  {
    FONT_TEXT_4,				GFX_SPECIAL_ARG_LEVELNR,
    IMG_FONT_TEXT_4_LEVELNR
  },
  {
    FONT_TEXT_4,				GFX_SPECIAL_ARG_SETUP,
    IMG_FONT_TEXT_4_SETUP
  },
  {
    FONT_TEXT_4,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_4_SCORES
  },
  {
    FONT_TEXT_4_ACTIVE,				GFX_SPECIAL_ARG_SCORES,
    IMG_FONT_TEXT_4_ACTIVE_SCORES
  },
  {
    FONT_ENVELOPE_1,				-1,
    IMG_FONT_ENVELOPE_1
  },
  {
    FONT_ENVELOPE_2,				-1,
    IMG_FONT_ENVELOPE_2
  },
  {
    FONT_ENVELOPE_3,				-1,
    IMG_FONT_ENVELOPE_3
  },
  {
    FONT_ENVELOPE_4,				-1,
    IMG_FONT_ENVELOPE_4
  },
  {
    FONT_REQUEST,				-1,
    IMG_FONT_REQUEST
  },
  {
    FONT_INPUT_1,				-1,
    IMG_FONT_INPUT_1
  },
  {
    FONT_INPUT_1,				GFX_SPECIAL_ARG_MAIN,
    IMG_FONT_INPUT_1_MAIN
  },
  {
    FONT_INPUT_1_ACTIVE,			-1,
    IMG_FONT_INPUT_1_ACTIVE
  },
  {
    FONT_INPUT_1_ACTIVE,			GFX_SPECIAL_ARG_MAIN,
    IMG_FONT_INPUT_1_ACTIVE_MAIN
  },
  {
    FONT_INPUT_1_ACTIVE,			GFX_SPECIAL_ARG_SETUP,
    IMG_FONT_INPUT_1_ACTIVE_SETUP
  },
  {
    FONT_INPUT_2,				-1,
    IMG_FONT_INPUT_2
  },
  {
    FONT_INPUT_2_ACTIVE,			-1,
    IMG_FONT_INPUT_2_ACTIVE
  },
  {
    FONT_OPTION_OFF,				-1,
    IMG_FONT_OPTION_OFF
  },
  {
    FONT_OPTION_OFF_NARROW,			-1,
    IMG_FONT_OPTION_OFF_NARROW
  },
  {
    FONT_OPTION_ON,				-1,
    IMG_FONT_OPTION_ON
  },
  {
    FONT_OPTION_ON_NARROW,			-1,
    IMG_FONT_OPTION_ON_NARROW
  },
  {
    FONT_VALUE_1,				-1,
    IMG_FONT_VALUE_1
  },
  {
    FONT_VALUE_2,				-1,
    IMG_FONT_VALUE_2
  },
  {
    FONT_VALUE_OLD,				-1,
    IMG_FONT_VALUE_OLD
  },
  {
    FONT_VALUE_NARROW,				-1,
    IMG_FONT_VALUE_NARROW
  },
  {
    FONT_LEVEL_NUMBER,				-1,
    IMG_FONT_LEVEL_NUMBER
  },
  {
    FONT_LEVEL_NUMBER_ACTIVE,			-1,
    IMG_FONT_LEVEL_NUMBER_ACTIVE
  },
  {
    FONT_TAPE_RECORDER,				-1,
    IMG_FONT_TAPE_RECORDER
  },
  {
    FONT_GAME_INFO,				-1,
    IMG_FONT_GAME_INFO
  },
  {
    FONT_INFO_ELEMENTS,				-1,
    IMG_FONT_INFO_ELEMENTS
  },
  {
    FONT_INFO_LEVELSET,				-1,
    IMG_FONT_INFO_LEVELSET
  },
  {
    -1,						-1,
    -1
  },
};

#endif	/* CONF_FNT_C */
