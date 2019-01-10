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
* util.h                                                   *
*----------------------------------------------------------*
* (c) by the owner of the alias/nickname "HerzAusGold"     *
*               e-mail: herzausgold@online.de              *
***********************************************************/
#ifndef UTIL_H
#define UTIL_H


/*---------------------------------------------------------------------------*/

#if defined(__cplusplus) || defined (__STDC__)
#define PROTOTYPE_2(a,b) a, b
#else
#define PROTOTYPE_2(a,b) 
#endif /* defined(__cplusplus) || defined (__STDC__) */

/*---------------------------------------------------------------------------*/

typedef int	bool_t;

typedef void	*Pointer_t; /* common pointer */

/*---------------------------------------------------------------------------*/

Pointer_t objectNew (const  size_t len);

bool_t    objectDelete (Pointer_t  *mem);

/*---------------------------------------------------------------------------*/

#define strNew(str) \
   (((str) != NULL) ? \
      (strcpy((char *)objectNew(strlen(str) + 1), str)) : NULL)
      
char * strFindWildcard (
         char  *curString,
         char  *subString); 

char * strUpper(char * curString);

char * strLower(char * curString);

int UTILstrCasePathCmp (
         const char  *firstString,
         const char  *secondString ); 

int UTILstrCaseCmp (
         const char  *firstString,
         const char  *secondString );

int UTILstrNCaseCmp (const char  *firstString,
                     const char  *secondString,
                     int         maxLength );


int strEmpty(char *s);

void strTrimRight (char *str, char *i_pCharSet);
void strTrimLeft (char *str, char *i_pCharSet);
void strTrim (char *str);

void strRemoveNL(char *str);

void strCut (char *str, char *i_pCharSet);

/*----------------------------------------------------------*/
 
char * UTIL_envGetString (char *envVar);

/*-----------------------------------------------------------------------------
  util.h - list
-----------------------------------------------------------------------------*/

#define LIST_BEFORE_CURRENT   TRUE
#define LIST_AFTER_CURRENT    FALSE
#define LIST_CURRENT_TO_PREV  TRUE
#define LIST_CURRENT_TO_NEXT  FALSE


/* private type "listElement_t" */
typedef struct listElement_rec {
       struct listElement_rec     *next;
       struct listElement_rec     *prev;
              void                *data;
              size_t               dataLen;
     } listElement_t;

typedef unsigned long listIndex_t;

/* abstract type "List_t" */
typedef struct {
        listElement_t *anchor;
        listElement_t *current;
        listIndex_t    index;
        listIndex_t    length;
      } List_t;

/* list functions */
void listInit(List_t *list);
bool_t listInsert(List_t  *list,
                  void    *data,
                  bool_t   before_current);
bool_t listEnter(List_t *list,
                 void   *data,
                 size_t  size,
                 bool_t  before_current);
bool_t listNext(List_t *list);
bool_t listPrev(List_t *list);
void listHead(List_t *list);
void listTail(List_t *list); 
void listMove(List_t *srcList, List_t *destList);
void listCopy(List_t *srcList, List_t *destList, size_t size);

bool_t listGet(List_t  *list,
               void   **data);
bool_t listGetNext(List_t  *list,
                   void   **data); 
bool_t listGetPrev(List_t  *list,
                   void   **data); 
bool_t listGetFirst(List_t  *list,
                    void   **data); 
bool_t listGetLast(List_t  *list,
                   void   **data);

bool_t listRemove(List_t     *list,
                  void      **data,
                  bool_t        current_to_prev);
void listFree(List_t *list);
void listLeave(List_t *list,
               bool_t  current_to_prev);

bool_t listEmpty(List_t *list);

listIndex_t listIndex(List_t *list); 
listIndex_t listLength(List_t *list); 

bool_t listPosition(List_t       *list,
                    listIndex_t  position); 

bool_t listSearch(List_t    *list,
                  void      *data,
                  bool_t    to_tail,
                  bool_t    searchAll,
                  bool_t    (*compare_func) (
                            PROTOTYPE_2 (const void *search_data,
                                         const void *list_data)));
void listSort(List_t  *list,
              bool_t  (*compare_func) (
                      PROTOTYPE_2 (const void *sort_data,
                                   const void *list_data ))); 




/*---------------------------------------------------------------------------*/
int fileScanDir (
          char          *dirNameIn,
          List_t		*dirNameList,
          int           (*selectFnk) (char *) 
		  ); 



#endif
