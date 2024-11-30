// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// types.h
// ============================================================================

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>


#if !defined(PLATFORM_WINDOWS)
typedef int boolean;
typedef unsigned char byte;
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef STATE_AUTO
#undef STATE_AUTO
#endif

#ifdef STATE_ASK
#undef STATE_ASK
#endif

#ifdef STATE_ONCE
#undef STATE_ONCE
#endif

// values for boolean data type
#define TRUE			1
#define FALSE			0

// values for 3-state data type (for "yes/no/auto" or "yes/no/ask")
#define STATE_TRUE		1
#define STATE_FALSE		0
#define STATE_AUTO		-1
#define STATE_ASK		-1
#define STATE_ONCE		-1

#ifndef MIN
#define MIN(a, b) 		((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) 		((a) > (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)			((a) < 0 ? -(a) : (a))
#endif

#ifndef SIGN
#define SIGN(a)			((a) < 0 ? -1 : ((a) > 0 ? 1 : 0))
#endif

#ifndef ODD
#define ODD(a)			(((a) & 1) == 1)
#endif

#ifndef EVEN
#define EVEN(a)			(((a) & 1) == 0)
#endif

#define ARRAY_SIZE(array)	(sizeof(array) / sizeof(array[0]))

#if defined(__x86_64__)

#define PTR_TO_INT(p)		((int) (long long) (p))
#define PTR_TO_UINT(p)		((unsigned int) (unsigned long long) (p))

#define INT_TO_PTR(i)		((void *) (long long) (i))
#define UINT_TO_PTR(u)		((void *) (unsigned long long) (u))

#else

#define PTR_TO_INT(p)		((int) (long) (p))
#define PTR_TO_UINT(p)		((unsigned int) (unsigned long) (p))

#define INT_TO_PTR(i)		((void *) (long) (i))
#define UINT_TO_PTR(u)		((void *) (unsigned long) (u))

#endif

#define STRUCT_OFFSET(s, m)	(offsetof(s, m))


struct ListNode
{
  char *key;
  void *content;
  struct ListNode *prev;
  struct ListNode *next;
};
typedef struct ListNode ListNode;

struct DelayCounter
{
  unsigned int value;
  unsigned int count;
};
typedef struct DelayCounter DelayCounter;

#endif // TYPES_H
