/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* platform.h                                               *
***********************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

/* define keywords for supported main platforms */

#if defined(MSDOS)
#define PLATFORM_MSDOS
#elif defined(WIN32)
#define PLATFORM_WIN32
#else
#define PLATFORM_UNIX
#endif

/* define additional keywords for several Unix platforms */

#if defined(linux)
#define PLATFORM_LINUX
#endif

#if defined(__FreeBSD__)
#define PLATFORM_FREEBSD
#endif

/* detecting HP-UX by the following compiler keyword definitions:
   - in K&R mode (the default), the HP C compiler defines "hpux"
   - in ANSI mode (-Aa or -Ae), the HP C compiler defines "__hpux"
   - the gcc (Gnu) C compiler defines "__hpux__"
   Thanks to Jarkko Hietaniemi  for this note. */

#if defined(__hpux__) || defined(__hpux) || defined(hpux)
#define PLATFORM_HPUX
#endif

#endif /* PLATFORM_H */
