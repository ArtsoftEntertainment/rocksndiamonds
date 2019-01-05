/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
*               Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* zfile.h                                                  *
*----------------------------------------------------------*
* (c) by the owner of the alias/nickname "HerzAusGold"     *
*               e-mail: herzausgold@online.de              *
***********************************************************/

#ifndef ZFILE_H_
#define ZFILE_H_


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "util.h"

#define MAX_DPATH 1000

/*----------------------------------------------------------*/


typedef unsigned char utl_u8;
typedef signed char utl_s8;




/*----------------------------------------------------------*/


/* While we're here, make abort more useful.  */
#define xabort() \
  do { \
    write_log ("Internal error; file %s, line %d\n", __FILE__, __LINE__); \
    (abort) (); \
} while (0)

/*----------------------------------------------------------*/


struct zfile {
    char    *name;
    char    *zipname;
    FILE    *f;
    utl_u8  *data;
    int     size;
    int     seek;
    int     deleteafterclose;
    struct zfile *next;
    int     writeskipbytes;
};

typedef int (*zfile_callback)(struct zfile*, void*);


extern struct zfile *zfile_fopen (const char *name, const char *mode);
extern struct zfile *zfile_fopen_empty (const char *name, int size);
extern int zfile_stat(const char *name, struct _stat *fileStatus);
extern int zfile_exists (const char *name);
extern int zfile_direxists(const char *name);
extern int zfile_fclose (struct zfile *);
extern int zfile_fseek (struct zfile *z, long offset, int mode);
extern long zfile_ftell (struct zfile *z);
extern size_t zfile_fread  (void *b, size_t size, size_t count, struct zfile *z);
extern size_t zfile_fwrite  (void *b, size_t size, size_t count, struct zfile *z);
extern size_t zfile_fputs (char *s, struct zfile *z);
extern void zfile_exit (void);
extern int zfile_iscompressed (struct zfile *z);
extern char *zfile_getname (struct zfile *f);
extern struct zfile *zfile_dup (struct zfile *f);
extern char * zfile_fgets(char *buf, int len, struct zfile *z);
extern int zfile_feof (struct zfile *z);
extern int zfile_fgetc(struct zfile *z);
extern int zfile_fputc(int c, struct zfile *z);
extern int zfile_ferror(struct zfile *z);
extern int zfile_fprintf(struct zfile *z, const char *format, ...);

extern int zfile_isZip (char *name);
extern int zfile_wasZip (char *name);
extern int zfile_checkZip(char* name, List_t *dirNameList, bool_t onlyDir, bool_t doExtendPath);
extern void zfile_freeZipFileList(void);

extern void zfile_freadAllLikeZip(struct zfile *file); 

#endif  /* ZFILE_H_ */
