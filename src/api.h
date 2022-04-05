// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2022 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// api.h
// ============================================================================

#ifndef API_H
#define API_H

void ApiAddScoreAsThread(int, boolean, char *);
void ApiGetScoreAsThread(int);
void ApiGetScoreTapeAsThread(int, int, char *);
void ApiRenamePlayerAsThread(void);
void ApiResetUUIDAsThread(char *);

#endif
