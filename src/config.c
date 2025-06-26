// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// config.c
// ============================================================================

#include "libgame/libgame.h"

#include "config.h"
#include "conftime.h"
#include "confhash.h"


char *getSourceDateString(void)
{
  return SOURCE_DATE_STRING;
}

char *getSourceHashString(void)
{
  return SOURCE_HASH_STRING;
}

char *getProgramTitleString(void)
{
  return program.program_title;
}

char *getVersionString(VersionType version)
{
  // this function can be called up to ten times before version string gets overwritten
  static char version_string_array[10][32];
  static int version_string_nr = 0;
  char *version_string = version_string_array[version_string_nr];

  version_string_nr = (version_string_nr + 1) % 10;

  sprintf(version_string, "%d.%d.%d.%d",
          VERSION_SUPER(version),
          VERSION_MAJOR(version),
          VERSION_MINOR(version),
          VERSION_PATCH(version));

  if (!VERSION_STABLE(version))
  {
    if (VERSION_EXTRA(version))
      sprintf(&version_string[strlen(version_string)], "-%s-%d",
              PROGRAM_VERSION_EXTRA_TEXT, VERSION_EXTRA(version));
    else if (VERSION_BUILD(version))
      sprintf(&version_string[strlen(version_string)], "-%s-%d",
              PROGRAM_VERSION_BUILD_TEXT, VERSION_BUILD(version));
  }

  return version_string;
}

char *getProgramRealVersionString(void)
{
  return getVersionString(GAME_VERSION_ACTUAL_FULL);
}

char *getProgramVersionString(void)
{
  return program.version_string;
}

char *getProgramPlatformString(void)
{
  return PLATFORM_STRING;
}

char *getProgramInitString(void)
{
  static char *program_init_string = NULL;

  if (program_init_string == NULL)
  {
    program_init_string = checked_malloc(strlen(getProgramTitleString()) + 1 +
					 strlen(getProgramVersionString()) + 1);

    sprintf(program_init_string, "%s %s",
	    getProgramTitleString(), getProgramVersionString());
  }

  return program_init_string;
}

char *getConfigProgramTitleString(void)
{
  TreeInfo *graphics_current = getArtworkTreeInfoForUserLevelSet(ARTWORK_TYPE_GRAPHICS);

  return (leveldir_current->text[TEXT_ID_PROGRAM_TITLE] ?
	  leveldir_current->text[TEXT_ID_PROGRAM_TITLE] :
	  graphics_current->text[TEXT_ID_PROGRAM_TITLE] ?
	  graphics_current->text[TEXT_ID_PROGRAM_TITLE] :
	  setup.internal.text[TEXT_ID_PROGRAM_TITLE]);
}

char *getConfigProgramCopyrightString(void)
{
  TreeInfo *graphics_current = getArtworkTreeInfoForUserLevelSet(ARTWORK_TYPE_GRAPHICS);

  return (leveldir_current->text[TEXT_ID_PROGRAM_COPYRIGHT] ?
	  leveldir_current->text[TEXT_ID_PROGRAM_COPYRIGHT] :
	  graphics_current->text[TEXT_ID_PROGRAM_COPYRIGHT] ?
	  graphics_current->text[TEXT_ID_PROGRAM_COPYRIGHT] :
	  setup.internal.text[TEXT_ID_PROGRAM_COPYRIGHT]);
}

char *getConfigProgramCompanyString(void)
{
  TreeInfo *graphics_current = getArtworkTreeInfoForUserLevelSet(ARTWORK_TYPE_GRAPHICS);

  return (leveldir_current->text[TEXT_ID_PROGRAM_COMPANY] ?
	  leveldir_current->text[TEXT_ID_PROGRAM_COMPANY] :
	  graphics_current->text[TEXT_ID_PROGRAM_COMPANY] ?
	  graphics_current->text[TEXT_ID_PROGRAM_COMPANY] :
	  setup.internal.text[TEXT_ID_PROGRAM_COMPANY]);
}

char *getWindowTitleString(void)
{
  static char *window_title_string = NULL;

  checked_free(window_title_string);

#ifdef DEBUG
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 20 +
				       strlen(getSourceDateString()) + 2 + 1);

  if (setup.internal.show_scaling_in_title)
    sprintf(window_title_string, "%s (%d %%) [%s]",
	    getProgramInitString(), video.window_scaling_percent,
	    getSourceDateString());
  else
    sprintf(window_title_string, "%s [%s]",
	    getProgramInitString(),
	    getSourceDateString());
#else
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 20);

  if (setup.internal.show_scaling_in_title)
    sprintf(window_title_string, "%s (%d %%)",
	    getProgramInitString(), video.window_scaling_percent);
  else
    sprintf(window_title_string, "%s",
	    getProgramInitString());
#endif

  return window_title_string;
}
