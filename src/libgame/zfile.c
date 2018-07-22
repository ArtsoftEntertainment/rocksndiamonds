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
* zfile.c                                                  *
*----------------------------------------------------------*
* (c) by the owner of the alias/nickname "HerzAusGold"     *
*               e-mail: herzausgold@online.de              *
***********************************************************/
/*
 *
 * routines to handle compressed file automatically
 *---------------------------------------------------------
 * only plain access from the winuae source "zfile.c"
 * credits to:
 *   Samuel Devulder, Tim Gunn, Toni Wilen
 *---------------------------------------------------------
 * bug fixed: fread, fwrite return number of items
 * bug fixed: unzClose after unzOpen
*/

#include "zfile.h"
#include "system.h"

#include "unzip.h"
#include "util.h"

#include <string.h>
#include <stdarg.h>
#include <zlib.h>
#ifndef VISUAL_CPP
#include <unistd.h>
#endif

#include "../lib7zip/7zCrc.h"
#include "../lib7zip/7zIn.h"
#include "../lib7zip/7zExtract.h"

//#define ZFILEDEBUG 1


#define CASESENSITIVITY (0)

// zipType is Index of array
#define ArchiveFormatZIP  0
#define ArchiveFormat7ZIP 1


// extensions handled
static char *archive_ext[] = { "zip", "7z", NULL };

// files in zip are in dirZipEntry_t
typedef struct dirZipEntry_s {
  char    *filename;
  int     st_size;                // size of file
  int     st_pos;                 // pos, offset in file
} dirZipEntry_t;

// real zip files are in zipFileEntry_t 
typedef struct zipFileEntry_s {
  char    *filename;
  int     zipType;
  List_t  dirZipList;            // dirZipEntry_t
  /* 7zip support */ 
  UInt32  blockIndex; 
  Byte    *outBuffer; 
  size_t  outBufferSize;  
  void    *pSzData;
} zipFileEntry_t;

// temp files stored in zzEntry_t 
typedef struct zzEntry_s {
  struct zfile * z;
} zzEntry_t;



// small list for opened files, should freed at exit  
static List_t   zzList      = {0,0,0,0};   // contain zzEntry_t
 
// the main and only list for real zip files 
static List_t   zipFileList = {0,0,0,0};   // contain zipFileEntry_t


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* 7zip callback                                                        */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct _CFileInStream
{
  ISzInStream InStream;
  FILE *File;
} CFileInStream;

static bool_t InitCrcTable7zip = TRUE;


// use input callback
#ifdef _LZMA_IN_CB

#define kBufferSize (1 << 12)
static Byte g_Buffer[kBufferSize];

SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize, size_t *processedSize)
{
  CFileInStream *s = (CFileInStream *)object;
  size_t processedSizeLoc;
  if (maxRequiredSize > kBufferSize)
    maxRequiredSize = kBufferSize;
  processedSizeLoc = fread(g_Buffer, 1, maxRequiredSize, s->File);
  *buffer = g_Buffer;
  if (processedSize != 0)
    *processedSize = processedSizeLoc;
  return SZ_OK;
}

#else

SZ_RESULT SzFileReadImp(void *object, void *buffer, size_t size, size_t *processedSize)
{
  CFileInStream *s = (CFileInStream *)object;
  size_t processedSizeLoc = fread(buffer, 1, size, s->File);
  if (processedSize != 0)
    *processedSize = processedSizeLoc;
  return SZ_OK;
}

#endif

SZ_RESULT SzFileSeekImp(void *object, CFileSize pos)
{
  CFileInStream *s = (CFileInStream *)object;
  int res = fseek(s->File, (long)pos, SEEK_SET);
  if (res == 0)
    return SZ_OK;
  return SZE_FAIL;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*----------------------------------------------------------------------*/
/* general functions                                                    */
/*----------------------------------------------------------------------*/
void write_log (const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  fflush (stderr);
}

/*----------------------------------------------------------------------*/
void *xcalloc (size_t n, size_t size)
{
    void *a = calloc (n, size);
    if (a == NULL) {
	  write_log ("xcalloc(%d): virtual memory exhausted\n", n * size);
	  xabort ();
    }
    return a;
}

void xfree (void *p)
{
    free (p);
}


/*----------------------------------------------------------------------*/
/* free lists                                                           */
/*----------------------------------------------------------------------*/
void freeDirZipList(List_t *dirZipList) 
{
  dirZipEntry_t *dze;
  while(listGetFirst(dirZipList, (void**) &dze)) {
    objectDelete((Pointer_t *)&(dze->filename));
    listLeave(dirZipList,TRUE);
  }
  listInit(dirZipList);
}

void freeZipFileList(List_t *zipFileList) 
{
  zipFileEntry_t *zfe;
  while(listGetFirst(zipFileList, (void**) &zfe)) {
    objectDelete((Pointer_t *)&(zfe->filename));
    freeDirZipList(&(zfe->dirZipList));
    /* 7zip support */
    if (zfe->zipType == ArchiveFormat7ZIP) {
      CArchiveDatabaseEx *padb = (CArchiveDatabaseEx*) zfe->pSzData;
      SzFree(zfe->outBuffer);
      SzArDbExFree(padb, SzFree);
    }

    listLeave(zipFileList,TRUE);
  }
  listInit(zipFileList);
}

void zfile_freeZipFileList(void)
{
  freeZipFileList(&zipFileList);
} 


/*----------------------------------------------------------------------*/
/* fill lists (directory related)                                       */
/*----------------------------------------------------------------------*/

/*--------------------------------------------------------------------
* function: zipScanDir2_zip
* descr   : scan the zip file and fill dirZipList (dirZipEntry_t)
* param   : dirNameIn  [in] zip-filename
*           dirZipList [in/out] list of files in zip
* return  : number of entries
* 
*-------------------------------------------------------------------*/
static int zipScanDir2_zip (zipFileEntry_t *zfep,
                            char      *dirNameIn,
                            List_t    *dirZipList  // dirZipEntry_t
                           )
{
    uLong i;
    unzFile         uz;
    unz_global_info gi;
    bool_t          error       = FALSE;
    int             err         = UNZ_OK;
    unz_file_info   file_info;
    char            filename_inzip[2048];
    dirZipEntry_t   dze;

    char           *dirName   = strNew(dirNameIn);  // prepared: extract environment here

    if (dirName == NULL) {
      return -1;  /* error no directory defined */
    }

    uz = unzOpen(dirName);
    if (!uz) 
      return -1;

    err = unzGetGlobalInfo (uz,&gi);
    if (err!=UNZ_OK)
      return -1;

    for (i=0;i<gi.number_entry;i++)
    {
	err = unzGetCurrentFileInfo(uz,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
	if (err != UNZ_OK) {
          error = TRUE;
          break;
        }

        dze.filename = strNew(filename_inzip);
        dze.st_pos   = 0;
        dze.st_size  = file_info.uncompressed_size;
        if (!listEnter (dirZipList,
                        &dze,
                        sizeof(dirZipEntry_t),
                        FALSE)) {
          error = TRUE;
          break;
        }

        if ((i+1)<gi.number_entry)
        {
          err = unzGoToNextFile(uz);
          if (err!=UNZ_OK)
          {
            error = TRUE;
            break;
          }
        }
    }

    if (uz) {
      unzClose(uz);
    }
    if (error) {
      freeDirZipList(dirZipList);
    }

    objectDelete ((Pointer_t *)&dirName);
    return (listLength(dirZipList));
} // zipScanDir2_zip


/*--------------------------------------------------------------------
* function: zipScanDir2_7zip
* descr   : scan the zip file and fill dirZipList (dirZipEntry_t)
* param   : dirNameIn  [in] zip-filename
*           dirZipList [in/out] list of files in zip
* return  : number of entries
* 
*-------------------------------------------------------------------*/
static int zipScanDir2_7zip (zipFileEntry_t *zfep,
                             char      *dirNameIn,
                             List_t    *dirZipList  // dirZipEntry_t
                            )
{
  CFileInStream       archiveStream;
  CArchiveDatabaseEx  *padb = NULL;
  SZ_RESULT           res   = SZ_OK;
  ISzAlloc            allocImp;
  ISzAlloc            allocTempImp;

  archiveStream.File = fopen(dirNameIn, "rb");
#ifdef ZFILEDEBUG
    if (archiveStream.File != NULL) {
      write_log("-I- scanzip fopen ok: '%s'\n", dirNameIn);
    } else {
      write_log("-W- scanzip fopen %ld err: '%s'\n", errno, dirNameIn);
    }
#endif
  if (archiveStream.File == 0)
  {
    // cant open file
    return -1;  
  }

  archiveStream.InStream.Read = SzFileReadImp;
  archiveStream.InStream.Seek = SzFileSeekImp;

  allocImp.Alloc = SzAlloc;
  allocImp.Free  = SzFree;

  allocTempImp.Alloc = SzAllocTemp;
  allocTempImp.Free  = SzFreeTemp;

  if (InitCrcTable7zip) {
    InitCrcTable();
    InitCrcTable7zip = FALSE;
  }

  if (zfep->pSzData == NULL) {
    padb = (CArchiveDatabaseEx*) SzAlloc(sizeof(CArchiveDatabaseEx));
    if (padb != NULL) {
      SzArDbExInit(padb);
      res = SzArchiveOpen(&archiveStream.InStream, padb, &allocImp, &allocTempImp);
      zfep->pSzData = padb;
    } else {
      res = SZE_OUTOFMEMORY;
    }
  } else {
    padb = (CArchiveDatabaseEx*) zfep->pSzData;
  }
  if (res == SZ_OK)
  {
      UInt32 i;

      for (i = 0; i < padb->Database.NumFiles; i++)
      {
        CFileItem  *f = padb->Database.Files + i;
        dirZipEntry_t dze;

        if (!f->IsDirectory) {
          dze.filename = strNew(f->Name);
          dze.st_pos   = i;   // file_index !!
          dze.st_size  = f->Size;  // uncompressed size
          if (!listEnter (dirZipList,
                          &dze,
                          sizeof(dirZipEntry_t),
                          FALSE)) {
            res = SZE_OUTOFMEMORY;
            break;
          }
        }
      }
  } else {
    if (padb != NULL) {
      SzArDbExFree(padb, allocImp.Free);
    }
    padb = NULL;
    zfep->pSzData = NULL;
  }

  fclose(archiveStream.File);
#ifdef ZFILEDEBUG
  write_log("-I- %ld fclose '%s'\n", errno, dirNameIn);
#endif
  if (res == SZ_OK)
  {
    // Everything is Ok
    return (listLength(dirZipList));
  }
  if (res == SZE_OUTOFMEMORY) {
    // can not allocate memory
    freeDirZipList(dirZipList);
  } else {    
    // ERROR res
    freeDirZipList(dirZipList);
  }
  return -1;
} // zipScanDir2_zip


/*--------------------------------------------------------------------
* function: addZipFileToList
* szipfind1 helper funtions for addZipFileToList 
*-------------------------------------------------------------------*/

/* 
* szipfind1 - exact file name - for addZipFileToList
*/
static bool_t szipfind1(const void *searchData,
                        const void *listData)
{ /* for files */
  zipFileEntry_t *zfe = (zipFileEntry_t*) listData;
  if (UTILstrCaseCmp(zfe->filename,searchData) == 0) 
    return TRUE;
  return FALSE;
} // szipfind1


/*--------------------------------------------------------------------
* function: addZipFileToList
* descr   : search zip-filename in zipFileList 
*           if not found create entry and scan zip dir
* param   : name  [in] zip-filename
*           zfep  [out] pointer to zip file entry
* return  : TRUE if all ok
* 
*-------------------------------------------------------------------*/
static bool_t addZipFileToList (char *name, zipFileEntry_t **zfep)
{
  bool_t retval = FALSE;
  // check current first then all
  if(listSearch(&zipFileList, name, TRUE, TRUE, szipfind1)) {
    // get entry
    if (listGet(&zipFileList, (void**) zfep)) {
      retval = TRUE;
    } 
  } else {
    // zip file entry
    zipFileEntry_t zfe;
    // which zip
    char *ext    = strrchr (name, '.');
    int  zipType = -1;

    if (ext != NULL) {
      int  j;
      ext++;
      for (j = 0; archive_ext[j]; j++) {        
	if (UTILstrCaseCmp (ext, archive_ext[j]) == 0) {
          zipType = j;
          break;
        }
      }
    }

    // fill entry
    zfe.zipType    = zipType;
    zfe.filename   = strNew(name);
    zfe.blockIndex = 0xFFFFFFFF;
    zfe.outBuffer  = NULL;
    zfe.outBufferSize = 0;
    zfe.pSzData    = NULL; 
    listInit(&zfe.dirZipList);    
    // append to zipFileList
    if (listEnter(&zipFileList, &zfe, sizeof(zipFileEntry_t), FALSE)) {
      // get entry
      if (listGet(&zipFileList, (void**) zfep)) {
        // full scan    
        // check zip not empty 
        switch ((*zfep)->zipType) {
          case ArchiveFormatZIP :
            if (zipScanDir2_zip(*zfep, name,&((*zfep)->dirZipList)) > 0) {  
              retval = TRUE;
            }
            break;
          case ArchiveFormat7ZIP :
            if (zipScanDir2_7zip(*zfep, name,&((*zfep)->dirZipList)) > 0) {  
              retval = TRUE;
            }
            break;
          default : break;
        }
      }
    }
  }
  return retval;
} // addZipFileToList


/*--------------------------------------------------------------------
* function: scanzip 
* descr   : call addZipFileToList and open file
*/
static FILE *scanzip (char *name, zipFileEntry_t **zfep)
{
  FILE * fp = NULL;

  if (addZipFileToList(name, zfep)) {
    fp = fopen(name, "rb");
#ifdef ZFILEDEBUG
    if (fp != NULL) {
      write_log("-I- scanzip fopen %ld  ok: '%s' '%s'\n", errno, name, (*zfep)->filename);
    } else {
      write_log("-W- scanzip fopen %ld err: '%s' '%s'\n", errno, name, (*zfep)->filename);
    }
#endif
  }
  return fp;
}


/*--------------------------------------------------------------------
* function: manglefilename 
* descr   : remove act dir "./" and trailing "/."
*/
static void manglefilename(char *out, const char *in)
{
    char *p1 = NULL;
    out[0] = 0;
    if ((in[0] == '/' || in[0] == '\\') || 
        (strlen(in) > 3 && in[1] == ':' && (in[2] == '/' || in[2] == '\\'))) {
      out[0] = 0;
    }
    strcat(out, in);

    if (zfile_wasZip(out)) {
      for(;;) {
        p1 = strstr(out, "./");
        if (p1 != NULL) {
          strcpy(p1, p1+2);   /* strlen(p1)-2); */
        } else {
          p1 = strstr(out, ".\\");
          if (p1 != NULL) {
            strcpy(p1, p1+2); /* strlen(p1)-2); */
          } else {
            break;
          }
        }
      }
      if (strlen(out) > 2) {
        if ((out[strlen(out)-2] == '/' || out[strlen(out)-2] == '\\') && 
            (out[strlen(out)-1] == '.')) {
          out[strlen(out)-1] = 0;
        }
      }
    }
} // manglefilename



/*--------------------------------------------------------------------
* function: openzip
* sinsfind1, sinsfind2 helper funtions for openzip 
*-------------------------------------------------------------------*/

/*  
* sdirfind1 - exact file name, for openzip
*/
static bool_t sdirfind1(const void *searchData,
                        const void *listData)
{ /* for files */
  dirZipEntry_t *dze = (dirZipEntry_t*) listData;
  if (UTILstrCasePathCmp(dze->filename,searchData) == 0) 
    return TRUE;
  return FALSE;
}

/* 
* sdirfind2 - exact path name, for openzip 
*/
static bool_t sdirfind2(const void *searchData,
                        const void *listData)
{ /* for dirs */
  dirZipEntry_t *dze = (dirZipEntry_t*) listData;
  if (strstr(dze->filename,searchData) == dze->filename) /* at the beginning!! */
    return TRUE;
  return FALSE;
}


/*--------------------------------------------------------------------
* function: openzip
* descr   : check if file is in zip or a zipfile itself
*           open the zip file
* param   : name  [in] zip-filename
*           zfep  [out] pointer to zip file entry
* return  : TRUE if all ok
* 
*-------------------------------------------------------------------*/
static FILE *openzip (char *name, char *zippath, int seamode)
{
    int i, j;
    char v;
    zipFileEntry_t  *zfep;

    i = strlen (name) - 2;
    if (zippath)
      zippath[0] = 0;
    while (i > 0) {
      if ((name[i] == '/' || name[i] == '\\') && i > 4) {
        v = name[i];
        name[i] = 0;
        for (j = 0; archive_ext[j]; j++) {
          int len = strlen (archive_ext[j]);
	      if (name[i - len - 1] == '.' && !UTILstrCaseCmp (name + i - len, archive_ext[j])) {
	        FILE *f = scanzip(name, &zfep);
	        if (f) {
	          if (zippath) {
                strcpy (zippath, name + i + 1);
                if ((seamode > 0) && (zippath[0] != 0)) {
                  bool_t found = FALSE;
                  if (seamode == 2) {  
                    found = listSearch(&zfep->dirZipList, 
                                       zippath, TRUE, TRUE, 
                                       sdirfind2);
                  } else {
                    found = listSearch(&zfep->dirZipList, 
                                       zippath, TRUE, TRUE, 
                                       sdirfind1);
                  }
                  if (found) {
                    return f;
                  }
                  fclose(f);
#ifdef ZFILEDEBUG
                  write_log("-I- %ld fclose: '%s' '%s'\n", errno, name, zippath);
#endif
                  return 0;
                } 
              }
	          return f;
	        }
	        break;
	      }
	    }
	    name[i] = v;
      }
      i--;
    }
    return 0;
} // openzip


/*--------------------------------------------------------------------
* function: zipScanDir
* descr   : Scan dir and call select or insert function
* param   : 
* return  : number of files in list
* 
*-------------------------------------------------------------------*/
int zipScanDir (char          *dirNameIn,
                List_t        *dirNameList,  // filename // char*
                char*         sfindstr,
                int           (*selectFnk) (char *, char *),
                int           (*insertFnk) (List_t *, char *, char *) 
               )
{
  zipFileEntry_t *zfep = NULL;
  dirZipEntry_t  *dzep = NULL;
  bool_t         appendEntry = FALSE;
  bool_t         error       = FALSE;
 
  if(addZipFileToList(dirNameIn, &zfep)) {
    if(listGetFirst(&(zfep->dirZipList), (void**) &dzep)) {
      do {
        /* shall the current entry be appended on the array */
        if (selectFnk) {
          appendEntry = (*selectFnk)(sfindstr, dzep->filename);
        } else {
          appendEntry = TRUE;
        }

        if ( appendEntry ) {
          if (insertFnk) {
            if (!insertFnk (dirNameList,
                            sfindstr,
                            dzep->filename)) {
              error = TRUE;
              break;
            }
          } else {
            if (!listEnter (dirNameList,
                            dzep->filename,
                            strlen(dzep->filename)+1,
                            FALSE)) {
              error = TRUE;
              break;
            }
          }
        }
      } while (listGetNext(&(zfep->dirZipList), (void**) &dzep));
    }
  }

  if (error) {
    listFree(dirNameList);
    listInit(dirNameList);
  }

  return (listLength(dirNameList));
} // zipScanDir


/*--------------------------------------------------------------------
* function: zfile_checkZip
* sinsfile, sinsdir helper funtions for zfile_checkZip 
*-------------------------------------------------------------------*/

/* sinsdir for 'checkzip'
   return 1 no error
   return 0 error!
*/
static int sinsdir(List_t *list, char *findstr, char *data)
{  
  char onlypath[1000];
  char *p1 = NULL;
  int i;
  char v;
  bool_t found = FALSE;
  char *dirEntry;

  onlypath[0] = 0;

  // from actdir
  if (findstr[0] != 0) {
    p1 = strstr(data, findstr);
    if (p1==NULL || p1!=data) 
      return 1;
  } else {
    p1 = data;
  }

  // extract dir
  p1 = p1+strlen(findstr);
  if ((*p1 != 0) && (*p1 == '/' || *p1 == '\\')) {
    (strlen(findstr)>0)?p1++:0;
  }
  for(i=strlen(p1)-1; i>0; i--) {
    if (p1[i] == '/' || p1[i] == '\\') {
      v = p1[i];
      p1[i] = 0;
      strcpy(onlypath, p1);
      p1[i] = v;
    }
  }

  // enter to list
  if (onlypath[0] != 0) {
    found = FALSE;
    if (listGetFirst(list, (void**) &dirEntry)) {
      do {
        if(UTILstrCaseCmp(onlypath, dirEntry) == 0) {
          found = TRUE;
          break;
        }
      } while (listGetNext(list, (void**) &dirEntry));
    }
    if (!found) {
      listTail(list);
      listEnter(list, onlypath, strlen(onlypath)+1, FALSE);
    }
    listTail(list);
  }

  return 1;
}


/* sinsfile for 'checkzip'
   return 1 no error
   return 0 error!
*/
static int sinsfile(List_t *list, char *findstr, char *data)
{  
  char onlypath[1000];
  char *p1 = NULL;
  int i;
  char v;
  bool_t found = FALSE;
  char *dirEntry;

  onlypath[0] = 0;

  // extract dir
  for(i=strlen(data)-1; i>0; i--) {
	if (data[i] == '/' || data[i] == '\\') {
      v = data[i];
      data[i] = 0;
      strcpy(onlypath, data);
      data[i] = v;
      break;
    }
  }

  // from actdir
  if (findstr[0] != 0) {
    if (UTILstrCaseCmp(onlypath, findstr) != 0) {
      strcat(onlypath, STRING_PATH_SEPARATOR);
      if (UTILstrCaseCmp(onlypath, findstr) != 0) 
        return 1;
    }
    (i>0)?i++:0;
    p1 = data + i;
  } else {
    p1 = data;
  }

  // enter to list
  if (*p1 != 0) {
    strcpy(onlypath, p1);  
    found = FALSE;
    if (listGetFirst(list, (void**) &dirEntry)) {
      do {
        if(UTILstrCaseCmp(onlypath, dirEntry) == 0) {
          found = TRUE;
          break;
        }
      } while (listGetNext(list, (void**) &dirEntry));
    }
    if (!found) {
      listTail(list);
      listEnter(list, onlypath, strlen(onlypath)+1, FALSE);
    }
    listTail(list);
  }

  return 1;
}


/*--------------------------------------------------------------------
* function: zfile_checkZip
* descr   : return a list of files which are in the dir 
*           can return only directories in a dir 
*           or only files in a dir
*           can extend the path name which is needed if 
*           we are searching in a dir an concat the fullpath 
*           outside 
* param   : name         [in] zip-filename
*           dirNameList  [in/out] names of files (was appended)
*           onlyDir      [in] TRUE is only dirs, FALSE only files
*           doExtendPath [in] extend to fullpath (with zipname) itself
* return  : number of files in list
* 
*-------------------------------------------------------------------*/
int zfile_checkZip(char* name, List_t *dirNameList, bool_t onlyDir, bool_t doExtendPath)
{
    int  i, j;
    char v;
    char fname[512];
    char onlyzipname[512];
    char zipname[512];
    char zippath[512];
    char actpath[512];
    char findstr[512];
    bool_t found = FALSE;
    List_t dList = {0,0,0,0};
    char *dirEntry;

    manglefilename(fname, name);
    onlyzipname[0] = 0;
    zipname[0] = 0;
    zippath[0] = 0;
    actpath[0] = 0;
    if(zfile_isZip(fname)) {
      strcpy(zipname, fname);
      strcpy(zippath, "");  /* no path */
      found = TRUE;
    } else {
      i = strlen (fname) - 1;  /* OK */ 
      while (i > 0) {
	if ((fname[i] == '/' || fname[i] == '\\') && i > 4) {
	  v = fname[i];
	  fname[i] = 0;
	  for (j = 0; archive_ext[j]; j++) {
            int len = strlen (archive_ext[j]);
            if (fname[i - len - 1] == '.' && !UTILstrCaseCmp (fname + i - len, archive_ext[j])) {
              strcpy (zipname, fname);
              strcpy (zippath, fname + i + 1);
              found = TRUE;
	      break;
            }
	  }
	  fname[i] = v;
	}
        if (found) break;
	i--;
      }
    }
      
    if (found && (zippath[0] != 0)) {
      strcpy(actpath, zippath);
    }

    if (found && (zipname[0] != 0)) {
      for (i=strlen(zipname)-1; i>0; i--) {
	if (zipname[i] == '/' || zipname[i] == '\\') {
          break;
        }    
      }
      (i>0)?i++:0;
      strcpy(onlyzipname, zipname+i);
    }

    if (found) {
      strcpy (findstr, actpath);
      listFree(&dList);
      listInit(&dList);
      if (onlyDir) {
        zipScanDir(zipname, &dList, findstr, NULL, sinsdir);
      } else {
        zipScanDir(zipname, &dList, findstr, NULL, sinsfile);
      }
      if (listLength(&dList) > 0) {
        listTail(dirNameList);
        if (listGetFirst(&dList, (void**) &dirEntry)) {
          do {
            if(doExtendPath) {
              strcpy(zippath, onlyzipname);
              strcat(zippath, STRING_PATH_SEPARATOR);
              if(actpath[0] != 0) {
                strcat(zippath, actpath);
                strcat(zippath, STRING_PATH_SEPARATOR);
              }
              strcat(zippath, dirEntry);
            } else {
              strcpy(zippath, dirEntry);
            }
            if (!listEnter(dirNameList, zippath, strlen(zippath)+1, FALSE)) {
              listFree(dirNameList);
              listInit(dirNameList);
              break;
            }
          } while (listGetNext(&dList, (void**) &dirEntry));
        }
      }
      listFree(&dList);
    }

    return found;   
} // zfile_checkZip


/*======================================================================*/
/* zfile related functions (access, unzip)                              */
/*======================================================================*/

/*----------------------------------------------------------------------*/
/* zfile_create, zfile_free, zfile_exit                                 */
/*----------------------------------------------------------------------*/

static struct zfile *zfile_create (void)
{
    struct zfile *z;
    zzEntry_t zze;

    z = malloc (sizeof *z);
    if (!z)
      return 0;
    memset (z, 0, sizeof *z);

    zze.z = z;
    listEnter(&zzList, &zze, sizeof(zzEntry_t), FALSE);
    return z;
} // zfile_create


static void zfile_free (struct zfile *f)
{
    if (f->f) {
      fclose (f->f);
#ifdef ZFILEDEBUG
      write_log("-I- fclose temp %ld file: '%s' '%s'\n", errno, f->name, f->zipname);
#endif
    }
    if (f->deleteafterclose) {
      _unlink (f->name);
      write_log("-I- deleted temp %ld file '%s' '%s'\n", errno, f->name, f->zipname);
    }
    xfree (f->name);
    xfree (f->data);
    if (f->zipname) {
      xfree (f->zipname);
    }
    xfree (f);
} // zfile_free


void zfile_exit (void)
{
    zzEntry_t *zze;
    while (listGetFirst(&zzList, (void**) &zze)) {
      zfile_free (zze->z);
      listLeave(&zzList, TRUE);
    }
} // zfile_exit


/*----------------------------------------------------------------------*/
/* unzip file                                                           */
/*----------------------------------------------------------------------*/
static struct zfile *unzip (struct zfile *z)
{
    unzFile         uz;
    unz_file_info   file_info;
    char            filename_inzip[2048];
    struct zfile    *zf;
    int             err;

    zf = 0;
    uz = unzOpen (z->name);  
    if (!uz)
      return z;

    err = unzLocateFile(uz,z->zipname,CASESENSITIVITY);
    if (err != UNZ_OK)
      return z;

    err = unzGetCurrentFileInfo(uz,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
    if (err != UNZ_OK)
      return z;

    if (file_info.uncompressed_size > 0) {
      err = unzOpenCurrentFile (uz);
      if (err == UNZ_OK) {
	zf = zfile_fopen_empty (filename_inzip, file_info.uncompressed_size);
	if (zf) {
	  err = unzReadCurrentFile  (uz, zf->data, file_info.uncompressed_size);
	}
	unzCloseCurrentFile (uz);
      }
    }

    if (uz) {
      unzClose(uz);
    }

    if (zf) {
      zfile_fclose (z);
      z = zf;
    }
    return z;
} // unzip


/*----------------------------------------------------------------------*/
/* un7zip file                                                           */
/*----------------------------------------------------------------------*/
static struct zfile *un7zip (struct zfile *z)
{
  zipFileEntry_t  *zfep = NULL;
  dirZipEntry_t   *dzep = NULL;
  struct zfile    *zf = 0;

  // search and get zipEntry
  if(listSearch(&zipFileList, z->name, TRUE, TRUE, szipfind1)) {
    if (listGet(&zipFileList, (void**) &zfep)) {
      // search and get dirEntry
      if (listSearch(&zfep->dirZipList, z->zipname, 
                     TRUE, TRUE, sdirfind1)) {
        if (listGet(&zfep->dirZipList, (void**) &dzep)) 
        { // get file now
          CFileInStream       archiveStream;
          CArchiveDatabaseEx  *padb = NULL;
          SZ_RESULT           res   = SZ_OK;
          ISzAlloc            allocImp;
          ISzAlloc            allocTempImp;

          archiveStream.File = fopen(z->name, "rb");
#ifdef ZFILEDEBUG
          if (archiveStream.File != NULL) {
            write_log("-I- scanzip fopen      ok: '%s' '%s'\n", z->name, z->zipname);
          } else {
            write_log("-W- scanzip fopen %ld err: '%s' '%s'\n", errno, z->name, z->zipname);
          }
#endif
          if (archiveStream.File == 0)
          {
            // cant open file
            return z;  
          }

          archiveStream.InStream.Read = SzFileReadImp;
          archiveStream.InStream.Seek = SzFileSeekImp;

          allocImp.Alloc = SzAlloc;
          allocImp.Free  = SzFree;

          allocTempImp.Alloc = SzAllocTemp;
          allocTempImp.Free  = SzFreeTemp;

          if (InitCrcTable7zip) {
            InitCrcTable();
            InitCrcTable7zip = FALSE;
          }
          if (zfep->pSzData == NULL) {
            padb = (CArchiveDatabaseEx*) SzAlloc(sizeof(CArchiveDatabaseEx));
            if (padb != NULL) {
              SzArDbExInit(padb);
              res = SzArchiveOpen(&archiveStream.InStream, padb, &allocImp, &allocTempImp);
              zfep->pSzData = padb;
            } else {
              res = SZE_OUTOFMEMORY;
            }
          } else {
            padb = (CArchiveDatabaseEx*) zfep->pSzData;
          }
          if (res == SZ_OK)
          {
              UInt32 i = dzep->st_pos; // file_index;

              size_t offset = 0;
              size_t outSizeProcessed = 0;

              CFileItem  *f = padb->Database.Files + i;

              if (!f->IsDirectory) {
                res = SzExtract(&archiveStream.InStream, padb, i, 
                                &(zfep->blockIndex), 
                                &(zfep->outBuffer), 
                                &(zfep->outBufferSize), 
                                &offset, &outSizeProcessed, 
                                &allocImp, &allocTempImp);
                if (res == SZ_OK) {
                  if (f->Size == outSizeProcessed) {
                      // copy data (safety reason)
	              zf = zfile_fopen_empty (dzep->filename, dzep->st_size);  // uncompressed size
	              if (zf) {
                        memcpy(zf->data, zfep->outBuffer + offset, dzep->st_size);
                      }
                  } else {
                    res = SZE_FAIL;
                  }
                }
              } 
          } else {
            if (padb != NULL) {
              SzArDbExFree(padb, allocImp.Free);
            }
            padb = NULL;
            zfep->pSzData = NULL;
          }

          fclose(archiveStream.File);
#ifdef ZFILEDEBUG
          write_log("-I- %ld fclose '%s' '%s'\n", errno, z->name, z->zipname);
#endif

        }
      }
    }
  }

  if (zf) {
    zfile_fclose (z);
    z = zf;
  }
  return z;
} // un7zip


/*----------------------------------------------------------------------*/
/* uncompress file                                                      */
/*----------------------------------------------------------------------*/
static struct zfile *zuncompress (struct zfile *z)
{
    char *name = z->name;
    char *ext = strrchr (name, '.');
    utl_u8 header[4];

    if (ext != NULL) {
      ext++;
      if (UTILstrCaseCmp (ext, "zip") == 0)
	return unzip (z);
//    if (UTILstrCaseCmp (ext, "gz") == 0)
//	return gunzip (z);
      if (UTILstrCaseCmp (ext, "7z") == 0)
	return un7zip (z);

      memset (header, 0, sizeof (header));
      zfile_fseek (z, 0, SEEK_SET);
      zfile_fread (header, 1, sizeof (header), z);
      zfile_fseek (z, 0, SEEK_SET);
//    if (header[0] == 0x1f && header[1] == 0x8b)
//	return gunzip (z);
      if (header[0] == 'P' && header[1] == 'K')
	return unzip (z);
      if (header[0] == '7' && header[1] == 'z')
	return un7zip (z);
    }
    return z;
} // zuncompress


/*----------------------------------------------------------------------*/
/* fopen helper                                                         */
/*----------------------------------------------------------------------*/

static struct zfile *zfile_fopen_2 (const char *name, const char *mode)
{
    struct  zfile *l;
    FILE    *f;
    char    zipname[1000];

    if( name == NULL || *name == '\0' )
      return NULL;
    l = zfile_create ();
    l->name = _strdup (name);
    f = openzip (l->name, zipname, 0);
    if (f) {
      /* not "rb", cant write in zip */
      if (UTILstrCaseCmp (mode, "rb") && !strchr(mode,'r') && !strchr(mode,'R')) {
	zfile_fclose (l);
	fclose (f);
#ifdef ZFILEDEBUG
        write_log("-I- %ld fclose '%s' '%s'\n", errno, name, zipname);
#endif
	return 0;
      }
      l->zipname = _strdup (zipname);
    }
    if (!f) {
      f = fopen (name, mode);
#ifdef ZFILEDEBUG
      if (f != NULL) {
        write_log("-I- scanzip fopen      ok: '%s' '%s'\n", name, zipname);
      } else {
        write_log("-W- scanzip fopen %ld err: '%s' '%s'\n", errno, name, zipname);
      }
#endif
      if (!f) {
	zfile_fclose (l);
	return 0;
      }
    }
    l->f = f;
    return l;
} // zfile_fopen_2



/*----------------------------------------------------------------------*/
/* zfile plain access                                                   */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/*
 * fopen() for a compressed file
 */
struct zfile *zfile_fopen (const char *name, const char *mode)
{
    struct zfile *l;
    char path[MAX_DPATH];

    manglefilename(path, name);
    l = zfile_fopen_2 (path, mode);
    if (!l)
      return 0;
    l = zuncompress (l);
    return l;
}


int zfile_fclose (struct zfile *f)
{
    zzEntry_t *zze;
    bool_t    found = FALSE;

    if (!f)
      return -1;

    if (listGetFirst(&zzList, (void**) &zze)) {
      do {
        if (zze->z == f) {
          zfile_free (f);
          listLeave(&zzList, FALSE);
          found = TRUE;
          break;
        }
      } while (listGetNext(&zzList, (void**) &zze));
    }

    if (!found) {
      zfile_free(f);
    }

    return 0;
}


struct zfile *zfile_dup (struct zfile *zf)
{
    struct zfile *nzf;
    if (!zf->data)
      return NULL;
    nzf = zfile_create();
    nzf->data = malloc (zf->size);
    memcpy (nzf->data, zf->data, zf->size);
    nzf->size = zf->size;
    return nzf;
}


int zfile_exists (const char *name)
{
    char fname[2000]   = {0};
    char zipname[1000] = {0};
    FILE *f;
    int  seamode;

    if (strlen (name) == 0)
      return 0;
    if (strstr(name, "/.") == (name+strlen(name)-2)) {
      seamode = 2;
    } else {
      seamode = 1;
    }
    manglefilename(fname, name);
    f = openzip (fname, zipname, seamode);
    if (!f) {
      manglefilename(fname, name);
      f = fopen(fname,"rb");
#ifdef ZFILEDEBUG
      if (f != NULL) {
        write_log("-I- scanzip fopen      ok: '%s' '%s'\n", fname, zipname);
      } else {
        write_log("-W- scanzip fopen %ld err: '%s' '%s'\n", errno, fname, zipname);
      }
#endif

    }
    if (!f)
      return 0;
    fclose (f);
#ifdef ZFILEDEBUG
    write_log("-I- %ld fclose '%s' '%s'\n", errno, fname, zipname);
#endif
    return 1;
}


int zfile_direxists(const char *name)
{
    char fname[2000]   = {0};
    char zipname[1000] = {0};
    FILE *f;
    int  seamode;

    if (strlen(name) == 0)
        return 0;
    if (strstr(name, "/.") == (name + strlen(name) - 2)) {
        seamode = 2;
    }
    else {
        seamode = 2;   /*#HAG#ZIP# for dir always 2 */
    }
    manglefilename(fname, name);
    f = openzip(fname, zipname, seamode);
    if (!f) {
        manglefilename(fname, name);
        f = fopen(fname, "rb");
#ifdef ZFILEDEBUG
        if (f != NULL) {
            write_log("-I- scanzip fopen      ok: '%s' '%s'\n", fname, zipname);
        }
        else {
            write_log("-W- scanzip fopen %ld err: '%s' '%s'\n", errno, fname, zipname);
        }
#endif

    }
    if (!f)
        return 0;
    fclose(f);
#ifdef ZFILEDEBUG
    write_log("-I- %ld fclose '%s' '%s'\n", errno, fname, zipname);
#endif
    return 1;
}


int zfile_iscompressed (struct zfile *z)
{
    return z->data ? 1 : 0;
}


struct zfile *zfile_fopen_empty (const char *name, int size)
{
    struct zfile *l;
    l = zfile_create ();
    l->name = _strdup (name);
    l->size = size;
    if (size > 0) {
      l->data = malloc (size);
      memset (l->data, 0, size);
    } else {
      l->data = NULL;
    }
    return l;
}


int zfile_feof (struct zfile *z)
{
    if (z->data) {
      long len;
      len = z->size - z->seek;
      if (len < 0)
	len = 0;
      return (len > 0) ? 0 : 1;
    }
    return feof (z->f);
}


long zfile_ftell (struct zfile *z)
{
    if (z->data)
      return z->seek;
    return ftell (z->f);
}


int zfile_fseek (struct zfile *z, long offset, int mode)
{
    if (z->data) {
      int old = z->seek;
      switch (mode)
      {
	case SEEK_SET:
	  z->seek = offset;
	  break;
	case SEEK_CUR:
	  z->seek += offset;
	  break;
	case SEEK_END:
	  z->seek = z->size - offset;
	  break;
      }
      if (z->seek < 0) z->seek = 0;
      if (z->seek > z->size) z->seek = z->size;
      return old;
    }
    return fseek (z->f, offset, mode);
}


size_t zfile_fread  (void *b, size_t size, size_t count,struct zfile *z)
{
    long len = size * count;
    if (z->data) {
      if (z->seek + len > z->size) {
	len = z->size - z->seek;
	if (len < 0)
	      len = 0;
      }
      memcpy (b, z->data + z->seek, len);
      z->seek += len;
      return (len/size);
    }
    return fread (b, size, count, z->f);
}


size_t zfile_fwrite  (void *b, size_t size, size_t count, struct zfile *z)
{
    long len = size * count;

    if (z->writeskipbytes) {
      z->writeskipbytes -= len;
      if (z->writeskipbytes >= 0)
	return len;
      len = -z->writeskipbytes;
      z->writeskipbytes = 0;
    }
    if (z->data) {
      if (z->seek + len > z->size) {
	len = z->size - z->seek;
	if (len < 0)
	  len = 0;
      }
      memcpy (z->data + z->seek, b, len);
      z->seek += len;
      return (len/size);
    }
    return fwrite (b, size, count, z->f);
}

/*
char * zfile_fgets(char *buf, int len, struct zfile *z)
{
    if (z->data) {
      char *b = buf;
      if (buf == NULL || len <= 0) return NULL;

      while (--len > 0 && zfile_fread(buf, 1, 1, z) == 1 && *buf++ != '\n') ;
      *buf = '\0';
      return b == buf && len > 0 ? NULL : b;
    }
    return fgets(buf, len, z->f);
}
*/

char * zfile_fgets(char *buf, int len, struct zfile *z)
{
    if (z->data) {
      register char *b  = buf;
      register int  ll  = len;
      if (b == NULL || ll <= 0) return NULL;

      // optimize read one byte (only if we have no cache!)
      // (z->seek + 1 > z->size) !-> (z->seek + 1 <= z->size) -> (z->seek < z->size)  
      /// while (--ll > 0 && zfile_fread(b, 1, 1, z) == 1 && *b++ != '\n') ;
      while (--ll > 0 && ((z->seek < z->size) ? (*b = z->data[z->seek++]) : 0) && *b++ != '\n') ;
      *b = '\0';
      return b == buf && ll > 0 ? NULL : buf;
    }
    return fgets(buf, len, z->f);
}


size_t zfile_fputs (char *s, struct zfile *z)
{
    return zfile_fwrite (s, 1, strlen (s), z);
}


int zfile_fgetc(struct zfile *z)
{
    if (z->data) {
      unsigned char c;

      return zfile_fread(&c, 1, 1, z) == 1 ? c : -1;
    }
    return fgetc(z->f);
}


int zfile_fputc (int c, struct zfile *z)
{
  unsigned char bc;
  bc = c;
  return zfile_fwrite (&bc, 1, 1, z);
}


int zfile_fprintf(struct zfile *z, const char *format, ...)
{
  char buf[2048];  // may be to short - bad
  if (format)
  {
    va_list ap;

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    return zfile_fwrite (buf, 1, strlen(buf), z);
  }
  return 0;
}


int zfile_ferror(struct zfile *z)
{
    if (z->data) {
      return 0;  // !no error!
    }
    return ferror(z->f);
}


/*----------------------------------------------------------------------*/
int zfile_isZip(char *name) 
{
    int  found = FALSE; 
    if (name != NULL) {
      int  j;
      char *ext = strrchr (name, '.');

      if (ext != NULL) {
	ext++;
	for (j = 0; archive_ext[j]; j++) {        
	  if (UTILstrCaseCmp (ext, archive_ext[j]) == 0) {
            found = TRUE;
            break;
          }
        }
      }
    }
    return found;
}

int zfile_wasZip(char *name) 
{
    int  found = FALSE; 
    if (name != NULL) {
      int  j;
      char *sname = strNew(name);
      char zname[10] = ".";  /* zname[0] = '.' */
      strLower(sname);

      for (j = 0; archive_ext[j]; j++) {
        strcpy(&zname[1], archive_ext[j]);
        strcat(zname, STRING_PATH_SEPARATOR);  /* like ".zip/" or ".zip\" */    
	if (strstr (sname, zname) != NULL) {
          found = TRUE;
          break;
        }
      }
      objectDelete((Pointer_t)&sname);
    }
    return found;
}


void zfile_freadAllLikeZip(struct zfile *file) 
{
  if (!file->data) {
    long sz,rsz;
    char *pdata = NULL;
    zfile_fseek(file, 0, SEEK_END);
    sz = zfile_ftell(file);
    zfile_fseek(file, 0, SEEK_SET);
    pdata = (char*) objectNew(sz);
    if (pdata) {
      rsz = zfile_fread(pdata, 1, sz, file);
      zfile_fseek(file, 0, SEEK_SET);
      if (sz == rsz) {
        file->data = (utl_u8*)pdata;
        file->size = sz;
        file->seek = 0;
      } else {
        objectDelete((Pointer_t)&pdata);
      }
    }
  }
}
