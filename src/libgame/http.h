// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2021 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// http.h
// ============================================================================

#ifndef HTTP_H
#define HTTP_H

#include "system.h"

#define MAX_HTTP_HEAD_SIZE		4096
#define MAX_HTTP_BODY_SIZE		1048576
#define MAX_HTTP_ERROR_SIZE		1024

#define HTTP_SUCCESS(c)			((c) >= 200 && (c) < 300)


struct HttpRequest
{
  char head[MAX_HTTP_HEAD_SIZE + 1];
  char body[MAX_HTTP_BODY_SIZE + 1];

  char *hostname;
  int port;
  char *method;
  char *uri;
};

struct HttpResponse
{
  char head[MAX_HTTP_HEAD_SIZE + 1];
  char body[MAX_HTTP_BODY_SIZE + 1];
  int body_size;

  int status_code;
  char status_text[MAX_HTTP_ERROR_SIZE + 1];
};


char *GetHttpError(void);
void ConvertHttpRequestBodyToServerEncoding(struct HttpRequest *);
void ConvertHttpResponseBodyToClientEncoding(struct HttpResponse *);
struct HttpResponse *GetHttpResponseFromBuffer(void *, int);
boolean DoHttpRequest(struct HttpRequest *, struct HttpResponse *);

#endif
