// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2021 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// http.c
// ============================================================================

#include <sys/stat.h>

#include "platform.h"

#include "http.h"
#include "misc.h"


static char http_error[MAX_HTTP_ERROR_SIZE];

static void SetHttpError(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vsnprintf(http_error, MAX_HTTP_ERROR_SIZE, format, ap);
  va_end(ap);
}

char *GetHttpError(void)
{
  return http_error;
}

void ConvertHttpRequestBodyToServerEncoding(struct HttpRequest *request)
{
  char *body_utf8 = getUTF8FromLatin1(request->body);

  strncpy(request->body, body_utf8, MAX_HTTP_BODY_SIZE);
  request->body[MAX_HTTP_BODY_SIZE] = '\0';

  checked_free(body_utf8);
}

void ConvertHttpResponseBodyToClientEncoding(struct HttpResponse *response)
{
  char *body_latin1 = getLatin1FromUTF8(response->body);

  strncpy(response->body, body_latin1, MAX_HTTP_BODY_SIZE);
  response->body[MAX_HTTP_BODY_SIZE] = '\0';

  response->body_size = strlen(response->body);

  checked_free(body_latin1);
}

static void SetHttpResponseToDefaults(struct HttpResponse *response)
{
  response->head[0] = '\0';
  response->body[0] = '\0';
  response->body_size = 0;

  response->status_code = 0;
  response->status_text[0] = '\0';
}

struct HttpResponse *GetHttpResponseFromBuffer(void *buffer, int body_size)
{
  if (body_size > MAX_HTTP_BODY_SIZE)
    return NULL;

  struct HttpResponse *response = checked_calloc(sizeof(struct HttpResponse));

  SetHttpResponseToDefaults(response);

  memcpy(response->body, buffer, body_size);
  response->body[body_size] = '\0';
  response->body_size = body_size;

  return response;
}

static boolean SetHTTPResponseCode(struct HttpResponse *response, char *buffer)
{
  char *prefix = "HTTP/1.1 ";
  char *prefix_start = strstr(buffer, prefix);

  if (prefix_start == NULL)
    return FALSE;

  char *status_code_start = prefix_start + strlen(prefix);
  char *status_code_end = strstr(status_code_start, " ");

  if (status_code_end == NULL)
    return FALSE;

  int status_code_size = status_code_end - status_code_start;

  if (status_code_size != 3)	// status code must have three digits
    return FALSE;

  char status_code[status_code_size + 1];

  strncpy(status_code, status_code_start, status_code_size);
  status_code[status_code_size] = '\0';

  response->status_code = atoi(status_code);

  char *status_text_start = status_code_end + 1;
  char *status_text_end = strstr(status_text_start, "\r\n");

  if (status_text_end == NULL)
    return FALSE;

  int status_text_size = status_text_end - status_text_start;

  if (status_text_size > MAX_HTTP_ERROR_SIZE)
    return FALSE;

  strncpy(response->status_text, status_text_start, status_text_size);
  response->status_text[status_text_size] = '\0';

  return TRUE;
}

static boolean SetHTTPResponseHead(struct HttpResponse *response, char *buffer)
{
  char *separator = "\r\n\r\n";
  char *separator_start = strstr(buffer, separator);

  if (separator_start == NULL)
    return FALSE;

  int head_size = separator_start - buffer;

  if (head_size > MAX_HTTP_HEAD_SIZE)
    return FALSE;

  strncpy(response->head, buffer, head_size);
  response->head[head_size] = '\0';

  return TRUE;
}

static boolean SetHTTPResponseBody(struct HttpResponse *response, char *buffer,
				   int buffer_size)
{
  char *separator = "\r\n\r\n";
  char *separator_start = strstr(buffer, separator);

  if (separator_start == NULL)
    return FALSE;

  int separator_size = strlen(separator);
  int full_head_size = separator_start + separator_size - buffer;
  int body_size = buffer_size - full_head_size;

  if (body_size > MAX_HTTP_BODY_SIZE)
    return FALSE;

  memcpy(response->body, buffer + full_head_size, body_size);
  response->body[body_size] = '\0';
  response->body_size = body_size;

  return TRUE;
}

static int GetHttpResponse(TCPsocket socket, char *buffer, int max_buffer_size)
{
  char *buffer_ptr = buffer;
  int buffer_left = max_buffer_size;
  int buffer_size = 0;
  int response_size = 0;

  while (1)
  {
    // read as many bytes to the buffer as possible
    int bytes = SDLNet_TCP_Recv(socket, buffer_ptr, buffer_left);

    if (bytes <= 0)
    {
      SetHttpError("receiving response from server failed");

      return -1;
    }

    buffer_ptr += bytes;
    buffer_size += bytes;
    buffer_left -= bytes;

    // check if response size was already determined
    if (response_size > 0)
    {
      // check if response data was completely received
      if (buffer_size >= response_size)
	break;

      // continue reading response body from server
      continue;
    }

    char *separator = "\r\n\r\n";
    char *separator_start = strstr(buffer, separator);
    int separator_size = strlen(separator);

    // check if response header was completely received
    if (separator_start == NULL)
    {
      // continue reading response header from server
      continue;
    }

    char *content_length = "Content-Length: ";
    char *content_length_start = strstr(buffer, content_length);
    int head_size = separator_start - buffer;

    // check if response header contains content length header
    if (content_length_start == NULL ||
	content_length_start >= buffer + head_size)
    {
      SetHttpError("receiving 'Content-Length' header from server failed");

      return -1;
    }

    char *content_length_value = content_length_start + strlen(content_length);
    char *content_length_end = strstr(content_length_value, "\r\n");

    // check if content length header has line termination
    if (content_length_end == NULL)
    {
      SetHttpError("receiving 'Content-Length' value from server failed");

      return -1;
    }

    int value_len = content_length_end - content_length_value;
    int max_value_len = 10;

    // check if content length header has valid size
    if (value_len > max_value_len)
    {
      SetHttpError("received invalid 'Content-Length' value from server");

      return -1;
    }

    char value_str[value_len + 1];

    strncpy(value_str, content_length_value, value_len);
    value_str[value_len] = '\0';

    int body_size = atoi(value_str);

    response_size = head_size + separator_size + body_size;

    // check if response data was completely received
    if (buffer_size >= response_size)
      break;
  }

  return buffer_size;
}

static boolean DoHttpRequestExt(struct HttpRequest *request,
				struct HttpResponse *response,
				char *send_buffer,
				char *recv_buffer,
				int max_http_buffer_size,
				SDLNet_SocketSet *socket_set,
				TCPsocket *socket)
{
  IPaddress ip;
  int server_host;

  SetHttpResponseToDefaults(response);

  *socket_set = SDLNet_AllocSocketSet(1);

  if (*socket_set == NULL)
  {
    SetHttpError("cannot allocate socket set");

    return FALSE;
  }

  SDLNet_ResolveHost(&ip, request->hostname, request->port);

  if (ip.host == INADDR_NONE)
  {
    SetHttpError("cannot resolve hostname '%s'", request->hostname);

    return FALSE;
  }

  server_host = SDLNet_Read32(&ip.host);

  Debug("network:http", "trying to connect to server at %d.%d.%d.%d ...",
        (server_host >> 24) & 0xff,
        (server_host >> 16) & 0xff,
        (server_host >>  8) & 0xff,
        (server_host >>  0) & 0xff);

  *socket = SDLNet_TCP_Open(&ip);

  if (*socket == NULL)
  {
    SetHttpError("cannot connect to host '%s': %s", request->hostname,
		 SDLNet_GetError());

    return FALSE;
  }

  if (SDLNet_TCP_AddSocket(*socket_set, *socket) == -1)
  {
    SetHttpError("cannot add socket to socket set");

    return FALSE;
  }

  Debug("network:http", "successfully connected to server");

  snprintf(request->head, MAX_HTTP_HEAD_SIZE,
	   "%s %s HTTP/1.1\r\n"
	   "Host: %s\r\n"
	   "X-Requested-With: XMLHttpRequest\r\n"
	   "Content-Type: application/json\r\n"
	   "Connection: close\r\n"
	   "Content-Length: %d\r\n",
	   request->method,
	   request->uri,
	   request->hostname,
	   (int)strlen(request->body));

  snprintf(send_buffer, max_http_buffer_size,
	   "%s\r\n%s", request->head, request->body);

  Debug("network:http", "client request:\n--- snip ---\n%s\n--- snip ---",
	send_buffer);

  int send_bytes = SDLNet_TCP_Send(*socket, send_buffer, strlen(send_buffer));

  if (send_bytes != strlen(send_buffer))
  {
    SetHttpError("sending request to server failed");

    return FALSE;
  }

  int recv_bytes = GetHttpResponse(*socket, recv_buffer, max_http_buffer_size);

  if (recv_bytes <= 0)
  {
    // HTTP error already set in GetHttpResponse()

    return FALSE;
  }

  recv_buffer[recv_bytes] = '\0';

  Debug("network:http", "server response:\n--- snip ---\n%s\n--- snip ---",
	recv_buffer);

  if (!SetHTTPResponseCode(response, recv_buffer))
  {
    SetHttpError("malformed HTTP response");

    return FALSE;
  }

  if (!SetHTTPResponseHead(response, recv_buffer))
  {
    SetHttpError("invalid HTTP response header");

    return FALSE;
  }

  if (!SetHTTPResponseBody(response, recv_buffer, recv_bytes))
  {
    SetHttpError("invalid HTTP response body");

    return FALSE;
  }

  Debug("network:http", "server response: %d %s",
	response->status_code,
	response->status_text);

  return TRUE;
}

boolean DoHttpRequest(struct HttpRequest *request,
		      struct HttpResponse *response)
{
  int max_http_buffer_size = MAX_HTTP_HEAD_SIZE + 2 + MAX_HTTP_BODY_SIZE + 1;
  char *send_buffer = checked_malloc(max_http_buffer_size);
  char *recv_buffer = checked_malloc(max_http_buffer_size);
  SDLNet_SocketSet socket_set = NULL;
  TCPsocket socket = NULL;

  boolean success = DoHttpRequestExt(request, response,
				     send_buffer, recv_buffer,
				     max_http_buffer_size,
				     &socket_set, &socket);
  if (socket_set != NULL)
  {
    if (socket != NULL)
    {
      SDLNet_TCP_DelSocket(socket_set, socket);
      SDLNet_TCP_Close(socket);
    }

    SDLNet_FreeSocketSet(socket_set);
  }

  checked_free(send_buffer);
  checked_free(recv_buffer);

  runtime.use_api_server = success;

  return success;
}
