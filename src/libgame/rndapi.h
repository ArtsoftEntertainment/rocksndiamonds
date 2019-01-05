#ifndef _RNDAPI_H
#define _RNDAPI_H



#ifdef VISUAL_CPP
//--> VISUAL_CPP
# include <sys/types.h>
# include <shlobj.h>
# include <io.h>
# include <direct.h>	/*#HAG#VC# use dirent for windows | @TODO@ zip support */
# include <errno.h>
# ifndef va_copy
# define va_copy(dest, src) (dest = src)
# endif
#if _MSC_VER <= 1600 
# define vsnprintf _vsnprintf
# define snprintf  _snprintf
#endif
//<-- VISUAL_CPP
#else
# ifdef CYGWIN
/* CYGWIN dirent is not compatible to standard!! */
#   include <sys/types.h>
#   include <dirent.h>
# else
#   include <sys/types.h>
#   include <dirent.h>
# endif

#include <unistd.h>
#include <ctype.h>
#endif








#ifdef VISUAL_CPP



/*
 * Constants for the stat st_mode member.
 */


#define	_S_IRWXU	(_S_IREAD | _S_IWRITE | _S_IEXEC)
#define	_S_IXUSR	_S_IEXEC
#define	_S_IWUSR	_S_IWRITE
#define	_S_IRUSR	_S_IREAD

#define	_S_ISDIR(m)		(((m) & _S_IFMT) == _S_IFDIR)
#define	_S_ISFIFO(m)	(((m) & _S_IFMT) == _S_IFIFO)
#define	_S_ISCHR(m)		(((m) & _S_IFMT) == _S_IFCHR)
#define	_S_ISBLK(m)		(((m) & _S_IFMT) == _S_IFBLK)
#define	_S_ISREG(m)		(((m) & _S_IFMT) == _S_IFREG)


#define	S_IFIFO		_S_IFIFO
#define	S_IFCHR		_S_IFCHR
#if !defined(S_IFBLK)
#define	S_IFBLK		_S_IFBLK
#endif
#define	S_IFDIR		_S_IFDIR
#define	S_IFREG		_S_IFREG
#define	S_IFMT		_S_IFMT
#define	S_IEXEC		_S_IEXEC
#define	S_IWRITE	_S_IWRITE
#define	S_IREAD		_S_IREAD
#define	S_IRWXU		_S_IRWXU
#if !defined(S_IXUSR)
#define	S_IXUSR		_S_IXUSR
#endif
#if !defined(S_IWUSR)
#define	S_IWUSR		_S_IWUSR
#endif
#if !defined(S_IRUSR)
#define	S_IRUSR		_S_IRUSR
#endif

#if !defined(S_ISDIR)
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISFIFO)
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISCHR)
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISBLK)
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#endif
#if !defined(S_ISREG)
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#endif


/*
 * not defined CONSTANTS.
 */
// Install MICROSOFT 2003 SDK !!!
//#define	CSIDL_COMMON_DOCUMENTS	46

#ifndef ENOTSUP
#define ENOTSUP   (-1)
#endif

#ifndef F_OK
#define F_OK    0       /* file exists */
#endif
#ifndef X_OK
#define	X_OK	1	/* MS access() doesn't check for execute permission. */
#endif
#ifndef W_OK
#define	W_OK	2	/* Check for write permission */
#endif
#ifndef R_OK
#define	R_OK	4
#endif


/*
 * not defined TYPES
 */

#define   off_t   _off_t



typedef        unsigned int    uint_t;
typedef        unsigned short  ushort_t;
typedef        unsigned char   uchar_t;


/*#HAG#VC#use windows version | @TODO@ zip support -->
struct  dirent {
        ino_t    d_ino;               // file number of entry 
        ushort_t d_reclen;            // length of this record 
        ushort_t d_namlen;            // length of string in d_name 
        char     d_name[256];         // DUMMY NAME LENGTH 
};
<--#HAG#VC#*/

typedef        int             key_t;

#ifndef _MODE_T_
typedef        ushort_t        mode_t;
#endif

typedef        int             pid_t;


/*
 * not defined FUNCTIONS.
 */
/*
#ifdef __cplusplus
extern "C" {
#endif
*/


/* SHGetFolderPath in shfolder.dll on W9x, NT4, also in shell32.dll on W2K */
/* and with the SDK2003 not longer necessary */
/*
SHSTDAPI SHGetFolderPathA (HWND,int,HANDLE,DWORD,LPSTR);

#ifdef UNICODE
#define SHGetFolderPath SHGetFolderPathW
#else
#define SHGetFolderPath SHGetFolderPathA
#endif
*/

/*
#if defined(__cplusplus)
}
#endif
*/

#endif  // VISUAL_CPP

#endif  // _RNDAPI_H
