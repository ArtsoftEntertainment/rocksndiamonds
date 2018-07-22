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
* util.c                                                   *
*----------------------------------------------------------*
* (c) by the owner of the alias/nickname "HerzAusGold"     *
*               e-mail: herzausgold@online.de              *
***********************************************************/

#include "libgame.h"
#include "rndapi.h"
#include "util.h"

/*===========================================================================*/
#ifdef VISUAL_CPP
//--> VISUAL_CPP
#include <sys/types.h>

#if !defined DIRENT_H
struct  dirent {
	_ino_t   d_ino;               // file number of entry 
	ushort_t d_reclen;            // length of this record 
	ushort_t d_namlen;            // length of string in d_name 
	char     d_name[256];         // DUMMY NAME LENGTH 
};
#endif
#endif


/*-- objectNew --------------------------------------------------------------*/
Pointer_t objectNew (const size_t	len) 
{
   Pointer_t pnt;

   pnt = (Pointer_t)checked_calloc(len);
   return pnt;
}

/*-- objectDelete -----------------------------------------------------------*/
bool_t objectDelete (Pointer_t		*mem) 
{
   if  (*mem != NULL) {
      checked_free (*mem);
      *mem = NULL;
   }
   return TRUE;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

int fileScanDir (
          char          *dirNameIn,
          List_t		*dirNameList,
          int           (*selectFnk) (char *) 
		  ) 
{
   char           *dirName   = strNew(dirNameIn);  // prepared: extract environment here

#ifndef VISUAL_CPP
  DIR              *dirHandle;
#else
  HANDLE            searchHndl;     /* for FindNextFile prpcess */
  WIN32_FIND_DATA   newEntry;       /* File data                */
  struct dirent     curLocalEntry;  /* dirent from file data    */
  char             *locDirFile   = NULL;
  bool_t            isFirstFile  = TRUE;
#endif /* VISUAL_CPP */

  struct dirent    *currentEntry = NULL;
  int               appendEntry  = TRUE;
  bool_t            error        = FALSE;

  if (dirName == NULL) {
     return -1;  /* error no directory defined */
  }

#ifndef VISUAL_CPP
  dirHandle = opendir(dirName);
  if ( dirHandle ) {
    do {
      currentEntry = readdir(dirHandle);
      if ( currentEntry ) {
#else
  locDirFile = objectNew (strlen (dirName) + 3);
  searchHndl = INVALID_HANDLE_VALUE;
  if (locDirFile) {
     strcpy (locDirFile, dirName);
     strcat (locDirFile, "\\*");
     searchHndl = FindFirstFile (locDirFile, &newEntry);
     objectDelete ((Pointer_t *)&locDirFile);
  }
  if (searchHndl != INVALID_HANDLE_VALUE) {
    currentEntry = &curLocalEntry;
    curLocalEntry.d_ino    = 0;
    curLocalEntry.d_reclen = sizeof(curLocalEntry);
    do {
      if (!isFirstFile) {
         if (!FindNextFile (searchHndl, &newEntry)) {
            currentEntry = NULL;
         }
      }
      isFirstFile = FALSE;
      if ( currentEntry ) {
        curLocalEntry.d_namlen = (short)strlen (newEntry.cFileName);
        strcpy (curLocalEntry.d_name, newEntry.cFileName);

#endif /* VISUAL_CPP */

        /* shall the current entry be appended on the array */
        if (selectFnk) {
          appendEntry = (*selectFnk)(currentEntry->d_name);
        } else {
          appendEntry = TRUE;
        }

        if ( appendEntry ) {

          if (!listEnter (dirNameList,
                          currentEntry->d_name,
						  strlen(currentEntry->d_name)+1,
                          FALSE)) {
             error = TRUE;
          }
        }
      }
    } while ( currentEntry && !error);
#ifndef VISUAL_CPP
    closedir(dirHandle);
#else
    FindClose (searchHndl);
#endif /* VISUAL_CPP */
  } else {
    error = TRUE;
  }

  if ( error ) {
    listFree(dirNameList);
	listInit(dirNameList);
  }
  
  objectDelete ((Pointer_t *)&dirName);
  return (listLength(dirNameList));
} // fileScanDir  


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
char * strUpper(char * curString) 
{
   char  *strRead;
   for (strRead = curString; *strRead; strRead++) {
      *strRead = toupper (*strRead);
   }
   return curString;
}


char * strLower(char * curString) 
{
   char  *strRead;
   for (strRead = curString; *strRead; strRead++) {
      *strRead = tolower (*strRead);
   }
   return curString;
}


int UTILstrCasePathCmp (
         const char  *firstString,
         const char  *secondString ) 
{

   int         retValue   = 0;  
   size_t      nMaxLength;     /* maxLength is minimum */
   size_t      firstLength;
   size_t      secondLength;
   long        nFirstChar;
   long        nSecondChar;

   firstLength  = strlen(firstString);
   secondLength = strlen(secondString);
   nMaxLength   = (firstLength < secondLength) ? firstLength : secondLength;
   if (nMaxLength > 0) {
      do {
         nFirstChar  = (unsigned char)(*(firstString++));
         nSecondChar = (unsigned char)(*(secondString++));
         if (('A' <= nFirstChar) && (nFirstChar <= 'Z')) {
            nFirstChar -= ('A' - 'a');
         }
         if (nFirstChar == '/') {
            nFirstChar = '\\';
         } 
         if (('A' <= nSecondChar) && (nSecondChar <= 'Z')) {
            nSecondChar -= ('A' - 'a');
         }
         if (nSecondChar == '/') {
            nSecondChar = '\\';
         } 
      } while (--nMaxLength && nFirstChar && (nFirstChar == nSecondChar));

      if (!nMaxLength && (firstLength != secondLength)) {
         if (firstLength > secondLength) {
            retValue = (unsigned char)(*(firstString));
         } else {
            retValue = -((unsigned char)(*(secondString)));
         }
      } else {
         retValue = nFirstChar - nSecondChar;
      }
   } else {
      if (firstLength > 0) {
         retValue = (unsigned char)(*(firstString));
      } else if (secondLength > 0) {
         retValue = -((unsigned char)(*(secondString)));
      }
   }

   if (retValue > 0) {
      retValue = 1;
   } else if (retValue < 0) {
      retValue = -1;
   }
   return retValue;
}


int UTILstrCaseCmp (
         const char  *firstString,
         const char  *secondString ) 
{

   int         retValue   = 0;  
   size_t      nMaxLength;     /* maxLength is minimum */
   size_t      firstLength;
   size_t      secondLength;
   long        nFirstChar;
   long        nSecondChar;

   firstLength  = strlen(firstString);
   secondLength = strlen(secondString);
   nMaxLength   = (firstLength < secondLength) ? firstLength : secondLength;
   if (nMaxLength > 0) {
      do {
         nFirstChar  = (unsigned char)(*(firstString++));
         nSecondChar = (unsigned char)(*(secondString++));
         if (('A' <= nFirstChar) && (nFirstChar <= 'Z')) {
            nFirstChar -= ('A' - 'a');
         }
         if (('A' <= nSecondChar) && (nSecondChar <= 'Z')) {
            nSecondChar -= ('A' - 'a');
         }
      } while (--nMaxLength && nFirstChar && (nFirstChar == nSecondChar));

      if (!nMaxLength && (firstLength != secondLength)) {
         if (firstLength > secondLength) {
            retValue = (unsigned char)(*(firstString));
         } else {
            retValue = -((unsigned char)(*(secondString)));
         }
      } else {
         retValue = nFirstChar - nSecondChar;
      }
   } else {
      if (firstLength > 0) {
         retValue = (unsigned char)(*(firstString));
      } else if (secondLength > 0) {
         retValue = -((unsigned char)(*(secondString)));
      }
   }

   if (retValue > 0) {
      retValue = 1;
   } else if (retValue < 0) {
      retValue = -1;
   }
   return retValue;
}


int UTILstrNCaseCmp (const char  *firstString,
                     const char  *secondString,
                     int         maxLength ) 
{
   int         retValue   = 0;
   size_t      nMaxLength = maxLength > 0 ? maxLength : 0;
   long        nFirstChar;
   long        nSecondChar;

   if (nMaxLength > 0) {
      do {
         nFirstChar  = (unsigned char)(*(firstString++));
         nSecondChar = (unsigned char)(*(secondString++));
         if (('A' <= nFirstChar) && (nFirstChar <= 'Z')) {
            nFirstChar -= ('A' - 'a');
         }
         if (('A' <= nSecondChar) && (nSecondChar <= 'Z')) {
            nSecondChar -= ('A' - 'a');
         }
      } while (--nMaxLength && nFirstChar && (nFirstChar == nSecondChar));

      retValue = nFirstChar - nSecondChar;
      if (retValue > 0) {
         retValue = 1;
      } else if (retValue < 0) {
         retValue = -1;
      }
   }
   return retValue;
}


char * strFindWildcard (
         char  *curString,
         char  *subString) 
{
   char    *retPnt   = NULL;
   char    *strPnt   = curString;
   char    *subPnt   = subString;
   char    *subPntEnd;
   char    *subPntMem;
   bool_t  found = TRUE;
   bool_t  starTerm = TRUE;
   size_t       len;
   size_t       lenMax;

   if (subPnt && curString) {
      if (*subPnt == '\0') {           /* no string */
         retPnt = curString;
      } else {
         subPnt    = strNew (subString);
         subPntMem = subPnt;
         while (*subPnt && found) {
            starTerm = FALSE;
            for (subPntEnd = subPnt, len = 0; *subPntEnd; subPntEnd++) {
               if (*subPntEnd == '*') {
                  starTerm     = TRUE;
                  *subPntEnd++ = '\0';
                  break;
               }
               len++;
            }
            if (!starTerm) {
               if ((lenMax = strlen (strPnt)) > len) {
                  strPnt += (lenMax - len);
               }
            }
            if (*subPnt) {
               if ((strPnt = strstr (strPnt, subPnt)) != NULL) {
                  if (!retPnt) {
                     retPnt = strPnt;
                  }
               } else {
                  found = FALSE;
               }
            } else {
               if (!retPnt) {
                  retPnt = strPnt;
               }
            }
            strPnt += len;
            subPnt  = subPntEnd;
         }
         if (found) {
            if (!retPnt) {
               retPnt = curString;
            }
         } else {
            retPnt = NULL;
         }
         objectDelete ((Pointer_t *)&subPntMem);
      }
   }
   return retPnt;
}



/*---------------------------------------------------------------------*/
int strEmpty(char *s)
{
  int rx = TRUE;
  int lg = strlen(s);
  if (lg > 0) {
    while(lg >= 0)
    {
      if ((s[lg] == ' ') || (s[lg] == '\t') || (s[lg] == '\n') || (s[lg] == '\r') || 
          (s[lg] == 0)) {
      } else {
        rx = FALSE;
        break;
      }
      lg--;
    }
  }
  return rx;
}


/*---------------------------------------------------------------------*/
void strTrimLeft (char *str, char *i_pCharSet) {

   if (!strEmpty(str) && i_pCharSet && (*i_pCharSet != '\0')) {
      char *pRead = NULL;
      char *pCheck;

      for (pCheck = str; *pCheck; pCheck++) {
         if (strchr (i_pCharSet, *pCheck) == NULL) {
            pRead = pCheck;
            break;
         }
      }
      if (pRead && (pRead != str)) {
         char *pWrite;
         for (pWrite = str; *pRead; pRead++) {
            *pWrite++ = *pRead;
         }
         *pWrite          = '\0';
      }
   }
}

/*---------------------------------------------------------------------*/
void strTrimRight (char *str, char *i_pCharSet) {

   if (!strEmpty(str) && i_pCharSet && (*i_pCharSet != '\0')) {
      char *pCheck;

      for (pCheck = str + strlen(str) - 1; pCheck >= str; pCheck--) {
         if (strchr (i_pCharSet, *pCheck) != NULL) {
            *pCheck = '\0';          
         } else {
            break;                   
         }
      }
   }
}


/*---------------------------------------------------------------------*/
void strTrim (char *str) {
   strTrimRight (str, " \t\n\r");
   strTrimLeft  (str, " \t\n\r");
}


/*---------------------------------------------------------------------*/
void strRemoveNL(char *str)
{
  strTrimRight (str, "\n\r");
}


/*---------------------------------------------------------------------*/
void strCut (char *str, char *i_pCharSet) {

   if (!strEmpty(str) && i_pCharSet && (*i_pCharSet != '\0')) {
      int  cnt=0;
      char *pWrite = str;
      char *pCheck;
      
      for (pCheck = str; *pCheck; pCheck++) {
         if (strchr (i_pCharSet, *pCheck) == NULL) {
            *pWrite++ = *pCheck;
            cnt = 0;
         } else {
            cnt++;
            if(cnt<=1) {
              *pWrite++ = *pCheck;
            }
         }
      }
      *pWrite = '\0';
   }
}

/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

char * envGet (char *envVar) 
{
	char	*tmpStr = NULL;

  tmpStr = getenv (envVar);

	if (tmpStr == NULL) {
    return(""); 
	} 
  return (tmpStr);
}


char * UTIL_envGetString (char *envVar)
{
   char *retStr = NULL;
   char *tmpStr = NULL;

   if (strstr (envVar, "$") == envVar) {
      char *pRest;
      char *pNewEnv = envVar + 1;
      char sChar    = '/';
      pRest = strchr (envVar, sChar);
      if (!pRest) {
        sChar = '\\';
        pRest = strchr (envVar, sChar);
      }
      if (pRest) {
         *pRest = '\0';
         tmpStr = envGet (pNewEnv);
         *pRest = sChar;
         
         retStr = objectNew(strlen(tmpStr) + 1 + strlen(pRest) + 1);
         strcpy (retStr, tmpStr);
         strcat (retStr, pRest);
         return (retStr);
      } else {
         tmpStr = envGet (pNewEnv); 
         retStr = strNew(tmpStr);
         return (retStr);
      }
   } else {
     retStr = strNew(envVar); 
 		 return (retStr); 
   }
}


/*=============================================================================
  list.c
=============================================================================*/

/*-- listInit ---------------------------------------------------------------*/
void listInit(List_t *list) 
{
   list->anchor  = NULL;
   list->current = NULL;
   list->index   = 0;
   list->length  = 0;
}


/*-- listInsert -------------------------------------------------------------*/
bool_t listInsert(List_t  *list,
                  void    *data,
                  bool_t   before_current) 
{
   listElement_t *le;

   le = objectNew (sizeof(listElement_t));
   if ( le == NULL ) {
      return (FALSE);
   }
   le->data = data;
   if ( list->length == 0 ) {
      list->anchor = le;
      le->next = le;
      le->prev = le;
   } else {
      if ( ! before_current ) {
         list->index++;
      } else {
         if ( list->anchor == list->current ) {
            list->anchor = le;
         }
         list->current = list->current->prev;
      }
      le->next = list->current->next;
      list->current->next->prev = le;
      list->current->next = le;
      le->prev = list->current;
   }
   list->length++;
   list->current = le;
   return(TRUE);
}


/*-- listEnter --------------------------------------------------------------*/
bool_t listEnter(List_t *list,
                 void   *data,
                 size_t  size,
                 bool_t  before_current) 
{
    bool_t       inserted;
    void         *new_element;

    new_element = (void*) objectNew (size);

    if ( new_element == NULL ) {
        return FALSE;
    }

    memcpy (new_element, data, size);
    inserted = listInsert (list, new_element, before_current);
    if ( !inserted ) {
      objectDelete (&new_element);
    } else {
      list->current->dataLen = size;
    }
    return inserted;
}


/*-- listNext ---------------------------------------------------------------*/
bool_t listNext(List_t *list) 
{
   if ( (list->length > 1) && (list->current->next != list->anchor) ) {
      list->current = list->current->next;
      list->index++;
      return (TRUE);
   } else {
      return (FALSE);
   }
}


/*-- listPrev ---------------------------------------------------------------*/
bool_t listPrev(List_t *list) 
{
   if ( list->current != list->anchor ) {
      list->current = list->current->prev;
      list->index--;
      return (TRUE);
   } else {
      return (FALSE);
   }
}


/*-- listHead ---------------------------------------------------------------*/
void listHead(List_t *list) 
{
   list->current = list->anchor;
   list->index = 0;
}


/*-- listTail ---------------------------------------------------------------*/
void listTail(List_t *list) 
{
   if ( list->length > 1 ) {
      list->current = list->anchor->prev;
      list->index = list->length - 1;
   }
}

/*-- listMove ---------------------------------------------------------------*/
void listMove(List_t *srcList, List_t *destList) 
{
	*destList = *srcList;
	listInit(srcList);
}

/*-- listCopy ---------------------------------------------------------------*/
void listCopy(List_t *srcList, List_t *destList, size_t size) 
{
  void *pData;
  if (listGetFirst(srcList, (void**) &pData)) {
    do {
      listEnter(destList, pData, size, FALSE);
    } while (listGetNext(srcList, (void**) &pData));
  }
}


/*-- listGet ----------------------------------------------------------------*/
bool_t listGet(List_t  *list,
               void   **data) 
{
   if ( list->length > 0 ) {
      *data = list->current->data;
      return (TRUE);
   } else {
      return (FALSE);
   }
}

bool_t listGetNext(List_t  *list,
                   void   **data) 
{
   if ( listNext(list) ) {
      *data = list->current->data;
      return (TRUE);
   } else {
      return (FALSE);
   }
}

bool_t listGetPrev(List_t  *list,
                   void   **data) 
{
   if ( listPrev (list) ) {
      *data = list->current->data;
      return (TRUE);
   } else {
      return (FALSE);
   }
}

bool_t listGetFirst(List_t  *list,
                    void   **data) 
{
   listHead (list);
   if ( list->length > 0 ) {
      *data = list->current->data;
      return (TRUE);
   } else {
      return (FALSE);
   }
}

bool_t listGetLast(List_t   *list,
                   void    **data) 
{
   listTail (list);
   if ( list->length > 0 ) {
      *data = list->current->data;
      return (TRUE);
   } else {
      return (FALSE);
   }
}


/*-- listRemove -------------------------------------------------------------*/
bool_t listRemove(List_t     *list,
                  void      **data,
                  bool_t      current_to_prev) 
{
   listElement_t *le;

   if ( list->length < 1 ) {
      return (FALSE);
   }
   le = list->current;
   *data = le->data;
   if ( list->length > 1 ) {
      le->next->prev = le->prev;
      le->prev->next = le->next;
      list->length--;
      if ( le == list->anchor ) {
         list->anchor = le->next;
         list->current = le->next;
      } else {
         if ( le->next == list->anchor || current_to_prev ) {
            list->index--;
            list->current = le->prev;
         } else {
            list->current = le->next;
         }
      }
   } else {
      listInit(list);
   }
   objectDelete ((Pointer_t *)&le);
   return(TRUE);
}


/*-- listFree ---------------------------------------------------------------*/
void listFree(List_t *list) 
{
    void         *element_ptr;
    bool_t       data_exists;

    listHead (list);
    data_exists = ! listEmpty(list);

    while ( data_exists ) {
        listRemove (list, &element_ptr, LIST_CURRENT_TO_NEXT);
        objectDelete (&element_ptr);
        data_exists = ! listEmpty(list);
    }
}


/*-- listLeave --------------------------------------------------------------*/
void listLeave(List_t *list,
               bool_t  current_to_prev) 
{
    void *element_ptr;

    listRemove (list, &element_ptr, current_to_prev);
    objectDelete (&element_ptr);
}


/*-- listEmpty --------------------------------------------------------------*/
bool_t listEmpty(List_t *list) 
{
   return (list->length == 0);
}


/*-- listIndex --------------------------------------------------------------*/
listIndex_t listIndex(List_t *list) 
{
   return (list->index);
}


/*-- listLength -------------------------------------------------------------*/
listIndex_t listLength(List_t *list) 
{
   return (list->length);
}


/*-- listPosition -----------------------------------------------------------*/
bool_t listPosition(List_t       *list,
                    listIndex_t  position) 
{
   listIndex_t i;
   listIndex_t prev_steps, next_steps;

   if ( position >= list->length ) {
      return (FALSE);
   }

   if ( list->index > position ) {
      prev_steps = list->index - position;
      next_steps = list->length - prev_steps;
   } else {
      next_steps = position - list->index;
      prev_steps = list->length - next_steps;
   }

   if ( prev_steps < next_steps ) {
      for ( i = 0; i < prev_steps; i++ ) {
         list->current = list->current->prev;
      }
   } else {
      for ( i = 0; i < next_steps; i++ ) {
         list->current = list->current->next;
      }
   }
   list->index = position;
   return(TRUE);
}


/*-- listSearch -------------------------------------------------------------*/
bool_t listSearch(List_t    *list,
                  void      *data,
                  bool_t    to_tail,
                  bool_t    searchAll,
                  bool_t    (*compare_func) (
                            PROTOTYPE_2 (const void *search_data,
                                         const void *list_data))) 
{
   if ( list->length < 1 ) {
      return (FALSE);
   }
   if ( compare_func (data, list->current->data) ) {
      return TRUE;
   } else {      
      listElement_t  *cur = list->current;
      if ( to_tail ) {
         while (listNext (list)) {
            if ( compare_func (data, list->current->data) ) {
               return TRUE;
            }
         }
         if ( searchAll ) {
           listHead(list);
           if ((cur != list->current) && compare_func (data, list->current->data) ) {
             return TRUE;
           }
           while ((cur != list->current) && listNext (list)) {
              if ( compare_func (data, list->current->data) ) {
                 return TRUE;
              }
           }
           listTail(list);
         }
         return FALSE;
      } else {
         while (listPrev (list)) {
            if ( compare_func (data, list->current->data) ) {
               return TRUE;
            }
         }
         // searchAll 
         if ( searchAll ) {
           listTail(list);
           if ((cur != list->current) && compare_func (data, list->current->data) ) {
             return TRUE;
           }
           while ((cur != list->current) && listPrev (list)) {
              if ( compare_func (data, list->current->data) ) {
                 return TRUE;
              }
           }
           listHead(list);
         }
         return FALSE;
      }
   }
}


/*-- listSort ---------------------------------------------------------------*/
void listSort(List_t  *list,
              bool_t  (*compare_func) (
                      PROTOTYPE_2 (const void *sort_data,
                                   const void *list_data ))) 
{
   register listElement_t *sorted  = list->anchor;
   register listElement_t *current;
   register listElement_t *max;

/* Select-Sort */

   if ( list->length > 1 ) {
      while ( sorted->next != list->anchor ) {
         max     = sorted;
         current = sorted;
         while ( current->next != list->anchor ) {
            current = current->next;
            if ( compare_func (max->data, current->data) ) {
               max = current;
            }
         }
         if ( max != sorted ) {
            max->next->prev    = max->prev;
            max->prev->next    = max->next;
            if ( list->anchor == sorted ) {
               list->anchor = max;
            }
            max->next          = sorted;
            sorted->prev->next = max;
            max->prev          = sorted->prev;
            sorted->prev       = max;
         } else {
            sorted             = sorted->next;
         }
      }
   }
   listHead(list);
}
