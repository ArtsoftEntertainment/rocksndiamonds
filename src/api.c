// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2022 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// api.c
// ============================================================================

#include "libgame/libgame.h"

#include "api.h"
#include "main.h"
#include "files.h"
#include "config.h"


// ============================================================================
// generic helper functions
// ============================================================================

static void ExecuteAsThread(SDL_ThreadFunction function, char *name, void *data,
			    char *error)
{
#if defined(PLATFORM_EMSCRIPTEN)
  // threads currently not fully supported by Emscripten/SDL and some browsers
  function(data);
#else
  SDL_Thread *thread = SDL_CreateThread(function, name, data);

  if (thread != NULL)
    SDL_DetachThread(thread);
  else
    Error("Cannot create thread to %s!", error);

  // nasty kludge to lower probability of intermingled thread error messages
  Delay(1);
#endif
}

static char *getPasswordJSON(char *password)
{
  static char password_json[MAX_FILENAME_LEN] = "";
  static boolean initialized = FALSE;

  if (!initialized)
  {
    if (password != NULL &&
	!strEqual(password, "") &&
	!strEqual(password, UNDEFINED_PASSWORD))
      snprintf(password_json, MAX_FILENAME_LEN,
	       "  \"password\":             \"%s\",\n",
	       setup.api_server_password);

    initialized = TRUE;
  }

  return password_json;
}

static char *getFileBase64(char *filename)
{
  struct stat file_status;

  if (stat(filename, &file_status) != 0)
  {
    Error("cannot stat file '%s'", filename);

    return NULL;
  }

  int buffer_size = file_status.st_size;
  byte *buffer = checked_malloc(buffer_size);
  FILE *file;
  int i;

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error("cannot open file '%s'", filename);

    checked_free(buffer);

    return NULL;
  }

  for (i = 0; i < buffer_size; i++)
  {
    int c = fgetc(file);

    if (c == EOF)
    {
      Error("cannot read from input file '%s'", filename);

      fclose(file);
      checked_free(buffer);

      return NULL;
    }

    buffer[i] = (byte)c;
  }

  fclose(file);

  int buffer_encoded_size = base64_encoded_size(buffer_size);
  char *buffer_encoded = checked_malloc(buffer_encoded_size);

  base64_encode(buffer_encoded, buffer, buffer_size);

  checked_free(buffer);

  return buffer_encoded;
}


// ============================================================================
// add score API functions
// ============================================================================

struct ApiAddScoreThreadData
{
  int level_nr;
  boolean tape_saved;
  char *leveldir_subdir;
  char *score_tape_filename;
  struct ScoreEntry score_entry;
};

static void *CreateThreadData_ApiAddScore(int nr, boolean tape_saved,
					  char *score_tape_filename)
{
  struct ApiAddScoreThreadData *data =
    checked_malloc(sizeof(struct ApiAddScoreThreadData));
  struct ScoreEntry *score_entry = &scores.entry[scores.last_added];

  if (score_tape_filename == NULL)
    score_tape_filename = getScoreTapeFilename(score_entry->tape_basename, nr);

  data->level_nr = nr;
  data->tape_saved = tape_saved;
  data->leveldir_subdir = getStringCopy(leveldir_current->subdir);
  data->score_tape_filename = getStringCopy(score_tape_filename);
  data->score_entry = *score_entry;

  return data;
}

static void FreeThreadData_ApiAddScore(void *data_raw)
{
  struct ApiAddScoreThreadData *data = data_raw;

  checked_free(data->leveldir_subdir);
  checked_free(data->score_tape_filename);
  checked_free(data);
}

static boolean SetRequest_ApiAddScore(struct HttpRequest *request,
				      void *data_raw)
{
  struct ApiAddScoreThreadData *data = data_raw;
  struct ScoreEntry *score_entry = &data->score_entry;
  char *score_tape_filename = data->score_tape_filename;
  boolean tape_saved = data->tape_saved;
  int level_nr = data->level_nr;

  request->hostname = setup.api_server_hostname;
  request->port     = API_SERVER_PORT;
  request->method   = API_SERVER_METHOD;
  request->uri      = API_SERVER_URI_ADD;

  char *tape_base64 = getFileBase64(score_tape_filename);

  if (tape_base64 == NULL)
  {
    Error("loading and base64 encoding score tape file failed");

    return FALSE;
  }

  char *player_name_raw = score_entry->name;
  char *player_uuid_raw = setup.player_uuid;

  if (options.player_name != NULL && global.autoplay_leveldir != NULL)
  {
    player_name_raw = options.player_name;
    player_uuid_raw = "";
  }

  char *levelset_identifier = getEscapedJSON(leveldir_current->identifier);
  char *levelset_name       = getEscapedJSON(leveldir_current->name);
  char *levelset_author     = getEscapedJSON(leveldir_current->author);
  char *level_name          = getEscapedJSON(level.name);
  char *level_author        = getEscapedJSON(level.author);
  char *player_name         = getEscapedJSON(player_name_raw);
  char *player_uuid         = getEscapedJSON(player_uuid_raw);

  snprintf(request->body, MAX_HTTP_BODY_SIZE,
	   "{\n"
	   "%s"
	   "  \"game_version\":         \"%s\",\n"
	   "  \"game_platform\":        \"%s\",\n"
	   "  \"batch_time\":           \"%d\",\n"
	   "  \"levelset_identifier\":  \"%s\",\n"
	   "  \"levelset_name\":        \"%s\",\n"
	   "  \"levelset_author\":      \"%s\",\n"
	   "  \"levelset_num_levels\":  \"%d\",\n"
	   "  \"levelset_first_level\": \"%d\",\n"
	   "  \"level_nr\":             \"%d\",\n"
	   "  \"level_name\":           \"%s\",\n"
	   "  \"level_author\":         \"%s\",\n"
	   "  \"use_step_counter\":     \"%d\",\n"
	   "  \"rate_time_over_score\": \"%d\",\n"
	   "  \"player_name\":          \"%s\",\n"
	   "  \"player_uuid\":          \"%s\",\n"
	   "  \"score\":                \"%d\",\n"
	   "  \"time\":                 \"%d\",\n"
	   "  \"tape_basename\":        \"%s\",\n"
	   "  \"tape_saved\":           \"%d\",\n"
	   "  \"tape\":                 \"%s\"\n"
	   "}\n",
	   getPasswordJSON(setup.api_server_password),
	   getProgramRealVersionString(),
	   getProgramPlatformString(),
	   (int)global.autoplay_time,
	   levelset_identifier,
	   levelset_name,
	   levelset_author,
	   leveldir_current->levels,
	   leveldir_current->first_level,
	   level_nr,
	   level_name,
	   level_author,
	   level.use_step_counter,
	   level.rate_time_over_score,
	   player_name,
	   player_uuid,
	   score_entry->score,
	   score_entry->time,
	   score_entry->tape_basename,
	   tape_saved,
	   tape_base64);

  checked_free(tape_base64);

  checked_free(levelset_identifier);
  checked_free(levelset_name);
  checked_free(levelset_author);
  checked_free(level_name);
  checked_free(level_author);
  checked_free(player_name);
  checked_free(player_uuid);

  ConvertHttpRequestBodyToServerEncoding(request);

  return TRUE;
}

static void HandleResponse_ApiAddScore(struct HttpResponse *response,
				       void *data_raw)
{
  server_scores.uploaded = TRUE;
}

static void HandleFailure_ApiAddScore(void *data_raw)
{
  struct ApiAddScoreThreadData *data = data_raw;

  PrepareScoreTapesForUpload(data->leveldir_subdir);
}

#if defined(PLATFORM_EMSCRIPTEN)
static void Emscripten_ApiAddScore_Loaded(unsigned handle, void *data_raw,
					  void *buffer, unsigned int size)
{
  struct HttpResponse *response = GetHttpResponseFromBuffer(buffer, size);

  if (response != NULL)
  {
    HandleResponse_ApiAddScore(response, data_raw);

    checked_free(response);
  }
  else
  {
    Error("server response too large to handle (%d bytes)", size);

    HandleFailure_ApiAddScore(data_raw);
  }

  FreeThreadData_ApiAddScore(data_raw);
}

static void Emscripten_ApiAddScore_Failed(unsigned handle, void *data_raw,
					  int code, const char *status)
{
  Error("server failed to handle request: %d %s", code, status);

  HandleFailure_ApiAddScore(data_raw);

  FreeThreadData_ApiAddScore(data_raw);
}

static void Emscripten_ApiAddScore_Progress(unsigned handle, void *data_raw,
					    int bytes, int size)
{
  // nothing to do here
}

static void Emscripten_ApiAddScore_HttpRequest(struct HttpRequest *request,
					       void *data_raw)
{
  if (!SetRequest_ApiAddScore(request, data_raw))
  {
    FreeThreadData_ApiAddScore(data_raw);

    return;
  }

  emscripten_async_wget2_data(request->uri,
			      request->method,
			      request->body,
			      data_raw,
			      TRUE,
			      Emscripten_ApiAddScore_Loaded,
			      Emscripten_ApiAddScore_Failed,
			      Emscripten_ApiAddScore_Progress);
}

#else

static void ApiAddScore_HttpRequestExt(struct HttpRequest *request,
				       struct HttpResponse *response,
				       void *data_raw)
{
  if (!SetRequest_ApiAddScore(request, data_raw))
    return;

  if (!DoHttpRequest(request, response))
  {
    Error("HTTP request failed: %s", GetHttpError());

    HandleFailure_ApiAddScore(data_raw);

    return;
  }

  if (!HTTP_SUCCESS(response->status_code))
  {
    Error("server failed to handle request: %d %s",
	  response->status_code,
	  response->status_text);

    HandleFailure_ApiAddScore(data_raw);

    return;
  }

  HandleResponse_ApiAddScore(response, data_raw);
}

static void ApiAddScore_HttpRequest(struct HttpRequest *request,
				    struct HttpResponse *response,
				    void *data_raw)
{
  ApiAddScore_HttpRequestExt(request, response, data_raw);

  FreeThreadData_ApiAddScore(data_raw);
}
#endif

static int ApiAddScoreThread(void *data_raw)
{
  struct HttpRequest *request = checked_calloc(sizeof(struct HttpRequest));
  struct HttpResponse *response = checked_calloc(sizeof(struct HttpResponse));

  program.api_thread_count++;

#if defined(PLATFORM_EMSCRIPTEN)
  Emscripten_ApiAddScore_HttpRequest(request, data_raw);
#else
  ApiAddScore_HttpRequest(request, response, data_raw);
#endif

  program.api_thread_count--;

  checked_free(request);
  checked_free(response);

  return 0;
}

void ApiAddScoreAsThread(int nr, boolean tape_saved, char *score_tape_filename)
{
  struct ApiAddScoreThreadData *data =
    CreateThreadData_ApiAddScore(nr, tape_saved, score_tape_filename);

  ExecuteAsThread(ApiAddScoreThread,
		  "ApiAddScore", data,
		  "upload score to server");
}


// ============================================================================
// get score API functions
// ============================================================================

struct ApiGetScoreThreadData
{
  int level_nr;
  char *score_cache_filename;
};

static void *CreateThreadData_ApiGetScore(int nr)
{
  struct ApiGetScoreThreadData *data =
    checked_malloc(sizeof(struct ApiGetScoreThreadData));
  char *score_cache_filename = getScoreCacheFilename(nr);

  data->level_nr = nr;
  data->score_cache_filename = getStringCopy(score_cache_filename);

  return data;
}

static void FreeThreadData_ApiGetScore(void *data_raw)
{
  struct ApiGetScoreThreadData *data = data_raw;

  checked_free(data->score_cache_filename);
  checked_free(data);
}

static boolean SetRequest_ApiGetScore(struct HttpRequest *request,
				      void *data_raw)
{
  struct ApiGetScoreThreadData *data = data_raw;
  int level_nr = data->level_nr;

  request->hostname = setup.api_server_hostname;
  request->port     = API_SERVER_PORT;
  request->method   = API_SERVER_METHOD;
  request->uri      = API_SERVER_URI_GET;

  char *levelset_identifier = getEscapedJSON(leveldir_current->identifier);
  char *levelset_name       = getEscapedJSON(leveldir_current->name);

  snprintf(request->body, MAX_HTTP_BODY_SIZE,
	   "{\n"
	   "%s"
	   "  \"game_version\":         \"%s\",\n"
	   "  \"game_platform\":        \"%s\",\n"
	   "  \"levelset_identifier\":  \"%s\",\n"
	   "  \"levelset_name\":        \"%s\",\n"
	   "  \"level_nr\":             \"%d\"\n"
	   "}\n",
	   getPasswordJSON(setup.api_server_password),
	   getProgramRealVersionString(),
	   getProgramPlatformString(),
	   levelset_identifier,
	   levelset_name,
	   level_nr);

  checked_free(levelset_identifier);
  checked_free(levelset_name);

  ConvertHttpRequestBodyToServerEncoding(request);

  return TRUE;
}

static void HandleResponse_ApiGetScore(struct HttpResponse *response,
				       void *data_raw)
{
  struct ApiGetScoreThreadData *data = data_raw;

  if (response->body_size == 0)
  {
    // no scores available for this level

    return;
  }

  ConvertHttpResponseBodyToClientEncoding(response);

  char *filename = data->score_cache_filename;
  FILE *file;
  int i;

  // used instead of "leveldir_current->subdir" (for network games)
  InitScoreCacheDirectory(levelset.identifier);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot save score cache file '%s'", filename);

    return;
  }

  for (i = 0; i < response->body_size; i++)
    fputc(response->body[i], file);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  server_scores.updated = TRUE;
}

#if defined(PLATFORM_EMSCRIPTEN)
static void Emscripten_ApiGetScore_Loaded(unsigned handle, void *data_raw,
					  void *buffer, unsigned int size)
{
  struct HttpResponse *response = GetHttpResponseFromBuffer(buffer, size);

  if (response != NULL)
  {
    HandleResponse_ApiGetScore(response, data_raw);

    checked_free(response);
  }
  else
  {
    Error("server response too large to handle (%d bytes)", size);
  }

  FreeThreadData_ApiGetScore(data_raw);
}

static void Emscripten_ApiGetScore_Failed(unsigned handle, void *data_raw,
					  int code, const char *status)
{
  Error("server failed to handle request: %d %s", code, status);

  FreeThreadData_ApiGetScore(data_raw);
}

static void Emscripten_ApiGetScore_Progress(unsigned handle, void *data_raw,
					    int bytes, int size)
{
  // nothing to do here
}

static void Emscripten_ApiGetScore_HttpRequest(struct HttpRequest *request,
					       void *data_raw)
{
  if (!SetRequest_ApiGetScore(request, data_raw))
  {
    FreeThreadData_ApiGetScore(data_raw);

    return;
  }

  emscripten_async_wget2_data(request->uri,
			      request->method,
			      request->body,
			      data_raw,
			      TRUE,
			      Emscripten_ApiGetScore_Loaded,
			      Emscripten_ApiGetScore_Failed,
			      Emscripten_ApiGetScore_Progress);
}

#else

static void ApiGetScore_HttpRequestExt(struct HttpRequest *request,
				       struct HttpResponse *response,
				       void *data_raw)
{
  if (!SetRequest_ApiGetScore(request, data_raw))
    return;

  if (!DoHttpRequest(request, response))
  {
    Error("HTTP request failed: %s", GetHttpError());

    return;
  }

  if (!HTTP_SUCCESS(response->status_code))
  {
    // do not show error message if no scores found for this level set
    if (response->status_code == 404)
      return;

    Error("server failed to handle request: %d %s",
	  response->status_code,
	  response->status_text);

    return;
  }

  HandleResponse_ApiGetScore(response, data_raw);
}

static void ApiGetScore_HttpRequest(struct HttpRequest *request,
				    struct HttpResponse *response,
				    void *data_raw)
{
  ApiGetScore_HttpRequestExt(request, response, data_raw);

  FreeThreadData_ApiGetScore(data_raw);
}
#endif

static int ApiGetScoreThread(void *data_raw)
{
  struct HttpRequest *request = checked_calloc(sizeof(struct HttpRequest));
  struct HttpResponse *response = checked_calloc(sizeof(struct HttpResponse));

  program.api_thread_count++;

#if defined(PLATFORM_EMSCRIPTEN)
  Emscripten_ApiGetScore_HttpRequest(request, data_raw);
#else
  ApiGetScore_HttpRequest(request, response, data_raw);
#endif

  program.api_thread_count--;

  checked_free(request);
  checked_free(response);

  return 0;
}

void ApiGetScoreAsThread(int nr)
{
  struct ApiGetScoreThreadData *data = CreateThreadData_ApiGetScore(nr);

  ExecuteAsThread(ApiGetScoreThread,
		  "ApiGetScore", data,
		  "download scores from server");
}


// ============================================================================
// get score tape API functions
// ============================================================================

struct ApiGetScoreTapeThreadData
{
  int level_nr;
  int score_id;
  char *score_tape_filename;
};

static void *CreateThreadData_ApiGetScoreTape(int nr, int id,
					      char *score_tape_basename)
{
  struct ApiGetScoreTapeThreadData *data =
    checked_malloc(sizeof(struct ApiGetScoreTapeThreadData));
  char *score_tape_filename = getScoreCacheTapeFilename(score_tape_basename, nr);

  data->level_nr = nr;
  data->score_id = id;
  data->score_tape_filename = getStringCopy(score_tape_filename);

  return data;
}

static void FreeThreadData_ApiGetScoreTape(void *data_raw)
{
  struct ApiGetScoreTapeThreadData *data = data_raw;

  checked_free(data->score_tape_filename);
  checked_free(data);
}

static boolean SetRequest_ApiGetScoreTape(struct HttpRequest *request,
                                          void *data_raw)
{
  struct ApiGetScoreTapeThreadData *data = data_raw;
  int score_id = data->score_id;

  request->hostname = setup.api_server_hostname;
  request->port     = API_SERVER_PORT;
  request->method   = API_SERVER_METHOD;
  request->uri      = API_SERVER_URI_GETTAPE;

  snprintf(request->body, MAX_HTTP_BODY_SIZE,
	   "{\n"
	   "%s"
	   "  \"game_version\":         \"%s\",\n"
	   "  \"game_platform\":        \"%s\",\n"
	   "  \"id\":                   \"%d\"\n"
	   "}\n",
	   getPasswordJSON(setup.api_server_password),
	   getProgramRealVersionString(),
	   getProgramPlatformString(),
	   score_id);

  ConvertHttpRequestBodyToServerEncoding(request);

  return TRUE;
}

static void HandleResponse_ApiGetScoreTape(struct HttpResponse *response,
                                           void *data_raw)
{
  struct ApiGetScoreTapeThreadData *data = data_raw;

  if (response->body_size == 0)
  {
    // no score tape available for this level

    return;
  }

  // (do not convert HTTP response body, as it contains binary data here)

  int level_nr = data->level_nr;
  char *filename = data->score_tape_filename;
  FILE *file;
  int i;

  // used instead of "leveldir_current->subdir" (for network games)
  InitScoreCacheTapeDirectory(levelset.identifier, level_nr);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot save score tape file '%s'", filename);

    return;
  }

  for (i = 0; i < response->body_size; i++)
    fputc(response->body[i], file);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  server_scores.tape_downloaded = TRUE;
}

#if defined(PLATFORM_EMSCRIPTEN)
static void Emscripten_ApiGetScoreTape_Loaded(unsigned handle, void *data_raw,
                                              void *buffer, unsigned int size)
{
  struct HttpResponse *response = GetHttpResponseFromBuffer(buffer, size);

  if (response != NULL)
  {
    HandleResponse_ApiGetScoreTape(response, data_raw);

    checked_free(response);
  }
  else
  {
    Error("server response too large to handle (%d bytes)", size);
  }

  FreeThreadData_ApiGetScoreTape(data_raw);
}

static void Emscripten_ApiGetScoreTape_Failed(unsigned handle, void *data_raw,
                                              int code, const char *status)
{
  Error("server failed to handle request: %d %s", code, status);

  FreeThreadData_ApiGetScoreTape(data_raw);
}

static void Emscripten_ApiGetScoreTape_Progress(unsigned handle, void *data_raw,
                                                int bytes, int size)
{
  // nothing to do here
}

static void Emscripten_ApiGetScoreTape_HttpRequest(struct HttpRequest *request,
                                                   void *data_raw)
{
  if (!SetRequest_ApiGetScoreTape(request, data_raw))
  {
    FreeThreadData_ApiGetScoreTape(data_raw);

    return;
  }

  emscripten_async_wget2_data(request->uri,
			      request->method,
			      request->body,
			      data_raw,
			      TRUE,
			      Emscripten_ApiGetScoreTape_Loaded,
			      Emscripten_ApiGetScoreTape_Failed,
			      Emscripten_ApiGetScoreTape_Progress);
}

#else

static void ApiGetScoreTape_HttpRequestExt(struct HttpRequest *request,
                                           struct HttpResponse *response,
                                           void *data_raw)
{
  if (!SetRequest_ApiGetScoreTape(request, data_raw))
    return;

  if (!DoHttpRequest(request, response))
  {
    Error("HTTP request failed: %s", GetHttpError());

    return;
  }

  if (!HTTP_SUCCESS(response->status_code))
  {
    // do not show error message if no scores found for this level set
    if (response->status_code == 404)
      return;

    Error("server failed to handle request: %d %s",
	  response->status_code,
	  response->status_text);

    return;
  }

  HandleResponse_ApiGetScoreTape(response, data_raw);
}

static void ApiGetScoreTape_HttpRequest(struct HttpRequest *request,
                                        struct HttpResponse *response,
                                        void *data_raw)
{
  ApiGetScoreTape_HttpRequestExt(request, response, data_raw);

  FreeThreadData_ApiGetScoreTape(data_raw);
}
#endif

static int ApiGetScoreTapeThread(void *data_raw)
{
  struct HttpRequest *request = checked_calloc(sizeof(struct HttpRequest));
  struct HttpResponse *response = checked_calloc(sizeof(struct HttpResponse));

  program.api_thread_count++;

#if defined(PLATFORM_EMSCRIPTEN)
  Emscripten_ApiGetScoreTape_HttpRequest(request, data_raw);
#else
  ApiGetScoreTape_HttpRequest(request, response, data_raw);
#endif

  program.api_thread_count--;

  checked_free(request);
  checked_free(response);

  return 0;
}

void ApiGetScoreTapeAsThread(int nr, int id, char *score_tape_basename)
{
  struct ApiGetScoreTapeThreadData *data =
    CreateThreadData_ApiGetScoreTape(nr, id, score_tape_basename);

  ExecuteAsThread(ApiGetScoreTapeThread,
		  "ApiGetScoreTape", data,
		  "download score tape from server");
}


// ============================================================================
// rename player API functions
// ============================================================================

struct ApiRenamePlayerThreadData
{
  char *player_name;
  char *player_uuid;
};

static void *CreateThreadData_ApiRenamePlayer(void)
{
  struct ApiRenamePlayerThreadData *data =
    checked_malloc(sizeof(struct ApiRenamePlayerThreadData));

  data->player_name = getStringCopy(setup.player_name);
  data->player_uuid = getStringCopy(setup.player_uuid);

  return data;
}

static void FreeThreadData_ApiRenamePlayer(void *data_raw)
{
  struct ApiRenamePlayerThreadData *data = data_raw;

  checked_free(data->player_name);
  checked_free(data->player_uuid);
  checked_free(data);
}

static boolean SetRequest_ApiRenamePlayer(struct HttpRequest *request,
					  void *data_raw)
{
  struct ApiRenamePlayerThreadData *data = data_raw;
  char *player_name_raw = data->player_name;
  char *player_uuid_raw = data->player_uuid;

  request->hostname = setup.api_server_hostname;
  request->port     = API_SERVER_PORT;
  request->method   = API_SERVER_METHOD;
  request->uri      = API_SERVER_URI_RENAME;

  char *player_name = getEscapedJSON(player_name_raw);
  char *player_uuid = getEscapedJSON(player_uuid_raw);

  snprintf(request->body, MAX_HTTP_BODY_SIZE,
	   "{\n"
	   "%s"
	   "  \"game_version\":         \"%s\",\n"
	   "  \"game_platform\":        \"%s\",\n"
	   "  \"name\":                 \"%s\",\n"
	   "  \"uuid\":                 \"%s\"\n"
	   "}\n",
	   getPasswordJSON(setup.api_server_password),
	   getProgramRealVersionString(),
	   getProgramPlatformString(),
	   player_name,
	   player_uuid);

  checked_free(player_name);
  checked_free(player_uuid);

  ConvertHttpRequestBodyToServerEncoding(request);

  return TRUE;
}

static void HandleResponse_ApiRenamePlayer(struct HttpResponse *response,
					   void *data_raw)
{
  // nothing to do here
}

#if defined(PLATFORM_EMSCRIPTEN)
static void Emscripten_ApiRenamePlayer_Loaded(unsigned handle, void *data_raw,
					      void *buffer, unsigned int size)
{
  struct HttpResponse *response = GetHttpResponseFromBuffer(buffer, size);

  if (response != NULL)
  {
    HandleResponse_ApiRenamePlayer(response, data_raw);

    checked_free(response);
  }
  else
  {
    Error("server response too large to handle (%d bytes)", size);
  }

  FreeThreadData_ApiRenamePlayer(data_raw);
}

static void Emscripten_ApiRenamePlayer_Failed(unsigned handle, void *data_raw,
					      int code, const char *status)
{
  Error("server failed to handle request: %d %s", code, status);

  FreeThreadData_ApiRenamePlayer(data_raw);
}

static void Emscripten_ApiRenamePlayer_Progress(unsigned handle, void *data_raw,
						int bytes, int size)
{
  // nothing to do here
}

static void Emscripten_ApiRenamePlayer_HttpRequest(struct HttpRequest *request,
						   void *data_raw)
{
  if (!SetRequest_ApiRenamePlayer(request, data_raw))
  {
    FreeThreadData_ApiRenamePlayer(data_raw);

    return;
  }

  emscripten_async_wget2_data(request->uri,
			      request->method,
			      request->body,
			      data_raw,
			      TRUE,
			      Emscripten_ApiRenamePlayer_Loaded,
			      Emscripten_ApiRenamePlayer_Failed,
			      Emscripten_ApiRenamePlayer_Progress);
}

#else

static void ApiRenamePlayer_HttpRequestExt(struct HttpRequest *request,
					   struct HttpResponse *response,
					   void *data_raw)
{
  if (!SetRequest_ApiRenamePlayer(request, data_raw))
    return;

  if (!DoHttpRequest(request, response))
  {
    Error("HTTP request failed: %s", GetHttpError());

    return;
  }

  if (!HTTP_SUCCESS(response->status_code))
  {
    Error("server failed to handle request: %d %s",
	  response->status_code,
	  response->status_text);

    return;
  }

  HandleResponse_ApiRenamePlayer(response, data_raw);
}

static void ApiRenamePlayer_HttpRequest(struct HttpRequest *request,
				    struct HttpResponse *response,
				    void *data_raw)
{
  ApiRenamePlayer_HttpRequestExt(request, response, data_raw);

  FreeThreadData_ApiRenamePlayer(data_raw);
}
#endif

static int ApiRenamePlayerThread(void *data_raw)
{
  struct HttpRequest *request = checked_calloc(sizeof(struct HttpRequest));
  struct HttpResponse *response = checked_calloc(sizeof(struct HttpResponse));

  program.api_thread_count++;

#if defined(PLATFORM_EMSCRIPTEN)
  Emscripten_ApiRenamePlayer_HttpRequest(request, data_raw);
#else
  ApiRenamePlayer_HttpRequest(request, response, data_raw);
#endif

  program.api_thread_count--;

  checked_free(request);
  checked_free(response);

  return 0;
}

void ApiRenamePlayerAsThread(void)
{
  struct ApiRenamePlayerThreadData *data = CreateThreadData_ApiRenamePlayer();

  ExecuteAsThread(ApiRenamePlayerThread,
		  "ApiRenamePlayer", data,
		  "rename player on server");
}


// ============================================================================
// reset player UUID API functions
// ============================================================================

struct ApiResetUUIDThreadData
{
  char *player_name;
  char *player_uuid_old;
  char *player_uuid_new;
};

static void *CreateThreadData_ApiResetUUID(char *uuid_new)
{
  struct ApiResetUUIDThreadData *data =
    checked_malloc(sizeof(struct ApiResetUUIDThreadData));

  data->player_name     = getStringCopy(setup.player_name);
  data->player_uuid_old = getStringCopy(setup.player_uuid);
  data->player_uuid_new = getStringCopy(uuid_new);

  return data;
}

static void FreeThreadData_ApiResetUUID(void *data_raw)
{
  struct ApiResetUUIDThreadData *data = data_raw;

  checked_free(data->player_name);
  checked_free(data->player_uuid_old);
  checked_free(data->player_uuid_new);
  checked_free(data);
}

static boolean SetRequest_ApiResetUUID(struct HttpRequest *request,
				       void *data_raw)
{
  struct ApiResetUUIDThreadData *data = data_raw;
  char *player_name_raw = data->player_name;
  char *player_uuid_old_raw = data->player_uuid_old;
  char *player_uuid_new_raw = data->player_uuid_new;

  request->hostname = setup.api_server_hostname;
  request->port     = API_SERVER_PORT;
  request->method   = API_SERVER_METHOD;
  request->uri      = API_SERVER_URI_RESETUUID;

  char *player_name = getEscapedJSON(player_name_raw);
  char *player_uuid_old = getEscapedJSON(player_uuid_old_raw);
  char *player_uuid_new = getEscapedJSON(player_uuid_new_raw);

  snprintf(request->body, MAX_HTTP_BODY_SIZE,
	   "{\n"
	   "%s"
	   "  \"game_version\":         \"%s\",\n"
	   "  \"game_platform\":        \"%s\",\n"
	   "  \"name\":                 \"%s\",\n"
	   "  \"uuid_old\":             \"%s\",\n"
	   "  \"uuid_new\":             \"%s\"\n"
	   "}\n",
	   getPasswordJSON(setup.api_server_password),
	   getProgramRealVersionString(),
	   getProgramPlatformString(),
	   player_name,
	   player_uuid_old,
	   player_uuid_new);

  checked_free(player_name);
  checked_free(player_uuid_old);
  checked_free(player_uuid_new);

  ConvertHttpRequestBodyToServerEncoding(request);

  return TRUE;
}

static void HandleResponse_ApiResetUUID(struct HttpResponse *response,
					void *data_raw)
{
  struct ApiResetUUIDThreadData *data = data_raw;

  // upgrade player UUID in server setup file
  setup.player_uuid = getStringCopy(data->player_uuid_new);
  setup.player_version = 2;

  SaveSetup_ServerSetup();
}

#if defined(PLATFORM_EMSCRIPTEN)
static void Emscripten_ApiResetUUID_Loaded(unsigned handle, void *data_raw,
					   void *buffer, unsigned int size)
{
  struct HttpResponse *response = GetHttpResponseFromBuffer(buffer, size);

  if (response != NULL)
  {
    HandleResponse_ApiResetUUID(response, data_raw);

    checked_free(response);
  }
  else
  {
    Error("server response too large to handle (%d bytes)", size);
  }

  FreeThreadData_ApiResetUUID(data_raw);
}

static void Emscripten_ApiResetUUID_Failed(unsigned handle, void *data_raw,
					   int code, const char *status)
{
  Error("server failed to handle request: %d %s", code, status);

  FreeThreadData_ApiResetUUID(data_raw);
}

static void Emscripten_ApiResetUUID_Progress(unsigned handle, void *data_raw,
					     int bytes, int size)
{
  // nothing to do here
}

static void Emscripten_ApiResetUUID_HttpRequest(struct HttpRequest *request,
						void *data_raw)
{
  if (!SetRequest_ApiResetUUID(request, data_raw))
  {
    FreeThreadData_ApiResetUUID(data_raw);

    return;
  }

  emscripten_async_wget2_data(request->uri,
			      request->method,
			      request->body,
			      data_raw,
			      TRUE,
			      Emscripten_ApiResetUUID_Loaded,
			      Emscripten_ApiResetUUID_Failed,
			      Emscripten_ApiResetUUID_Progress);
}

#else

static void ApiResetUUID_HttpRequestExt(struct HttpRequest *request,
					struct HttpResponse *response,
					void *data_raw)
{
  if (!SetRequest_ApiResetUUID(request, data_raw))
    return;

  if (!DoHttpRequest(request, response))
  {
    Error("HTTP request failed: %s", GetHttpError());

    return;
  }

  if (!HTTP_SUCCESS(response->status_code))
  {
    Error("server failed to handle request: %d %s",
	  response->status_code,
	  response->status_text);

    return;
  }

  HandleResponse_ApiResetUUID(response, data_raw);
}

static void ApiResetUUID_HttpRequest(struct HttpRequest *request,
				     struct HttpResponse *response,
				     void *data_raw)
{
  ApiResetUUID_HttpRequestExt(request, response, data_raw);

  FreeThreadData_ApiResetUUID(data_raw);
}
#endif

static int ApiResetUUIDThread(void *data_raw)
{
  struct HttpRequest *request = checked_calloc(sizeof(struct HttpRequest));
  struct HttpResponse *response = checked_calloc(sizeof(struct HttpResponse));

  program.api_thread_count++;

#if defined(PLATFORM_EMSCRIPTEN)
  Emscripten_ApiResetUUID_HttpRequest(request, data_raw);
#else
  ApiResetUUID_HttpRequest(request, response, data_raw);
#endif

  program.api_thread_count--;

  checked_free(request);
  checked_free(response);

  return 0;
}

void ApiResetUUIDAsThread(char *uuid_new)
{
  struct ApiResetUUIDThreadData *data = CreateThreadData_ApiResetUUID(uuid_new);

  ExecuteAsThread(ApiResetUUIDThread,
		  "ApiResetUUID", data,
		  "reset UUID on server");
}
