// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// misc.c
// ============================================================================

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "platform.h"

#if defined(PLATFORM_UNIX)
#include <pwd.h>
#include <sys/param.h>
#endif

#include "misc.h"
#include "setup.h"
#include "random.h"
#include "text.h"
#include "image.h"


// ============================================================================
// some generic helper functions
// ============================================================================

// ----------------------------------------------------------------------------
// logging functions
// ----------------------------------------------------------------------------

#define DUPLICATE_LOGGING_TO_STDOUT		TRUE


#if defined(PLATFORM_ANDROID)
static int android_log_prio = ANDROID_LOG_INFO;
static char *android_log_buffer = NULL;

static void append_to_android_log_buffer(char *format, va_list ap)
{
  char text_new[MAX_OUTPUT_LINESIZE];

  // print text to temporary string
  vsnprintf(text_new, MAX_OUTPUT_LINESIZE, format, ap);

  if (android_log_buffer == NULL)
  {
    android_log_buffer = getStringCopy(text_new);
  }
  else
  {
    char *android_log_buffer_old = android_log_buffer;

    // append new text to existing text
    android_log_buffer = getStringCat2(android_log_buffer, text_new);

    checked_free(android_log_buffer_old);
  }
}

static void vprintf_log_nonewline(char *format, va_list ap)
{
  // add log output to buffer until text with newline is printed
  append_to_android_log_buffer(format, ap);
}

static void vprintf_log(char *format, va_list ap)
{
  // add log output to buffer
  append_to_android_log_buffer(format, ap);

  // __android_log_vprint(android_log_prio, program.program_title, format, ap);
  __android_log_write(android_log_prio, program.program_title,
		      android_log_buffer);

  checked_free(android_log_buffer);
  android_log_buffer = NULL;
}

#else

static void vprintf_log_nonewline(char *format, va_list ap)
{
  FILE *file = program.log_file;

#if DUPLICATE_LOGGING_TO_STDOUT
  if (file != program.log_file_default)
  {
    va_list ap2;
    va_copy(ap2, ap);

    vfprintf(program.log_file_default, format, ap2);

    va_end(ap2);
  }
#endif

  vfprintf(file, format, ap);
}

static void vprintf_log(char *format, va_list ap)
{
  FILE *file = program.log_file;
  char *newline = STRING_NEWLINE;

#if DUPLICATE_LOGGING_TO_STDOUT
  if (file != program.log_file_default)
  {
    va_list ap2;
    va_copy(ap2, ap);

    vfprintf(program.log_file_default, format, ap2);
    fprintf(program.log_file_default, "%s", newline);

    va_end(ap2);
  }
#endif

  vfprintf(file, format, ap);
  fprintf(file, "%s", newline);
}
#endif

static void printf_log_nonewline(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vprintf_log_nonewline(format, ap);
  va_end(ap);
}

static void printf_log(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vprintf_log(format, ap);
  va_end(ap);
}

static void printf_log_line(char *line_chars, int line_length)
{
  int i;

  for (i = 0; i < line_length; i++)
    printf_log_nonewline("%s", line_chars);

  printf_log("");
}


// ----------------------------------------------------------------------------
// platform independent wrappers for printf() et al.
// ----------------------------------------------------------------------------

void fprintf_line(FILE *file, char *line_chars, int line_length)
{
  int i;

  for (i = 0; i < line_length; i++)
    fprintf(file, "%s", line_chars);

  fprintf(file, "\n");
}

void fprintf_line_with_prefix(FILE *file, char *prefix, char *line_chars,
			      int line_length)
{
  fprintf(file, "%s", prefix);
  fprintf_line(file, line_chars, line_length);
}

void printf_line(char *line_chars, int line_length)
{
  fprintf_line(stdout, line_chars, line_length);
}

void printf_line_with_prefix(char *prefix, char *line_chars, int line_length)
{
  fprintf_line_with_prefix(stdout, prefix, line_chars, line_length);
}

static void vPrint(char *format, va_list ap)
{
  FILE *file = program.log_file;

#if DUPLICATE_LOGGING_TO_STDOUT
  if (file != program.log_file_default)
  {
    va_list ap2;
    va_copy(ap2, ap);

    vfprintf(program.log_file_default, format, ap2);

    va_end(ap2);
  }
#endif

  vfprintf(file, format, ap);
}

void Print(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vPrint(format, ap);
  va_end(ap);
}

void PrintNoLog(char *format, ...)
{
  FILE *file = program.log_file_default;
  va_list ap;

  va_start(ap, format);
  vfprintf(file, format, ap);
  va_end(ap);

  fflush(file);
}

void PrintLine(char *line_chars, int line_length)
{
  int i;

  for (i = 0; i < line_length; i++)
    Print(line_chars);

  Print("\n");
}

void PrintLineWithPrefix(char *prefix, char *line_chars, int line_length)
{
  Print(prefix);
  PrintLine(line_chars, line_length);
}


// ----------------------------------------------------------------------------
// generic logging and error handling functions
// ----------------------------------------------------------------------------

enum log_levels
{
  LOG_UNKNOWN = 0,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
};

static char *log_tokens[] =
{
  "UNKNOWN",
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};

static void printf_log_prefix(int log_level, char *mode)
{
  if (log_level < 0 || log_level > LOG_FATAL)
    return;

  char *log_token = log_tokens[log_level];

  if (log_level == LOG_DEBUG)
    printf_log_nonewline("[%s] [%s] ", log_token, mode);
  else
    printf_log_nonewline("[%s] ", log_token);
}

static void vLog(int log_level, char *mode, char *format, va_list ap)
{
  if (log_level < 0 || log_level > LOG_FATAL)
    return;

  if (log_level == LOG_DEBUG)
  {
    // only show debug messages when in debug mode
    if (!options.debug)
      return;

    // if optional debug mode specified, limit debug output accordingly
    if (options.debug_mode != NULL &&
	strstr(mode, options.debug_mode) == NULL)
      return;
  }
  else if (log_level == LOG_WARN)
  {
    // only show warning messages when in verbose mode
    if (!options.verbose)
      return;
  }

#if defined(PLATFORM_ANDROID)
  android_log_prio = (log_level == LOG_DEBUG ? ANDROID_LOG_DEBUG :
		      log_level == LOG_INFO  ? ANDROID_LOG_INFO :
		      log_level == LOG_WARN  ? ANDROID_LOG_WARN :
		      log_level == LOG_ERROR ? ANDROID_LOG_ERROR :
		      log_level == LOG_FATAL ? ANDROID_LOG_FATAL :
		      ANDROID_LOG_UNKNOWN);
#endif

  static boolean last_line_was_separator = FALSE;
  char *log_token = log_tokens[log_level];

  if (strEqual(format, "===") ||
      strEqual(format, "---"))
  {
    static char *mode_last = NULL;
    char line_char[2] = { format[0], '\0' };
    int line_length = 80 - strlen(log_token) - 3;

    if (log_level == LOG_DEBUG)
      line_length -= strlen(mode) + 3;

    if (last_line_was_separator && strEqual(mode, mode_last))
      return;

    printf_log_prefix(log_level, mode);
    printf_log_line(line_char, line_length);

    if (!strEqual(mode, mode_last))
      setString(&mode_last, mode);

    last_line_was_separator = TRUE;

    return;
  }

  last_line_was_separator = FALSE;

  printf_log_prefix(log_level, mode);

  vprintf_log(format, ap);
}

static void Log(int log_level, char *mode, char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(log_level, mode, format, ap);
  va_end(ap);
}

void DebugContinued(char *mode, char *format, ...)
{
  static char message[MAX_LINE_LEN] = { 0 };

  // initialize message (optional)
  if (strEqual(format, ""))
  {
    message[0] = '\0';

    return;
  }

  char *message_ptr = message + strlen(message);
  int size_left = MAX_LINE_LEN - strlen(message);
  va_list ap;

  // append message
  va_start(ap, format);
  vsnprintf(message_ptr, size_left, format, ap);
  va_end(ap);

  // finalize message
  if (strSuffix(format, "\n"))
  {
    message[strlen(message) - 1] = '\0';

    Debug(mode, message);

    message[0] = '\0';
  }
}

void Debug(char *mode, char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(LOG_DEBUG, mode, format, ap);
  va_end(ap);
}

void Info(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(LOG_INFO, NULL, format, ap);
  va_end(ap);
}

void Warn(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(LOG_WARN, NULL, format, ap);
  va_end(ap);
}

void Error(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(LOG_ERROR, NULL, format, ap);
  va_end(ap);
}

void Fail(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(LOG_FATAL, NULL, format, ap);
  va_end(ap);

  if (!network.is_server_thread)
  {
    va_start(ap, format);
    program.exit_message_function(format, ap);
    va_end(ap);
  }

  Log(LOG_FATAL, NULL, "aborting");

  // network server thread: normal exit
  if (network.is_server_thread)
    exit(1);

  // main process: clean up stuff
  program.exit_function(1);
}

void FailWithHelp(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vLog(LOG_FATAL, NULL, format, ap);
  va_end(ap);

  Log(LOG_FATAL, NULL, "try option '--help' for more information");
  Log(LOG_FATAL, NULL, "aborting");

  // main process: clean up stuff
  program.exit_function(1);
}


// ----------------------------------------------------------------------------
// string functions
// ----------------------------------------------------------------------------

/* int2str() returns a number converted to a string;
   the used memory is static, but will be overwritten by later calls,
   so if you want to save the result, copy it to a private string buffer;
   there can be 10 local calls of int2str() without buffering the result --
   the 11th call will then destroy the result from the first call and so on. */

char *int2str(int number, int size)
{
  static char shift_array[10][40];
  static int shift_counter = 0;
  char *s = shift_array[shift_counter];

  shift_counter = (shift_counter + 1) % 10;

  if (size > 20)
    size = 20;

  if (size > 0)
  {
    sprintf(s, "                    %09d", number);
    return &s[strlen(s) - size];
  }
  else
  {
    sprintf(s, "%d", number);
    return s;
  }
}


// something similar to "int2str()" above, but allocates its own memory
// and has a different interface; we cannot use "itoa()", because this
// seems to be already defined when cross-compiling to the win32 target

char *i_to_a(unsigned int i)
{
  static char *a = NULL;

  checked_free(a);

  if (i > 2147483647)	// yes, this is a kludge
    i = 2147483647;

  a = checked_malloc(10 + 1);

  sprintf(a, "%d", i);

  return a;
}


// calculate base-2 logarithm of argument (rounded down to integer;
// this function returns the number of the highest bit set in argument)

int log_2(unsigned int x)
{
  int e = 0;

  while ((1 << e) < x)
  {
    x -= (1 << e);	// for rounding down (rounding up: remove this line)
    e++;
  }

  return e;
}

boolean getTokenValueFromString(char *string, char **token, char **value)
{
  return getTokenValueFromSetupLine(string, token, value);
}


// ----------------------------------------------------------------------------
// UUID functions
// ----------------------------------------------------------------------------

#define UUID_BYTES		16
#define UUID_CHARS		(UUID_BYTES * 2)
#define UUID_LENGTH		(UUID_CHARS + 4)

static unsigned int uuid_random_function(int max)
{
  return GetBetterRandom(max);
}

char *getUUIDExt(unsigned int (*random_function)(int max))
{
  static char uuid[UUID_LENGTH + 1];
  int data[UUID_BYTES];
  int count = 0;
  int i;

  for (i = 0; i < UUID_BYTES; i++)
    data[i] = random_function(256);

  data[6] = 0x40 | (data[6] & 0x0f);
  data[8] = 0x80 | (data[8] & 0x3f);

  for (i = 0; i < UUID_BYTES; i++)
  {
    sprintf(&uuid[count], "%02x", data[i]);
    count += 2;

    if (i == 3 || i == 5 || i == 7 || i == 9)
      strcat(&uuid[count++], "-");
  }

  return uuid;
}

char *getUUID(void)
{
  return getUUIDExt(uuid_random_function);
}


// ----------------------------------------------------------------------------
// counter functions
// ----------------------------------------------------------------------------

// maximal allowed length of a command line option
#define MAX_OPTION_LEN		256

static unsigned int getCurrentMS(void)
{
  return SDL_GetTicks();
}

static unsigned int mainCounter(int mode)
{
  static unsigned int base_ms = 0;
  unsigned int current_ms;

  // get current system milliseconds
  current_ms = getCurrentMS();

  // reset base timestamp in case of counter reset or wrap-around
  if (mode == INIT_COUNTER || current_ms < base_ms)
    base_ms = current_ms;

  // return milliseconds since last counter reset
  return current_ms - base_ms;
}

void InitCounter(void)		// set counter back to zero
{
  mainCounter(INIT_COUNTER);
}

unsigned int Counter(void)	// get milliseconds since last call of InitCounter()
{
  return mainCounter(READ_COUNTER);
}

static void sleep_milliseconds(unsigned int milliseconds_delay)
{
  SDL_Delay(milliseconds_delay);
}

void Delay(unsigned int delay)	// Sleep specified number of milliseconds
{
  sleep_milliseconds(delay);
}

boolean DelayReachedExt2(unsigned int *counter_var, unsigned int delay,
			 unsigned int actual_counter)
{
  if (actual_counter >= *counter_var &&
      actual_counter < *counter_var + delay)
    return FALSE;

  *counter_var = actual_counter;

  return TRUE;
}

boolean DelayReachedExt(DelayCounter *counter, unsigned int actual_counter)
{
  return DelayReachedExt2(&counter->count, counter->value, actual_counter);
}

boolean FrameReached(DelayCounter *counter)
{
  return DelayReachedExt(counter, FrameCounter);
}

boolean DelayReached(DelayCounter *counter)
{
  return DelayReachedExt(counter, Counter());
}

void ResetDelayCounterExt(DelayCounter *counter, unsigned int actual_counter)
{
  DelayReachedExt2(&counter->count, 0, actual_counter);
}

void ResetFrameCounter(DelayCounter *counter)
{
  ResetDelayCounterExt(counter, FrameCounter);
}

void ResetDelayCounter(DelayCounter *counter)
{
  ResetDelayCounterExt(counter, Counter());
}

int WaitUntilDelayReached(DelayCounter *counter)
{
  unsigned int *counter_var = &counter->count;
  unsigned int delay = counter->value;
  unsigned int actual_counter;
  int skip_frames = 0;

  while (1)
  {
    actual_counter = Counter();

    if (actual_counter >= *counter_var &&
	actual_counter < *counter_var + delay)
      sleep_milliseconds((*counter_var + delay - actual_counter) / 2);
    else
      break;
  }

  if (*counter_var != 0 &&
      delay != 0 &&
      actual_counter >= *counter_var + delay)
  {
    int lag = actual_counter - (*counter_var + delay);
    int delay2 = (delay + 1) / 2;

    if (lag >= delay2)
      skip_frames = (lag + delay2) / delay;
  }

  *counter_var = actual_counter;

  return skip_frames;
}

void SkipUntilDelayReached(DelayCounter *counter,
			   int *loop_var, int last_loop_value)
{
  int skip_frames = WaitUntilDelayReached(counter);

#if 0
#if DEBUG
  if (skip_frames)
    Debug("internal:SkipUntilDelayReached",
	  "%d: %d ms -> SKIP %d FRAME(S) [%d ms]",
	  *loop_var, counter->value,
	  skip_frames, skip_frames * counter->value);
  else
    Debug("internal:SkipUntilDelayReached",
	  "%d: %d ms",
	  *loop_var, counter->value);
#endif
#endif

  if (skip_frames == 0)
    return;

  // when skipping frames, make sure to never skip the last frame, as
  // this may be needed for animations to reach a defined end state;
  // furthermore, we assume that this function is called at the end
  // of a "for" loop, which continues by incrementing the loop variable
  // by one before checking the loop condition again; therefore we have
  // to check against the last loop value minus one here

  last_loop_value--;

  if (*loop_var < last_loop_value)	// never skip the last frame
  {
    *loop_var += skip_frames;

    if (*loop_var > last_loop_value)	// never skip the last frame
      *loop_var = last_loop_value;
  }
}


// ----------------------------------------------------------------------------
// random generator functions
// ----------------------------------------------------------------------------

static unsigned int init_random_number_ext(int nr, int seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    // default random seed
    seed = (int)time(NULL);			// seconds since the epoch

#if !defined(PLATFORM_WINDOWS)
    // add some more randomness
    struct timeval current_time;

    gettimeofday(&current_time, NULL);

    seed += (int)current_time.tv_usec;		// microseconds since the epoch
#endif

    // add some more randomness
    seed += (int)SDL_GetTicks();		// milliseconds since SDL init

    // add some more randomness
    seed += GetSimpleRandom(1000000);
  }

  srandom_linux_libc(nr, (unsigned int) seed);

  return (unsigned int) seed;
}

static unsigned int prng_seed_gettimeofday(void)
{
  struct timeval current_time;

  gettimeofday(&current_time, NULL);

  prng_seed_bytes(&current_time, sizeof(current_time));

  return 0;
}

unsigned int init_random_number(int nr, int seed)
{
  return (nr == RANDOM_BETTER ? prng_seed_gettimeofday() :
	  init_random_number_ext(nr, seed));
}

static unsigned int get_random_number_ext(int nr)
{
  return (nr == RANDOM_BETTER ? prng_get_uint() :
	  random_linux_libc(nr));
}

unsigned int get_random_number(int nr, int max)
{
  return (max > 0 ? get_random_number_ext(nr) % max : 0);
}


// ----------------------------------------------------------------------------
// system info functions
// ----------------------------------------------------------------------------

#if !defined(PLATFORM_ANDROID)
static char *get_corrected_real_name(char *real_name)
{
  char *real_name_new = checked_malloc(MAX_USERNAME_LEN + 1);
  char *from_ptr = real_name;
  char *to_ptr   = real_name_new;

  // copy the name string, but not more than MAX_USERNAME_LEN characters
  while (*from_ptr && (int)(to_ptr - real_name_new) < MAX_USERNAME_LEN - 1)
  {
    // the name field read from "passwd" file may also contain additional
    // user information, separated by commas, which will be removed here
    if (*from_ptr == ',')
      break;

    // the user's real name may contain 'german sharp s' characters,
    // which have no equivalent in upper case letters (used by our fonts)
    if (*from_ptr == CHAR_BYTE_SHARP_S)
    {
      from_ptr++;
      *to_ptr++ = 's';
      *to_ptr++ = 's';
    }
    else
      *to_ptr++ = *from_ptr++;
  }

  *to_ptr = '\0';

  return real_name_new;
}
#endif

#if defined(PLATFORM_UNIX)
static struct passwd *getPasswdEntry(void)
{
#if defined(PLATFORM_EMSCRIPTEN)
  // currently not fully supported; force fallback to default values
  return NULL;
#else
  return getpwuid(getuid());
#endif
}

char *getUnixLoginName(void)
{
  struct passwd *pwd = getPasswdEntry();

  if (pwd != NULL && strlen(pwd->pw_name) != 0)
    return pwd->pw_name;

  return NULL;
}

char *getUnixRealName(void)
{
  struct passwd *pwd = getPasswdEntry();

  if (pwd != NULL && strlen(pwd->pw_gecos) != 0)
    return pwd->pw_gecos;

  return NULL;
}

char *getUnixHomeDir(void)
{
  struct passwd *pwd = getPasswdEntry();

  if (pwd != NULL && strlen(pwd->pw_dir) != 0)
    return pwd->pw_dir;

  return NULL;
}
#endif

char *getLoginName(void)
{
  static char *login_name = NULL;

#if defined(PLATFORM_WINDOWS)
  if (login_name == NULL)
  {
    unsigned long buffer_size = MAX_USERNAME_LEN + 1;

    login_name = checked_malloc(buffer_size);

    if (GetUserName(login_name, &buffer_size) == 0)
      strcpy(login_name, ANONYMOUS_NAME);
  }
#elif defined(PLATFORM_UNIX) && !defined(PLATFORM_ANDROID)
  if (login_name == NULL)
  {
    char *name = getUnixLoginName();

    if (name != NULL)
      login_name = getStringCopy(name);
    else
      login_name = ANONYMOUS_NAME;
  }
#else
  login_name = ANONYMOUS_NAME;
#endif

  return login_name;
}

char *getRealName(void)
{
  static char *real_name = NULL;

#if defined(PLATFORM_WINDOWS)
  if (real_name == NULL)
  {
    static char buffer[MAX_USERNAME_LEN + 1];
    unsigned long buffer_size = MAX_USERNAME_LEN + 1;

    if (GetUserName(buffer, &buffer_size) != 0)
      real_name = get_corrected_real_name(buffer);
    else
      real_name = ANONYMOUS_NAME;
  }
#elif defined(PLATFORM_UNIX) && !defined(PLATFORM_ANDROID)
  if (real_name == NULL)
  {
    char *name = getUnixRealName();

    if (name != NULL)
      real_name = get_corrected_real_name(name);
    else
      real_name = ANONYMOUS_NAME;
  }
#else
  real_name = ANONYMOUS_NAME;
#endif

  return real_name;
}

char *getFixedUserName(char *name)
{
  // needed because player name must be a fixed length string
  char *name_new = checked_malloc(MAX_PLAYER_NAME_LEN + 1);

  strncpy(name_new, name, MAX_PLAYER_NAME_LEN);
  name_new[MAX_PLAYER_NAME_LEN] = '\0';

  if (strlen(name) > MAX_PLAYER_NAME_LEN)		// name has been cut
    if (strchr(name_new, ' '))
      *strchr(name_new, ' ') = '\0';

  return name_new;
}

char *getDefaultUserName(int nr)
{
  static char *user_name[MAX_PLAYER_NAMES] = { NULL };

  nr = MIN(MAX(0, nr), MAX_PLAYER_NAMES - 1);

  if (user_name[nr] == NULL)
  {
    user_name[nr] = (nr == 0 ? getLoginName() : EMPTY_PLAYER_NAME);
    user_name[nr] = getFixedUserName(user_name[nr]);
  }

  return user_name[nr];
}

char *getTimestampFromEpoch(time_t epoch_seconds)
{
  struct tm *now = localtime(&epoch_seconds);
  static char timestamp[100];

  sprintf(timestamp, "%04d%02d%02d-%02d%02d%02d",
	  now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
	  now->tm_hour, now->tm_min, now->tm_sec);

  return timestamp;
}

char *getCurrentTimestamp(void)
{
  return getTimestampFromEpoch(time(NULL));
}

time_t getFileTimestampEpochSeconds(char *filename)
{
  struct stat file_status;

  if (stat(filename, &file_status) != 0)	// cannot stat file
    return 0;

  return file_status.st_mtime;
}


// ----------------------------------------------------------------------------
// path manipulation functions
// ----------------------------------------------------------------------------

static char *getLastPathSeparatorPtr(char *filename)
{
  char *last_separator = strrchr(filename, CHAR_PATH_SEPARATOR_UNIX);

  if (last_separator == NULL)	// also try DOS/Windows variant
    last_separator = strrchr(filename, CHAR_PATH_SEPARATOR_DOS);

  return last_separator;
}

char *getBaseNamePtr(char *filename)
{
  char *last_separator = getLastPathSeparatorPtr(filename);

  if (last_separator != NULL)
    return last_separator + 1;	// separator found: strip base path
  else
    return filename;		// no separator found: filename has no path
}

char *getBaseName(char *filename)
{
  return getStringCopy(getBaseNamePtr(filename));
}

char *getBaseNameNoSuffix(char *filename)
{
  char *basename = getStringCopy(getBaseNamePtr(filename));

  // remove trailing suffix (separated by dot or hyphen)
  if (basename[0] != '.' && basename[0] != '-')
  {
    if (strchr(basename, '.') != NULL)
      *strchr(basename, '.') = '\0';

    if (strchr(basename, '-') != NULL)
      *strchr(basename, '-') = '\0';
  }

  return basename;
}

char *getBasePath(char *filename)
{
  char *basepath = getStringCopy(filename);
  char *last_separator = getLastPathSeparatorPtr(basepath);

  // if no separator was found, use current directory
  if (last_separator == NULL)
  {
    free(basepath);

    return getStringCopy(".");
  }

  // separator found: strip basename
  *last_separator = '\0';

  return basepath;
}


// ----------------------------------------------------------------------------
// various string functions
// ----------------------------------------------------------------------------

char *getStringCat2WithSeparator(const char *s1,
				 const char *s2,
				 const char *sep)
{
  if (s1 == NULL || s2 == NULL || sep == NULL)
    return NULL;

  char *complete_string = checked_malloc(strlen(s1) + strlen(sep) +
					 strlen(s2) + 1);

  sprintf(complete_string, "%s%s%s", s1, sep, s2);

  return complete_string;
}

char *getStringCat3WithSeparator(const char *s1,
				 const char *s2,
				 const char *s3,
				 const char *sep)
{
  if (s1 == NULL || s2 == NULL || s3 == NULL || sep == NULL)
    return NULL;

  char *complete_string = checked_malloc(strlen(s1) + strlen(sep) +
					 strlen(s2) + strlen(sep) +
					 strlen(s3) + 1);

  sprintf(complete_string, "%s%s%s%s%s", s1, sep, s2, sep, s3);

  return complete_string;
}

char *getStringCat2(const char *s1, const char *s2)
{
  return getStringCat2WithSeparator(s1, s2, "");
}

char *getStringCat3(const char *s1, const char *s2, const char *s3)
{
  return getStringCat3WithSeparator(s1, s2, s3, "");
}

char *getPath2(const char *path1, const char *path2)
{
#if defined(PLATFORM_ANDROID)
  // workaround for reading from assets directory -- skip "." subdirs in path
  if (strEqual(path1, "."))
    return getStringCopy(path2);
  else if (strEqual(path2, "."))
    return getStringCopy(path1);
#endif

  return getStringCat2WithSeparator(path1, path2, STRING_PATH_SEPARATOR);
}

char *getPath3(const char *path1, const char *path2, const char *path3)
{
#if defined(PLATFORM_ANDROID)
  // workaround for reading from assets directory -- skip "." subdirs in path
  if (strEqual(path1, "."))
    return getStringCat2WithSeparator(path2, path3, STRING_PATH_SEPARATOR);
  else if (strEqual(path2, "."))
    return getStringCat2WithSeparator(path1, path3, STRING_PATH_SEPARATOR);
  else if (strEqual(path3, "."))
    return getStringCat2WithSeparator(path1, path2, STRING_PATH_SEPARATOR);
#endif

  return getStringCat3WithSeparator(path1, path2, path3, STRING_PATH_SEPARATOR);
}

static char *getPngOrPcxIfNotExists(char *filename)
{
  // switch from PNG to PCX file and vice versa, if file does not exist
  // (backwards compatibility with PCX files used in previous versions)

  if (!fileExists(filename) && strSuffix(filename, ".png"))
    strcpy(&filename[strlen(filename) - 3], "pcx");
  else if (!fileExists(filename) && strSuffix(filename, ".pcx"))
    strcpy(&filename[strlen(filename) - 3], "png");

  return filename;
}

char *getImg2(const char *path1, const char *path2)
{
  return getPngOrPcxIfNotExists(getPath2(path1, path2));
}

char *getImg3(const char *path1, const char *path2, const char *path3)
{
  return getPngOrPcxIfNotExists(getPath3(path1, path2, path3));
}

char *getStringCopy(const char *s)
{
  char *s_copy;

  if (s == NULL)
    return NULL;

  s_copy = checked_malloc(strlen(s) + 1);
  strcpy(s_copy, s);

  return s_copy;
}

char *getStringCopyN(const char *s, int n)
{
  char *s_copy;
  int s_len = MAX(0, n);

  if (s == NULL)
    return NULL;

  s_copy = checked_malloc(s_len + 1);
  strncpy(s_copy, s, s_len);
  s_copy[s_len] = '\0';

  return s_copy;
}

char *getStringCopyNStatic(const char *s, int n)
{
  static char *s_copy = NULL;

  checked_free(s_copy);

  s_copy = getStringCopyN(s, n);

  return s_copy;
}

char *getStringToUpper(const char *s)
{
  if (s == NULL)
    return NULL;

  char *s_copy = checked_malloc(strlen(s) + 1);
  char *s_ptr = s_copy;

  while (*s)
    *s_ptr++ = toupper(*s++);
  *s_ptr = '\0';

  return s_copy;
}

char *getStringToLower(const char *s)
{
  if (s == NULL)
    return NULL;

  char *s_copy = checked_malloc(strlen(s) + 1);
  char *s_ptr = s_copy;

  while (*s)
    *s_ptr++ = tolower(*s++);
  *s_ptr = '\0';

  return s_copy;
}

static char *getStringVPrint(char *format, va_list ap)
{
  va_list ap_test;

  va_copy(ap_test, ap);

  // determine required size of string to be printed
  int size = vsnprintf(NULL, 0, format, ap_test);

  va_end(ap_test);

  // check if something went wrong (should not happen)
  if (size < 0)
    size = MAX_LINE_LEN;

  char *s = checked_malloc(size + 1);

  vsnprintf(s, size + 1, format, ap);

  return s;
}

char *getStringPrint(char *format, ...)
{
  va_list ap;
  char *s;

  va_start(ap, format);
  s = getStringVPrint(format, ap);
  va_end(ap);

  return s;
}

void setStringPrint(char **s, char *format, ...)
{
  va_list ap;

  checked_free(*s);

  va_start(ap, format);
  *s = getStringVPrint(format, ap);
  va_end(ap);
}

void appendStringPrint(char **s_old, char *format, ...)
{
  va_list ap;
  char *s_new;

  va_start(ap, format);
  s_new = getStringVPrint(format, ap);
  va_end(ap);

  char *s_combined = getStringCat2(*s_old, s_new);

  checked_free(*s_old);
  checked_free(s_new);

  *s_old = s_combined;
}

void setString(char **old_value, const char *new_value)
{
  checked_free(*old_value);

  *old_value = getStringCopy(new_value);
}

char **getSplitStringArray(const char *s, const char *separators, int max_tokens)
{
  const char *s_ptr, *s_last = s;
  byte separator_table[256] = { FALSE };
  int num_tokens;
  char **tokens = NULL;

  if (s == NULL)
    return NULL;

  if (separators == NULL)
    return NULL;

  if (max_tokens < 1)
    max_tokens = INT_MAX;

  // if string is empty, return empty array
  if (*s == '\0')
  {
    tokens = checked_malloc(sizeof(char *));
    tokens[0] = NULL;

    return tokens;
  }

  // initialize separator table for all characters in separators string
  for (s_ptr = separators; *s_ptr != '\0'; s_ptr++)
    separator_table[*(byte *)s_ptr] = TRUE;

  // count number of tokens in string
  for (num_tokens = 1, s_ptr = s; *s_ptr != '\0'; s_ptr++)
    if (separator_table[*(byte *)s_ptr] && num_tokens < max_tokens)
      num_tokens++;

  // allocate array for determined number of tokens
  tokens = checked_malloc((num_tokens + 1) * sizeof(char *));

  // copy all but last separated sub-strings to array
  for (num_tokens = 0, s_ptr = s; *s_ptr != '\0'; s_ptr++)
  {
    if (separator_table[*(byte *)s_ptr] && num_tokens + 1 < max_tokens)
    {
      tokens[num_tokens++] = getStringCopyN(s_last, s_ptr - s_last);
      s_last = s_ptr + 1;
    }
  }

  // copy last separated sub-string to array
  tokens[num_tokens++] = getStringCopyN(s_last, s_ptr - s_last);

  // terminate array
  tokens[num_tokens] = NULL;

  return tokens;
}

int getStringArrayLength(char **s_array)
{
  int num_strings = 0;

  if (s_array == NULL)
    return 0;

  while (s_array[num_strings] != NULL)
    num_strings++;

  return num_strings;
}

void freeStringArray(char **s_array)
{
  int i;

  if (s_array == NULL)
    return;

  for (i = 0; s_array[i] != NULL; i++)
    checked_free(s_array[i]);

  checked_free(s_array);
}

char *getEscapedString(const char *s)
{
  const unsigned char *s_ptr = (unsigned char *)s;
  char *s_escaped;
  char *s_escaped_ptr;

  if (s == NULL)
    return NULL;

  /* Each source byte needs maximally four target chars (\777) */
  s_escaped = checked_malloc(strlen(s) * 4 + 1);
  s_escaped_ptr = s_escaped;

  while (*s_ptr != '\0')
  {
    switch (*s_ptr)
    {
      case '\b':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = 'b';
	break;

      case '\f':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = 'f';
	break;

      case '\n':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = 'n';
	break;

      case '\r':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = 'r';
	break;

      case '\t':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = 't';
	break;

      case '\v':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = 'v';
	break;

      case '\\':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = '\\';
	break;

      case '"':
	*s_escaped_ptr++ = '\\';
	*s_escaped_ptr++ = '"';
	break;

      default:
	if ((*s_ptr < ' ') || (*s_ptr >= 0177))
	{
	  *s_escaped_ptr++ = '\\';
	  *s_escaped_ptr++ = '0' + (((*s_ptr) >> 6) & 07);
	  *s_escaped_ptr++ = '0' + (((*s_ptr) >> 3) & 07);
	  *s_escaped_ptr++ = '0' + ( (*s_ptr)       & 07);
	}
	else
	{
	  *s_escaped_ptr++ = *s_ptr;
	}
	break;
    }

    s_ptr++;
  }

  *s_escaped_ptr = '\0';

  return s_escaped;
}

char *getUnescapedString(const char *s)
{
  const char *s_ptr = s;
  const char *octal_ptr;
  char *s_unescaped;
  char *s_unescaped_ptr;

  if (s == NULL)
    return NULL;

  s_unescaped = checked_malloc(strlen(s) + 1);
  s_unescaped_ptr = s_unescaped;

  while (*s_ptr != '\0')
  {
    if (*s_ptr == '\\')
    {
      s_ptr++;

      switch (*s_ptr)
      {
	case '\0':
	  Warn("getUnescapedString: trailing \\");
	  goto out;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	  *s_unescaped_ptr = 0;
	  octal_ptr = s_ptr;

	  while (s_ptr < octal_ptr + 3 && *s_ptr >= '0' && *s_ptr <= '7')
	  {
	    *s_unescaped_ptr = (*s_unescaped_ptr * 8) + (*s_ptr - '0');
	    s_ptr++;
	  }

	  s_unescaped_ptr++;
	  s_ptr--;
	  break;

	case 'b':
	  *s_unescaped_ptr++ = '\b';
	  break;

	case 'f':
	  *s_unescaped_ptr++ = '\f';
	  break;

	case 'n':
	  *s_unescaped_ptr++ = '\n';
	  break;

	case 'r':
	  *s_unescaped_ptr++ = '\r';
	  break;

	case 't':
	  *s_unescaped_ptr++ = '\t';
	  break;

	case 'v':
	  *s_unescaped_ptr++ = '\v';
	  break;

	default:
	  /* also handles \" and \\ */
	  *s_unescaped_ptr++ = *s_ptr;
	  break;
      }
    }
    else
    {
      *s_unescaped_ptr++ = *s_ptr;
    }

    s_ptr++;
  }

 out:
  *s_unescaped_ptr = '\0';

  return s_unescaped;
}

char *chugString(char *s)
{
  if (s == NULL)
    return NULL;

  char *start;

  for (start = (char *)s; *start && isspace(*start); start++)
    ;

  memmove(s, start, strlen(start) + 1);

  return s;
}

char *chompString(char *s)
{
  if (s == NULL)
    return NULL;

  int len = strlen(s);

  while (len--)
  {
    if (isspace(s[len]))
      s[len] = '\0';
    else
      break;
  }

  return s;
}

char *stripString(char *s)
{
  return chugString(chompString(s));
}

boolean strEqual(const char *s1, const char *s2)
{
  return (s1 == NULL && s2 == NULL ? TRUE  :
	  s1 == NULL && s2 != NULL ? FALSE :
	  s1 != NULL && s2 == NULL ? FALSE :
	  strcmp(s1, s2) == 0);
}

boolean strEqualN(const char *s1, const char *s2, int n)
{
  return (s1 == NULL && s2 == NULL ? TRUE  :
	  s1 == NULL && s2 != NULL ? FALSE :
	  s1 != NULL && s2 == NULL ? FALSE :
	  strncmp(s1, s2, n) == 0);
}

boolean strEqualCase(const char *s1, const char *s2)
{
  return (s1 == NULL && s2 == NULL ? TRUE  :
	  s1 == NULL && s2 != NULL ? FALSE :
	  s1 != NULL && s2 == NULL ? FALSE :
	  strcasecmp(s1, s2) == 0);
}

boolean strEqualCaseN(const char *s1, const char *s2, int n)
{
  return (s1 == NULL && s2 == NULL ? TRUE  :
	  s1 == NULL && s2 != NULL ? FALSE :
	  s1 != NULL && s2 == NULL ? FALSE :
	  strncasecmp(s1, s2, n) == 0);
}

boolean strPrefix(const char *s, const char *prefix)
{
  return (s == NULL && prefix == NULL ? TRUE  :
	  s == NULL && prefix != NULL ? FALSE :
	  s != NULL && prefix == NULL ? FALSE :
	  strncmp(s, prefix, strlen(prefix)) == 0);
}

boolean strSuffix(const char *s, const char *suffix)
{
  return (s == NULL && suffix == NULL ? TRUE  :
	  s == NULL && suffix != NULL ? FALSE :
	  s != NULL && suffix == NULL ? FALSE :
	  strlen(s) < strlen(suffix)  ? FALSE :
	  strcmp(&s[strlen(s) - strlen(suffix)], suffix) == 0);
}

boolean strPrefixLower(const char *s, const char *prefix)
{
  char *s_lower = getStringToLower(s);
  boolean match = strPrefix(s_lower, prefix);

  free(s_lower);

  return match;
}

boolean strSuffixLower(const char *s, const char *suffix)
{
  char *s_lower = getStringToLower(s);
  boolean match = strSuffix(s_lower, suffix);

  free(s_lower);

  return match;
}

boolean isURL(const char *s)
{
  while (*s && *s >= 'a' && *s <= 'z')
    s++;

  return strPrefix(s, "://");
}


// ----------------------------------------------------------------------------
// command line option handling functions
// ----------------------------------------------------------------------------

void GetOptions(int argc, char *argv[],
		void (*print_usage_function)(void),
		void (*print_version_function)(void),
		void (*print_render_drivers_function)(void))
{
  char *base_path = getProgramMainDataPath(argv[0], BASE_PATH);
  char **argvplus = checked_calloc((argc + 1) * sizeof(char **));
  char **options_left = &argvplus[1];

  // replace original "argv" with null-terminated array of string pointers
  while (argc--)
    argvplus[argc] = argv[argc];

  // initialize global program options
  options.server_host = NULL;
  options.server_port = 0;

  options.base_directory = base_path;

  options.level_directory    = getPath2(base_path, LEVELS_DIRECTORY);
  options.graphics_directory = getPath2(base_path, GRAPHICS_DIRECTORY);
  options.sounds_directory   = getPath2(base_path, SOUNDS_DIRECTORY);
  options.music_directory    = getPath2(base_path, MUSIC_DIRECTORY);
  options.docs_directory     = getPath2(base_path, DOCS_DIRECTORY);
  options.conf_directory     = getPath2(base_path, CONF_DIRECTORY);

  options.execute_command = NULL;
  options.tape_log_filename = NULL;
  options.special_flags = NULL;
  options.debug_mode = NULL;
  options.player_name = NULL;
  options.identifier = NULL;
  options.level_nr = NULL;
  options.drop_file = NULL;

  options.display_nr = 0;

  options.mytapes = FALSE;
  options.serveronly = FALSE;
  options.network = FALSE;
  options.verbose = FALSE;
  options.debug = FALSE;

#if 1
  options.verbose = TRUE;
#else
#if !defined(PLATFORM_UNIX)
  if (*options_left == NULL)	// no options given -- enable verbose mode
    options.verbose = TRUE;
#endif
#endif

#if DEBUG
#if defined(PLATFORM_ANDROID)
  options.debug = TRUE;
#endif
#endif

  while (*options_left)
  {
    char option_str[MAX_OPTION_LEN];
    char *option = options_left[0];
    char *next_option = options_left[1];
    char *option_arg = NULL;
    int option_len = strlen(option);

    if (option_len >= MAX_OPTION_LEN)
      FailWithHelp("unrecognized option '%s'", option);

    strcpy(option_str, option);			// copy argument into buffer
    option = option_str;

    if (strEqual(option, "--"))			// stop scanning arguments
      break;

    if (strPrefix(option, "--"))		// treat '--' like '-'
      option++;

    option_arg = strchr(option, '=');
    if (option_arg == NULL)			// no '=' in option
      option_arg = next_option;
    else
    {
      *option_arg++ = '\0';			// cut argument from option
      if (*option_arg == '\0')			// no argument after '='
	FailWithHelp("option '%s' has invalid argument", option_str);
    }

    option_len = strlen(option);

    if (strEqual(option, "-"))
    {
      FailWithHelp("unrecognized option '%s'", option);
    }
    else if (strncmp(option, "-help", option_len) == 0)
    {
      print_usage_function();

      exit(0);
    }
    else if (strncmp(option, "-basepath", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.base_directory = base_path = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;

      // adjust paths for sub-directories in base directory accordingly
      options.level_directory    = getPath2(base_path, LEVELS_DIRECTORY);
      options.graphics_directory = getPath2(base_path, GRAPHICS_DIRECTORY);
      options.sounds_directory   = getPath2(base_path, SOUNDS_DIRECTORY);
      options.music_directory    = getPath2(base_path, MUSIC_DIRECTORY);
      options.docs_directory     = getPath2(base_path, DOCS_DIRECTORY);
      options.conf_directory     = getPath2(base_path, CONF_DIRECTORY);
    }
    else if (strncmp(option, "-levels", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.level_directory = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-graphics", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.graphics_directory = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-sounds", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.sounds_directory = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-music", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.music_directory = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-mytapes", option_len) == 0)
    {
      options.mytapes = TRUE;
    }
    else if (strncmp(option, "-network", option_len) == 0)
    {
      options.network = TRUE;
    }
    else if (strncmp(option, "-serveronly", option_len) == 0)
    {
      options.serveronly = TRUE;
    }
    else if (strncmp(option, "-debug", option_len) == 0)
    {
      options.debug = TRUE;

      // optionally, debug output can be limited to a specific debug mode
      if (option_arg != next_option)
	options.debug_mode = getStringCopy(option_arg);
    }
    else if (strncmp(option, "-player-name", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.player_name = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-identifier", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.identifier = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-level-nr", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.level_nr = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-drop-file", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.drop_file = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-verbose", option_len) == 0)
    {
      options.verbose = TRUE;
    }
    else if (strncmp(option, "-version", option_len) == 0 ||
	     strncmp(option, "-V", option_len) == 0)
    {
      print_version_function();

      exit(0);
    }
    else if (strncmp(option, "-list-render-drivers", option_len) == 0)
    {
      print_render_drivers_function();

      exit(0);
    }
    else if (strPrefix(option, "-D"))
    {
      options.special_flags = getStringCopy(&option[2]);
    }
    else if (strncmp(option, "-execute", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.execute_command = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;

      // when doing batch processing, always enable verbose mode (warnings)
      options.verbose = TRUE;
    }
    else if (strncmp(option, "-tape_logfile", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      options.tape_log_filename = getStringCopy(option_arg);
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-display", option_len) == 0)
    {
      if (option_arg == NULL)
	FailWithHelp("option '%s' requires an argument", option_str);

      if (option_arg == next_option)
	options_left++;

      int display_nr = atoi(option_arg);

#if 1
      // dirty hack: SDL_GetNumVideoDisplays() seems broken on some systems
      options.display_nr = display_nr;
#else
      options.display_nr =
	MAX(0, MIN(display_nr, SDL_GetNumVideoDisplays() - 1));

      if (display_nr != options.display_nr)
	Warn("invalid display %d -- using display %d",
	     display_nr, options.display_nr);
#endif
    }
#if defined(PLATFORM_MAC)
    else if (strPrefix(option, "-psn"))
    {
      // ignore process serial number when launched via GUI on Mac OS X
    }
#endif
    else if (*option == '-')
    {
      FailWithHelp("unrecognized option '%s'", option_str);
    }
    else if (options.server_host == NULL)
    {
      options.server_host = *options_left;
    }
    else if (options.server_port == 0)
    {
      options.server_port = atoi(*options_left);
      if (options.server_port < 1024)
	FailWithHelp("bad port number '%d'", options.server_port);
    }
    else
      FailWithHelp("too many arguments");

    options_left++;
  }
}


// ----------------------------------------------------------------------------
// checked memory allocation and freeing functions
// ----------------------------------------------------------------------------

void *checked_malloc(unsigned int size)
{
  void *ptr;

  ptr = malloc(size);

  if (ptr == NULL)
    Fail("cannot allocate %d bytes -- out of memory", size);

  return ptr;
}

void *checked_calloc(unsigned int size)
{
  void *ptr;

  ptr = calloc(1, size);

  if (ptr == NULL)
    Fail("cannot allocate %d bytes -- out of memory", size);

  return ptr;
}

void *checked_realloc(void *ptr, unsigned int size)
{
  ptr = realloc(ptr, size);

  if (ptr == NULL)
    Fail("cannot allocate %d bytes -- out of memory", size);

  return ptr;
}

void checked_free(void *ptr)
{
  if (ptr != NULL)	// this check should be done by free() anyway
    free(ptr);
}

void clear_mem(void *ptr, unsigned int size)
{
#if defined(PLATFORM_WINDOWS)
  // for unknown reason, memset() sometimes crashes when compiled with MinGW
  char *cptr = (char *)ptr;

  while (size--)
    *cptr++ = 0;
#else
  memset(ptr, 0, size);
#endif
}

void *get_memcpy(const void *m, size_t size)
{
  void *m_copy;

  if (m == NULL)
    return NULL;

  m_copy = checked_malloc(size);
  memcpy(m_copy, m, size);

  return m_copy;
}


// ----------------------------------------------------------------------------
// various helper functions
// ----------------------------------------------------------------------------

void swap_numbers(int *i1, int *i2)
{
  int help = *i1;

  *i1 = *i2;
  *i2 = help;
}

void swap_number_pairs(int *x1, int *y1, int *x2, int *y2)
{
  int help_x = *x1;
  int help_y = *y1;

  *x1 = *x2;
  *x2 = help_x;

  *y1 = *y2;
  *y2 = help_y;
}

int get_number_of_bits(int bits)
{
  /*
    Counting bits set, Brian Kernighan's way

    Brian Kernighan's method goes through as many iterations as there are set
    bits. So if we have a 32-bit word with only the high bit set, then it will
    only go once through the loop.

    Published in 1988, the C Programming Language 2nd Ed. (by Brian W. Kernighan
    and Dennis M. Ritchie) mentions this in exercise 2-9.
    First published by Peter Wegner in CACM 3 (1960), 322.
  */

  int num_bits = 0;

  while (bits)
  {
    bits &= bits - 1;	// clear the least significant bit set
    num_bits++;
  }

  return num_bits;
}

/* the "put" variants of the following file access functions check for the file
   pointer being != NULL and return the number of bytes they have or would have
   written; this allows for chunk writing functions to first determine the size
   of the (not yet written) chunk, write the correct chunk size and finally
   write the chunk itself */

int getFile8BitInteger(File *file)
{
  return getByteFromFile(file);
}

int putFile8BitInteger(FILE *file, int value)
{
  if (file != NULL)
    fputc(value, file);

  return 1;
}

int getFile16BitInteger(File *file, int byte_order)
{
  if (byte_order == BYTE_ORDER_BIG_ENDIAN)
    return ((getByteFromFile(file) << 8) |
	    (getByteFromFile(file) << 0));
  else		 // BYTE_ORDER_LITTLE_ENDIAN
    return ((getByteFromFile(file) << 0) |
	    (getByteFromFile(file) << 8));
}

int putFile16BitInteger(FILE *file, int value, int byte_order)
{
  if (file != NULL)
  {
    if (byte_order == BYTE_ORDER_BIG_ENDIAN)
    {
      fputc((value >> 8) & 0xff, file);
      fputc((value >> 0) & 0xff, file);
    }
    else	   // BYTE_ORDER_LITTLE_ENDIAN
    {
      fputc((value >> 0) & 0xff, file);
      fputc((value >> 8) & 0xff, file);
    }
  }

  return 2;
}

int getFile32BitInteger(File *file, int byte_order)
{
  if (byte_order == BYTE_ORDER_BIG_ENDIAN)
    return ((getByteFromFile(file) << 24) |
	    (getByteFromFile(file) << 16) |
	    (getByteFromFile(file) <<  8) |
	    (getByteFromFile(file) <<  0));
  else		 // BYTE_ORDER_LITTLE_ENDIAN
    return ((getByteFromFile(file) <<  0) |
	    (getByteFromFile(file) <<  8) |
	    (getByteFromFile(file) << 16) |
	    (getByteFromFile(file) << 24));
}

int putFile32BitInteger(FILE *file, int value, int byte_order)
{
  if (file != NULL)
  {
    if (byte_order == BYTE_ORDER_BIG_ENDIAN)
    {
      fputc((value >> 24) & 0xff, file);
      fputc((value >> 16) & 0xff, file);
      fputc((value >>  8) & 0xff, file);
      fputc((value >>  0) & 0xff, file);
    }
    else	   // BYTE_ORDER_LITTLE_ENDIAN
    {
      fputc((value >>  0) & 0xff, file);
      fputc((value >>  8) & 0xff, file);
      fputc((value >> 16) & 0xff, file);
      fputc((value >> 24) & 0xff, file);
    }
  }

  return 4;
}

boolean getFileChunk(File *file, char *chunk_name, int *chunk_size,
		     int byte_order)
{
  const int chunk_name_length = 4;

  // read chunk name
  if (getStringFromFile(file, chunk_name, chunk_name_length + 1) == NULL)
    return FALSE;

  if (chunk_size != NULL)
  {
    // read chunk size
    *chunk_size = getFile32BitInteger(file, byte_order);
  }

  return (checkEndOfFile(file) ? FALSE : TRUE);
}

int putFileChunk(FILE *file, char *chunk_name, int chunk_size,
		 int byte_order)
{
  int num_bytes = 0;

  // write chunk name
  if (file != NULL)
    fputs(chunk_name, file);

  num_bytes += strlen(chunk_name);

  if (chunk_size >= 0)
  {
    // write chunk size
    if (file != NULL)
      putFile32BitInteger(file, chunk_size, byte_order);

    num_bytes += 4;
  }

  return num_bytes;
}

VersionType getFileVersion(File *file)
{
  VersionSubType version_super = getByteFromFile(file);
  VersionSubType version_major = getByteFromFile(file);
  VersionSubType version_minor = getByteFromFile(file);
  VersionSubType version_patch = getByteFromFile(file);

  return VERSION_IDENT(version_super, version_major, version_minor, version_patch);
}

VersionType getFileVersionExtended(File *file)
{
  return getFileVersion(file) >> 32;
}

int putFileVersion(FILE *file, VersionType version)
{
  if (file != NULL)
  {
    VersionSubType version_super = VERSION_SUPER(version);
    VersionSubType version_major = VERSION_MAJOR(version);
    VersionSubType version_minor = VERSION_MINOR(version);
    VersionSubType version_patch = VERSION_PATCH(version);

    fputc(version_super, file);
    fputc(version_major, file);
    fputc(version_minor, file);
    fputc(version_patch, file);
  }

  return 4;
}

int putFileVersionExtended(FILE *file, VersionType version)
{
  return putFileVersion(file, version << 32);
}

void ReadBytesFromFile(File *file, byte *buffer, unsigned int bytes)
{
  int i;

  for (i = 0; i < bytes && !checkEndOfFile(file); i++)
    buffer[i] = getByteFromFile(file);
}

void WriteBytesToFile(FILE *file, byte *buffer, unsigned int bytes)
{
  int i;

  for (i = 0; i < bytes; i++)
    fputc(buffer[i], file);
}

void ReadUnusedBytesFromFile(File *file, unsigned int bytes)
{
  while (bytes-- && !checkEndOfFile(file))
    getByteFromFile(file);
}

void WriteUnusedBytesToFile(FILE *file, unsigned int bytes)
{
  while (bytes--)
    fputc(0, file);
}


// ----------------------------------------------------------------------------
// functions to convert between ISO-8859-1 and UTF-8
// ----------------------------------------------------------------------------

char *getUTF8FromLatin1(char *latin1)
{
  if (latin1 == NULL)
    return NULL;

  int max_utf8_size = 2 * strlen(latin1) + 1;
  char *utf8 = checked_calloc(max_utf8_size);
  unsigned char *src = (unsigned char *)latin1;
  unsigned char *dst = (unsigned char *)utf8;

  while (*src)
  {
    if (*src < 128)		// pure 7-bit ASCII
    {
      *dst++ = *src;
    }
    else if (*src >= 160)	// non-ASCII characters
    {
      *dst++ = 194 + (*src >= 192);
      *dst++ = 128 + (*src & 63);
    }
    else			// undefined in ISO-8859-1
    {
      *dst++ = '?';
    }

    src++;
  }

  // only use the smallest possible string buffer size
  utf8 = checked_realloc(utf8, strlen(utf8) + 1);

  return utf8;
}

char *getLatin1FromUTF8(char *utf8)
{
  if (utf8 == NULL)
    return NULL;

  int max_latin1_size = strlen(utf8) + 1;
  char *latin1 = checked_calloc(max_latin1_size);
  unsigned char *src = (unsigned char *)utf8;
  unsigned char *dst = (unsigned char *)latin1;

  while (*src)
  {
    if (*src < 128)				// pure 7-bit ASCII
    {
      *dst++ = *src++;
    }
    else if (src[0] == 194 &&
	     src[1] >= 128 && src[1] < 192)	// non-ASCII characters
    {
      *dst++ = src[1];
      src += 2;
    }
    else if (src[0] == 195 &&
	     src[1] >= 128 && src[1] < 192)	// non-ASCII characters
    {
      *dst++ = src[1] + 64;
      src += 2;
    }

    // all other UTF-8 characters are undefined in ISO-8859-1

    else if (src[0] >= 192 && src[0] < 224 &&
	     src[1] >= 128 && src[1] < 192)
    {
      *dst++ = '?';
      src += 2;
    }
    else if (src[0] >= 224 && src[0] < 240 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192)
    {
      // detect and substitute selected UTF-8 characters
      if (src[0] == 226 &&
          src[1] == 128 &&
          src[2] >= 152 && src[2] <= 155)
        *dst++ = '\'';
      else if (src[0] == 226 &&
               src[1] == 128 &&
               src[2] >= 156 && src[2] <= 159)
        *dst++ = '"';
      else
        *dst++ = '?';

      src += 3;
    }
    else if (src[0] >= 240 && src[0] < 248 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192 &&
	     src[3] >= 128 && src[3] < 192)
    {
      *dst++ = '?';
      src += 4;
    }
    else if (src[0] >= 248 && src[0] < 252 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192 &&
	     src[3] >= 128 && src[3] < 192 &&
	     src[4] >= 128 && src[4] < 192)
    {
      *dst++ = '?';
      src += 5;
    }
    else if (src[0] >= 252 && src[0] < 254 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192 &&
	     src[3] >= 128 && src[3] < 192 &&
	     src[4] >= 128 && src[4] < 192 &&
	     src[5] >= 128 && src[5] < 192)
    {
      *dst++ = '?';
      src += 6;
    }
    else
    {
      *dst++ = '?';
      src++;
    }
  }

  // only use the smallest possible string buffer size
  latin1 = checked_realloc(latin1, strlen(latin1) + 1);

  return latin1;
}

int getTextEncoding(char *text)
{
  unsigned char *src = (unsigned char *)text;
  int encoding = TEXT_ENCODING_ASCII;	// default: assume encoding is ASCII

  while (*src)
  {
    if (*src >= 128)
      encoding = TEXT_ENCODING_UTF_8;	// non-ASCII character: assume UTF-8

    if (*src < 128)
    {
      src++;
    }
    else if (src[0] >= 192 && src[0] < 224 &&
	     src[1] >= 128 && src[1] < 192)
    {
      src += 2;
    }
    else if (src[0] >= 224 && src[0] < 240 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192)
    {
      src += 3;
    }
    else if (src[0] >= 240 && src[0] < 248 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192 &&
	     src[3] >= 128 && src[3] < 192)
    {
      src += 4;
    }
    else if (src[0] >= 248 && src[0] < 252 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192 &&
	     src[3] >= 128 && src[3] < 192 &&
	     src[4] >= 128 && src[4] < 192)
    {
      src += 5;
    }
    else if (src[0] >= 252 && src[0] < 254 &&
	     src[1] >= 128 && src[1] < 192 &&
	     src[2] >= 128 && src[2] < 192 &&
	     src[3] >= 128 && src[3] < 192 &&
	     src[4] >= 128 && src[4] < 192 &&
	     src[5] >= 128 && src[5] < 192)
    {
      src += 6;
    }
    else
    {
      return TEXT_ENCODING_UNKNOWN;	// non-UTF-8 character: unknown encoding
    }
  }

  return encoding;
}


// ----------------------------------------------------------------------------
// functions for JSON handling
// ----------------------------------------------------------------------------

char *getEscapedJSON(char *s)
{
  int max_json_size = 2 * strlen(s) + 1;
  char *json = checked_calloc(max_json_size);
  unsigned char *src = (unsigned char *)s;
  unsigned char *dst = (unsigned char *)json;
  char *escaped[256] =
  {
    ['\b'] = "\\b",
    ['\f'] = "\\f",
    ['\n'] = "\\n",
    ['\r'] = "\\r",
    ['\t'] = "\\t",
    ['\"'] = "\\\"",
    ['\\'] = "\\\\",
  };

  while (*src)
  {
    if (escaped[*src] != NULL)
    {
      char *esc = escaped[*src++];

      while (*esc)
	*dst++ = *esc++;
    }
    else
    {
      *dst++ = *src++;
    }
  }

  // only use the smallest possible string buffer size
  json = checked_realloc(json, strlen(json) + 1);

  return json;
}


// ----------------------------------------------------------------------------
// functions to translate key identifiers between different format
// ----------------------------------------------------------------------------

#define TRANSLATE_KEYSYM_TO_KEYNAME	0
#define TRANSLATE_KEYSYM_TO_X11KEYNAME	1
#define TRANSLATE_KEYNAME_TO_KEYSYM	2
#define TRANSLATE_X11KEYNAME_TO_KEYSYM	3

static void translate_keyname(Key *keysym, char **x11name, char **name, int mode)
{
  static struct
  {
    Key key;
    char *x11name;
    char *name;
  } translate_key[] =
  {
    // return and escape keys
    { KSYM_Return,	"XK_Return",		"return" },
    { KSYM_Escape,	"XK_Escape",		"escape" },

    // normal cursor keys
    { KSYM_Left,	"XK_Left",		"cursor left" },
    { KSYM_Right,	"XK_Right",		"cursor right" },
    { KSYM_Up,		"XK_Up",		"cursor up" },
    { KSYM_Down,	"XK_Down",		"cursor down" },

    // keypad cursor keys
#ifdef KSYM_KP_Left
    { KSYM_KP_Left,	"XK_KP_Left",		"keypad left" },
    { KSYM_KP_Right,	"XK_KP_Right",		"keypad right" },
    { KSYM_KP_Up,	"XK_KP_Up",		"keypad up" },
    { KSYM_KP_Down,	"XK_KP_Down",		"keypad down" },
#endif

    // other keypad keys
#ifdef KSYM_KP_Enter
    { KSYM_KP_Enter,	"XK_KP_Enter",		"keypad enter" },
    { KSYM_KP_Add,	"XK_KP_Add",		"keypad +" },
    { KSYM_KP_Subtract,	"XK_KP_Subtract",	"keypad -" },
    { KSYM_KP_Multiply,	"XK_KP_Multiply",	"keypad mltply" },
    { KSYM_KP_Divide,	"XK_KP_Divide",		"keypad /" },
    { KSYM_KP_Separator,"XK_KP_Separator",	"keypad ," },
#endif

    // modifier keys
    { KSYM_Shift_L,	"XK_Shift_L",		"left shift" },
    { KSYM_Shift_R,	"XK_Shift_R",		"right shift" },
    { KSYM_Control_L,	"XK_Control_L",		"left control" },
    { KSYM_Control_R,	"XK_Control_R",		"right control" },
    { KSYM_Meta_L,	"XK_Meta_L",		"left meta" },
    { KSYM_Meta_R,	"XK_Meta_R",		"right meta" },
    { KSYM_Alt_L,	"XK_Alt_L",		"left alt" },
    { KSYM_Alt_R,	"XK_Alt_R",		"right alt" },
    { KSYM_Mode_switch,	"XK_Mode_switch",	"mode switch" }, // Alt-R
    { KSYM_Multi_key,	"XK_Multi_key",		"multi key" },	 // Ctrl-R

    // some special keys
    { KSYM_BackSpace,	"XK_BackSpace",		"backspace" },
    { KSYM_Delete,	"XK_Delete",		"delete" },
    { KSYM_Insert,	"XK_Insert",		"insert" },
    { KSYM_Tab,		"XK_Tab",		"tab" },
    { KSYM_Home,	"XK_Home",		"home" },
    { KSYM_End,		"XK_End",		"end" },
    { KSYM_Page_Up,	"XK_Page_Up",		"page up" },
    { KSYM_Page_Down,	"XK_Page_Down",		"page down" },

    { KSYM_Select,	"XK_Select",		"select" },
    { KSYM_Menu,	"XK_Menu",		"menu" },	 // menu key
    { KSYM_Back,	"XK_Back",		"back" },	 // back key
    { KSYM_PlayPause,	"XK_PlayPause",		"play/pause" },
#if defined(PLATFORM_ANDROID)
    { KSYM_Rewind,	"XK_Rewind",		"rewind" },
    { KSYM_FastForward,	"XK_FastForward",	"fast forward" },
#endif

    // ASCII 0x20 to 0x40 keys (except numbers)
    { KSYM_space,	"XK_space",		"space" },
    { KSYM_exclam,	"XK_exclam",		"!" },
    { KSYM_quotedbl,	"XK_quotedbl",		"\"" },
    { KSYM_numbersign,	"XK_numbersign",	"#" },
    { KSYM_dollar,	"XK_dollar",		"$" },
    { KSYM_percent,	"XK_percent",		"%" },
    { KSYM_ampersand,	"XK_ampersand",		"&" },
    { KSYM_apostrophe,	"XK_apostrophe",	"'" },
    { KSYM_parenleft,	"XK_parenleft",		"(" },
    { KSYM_parenright,	"XK_parenright",	")" },
    { KSYM_asterisk,	"XK_asterisk",		"*" },
    { KSYM_plus,	"XK_plus",		"+" },
    { KSYM_comma,	"XK_comma",		"," },
    { KSYM_minus,	"XK_minus",		"-" },
    { KSYM_period,	"XK_period",		"." },
    { KSYM_slash,	"XK_slash",		"/" },
    { KSYM_colon,	"XK_colon",		":" },
    { KSYM_semicolon,	"XK_semicolon",		";" },
    { KSYM_less,	"XK_less",		"<" },
    { KSYM_equal,	"XK_equal",		"=" },
    { KSYM_greater,	"XK_greater",		">" },
    { KSYM_question,	"XK_question",		"?" },
    { KSYM_at,		"XK_at",		"@" },

    // more ASCII keys
    { KSYM_bracketleft,	"XK_bracketleft",	"[" },
    { KSYM_backslash,	"XK_backslash",		"\\" },
    { KSYM_bracketright,"XK_bracketright",	"]" },
    { KSYM_asciicircum,	"XK_asciicircum",	"^" },
    { KSYM_underscore,	"XK_underscore",	"_" },
    { KSYM_grave,	"XK_grave",		"grave" },
    { KSYM_quoteleft,	"XK_quoteleft",		"quote left" },
    { KSYM_braceleft,	"XK_braceleft",		"brace left" },
    { KSYM_bar,		"XK_bar",		"bar" },
    { KSYM_braceright,	"XK_braceright",	"brace right" },
    { KSYM_asciitilde,	"XK_asciitilde",	"~" },

    // special (non-ASCII) keys
    { KSYM_degree,	"XK_degree",		"degree" },
    { KSYM_Adiaeresis,	"XK_Adiaeresis",	"A umlaut" },
    { KSYM_Odiaeresis,	"XK_Odiaeresis",	"O umlaut" },
    { KSYM_Udiaeresis,	"XK_Udiaeresis",	"U umlaut" },
    { KSYM_adiaeresis,	"XK_adiaeresis",	"a umlaut" },
    { KSYM_odiaeresis,	"XK_odiaeresis",	"o umlaut" },
    { KSYM_udiaeresis,	"XK_udiaeresis",	"u umlaut" },
    { KSYM_ssharp,	"XK_ssharp",		"sharp s" },

    // special (non-ASCII) keys (UTF-8, for reverse mapping only)
    { KSYM_degree,	"XK_degree",		"\xc2\xb0" },
    { KSYM_Adiaeresis,	"XK_Adiaeresis",	"\xc3\x84" },
    { KSYM_Odiaeresis,	"XK_Odiaeresis",	"\xc3\x96" },
    { KSYM_Udiaeresis,	"XK_Udiaeresis",	"\xc3\x9c" },
    { KSYM_adiaeresis,	"XK_adiaeresis",	"\xc3\xa4" },
    { KSYM_odiaeresis,	"XK_odiaeresis",	"\xc3\xb6" },
    { KSYM_udiaeresis,	"XK_udiaeresis",	"\xc3\xbc" },
    { KSYM_ssharp,	"XK_ssharp",		"\xc3\x9f" },

    // other keys (for reverse mapping only)
    { KSYM_space,	"XK_space",		" " },

    // keypad keys are not in numerical order in SDL2
    { KSYM_KP_0,	"XK_KP_0",		"keypad 0" },
    { KSYM_KP_1,	"XK_KP_1",		"keypad 1" },
    { KSYM_KP_2,	"XK_KP_2",		"keypad 2" },
    { KSYM_KP_3,	"XK_KP_3",		"keypad 3" },
    { KSYM_KP_4,	"XK_KP_4",		"keypad 4" },
    { KSYM_KP_5,	"XK_KP_5",		"keypad 5" },
    { KSYM_KP_6,	"XK_KP_6",		"keypad 6" },
    { KSYM_KP_7,	"XK_KP_7",		"keypad 7" },
    { KSYM_KP_8,	"XK_KP_8",		"keypad 8" },
    { KSYM_KP_9,	"XK_KP_9",		"keypad 9" },

    // end-of-array identifier
    { 0,                NULL,			NULL }
  };

  int i;

  if (mode == TRANSLATE_KEYSYM_TO_KEYNAME)
  {
    static char name_buffer[30];
    Key key = *keysym;

    if (key >= KSYM_A && key <= KSYM_Z)
      sprintf(name_buffer, "%c", 'A' + (char)(key - KSYM_A));
    else if (key >= KSYM_a && key <= KSYM_z)
      sprintf(name_buffer, "%c", 'a' + (char)(key - KSYM_a));
    else if (key >= KSYM_0 && key <= KSYM_9)
      sprintf(name_buffer, "%c", '0' + (char)(key - KSYM_0));
    else if (key >= KSYM_FKEY_FIRST && key <= KSYM_FKEY_LAST)
      sprintf(name_buffer, "F%d", (int)(key - KSYM_FKEY_FIRST + 1));
    else if (key == KSYM_UNDEFINED)
      strcpy(name_buffer, "(undefined)");
    else
    {
      i = 0;

      do
      {
	if (key == translate_key[i].key)
	{
	  strcpy(name_buffer, translate_key[i].name);
	  break;
	}
      }
      while (translate_key[++i].name);

      if (!translate_key[i].name)
	strcpy(name_buffer, "(unknown)");
    }

    *name = name_buffer;
  }
  else if (mode == TRANSLATE_KEYSYM_TO_X11KEYNAME)
  {
    static char name_buffer[30];
    Key key = *keysym;

    if (key >= KSYM_A && key <= KSYM_Z)
      sprintf(name_buffer, "XK_%c", 'A' + (char)(key - KSYM_A));
    else if (key >= KSYM_a && key <= KSYM_z)
      sprintf(name_buffer, "XK_%c", 'a' + (char)(key - KSYM_a));
    else if (key >= KSYM_0 && key <= KSYM_9)
      sprintf(name_buffer, "XK_%c", '0' + (char)(key - KSYM_0));
    else if (key >= KSYM_FKEY_FIRST && key <= KSYM_FKEY_LAST)
      sprintf(name_buffer, "XK_F%d", (int)(key - KSYM_FKEY_FIRST + 1));
    else if (key == KSYM_UNDEFINED)
      strcpy(name_buffer, "[undefined]");
    else
    {
      i = 0;

      do
      {
	if (key == translate_key[i].key)
	{
	  strcpy(name_buffer, translate_key[i].x11name);
	  break;
	}
      }
      while (translate_key[++i].x11name);

      if (!translate_key[i].x11name)
	sprintf(name_buffer, "0x%04x", (unsigned int)key);
    }

    *x11name = name_buffer;
  }
  else if (mode == TRANSLATE_KEYNAME_TO_KEYSYM)
  {
    Key key = KSYM_UNDEFINED;
    char *name_ptr = *name;

    if (strlen(*name) == 1)
    {
      char c = name_ptr[0];

      if (c >= 'A' && c <= 'Z')
	key = KSYM_A + (Key)(c - 'A');
      else if (c >= 'a' && c <= 'z')
	key = KSYM_a + (Key)(c - 'a');
      else if (c >= '0' && c <= '9')
	key = KSYM_0 + (Key)(c - '0');
    }

    if (key == KSYM_UNDEFINED)
    {
      i = 0;

      do
      {
	if (strEqual(translate_key[i].name, *name))
	{
	  key = translate_key[i].key;
	  break;
	}
      }
      while (translate_key[++i].x11name);
    }

    if (key == KSYM_UNDEFINED)
      Warn("getKeyFromKeyName(): not completely implemented");

    *keysym = key;
  }
  else if (mode == TRANSLATE_X11KEYNAME_TO_KEYSYM)
  {
    Key key = KSYM_UNDEFINED;
    char *name_ptr = *x11name;

    if (strPrefix(name_ptr, "XK_") && strlen(name_ptr) == 4)
    {
      char c = name_ptr[3];

      if (c >= 'A' && c <= 'Z')
	key = KSYM_A + (Key)(c - 'A');
      else if (c >= 'a' && c <= 'z')
	key = KSYM_a + (Key)(c - 'a');
      else if (c >= '0' && c <= '9')
	key = KSYM_0 + (Key)(c - '0');
    }
    else if (strPrefix(name_ptr, "XK_F") && strlen(name_ptr) <= 6)
    {
      char c1 = name_ptr[4];
      char c2 = name_ptr[5];
      int d = 0;

      if ((c1 >= '0' && c1 <= '9') &&
	  ((c2 >= '0' && c1 <= '9') || c2 == '\0'))
	d = atoi(&name_ptr[4]);

      if (d >= 1 && d <= KSYM_NUM_FKEYS)
	key = KSYM_F1 + (Key)(d - 1);
    }
    else if (strPrefix(name_ptr, "XK_"))
    {
      i = 0;

      do
      {
	if (strEqual(name_ptr, translate_key[i].x11name))
	{
	  key = translate_key[i].key;
	  break;
	}
      }
      while (translate_key[++i].x11name);
    }
    else if (strPrefix(name_ptr, "0x"))
    {
      unsigned int value = 0;

      name_ptr += 2;

      while (name_ptr)
      {
	char c = *name_ptr++;
	int d = -1;

	if (c >= '0' && c <= '9')
	  d = (int)(c - '0');
	else if (c >= 'a' && c <= 'f')
	  d = (int)(c - 'a' + 10);
	else if (c >= 'A' && c <= 'F')
	  d = (int)(c - 'A' + 10);

	if (d == -1)
	{
	  value = -1;
	  break;
	}

	value = value * 16 + d;
      }

      if (value != -1)
	key = (Key)value;
    }

    *keysym = key;
  }
}

char *getKeyNameFromKey(Key key)
{
  char *name;

  translate_keyname(&key, NULL, &name, TRANSLATE_KEYSYM_TO_KEYNAME);
  return name;
}

char *getX11KeyNameFromKey(Key key)
{
  char *x11name;

  translate_keyname(&key, &x11name, NULL, TRANSLATE_KEYSYM_TO_X11KEYNAME);
  return x11name;
}

Key getKeyFromKeyName(char *name)
{
  Key key;

  translate_keyname(&key, NULL, &name, TRANSLATE_KEYNAME_TO_KEYSYM);
  return key;
}

Key getKeyFromX11KeyName(char *x11name)
{
  Key key;

  translate_keyname(&key, &x11name, NULL, TRANSLATE_X11KEYNAME_TO_KEYSYM);
  return key;
}

char getCharFromKey(Key key)
{
  static struct
  {
    Key key;
    byte key_char;
  } translate_key_char[] =
  {
    // special (non-ASCII) keys (ISO-8859-1)
    { KSYM_degree,	CHAR_BYTE_DEGREE	},
    { KSYM_Adiaeresis,	CHAR_BYTE_UMLAUT_A	},
    { KSYM_Odiaeresis,	CHAR_BYTE_UMLAUT_O	},
    { KSYM_Udiaeresis,	CHAR_BYTE_UMLAUT_U	},
    { KSYM_adiaeresis,	CHAR_BYTE_UMLAUT_a	},
    { KSYM_odiaeresis,	CHAR_BYTE_UMLAUT_o	},
    { KSYM_udiaeresis,	CHAR_BYTE_UMLAUT_u	},
    { KSYM_ssharp,	CHAR_BYTE_SHARP_S	},

    // end-of-array identifier
    { 0,                0			}
  };

  char *keyname = getKeyNameFromKey(key);
  char c = 0;

  if (strlen(keyname) == 1)
    c = keyname[0];
  else if (strEqual(keyname, "space"))
    c = ' ';
  else
  {
    int i = 0;

    do
    {
      if (key == translate_key_char[i].key)
      {
	c = translate_key_char[i].key_char;

	break;
      }
    }
    while (translate_key_char[++i].key_char);
  }

  return c;
}

char getValidConfigValueChar(char c)
{
  if (c == '#' ||	// used to mark comments
      c == '\\')	// used to mark continued lines
    c = 0;

  return c;
}


// ----------------------------------------------------------------------------
// functions to translate string identifiers to integer or boolean value
// ----------------------------------------------------------------------------

int get_integer_from_string(char *s)
{
  // check for the most common case first
  if (s[0] >= '0' && s[0] <= '9')
    return atoi(s);

  char *s_lower = getStringToLower(s);
  int result = -1;

  if (strEqual(s_lower, "false") ||
      strEqual(s_lower, "no") ||
      strEqual(s_lower, "off"))
    result = 0;
  else if (strEqual(s_lower, "true") ||
	   strEqual(s_lower, "yes") ||
	   strEqual(s_lower, "on"))
    result = 1;
  else
    result = atoi(s);

  free(s_lower);

  return result;
}

boolean get_boolean_from_string(char *s)
{
  char *s_lower = getStringToLower(s);
  boolean result = FALSE;

  if (strEqual(s_lower, "true") ||
      strEqual(s_lower, "yes") ||
      strEqual(s_lower, "on") ||
      get_integer_from_string(s) == 1)
    result = TRUE;

  free(s_lower);

  return result;
}

int get_switch_3_state_from_string(char *s)
{
  char *s_lower = getStringToLower(s);
  int result = FALSE;

  if (strEqual(s_lower, "true") ||
      strEqual(s_lower, "yes") ||
      strEqual(s_lower, "on") ||
      get_integer_from_string(s) == 1)
    result = TRUE;
  else if (strEqual(s_lower, "auto"))
    result = STATE_AUTO;
  else if (strEqual(s_lower, "ask"))
    result = STATE_ASK;

  free(s_lower);

  return result;
}

int get_player_nr_from_string(char *s)
{
  static char *player_text[] =
  {
    "player_1",
    "player_2",
    "player_3",
    "player_4",

    NULL
  };

  char *s_lower = getStringToLower(s);
  int result = 0;
  int i;

  for (i = 0; player_text[i] != NULL; i++)
    if (strEqual(s_lower, player_text[i]))
      result = i;

  free(s_lower);

  return result;
}


// ----------------------------------------------------------------------------
// functions for generic lists
// ----------------------------------------------------------------------------

ListNode *newListNode(void)
{
  return checked_calloc(sizeof(ListNode));
}

void addNodeToList(ListNode **node_first, char *key, void *content)
{
  ListNode *node_new = newListNode();

  node_new->key = getStringCopy(key);
  node_new->content = content;
  node_new->next = *node_first;

  if (*node_first)
    (*node_first)->prev = node_new;

  *node_first = node_new;
}

void deleteNodeFromList(ListNode **node_first, char *key,
			void (*destructor_function)(void *))
{
  if (node_first == NULL || *node_first == NULL)
    return;

  if (strEqual((*node_first)->key, key))
  {
    // after first recursion, (*node_first)->prev->next == *node_first,
    // so *node_first would be overwritten with (*node_first)->next
    // => use a copy of *node_first (and later of (*node_first)->next)
    ListNode *node = *node_first;
    ListNode *node_next = node->next;

    checked_free(node->key);

    if (destructor_function)
      destructor_function(node->content);

    if (node->prev)
      node->prev->next = node->next;

    if (node->next)
      node->next->prev = node->prev;

    checked_free(node);

    // after removing node, set list pointer to next valid list node
    // (this is important if the first node of the list was deleted)
    *node_first = node_next;
  }
  else
  {
    deleteNodeFromList(&(*node_first)->next, key, destructor_function);
  }
}

ListNode *getNodeFromKey(ListNode *node_first, char *key)
{
  if (node_first == NULL)
    return NULL;

  if (strEqual(node_first->key, key))
    return node_first;
  else
    return getNodeFromKey(node_first->next, key);
}

int getNumNodes(ListNode *node_first)
{
  return (node_first ? 1 + getNumNodes(node_first->next) : 0);
}

#if 0
static void dumpList(ListNode *node_first)
{
  ListNode *node = node_first;

  while (node)
  {
    Debug("internal:dumpList", "['%s' (%d)]", node->key,
	  ((struct ListNodeInfo *)node->content)->num_references);
    node = node->next;
  }

  Debug("internal:dumpList", "[%d nodes]", getNumNodes(node_first));
}
#endif


// ----------------------------------------------------------------------------
// functions for file handling
// ----------------------------------------------------------------------------

#define MAX_BUFFER_SIZE			4096

File *openFile(const char *filename, const char *mode)
{
  File *file = checked_calloc(sizeof(File));

  file->file = fopen(filename, mode);

  if (file->file != NULL)
  {
    file->filename = getStringCopy(filename);

    return file;
  }

#if defined(PLATFORM_ANDROID)
  file->asset_file = SDL_RWFromFile(filename, mode);

  if (file->asset_file != NULL)
  {
    file->file_is_asset = TRUE;
    file->filename = getStringCopy(filename);

    return file;
  }
#endif

  checked_free(file);

  return NULL;
}

int closeFile(File *file)
{
  if (file == NULL)
    return -1;

  int result = 0;

#if defined(PLATFORM_ANDROID)
  if (file->asset_file)
    result = SDL_RWclose(file->asset_file);
#endif

  if (file->file)
    result = fclose(file->file);

  checked_free(file->filename);
  checked_free(file);

  return result;
}

int checkEndOfFile(File *file)
{
#if defined(PLATFORM_ANDROID)
  if (file->file_is_asset)
    return file->end_of_file;
#endif

  return feof(file->file);
}

size_t readFile(File *file, void *buffer, size_t item_size, size_t num_items)
{
#if defined(PLATFORM_ANDROID)
  if (file->file_is_asset)
  {
    if (file->end_of_file)
      return 0;

    size_t num_items_read =
      SDL_RWread(file->asset_file, buffer, item_size, num_items);

    if (num_items_read < num_items)
      file->end_of_file = TRUE;

    return num_items_read;
  }
#endif

  return fread(buffer, item_size, num_items, file->file);
}

size_t writeFile(File *file, void *buffer, size_t item_size, size_t num_items)
{
  return fwrite(buffer, item_size, num_items, file->file);
}

int seekFile(File *file, long offset, int whence)
{
#if defined(PLATFORM_ANDROID)
  if (file->file_is_asset)
  {
    int sdl_whence = (whence == SEEK_SET ? RW_SEEK_SET :
		      whence == SEEK_CUR ? RW_SEEK_CUR :
		      whence == SEEK_END ? RW_SEEK_END : 0);

    return (SDL_RWseek(file->asset_file, offset, sdl_whence) == -1 ? -1 : 0);
  }
#endif

  return fseek(file->file, offset, whence);
}

int getByteFromFile(File *file)
{
#if defined(PLATFORM_ANDROID)
  if (file->file_is_asset)
  {
    if (file->end_of_file)
      return EOF;

    byte c;
    size_t num_bytes_read = SDL_RWread(file->asset_file, &c, 1, 1);

    if (num_bytes_read < 1)
      file->end_of_file = TRUE;

    return (file->end_of_file ? EOF : (int)c);
  }
#endif

  return fgetc(file->file);
}

char *getStringFromFile(File *file, char *line, int size)
{
#if defined(PLATFORM_ANDROID)
  if (file->file_is_asset)
  {
    if (file->end_of_file)
      return NULL;

    char *line_ptr = line;
    int num_bytes_read = 0;

    while (num_bytes_read < size - 1 &&
	   SDL_RWread(file->asset_file, line_ptr, 1, 1) == 1 &&
	   *line_ptr++ != '\n')
      num_bytes_read++;

    *line_ptr = '\0';

    if (strlen(line) == 0)
    {
      file->end_of_file = TRUE;

      return NULL;
    }

    return line;
  }
#endif

  return fgets(line, size, file->file);
}

int copyFile(const char *filename_from, const char *filename_to)
{
  File *file_from, *file_to;

  if ((file_from = openFile(filename_from, MODE_READ)) == NULL)
  {
    return -1;
  }

  if ((file_to = openFile(filename_to, MODE_WRITE)) == NULL)
  {
    closeFile(file_from);

    return -1;
  }

  while (!checkEndOfFile(file_from))
  {
    byte buffer[MAX_BUFFER_SIZE];
    size_t bytes_read = readFile(file_from, buffer, 1, MAX_BUFFER_SIZE);

    writeFile(file_to, buffer, 1, bytes_read);
  }

  closeFile(file_from);
  closeFile(file_to);

  return 0;
}

boolean touchFile(const char *filename)
{
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot touch file '%s'", filename);

    return FALSE;
  }

  fclose(file);

  return TRUE;
}

size_t getSizeOfFile(const char *filename)
{
  struct stat file_status;

  if (stat(filename, &file_status) == 0)
    return file_status.st_size;

#if defined(PLATFORM_ANDROID)
  SDL_RWops *rw = SDL_RWFromFile(filename, MODE_READ);

  if (rw != NULL)
  {
    int size = SDL_RWsize(rw);

    SDL_RWclose(rw);

    return size;
  }
#endif

  return -1;
}


// ----------------------------------------------------------------------------
// functions for directory handling
// ----------------------------------------------------------------------------

Directory *openDirectory(char *dir_name)
{
  Directory *dir = checked_calloc(sizeof(Directory));

  dir->dir = opendir(dir_name);

  if (dir->dir != NULL)
  {
    dir->filename = getStringCopy(dir_name);

    return dir;
  }

#if defined(PLATFORM_ANDROID)
  char *asset_toc_filename = getPath2(dir_name, ASSET_TOC_BASENAME);

  dir->asset_toc_file = SDL_RWFromFile(asset_toc_filename, MODE_READ);

  checked_free(asset_toc_filename);

  if (dir->asset_toc_file != NULL)
  {
    dir->directory_is_asset = TRUE;
    dir->filename = getStringCopy(dir_name);

    return dir;
  }
#endif

  checked_free(dir);

  return NULL;
}

int closeDirectory(Directory *dir)
{
  if (dir == NULL)
    return -1;

  int result = 0;

#if defined(PLATFORM_ANDROID)
  if (dir->asset_toc_file)
    result = SDL_RWclose(dir->asset_toc_file);
#endif

  if (dir->dir)
    result = closedir(dir->dir);

  if (dir->dir_entry)
    freeDirectoryEntry(dir->dir_entry);

  checked_free(dir->filename);
  checked_free(dir);

  return result;
}

DirectoryEntry *readDirectory(Directory *dir)
{
  if (dir->dir_entry)
    freeDirectoryEntry(dir->dir_entry);

  dir->dir_entry = NULL;

#if defined(PLATFORM_ANDROID)
  if (dir->directory_is_asset)
  {
    char line[MAX_LINE_LEN];
    char *line_ptr = line;
    int num_bytes_read = 0;

    while (num_bytes_read < MAX_LINE_LEN - 1 &&
	   SDL_RWread(dir->asset_toc_file, line_ptr, 1, 1) == 1 &&
	   *line_ptr != '\n')
    {
      line_ptr++;
      num_bytes_read++;
    }

    *line_ptr = '\0';

    if (strlen(line) == 0)
      return NULL;

    dir->dir_entry = checked_calloc(sizeof(DirectoryEntry));

    dir->dir_entry->is_directory = FALSE;
    if (line[strlen(line) - 1] == '/')
    {
      dir->dir_entry->is_directory = TRUE;

      line[strlen(line) - 1] = '\0';
    }

    dir->dir_entry->basename = getStringCopy(line);
    dir->dir_entry->filename = getPath2(dir->filename, line);

    return dir->dir_entry;
  }
#endif

  struct dirent *dir_entry = readdir(dir->dir);

  if (dir_entry == NULL)
    return NULL;

  dir->dir_entry = checked_calloc(sizeof(DirectoryEntry));

  dir->dir_entry->basename = getStringCopy(dir_entry->d_name);
  dir->dir_entry->filename = getPath2(dir->filename, dir_entry->d_name);

  struct stat file_status;

  dir->dir_entry->is_directory =
    (stat(dir->dir_entry->filename, &file_status) == 0 &&
     S_ISDIR(file_status.st_mode));

  return dir->dir_entry;
}

void freeDirectoryEntry(DirectoryEntry *dir_entry)
{
  if (dir_entry == NULL)
    return;

  checked_free(dir_entry->basename);
  checked_free(dir_entry->filename);
  checked_free(dir_entry);
}


// ----------------------------------------------------------------------------
// functions for checking files and filenames
// ----------------------------------------------------------------------------

boolean directoryExists(const char *dir_name)
{
  if (dir_name == NULL)
    return FALSE;

  struct stat file_status;
  boolean success = (stat(dir_name, &file_status) == 0 &&
		     S_ISDIR(file_status.st_mode));

#if defined(PLATFORM_ANDROID)
  if (!success)
  {
    // this might be an asset directory; check by trying to open toc file
    char *asset_toc_filename = getPath2(dir_name, ASSET_TOC_BASENAME);
    SDL_RWops *file = SDL_RWFromFile(asset_toc_filename, MODE_READ);

    checked_free(asset_toc_filename);

    success = (file != NULL);

    if (success)
      SDL_RWclose(file);
  }
#endif

  return success;
}

boolean fileExists(const char *filename)
{
  if (filename == NULL)
    return FALSE;

  boolean success = (access(filename, F_OK) == 0);

#if defined(PLATFORM_ANDROID)
  if (!success)
  {
    // this might be an asset file; check by trying to open it
    SDL_RWops *file = SDL_RWFromFile(filename, MODE_READ);

    success = (file != NULL);

    if (success)
      SDL_RWclose(file);
  }
#endif

  return success;
}

#if 0
static boolean fileHasPrefix(const char *basename, const char *prefix)
{
  static char *basename_lower = NULL;
  int basename_length, prefix_length;

  checked_free(basename_lower);

  if (basename == NULL || prefix == NULL)
    return FALSE;

  basename_lower = getStringToLower(basename);
  basename_length = strlen(basename_lower);
  prefix_length = strlen(prefix);

  if (basename_length > prefix_length + 1 &&
      basename_lower[prefix_length] == '.' &&
      strncmp(basename_lower, prefix, prefix_length) == 0)
    return TRUE;

  return FALSE;
}
#endif

static boolean fileHasSuffix(const char *basename, const char *suffix)
{
  static char *basename_lower = NULL;
  int basename_length, suffix_length;

  checked_free(basename_lower);

  if (basename == NULL || suffix == NULL)
    return FALSE;

  basename_lower = getStringToLower(basename);
  basename_length = strlen(basename_lower);
  suffix_length = strlen(suffix);

  if (basename_length > suffix_length + 1 &&
      basename_lower[basename_length - suffix_length - 1] == '.' &&
      strEqual(&basename_lower[basename_length - suffix_length], suffix))
    return TRUE;

  return FALSE;
}

static boolean FileCouldBeArtwork(char *filename)
{
  char *basename = getBaseNamePtr(filename);

  return (!strEqual(basename, ".") &&
	  !strEqual(basename, "..") &&
	  !fileHasSuffix(basename, "txt") &&
	  !fileHasSuffix(basename, "conf") &&
	  !directoryExists(filename));
}

boolean FileIsGraphic(char *filename)
{
  return FileCouldBeArtwork(filename);
}

boolean FileIsSound(char *filename)
{
  return FileCouldBeArtwork(filename);
}

boolean FileIsMusic(char *filename)
{
  return FileCouldBeArtwork(filename);
}

boolean FileIsArtworkType(char *filename, int type)
{
  if ((type == TREE_TYPE_GRAPHICS_DIR && FileIsGraphic(filename)) ||
      (type == TREE_TYPE_SOUNDS_DIR && FileIsSound(filename)) ||
      (type == TREE_TYPE_MUSIC_DIR && FileIsMusic(filename)))
    return TRUE;

  return FALSE;
}

// ----------------------------------------------------------------------------
// functions for loading artwork configuration information
// ----------------------------------------------------------------------------

char *get_mapped_token(char *token)
{
  // !!! make this dynamically configurable (init.c:InitArtworkConfig) !!!
  static char *map_token_prefix[][2] =
  {
    { "char_procent",		"char_percent"			},
    { "bd_magic_wall_filling",	"bd_magic_wall.filling"		},
    { "bd_magic_wall_emptying",	"bd_magic_wall.emptying"	},
    { "bd_butterfly_left",	"bd_butterfly.left"		},
    { "bd_butterfly_right",	"bd_butterfly.right"		},
    { "bd_butterfly_up",	"bd_butterfly.up"		},
    { "bd_butterfly_down",	"bd_butterfly.down"		},
    { "bd_firefly_left",	"bd_firefly.left"		},
    { "bd_firefly_right",	"bd_firefly.right"		},
    { "bd_firefly_up",		"bd_firefly.up"			},
    { "bd_firefly_down",	"bd_firefly.down"		},

    { NULL,							}
  };
  int i;

  for (i = 0; map_token_prefix[i][0] != NULL; i++)
  {
    int len_token_prefix = strlen(map_token_prefix[i][0]);

    if (strncmp(token, map_token_prefix[i][0], len_token_prefix) == 0)
      return getStringCat2(map_token_prefix[i][1], &token[len_token_prefix]);
  }

  // change tokens containing ".gfx" by moving the "gfx" part to the very left
  char *gfx_substring = ".gfx";
  char *gfx_prefix = "gfx.";
  if (strstr(token, gfx_substring) != NULL)
  {
    char *token_prefix = getStringCopy(token);
    char *token_gfx_pos = strstr(token_prefix, gfx_substring);
    char *token_suffix = &token_gfx_pos[strlen(gfx_substring)];
    char *mapped_token;

    // cut off token string at ".gfx" substring position
    *token_gfx_pos = '\0';

    // put together prefix "gfx." and token prefix and suffix without ".gfx"
    mapped_token = getStringCat3(gfx_prefix, token_prefix, token_suffix);

    free(token_prefix);

    return mapped_token;
  }

  return NULL;
}

static char *get_special_base_token(struct ArtworkListInfo *artwork_info,
				    char *token)
{
  // !!! make this dynamically configurable (init.c:InitArtworkConfig) !!!
  static struct ConfigTypeInfo prefix_list[] =
  {
    { "global.anim_1"	},
    { "global.anim_2"	},
    { "global.anim_3"	},
    { "global.anim_4"	},
    { "global.anim_5"	},
    { "global.anim_6"	},
    { "global.anim_7"	},
    { "global.anim_8"	},
    { "global.anim_9"	},
    { "global.anim_10"	},
    { "global.anim_11"	},
    { "global.anim_12"	},
    { "global.anim_13"	},
    { "global.anim_14"	},
    { "global.anim_15"	},
    { "global.anim_16"	},
    { "global.anim_17"	},
    { "global.anim_18"	},
    { "global.anim_19"	},
    { "global.anim_20"	},
    { "global.anim_21"	},
    { "global.anim_22"	},
    { "global.anim_23"	},
    { "global.anim_24"	},
    { "global.anim_25"	},
    { "global.anim_26"	},
    { "global.anim_27"	},
    { "global.anim_28"	},
    { "global.anim_29"	},
    { "global.anim_30"	},
    { "global.anim_31"	},
    { "global.anim_32"	},

    { NULL		}
  };
  struct ConfigTypeInfo *suffix_list = artwork_info->suffix_list;
  boolean prefix_found = FALSE;
  int len_suffix = 0;
  int i;

  // search for prefix to check if base token has to be created
  for (i = 0; prefix_list[i].token != NULL; i++)
    if (strPrefix(token, prefix_list[i].token))
      prefix_found = TRUE;

  if (!prefix_found)
    return NULL;

  // search for suffix (parameter) to determine base token length
  for (i = 0; suffix_list[i].token != NULL; i++)
    if (strSuffix(token, suffix_list[i].token))
      len_suffix = strlen(suffix_list[i].token);

  return getStringCopyN(token, strlen(token) - len_suffix);
}

static void FreeCustomArtworkList(struct ArtworkListInfo *,
				  struct ListNodeInfo ***, int *);

struct FileInfo *getFileListFromConfigList(struct ConfigInfo *config_list,
					   struct ConfigTypeInfo *suffix_list,
					   char **ignore_tokens,
					   int num_file_list_entries)
{
  SetupFileHash *ignore_tokens_hash;
  struct FileInfo *file_list;
  int num_file_list_entries_found = 0;
  int num_suffix_list_entries = 0;
  int list_pos;
  int i, j;

  // create hash from list of tokens to be ignored (for quick access)
  ignore_tokens_hash = newSetupFileHash();
  for (i = 0; ignore_tokens[i] != NULL; i++)
    setHashEntry(ignore_tokens_hash, ignore_tokens[i], "");

  file_list = checked_calloc(num_file_list_entries * sizeof(struct FileInfo));

  for (i = 0; suffix_list[i].token != NULL; i++)
    num_suffix_list_entries++;

  // always start with reliable default values
  for (i = 0; i < num_file_list_entries; i++)
  {
    file_list[i].token = NULL;

    file_list[i].default_filename = NULL;
    file_list[i].filename = NULL;

    if (num_suffix_list_entries > 0)
    {
      int parameter_array_size = num_suffix_list_entries * sizeof(char *);

      file_list[i].default_parameter = checked_calloc(parameter_array_size);
      file_list[i].parameter = checked_calloc(parameter_array_size);

      for (j = 0; j < num_suffix_list_entries; j++)
      {
	setString(&file_list[i].default_parameter[j], suffix_list[j].value);
	setString(&file_list[i].parameter[j], suffix_list[j].value);
      }

      file_list[i].redefined = FALSE;
      file_list[i].fallback_to_default = FALSE;
      file_list[i].default_is_cloned = FALSE;
    }
  }

  list_pos = 0;

  for (i = 0; config_list[i].token != NULL; i++)
  {
    int len_config_token = strlen(config_list[i].token);
    boolean is_file_entry = TRUE;

    for (j = 0; suffix_list[j].token != NULL; j++)
    {
      int len_suffix = strlen(suffix_list[j].token);

      if (len_suffix < len_config_token &&
	  strEqual(&config_list[i].token[len_config_token - len_suffix],
		   suffix_list[j].token))
      {
	setString(&file_list[list_pos].default_parameter[j],
		  config_list[i].value);

	is_file_entry = FALSE;

	break;
      }
    }

    // the following tokens are no file definitions, but other config tokens
    if (getHashEntry(ignore_tokens_hash, config_list[i].token) != NULL)
      is_file_entry = FALSE;

    if (is_file_entry)
    {
      if (i > 0)
	list_pos++;

      if (list_pos >= num_file_list_entries)
	break;

      file_list[list_pos].token = config_list[i].token;
      file_list[list_pos].default_filename = config_list[i].value;
    }

    if (strSuffix(config_list[i].token, ".clone_from"))
      file_list[list_pos].default_is_cloned = TRUE;
  }

  num_file_list_entries_found = list_pos + 1;
  if (num_file_list_entries_found != num_file_list_entries)
  {
    Error("---");
    Error("inconsistent config list information:");
    Error("- should be:   %d (according to 'src/conf_xxx.h')",
	  num_file_list_entries);
    Error("- found to be: %d (according to 'src/conf_xxx.c')",
	  num_file_list_entries_found);

    Fail("please fix");
  }

  freeSetupFileHash(ignore_tokens_hash);

  return file_list;
}

static boolean token_suffix_match(char *token, char *suffix, int start_pos)
{
  int len_token = strlen(token);
  int len_suffix = strlen(suffix);

  if (start_pos < 0)	// compare suffix from end of string
    start_pos += len_token;

  if (start_pos < 0 || start_pos + len_suffix > len_token)
    return FALSE;

  if (strncmp(&token[start_pos], suffix, len_suffix) != 0)
    return FALSE;

  if (token[start_pos + len_suffix] == '\0')
    return TRUE;

  if (token[start_pos + len_suffix] == '.')
    return TRUE;

  return FALSE;
}

#define KNOWN_TOKEN_VALUE	"[KNOWN_TOKEN_VALUE]"

static void read_token_parameters(SetupFileHash *setup_file_hash,
				  struct ConfigTypeInfo *suffix_list,
				  struct FileInfo *file_list_entry)
{
  // check for config token that is the base token without any suffixes
  char *filename = getHashEntry(setup_file_hash, file_list_entry->token);
  char *known_token_value = KNOWN_TOKEN_VALUE;
  int i;

  if (filename != NULL)
  {
    setString(&file_list_entry->filename, filename);

    // when file definition found, set all parameters to default values
    for (i = 0; suffix_list[i].token != NULL; i++)
      setString(&file_list_entry->parameter[i], suffix_list[i].value);

    file_list_entry->redefined = TRUE;

    // mark config file token as well known from default config
    setHashEntry(setup_file_hash, file_list_entry->token, known_token_value);
  }

  // check for config tokens that can be build by base token and suffixes
  for (i = 0; suffix_list[i].token != NULL; i++)
  {
    char *token = getStringCat2(file_list_entry->token, suffix_list[i].token);
    char *value = getHashEntry(setup_file_hash, token);

    if (value != NULL)
    {
      setString(&file_list_entry->parameter[i], value);

      // mark config file token as well known from default config
      setHashEntry(setup_file_hash, token, known_token_value);
    }

    free(token);
  }
}

static void add_dynamic_file_list_entry(struct FileInfo **list,
					int *num_list_entries,
					SetupFileHash *extra_file_hash,
					struct ConfigTypeInfo *suffix_list,
					int num_suffix_list_entries,
					char *token)
{
  struct FileInfo *new_list_entry;
  int parameter_array_size = num_suffix_list_entries * sizeof(char *);

  (*num_list_entries)++;
  *list = checked_realloc(*list, *num_list_entries * sizeof(struct FileInfo));
  new_list_entry = &(*list)[*num_list_entries - 1];

  new_list_entry->token = getStringCopy(token);
  new_list_entry->default_filename = NULL;
  new_list_entry->filename = NULL;
  new_list_entry->parameter = checked_calloc(parameter_array_size);

  new_list_entry->redefined = FALSE;
  new_list_entry->fallback_to_default = FALSE;
  new_list_entry->default_is_cloned = FALSE;

  read_token_parameters(extra_file_hash, suffix_list, new_list_entry);
}

static void add_property_mapping(struct PropertyMapping **list,
				 int *num_list_entries,
				 int base_index, int ext1_index,
				 int ext2_index, int ext3_index,
				 int artwork_index)
{
  struct PropertyMapping *new_list_entry;

  (*num_list_entries)++;
  *list = checked_realloc(*list,
			  *num_list_entries * sizeof(struct PropertyMapping));
  new_list_entry = &(*list)[*num_list_entries - 1];

  new_list_entry->base_index = base_index;
  new_list_entry->ext1_index = ext1_index;
  new_list_entry->ext2_index = ext2_index;
  new_list_entry->ext3_index = ext3_index;

  new_list_entry->artwork_index = artwork_index;
}

static void LoadArtworkConfigFromFilename(struct ArtworkListInfo *artwork_info,
					  char *filename)
{
  struct FileInfo *file_list = artwork_info->file_list;
  struct ConfigTypeInfo *suffix_list = artwork_info->suffix_list;
  char **base_prefixes = artwork_info->base_prefixes;
  char **ext1_suffixes = artwork_info->ext1_suffixes;
  char **ext2_suffixes = artwork_info->ext2_suffixes;
  char **ext3_suffixes = artwork_info->ext3_suffixes;
  char **ignore_tokens = artwork_info->ignore_tokens;
  int num_file_list_entries = artwork_info->num_file_list_entries;
  int num_suffix_list_entries = artwork_info->num_suffix_list_entries;
  int num_base_prefixes = artwork_info->num_base_prefixes;
  int num_ext1_suffixes = artwork_info->num_ext1_suffixes;
  int num_ext2_suffixes = artwork_info->num_ext2_suffixes;
  int num_ext3_suffixes = artwork_info->num_ext3_suffixes;
  int num_ignore_tokens = artwork_info->num_ignore_tokens;
  SetupFileHash *setup_file_hash, *valid_file_hash, *valid_file_hash_tmp;
  SetupFileHash *extra_file_hash, *empty_file_hash;
  char *known_token_value = KNOWN_TOKEN_VALUE;
  char *base_token_value = UNDEFINED_FILENAME;
  int i, j, k, l;

  if (filename == NULL)
    return;

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  // separate valid (defined) from empty (undefined) config token values
  valid_file_hash = newSetupFileHash();
  empty_file_hash = newSetupFileHash();
  BEGIN_HASH_ITERATION(setup_file_hash, itr)
  {
    char *value = HASH_ITERATION_VALUE(itr);

    setHashEntry(*value ? valid_file_hash : empty_file_hash,
		 HASH_ITERATION_TOKEN(itr), value);
  }
  END_HASH_ITERATION(setup_file_hash, itr)

  // at this point, we do not need the setup file hash anymore -- free it
  freeSetupFileHash(setup_file_hash);

  // prevent changing hash while iterating over it by using a temporary copy
  valid_file_hash_tmp = newSetupFileHash();
  BEGIN_HASH_ITERATION(valid_file_hash, itr)
  {
    setHashEntry(valid_file_hash_tmp,
		 HASH_ITERATION_TOKEN(itr),
		 HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(valid_file_hash, itr)

  // (iterate over same temporary hash, as modifications are independent)

  // map deprecated to current tokens (using prefix match and replace)
  BEGIN_HASH_ITERATION(valid_file_hash_tmp, itr)
  {
    char *token = HASH_ITERATION_TOKEN(itr);
    char *mapped_token = get_mapped_token(token);

    if (mapped_token != NULL)
    {
      char *value = HASH_ITERATION_VALUE(itr);

      // add mapped token
      setHashEntry(valid_file_hash, mapped_token, value);

      // ignore old token (by setting it to "known" keyword)
      setHashEntry(valid_file_hash, token, known_token_value);

      free(mapped_token);
    }
  }
  END_HASH_ITERATION(valid_file_hash_tmp, itr)

  // add special base tokens (using prefix match and replace)
  BEGIN_HASH_ITERATION(valid_file_hash_tmp, itr)
  {
    char *token = HASH_ITERATION_TOKEN(itr);
    char *base_token = get_special_base_token(artwork_info, token);

    if (base_token != NULL)
    {
      // add base token only if it does not already exist
      if (getHashEntry(valid_file_hash, base_token) == NULL)
	setHashEntry(valid_file_hash, base_token, base_token_value);

      free(base_token);
    }
  }
  END_HASH_ITERATION(valid_file_hash_tmp, itr)

  // free temporary hash used for iteration
  freeSetupFileHash(valid_file_hash_tmp);

  // read parameters for all known config file tokens
  for (i = 0; i < num_file_list_entries; i++)
    read_token_parameters(valid_file_hash, suffix_list, &file_list[i]);

  // set all tokens that can be ignored here to "known" keyword
  for (i = 0; i < num_ignore_tokens; i++)
    setHashEntry(valid_file_hash, ignore_tokens[i], known_token_value);

  // copy all unknown config file tokens to extra config hash
  extra_file_hash = newSetupFileHash();
  BEGIN_HASH_ITERATION(valid_file_hash, itr)
  {
    char *value = HASH_ITERATION_VALUE(itr);

    if (!strEqual(value, known_token_value))
      setHashEntry(extra_file_hash, HASH_ITERATION_TOKEN(itr), value);
  }
  END_HASH_ITERATION(valid_file_hash, itr)

  // at this point, we do not need the valid file hash anymore -- free it
  freeSetupFileHash(valid_file_hash);

  // now try to determine valid, dynamically defined config tokens

  BEGIN_HASH_ITERATION(extra_file_hash, itr)
  {
    struct FileInfo **dynamic_file_list =
      &artwork_info->dynamic_file_list;
    int *num_dynamic_file_list_entries =
      &artwork_info->num_dynamic_file_list_entries;
    struct PropertyMapping **property_mapping =
      &artwork_info->property_mapping;
    int *num_property_mapping_entries =
      &artwork_info->num_property_mapping_entries;
    int current_summarized_file_list_entry =
      artwork_info->num_file_list_entries +
      artwork_info->num_dynamic_file_list_entries;
    char *token = HASH_ITERATION_TOKEN(itr);
    int len_token = strlen(token);
    int start_pos;
    boolean base_prefix_found = FALSE;
    boolean parameter_suffix_found = FALSE;

    // skip all parameter definitions (handled by read_token_parameters())
    for (i = 0; i < num_suffix_list_entries && !parameter_suffix_found; i++)
    {
      int len_suffix = strlen(suffix_list[i].token);

      if (token_suffix_match(token, suffix_list[i].token, -len_suffix))
	parameter_suffix_found = TRUE;
    }

    if (parameter_suffix_found)
      continue;

    // ---------- step 0: search for matching base prefix ----------

    start_pos = 0;
    for (i = 0; i < num_base_prefixes && !base_prefix_found; i++)
    {
      char *base_prefix = base_prefixes[i];
      int len_base_prefix = strlen(base_prefix);
      boolean ext1_suffix_found = FALSE;
      boolean ext2_suffix_found = FALSE;
      boolean ext3_suffix_found = FALSE;
      boolean exact_match = FALSE;
      int base_index = -1;
      int ext1_index = -1;
      int ext2_index = -1;
      int ext3_index = -1;

      base_prefix_found = token_suffix_match(token, base_prefix, start_pos);

      if (!base_prefix_found)
	continue;

      base_index = i;

      if (start_pos + len_base_prefix == len_token)	// exact match
      {
	exact_match = TRUE;

	add_dynamic_file_list_entry(dynamic_file_list,
				    num_dynamic_file_list_entries,
				    extra_file_hash,
				    suffix_list,
				    num_suffix_list_entries,
				    token);
	add_property_mapping(property_mapping,
			     num_property_mapping_entries,
			     base_index, -1, -1, -1,
			     current_summarized_file_list_entry);
	continue;
      }

      // ---------- step 1: search for matching first suffix ----------

      start_pos += len_base_prefix;
      for (j = 0; j < num_ext1_suffixes && !ext1_suffix_found; j++)
      {
	char *ext1_suffix = ext1_suffixes[j];
	int len_ext1_suffix = strlen(ext1_suffix);

	ext1_suffix_found = token_suffix_match(token, ext1_suffix, start_pos);

	if (!ext1_suffix_found)
	  continue;

	ext1_index = j;

	if (start_pos + len_ext1_suffix == len_token)	// exact match
	{
	  exact_match = TRUE;

	  add_dynamic_file_list_entry(dynamic_file_list,
				      num_dynamic_file_list_entries,
				      extra_file_hash,
				      suffix_list,
				      num_suffix_list_entries,
				      token);
	  add_property_mapping(property_mapping,
			       num_property_mapping_entries,
			       base_index, ext1_index, -1, -1,
			       current_summarized_file_list_entry);
	  continue;
	}

	start_pos += len_ext1_suffix;
      }

      if (exact_match)
	break;

      // ---------- step 2: search for matching second suffix ----------

      for (k = 0; k < num_ext2_suffixes && !ext2_suffix_found; k++)
      {
	char *ext2_suffix = ext2_suffixes[k];
	int len_ext2_suffix = strlen(ext2_suffix);

	ext2_suffix_found = token_suffix_match(token, ext2_suffix, start_pos);

	if (!ext2_suffix_found)
	  continue;

	ext2_index = k;

	if (start_pos + len_ext2_suffix == len_token)	// exact match
	{
	  exact_match = TRUE;

	  add_dynamic_file_list_entry(dynamic_file_list,
				      num_dynamic_file_list_entries,
				      extra_file_hash,
				      suffix_list,
				      num_suffix_list_entries,
				      token);
	  add_property_mapping(property_mapping,
			       num_property_mapping_entries,
			       base_index, ext1_index, ext2_index, -1,
			       current_summarized_file_list_entry);
	  continue;
	}

	start_pos += len_ext2_suffix;
      }

      if (exact_match)
	break;

      // ---------- step 3: search for matching third suffix ----------

      for (l = 0; l < num_ext3_suffixes && !ext3_suffix_found; l++)
      {
	char *ext3_suffix = ext3_suffixes[l];
	int len_ext3_suffix = strlen(ext3_suffix);

	ext3_suffix_found = token_suffix_match(token, ext3_suffix, start_pos);

	if (!ext3_suffix_found)
	  continue;

	ext3_index = l;

	if (start_pos + len_ext3_suffix == len_token) // exact match
	{
	  exact_match = TRUE;

	  add_dynamic_file_list_entry(dynamic_file_list,
				      num_dynamic_file_list_entries,
				      extra_file_hash,
				      suffix_list,
				      num_suffix_list_entries,
				      token);
	  add_property_mapping(property_mapping,
			       num_property_mapping_entries,
			       base_index, ext1_index, ext2_index, ext3_index,
			       current_summarized_file_list_entry);
	  continue;
	}
      }
    }
  }
  END_HASH_ITERATION(extra_file_hash, itr)

  if (artwork_info->num_dynamic_file_list_entries > 0)
  {
    artwork_info->dynamic_artwork_list =
      checked_calloc(artwork_info->num_dynamic_file_list_entries *
		     artwork_info->sizeof_artwork_list_entry);
  }

  if (options.verbose && IS_PARENT_PROCESS())
  {
    SetupFileList *setup_file_list, *list;
    boolean dynamic_tokens_found = FALSE;
    boolean unknown_tokens_found = FALSE;
    boolean undefined_values_found = (hashtable_count(empty_file_hash) != 0);

    // list may be NULL for empty artwork config files
    setup_file_list = loadSetupFileList(filename);

    BEGIN_HASH_ITERATION(extra_file_hash, itr)
    {
      if (strEqual(HASH_ITERATION_VALUE(itr), known_token_value))
	dynamic_tokens_found = TRUE;
      else
	unknown_tokens_found = TRUE;
    }
    END_HASH_ITERATION(extra_file_hash, itr)

    if (options.debug && dynamic_tokens_found)
    {
      Debug("config", "---");
      Debug("config", "dynamic token(s) found in config file:");
      Debug("config", "- config file: '%s'", filename);

      for (list = setup_file_list; list != NULL; list = list->next)
      {
	char *value = getHashEntry(extra_file_hash, list->token);

	if (value != NULL && strEqual(value, known_token_value))
	  Debug("config", "- dynamic token: '%s'", list->token);
      }

      Debug("config", "---");
    }

    if (unknown_tokens_found)
    {
      Warn("---");
      Warn("unknown token(s) found in config file:");
      Warn("- config file: '%s'", filename);

      for (list = setup_file_list; list != NULL; list = list->next)
      {
	char *value = getHashEntry(extra_file_hash, list->token);

	if (value != NULL && !strEqual(value, known_token_value))
	  Warn("- dynamic token: '%s'", list->token);
      }

      Warn("---");
    }

    if (undefined_values_found)
    {
      Warn("---");
      Warn("undefined values found in config file:");
      Warn("- config file: '%s'", filename);

      for (list = setup_file_list; list != NULL; list = list->next)
      {
	char *value = getHashEntry(empty_file_hash, list->token);

	if (value != NULL)
	  Warn("- undefined value for token: '%s'", list->token);
      }

      Warn("---");
    }

    freeSetupFileList(setup_file_list);
  }

  freeSetupFileHash(extra_file_hash);
  freeSetupFileHash(empty_file_hash);
}

void LoadArtworkConfig(struct ArtworkListInfo *artwork_info)
{
  struct FileInfo *file_list = artwork_info->file_list;
  int num_file_list_entries = artwork_info->num_file_list_entries;
  int num_suffix_list_entries = artwork_info->num_suffix_list_entries;
  char *filename_base = UNDEFINED_FILENAME, *filename_local;
  int i, j;

  DrawInitTextHead("Loading artwork config");
  DrawInitTextItem(ARTWORKINFO_FILENAME(artwork_info->type));

  // always start with reliable default values
  for (i = 0; i < num_file_list_entries; i++)
  {
    setString(&file_list[i].filename, file_list[i].default_filename);

    for (j = 0; j < num_suffix_list_entries; j++)
      setString(&file_list[i].parameter[j], file_list[i].default_parameter[j]);

    file_list[i].redefined = FALSE;
    file_list[i].fallback_to_default = FALSE;
  }

  // free previous dynamic artwork file array
  if (artwork_info->dynamic_file_list != NULL)
  {
    for (i = 0; i < artwork_info->num_dynamic_file_list_entries; i++)
    {
      free(artwork_info->dynamic_file_list[i].token);
      free(artwork_info->dynamic_file_list[i].filename);
      free(artwork_info->dynamic_file_list[i].parameter);
    }

    free(artwork_info->dynamic_file_list);
    artwork_info->dynamic_file_list = NULL;

    FreeCustomArtworkList(artwork_info, &artwork_info->dynamic_artwork_list,
			  &artwork_info->num_dynamic_file_list_entries);
  }

  // free previous property mapping
  if (artwork_info->property_mapping != NULL)
  {
    free(artwork_info->property_mapping);

    artwork_info->property_mapping = NULL;
    artwork_info->num_property_mapping_entries = 0;
  }

  if (!GFX_OVERRIDE_ARTWORK(artwork_info->type))
  {
    // first look for special artwork configured in level series config
    filename_base = getCustomArtworkLevelConfigFilename(artwork_info->type);

    if (fileExists(filename_base))
      LoadArtworkConfigFromFilename(artwork_info, filename_base);
  }

  filename_local = getCustomArtworkConfigFilename(artwork_info->type);

  if (filename_local != NULL && !strEqual(filename_base, filename_local))
    LoadArtworkConfigFromFilename(artwork_info, filename_local);
}

static void deleteArtworkListEntry(struct ArtworkListInfo *artwork_info,
				   struct ListNodeInfo **listnode)
{
  if (*listnode)
  {
    char *filename = (*listnode)->source_filename;

    if (--(*listnode)->num_references <= 0)
      deleteNodeFromList(&artwork_info->content_list, filename,
			 artwork_info->free_artwork);

    *listnode = NULL;
  }
}

static void replaceArtworkListEntry(struct ArtworkListInfo *artwork_info,
				    struct ListNodeInfo **listnode,
				    struct FileInfo *file_list_entry)
{
  char *init_text[] =
  {
    "Loading graphics",
    "Loading sounds",
    "Loading music"
  };

  ListNode *node;
  char *basename = file_list_entry->filename;
  char *filename = getCustomArtworkFilename(basename, artwork_info->type);

  // mark all images from non-default graphics directory as "redefined"
  if (artwork_info->type == ARTWORK_TYPE_GRAPHICS &&
      !strPrefix(filename, options.graphics_directory))
    file_list_entry->redefined = TRUE;

  if (filename == NULL)
  {
    Warn("cannot find artwork file '%s'", basename);

    basename = file_list_entry->default_filename;

    // fail for cloned default artwork that has no default filename defined
    if (file_list_entry->default_is_cloned &&
	strEqual(basename, UNDEFINED_FILENAME))
    {
      void (*error_func)(char *, ...) = Warn;

      // we can get away without sounds and music, but not without graphics
      if (*listnode == NULL && artwork_info->type == ARTWORK_TYPE_GRAPHICS)
	error_func = Fail;

      error_func("token '%s' was cloned and has no default filename",
		 file_list_entry->token);

      return;
    }

    // dynamic artwork has no default filename / skip empty default artwork
    if (basename == NULL || strEqual(basename, UNDEFINED_FILENAME))
      return;

    file_list_entry->fallback_to_default = TRUE;

    Warn("trying default artwork file '%s'", basename);

    filename = getCustomArtworkFilename(basename, artwork_info->type);

    if (filename == NULL)
    {
      void (*error_func)(char *, ...) = Warn;

      // we can get away without sounds and music, but not without graphics
      if (*listnode == NULL && artwork_info->type == ARTWORK_TYPE_GRAPHICS)
	error_func = Fail;

      error_func("cannot find default artwork file '%s'", basename);

      return;
    }
  }

  // check if the old and the new artwork file are the same
  if (*listnode && strEqual((*listnode)->source_filename, filename))
  {
    // The old and new artwork are the same (have the same filename and path).
    // This usually means that this artwork does not exist in this artwork set
    // and a fallback to the existing artwork is done.

    return;
  }

  // delete existing artwork file entry
  deleteArtworkListEntry(artwork_info, listnode);

  // check if the new artwork file already exists in the list of artwork
  if ((node = getNodeFromKey(artwork_info->content_list, filename)) != NULL)
  {
      *listnode = (struct ListNodeInfo *)node->content;
      (*listnode)->num_references++;

      return;
  }

  DrawInitTextHead(init_text[artwork_info->type]);
  DrawInitTextItem(basename);

  if ((*listnode = artwork_info->load_artwork(filename)) != NULL)
  {
    // add new artwork file entry to the list of artwork files
    (*listnode)->num_references = 1;
    addNodeToList(&artwork_info->content_list, (*listnode)->source_filename,
		  *listnode);
  }
  else
  {
    void (*error_func)(char *, ...) = Warn;

    // we can get away without sounds and music, but not without graphics
    if (artwork_info->type == ARTWORK_TYPE_GRAPHICS)
      error_func = Fail;

    error_func("cannot load artwork file '%s'", basename);

    return;
  }
}

static void LoadCustomArtwork(struct ArtworkListInfo *artwork_info,
			      struct ListNodeInfo **listnode,
			      struct FileInfo *file_list_entry)
{
  if (strEqual(file_list_entry->filename, UNDEFINED_FILENAME))
  {
    deleteArtworkListEntry(artwork_info, listnode);

    return;
  }

  replaceArtworkListEntry(artwork_info, listnode, file_list_entry);
}

void ReloadCustomArtworkList(struct ArtworkListInfo *artwork_info)
{
  struct FileInfo *file_list = artwork_info->file_list;
  struct FileInfo *dynamic_file_list = artwork_info->dynamic_file_list;
  int num_file_list_entries = artwork_info->num_file_list_entries;
  int num_dynamic_file_list_entries =
    artwork_info->num_dynamic_file_list_entries;
  int i;

  print_timestamp_init("ReloadCustomArtworkList");

  for (i = 0; i < num_file_list_entries; i++)
    LoadCustomArtwork(artwork_info, &artwork_info->artwork_list[i],
		      &file_list[i]);

  for (i = 0; i < num_dynamic_file_list_entries; i++)
    LoadCustomArtwork(artwork_info, &artwork_info->dynamic_artwork_list[i],
		      &dynamic_file_list[i]);

  print_timestamp_done("ReloadCustomArtworkList");

#if 0
  dumpList(artwork_info->content_list);
#endif
}

static void FreeCustomArtworkList(struct ArtworkListInfo *artwork_info,
				  struct ListNodeInfo ***list,
				  int *num_list_entries)
{
  int i;

  if (*list == NULL)
    return;

  for (i = 0; i < *num_list_entries; i++)
    deleteArtworkListEntry(artwork_info, &(*list)[i]);
  free(*list);

  *list = NULL;
  *num_list_entries = 0;
}

void FreeCustomArtworkLists(struct ArtworkListInfo *artwork_info)
{
  if (artwork_info == NULL)
    return;

  FreeCustomArtworkList(artwork_info, &artwork_info->artwork_list,
			&artwork_info->num_file_list_entries);

  FreeCustomArtworkList(artwork_info, &artwork_info->dynamic_artwork_list,
			&artwork_info->num_dynamic_file_list_entries);
}


// ----------------------------------------------------------------------------
// functions only needed for non-Unix (non-command-line) systems
// (MS-DOS only; SDL/Windows creates files "stdout.txt" and "stderr.txt")
// (now also added for Windows, to create files in user data directory)
// ----------------------------------------------------------------------------

char *getLogBasename(char *basename)
{
  return getStringCat2(basename, ".log");
}

char *getLogFilename(char *basename)
{
  return getPath2(getMainUserGameDataDir(), basename);
}

void OpenLogFile(void)
{
  InitMainUserDataDirectory();

  if ((program.log_file = fopen(program.log_filename, MODE_WRITE)) == NULL)
  {
    program.log_file = program.log_file_default;   // reset to default

    Warn("cannot open file '%s' for writing: %s",
	 program.log_filename, strerror(errno));
  }

  // output should be unbuffered so it is not truncated in a crash
  setbuf(program.log_file, NULL);
}

void CloseLogFile(void)
{
  if (program.log_file != program.log_file_default)
    fclose(program.log_file);
}

void DumpLogFile(void)
{
  FILE *log_file = fopen(program.log_filename, MODE_READ);

  if (log_file == NULL)
    return;

  while (!feof(log_file))
    fputc(fgetc(log_file), stdout);

  fclose(log_file);
}

void NotifyUserAboutErrorFile(void)
{
#if defined(PLATFORM_WINDOWS)
  char *title_text = getStringCat2(program.program_title, " Error Message");
  char *error_text = getStringCat2("The program was aborted due to an error; "
				   "for details, see the following error file:"
				   STRING_NEWLINE,
				   program.log_filename);

  MessageBox(NULL, error_text, title_text, MB_OK);
#endif
}


// ----------------------------------------------------------------------------
// the following is only for debugging purpose and normally not used
// ----------------------------------------------------------------------------

#if DEBUG

#define DEBUG_PRINT_INIT_TIMESTAMPS		TRUE
#define DEBUG_PRINT_INIT_TIMESTAMPS_DEPTH	10

#define DEBUG_NUM_TIMESTAMPS			10
#define DEBUG_TIME_IN_MICROSECONDS		0

#if DEBUG_TIME_IN_MICROSECONDS
static double Counter_Microseconds(void)
{
  static struct timeval base_time = { 0, 0 };
  struct timeval current_time;
  double counter;

  gettimeofday(&current_time, NULL);

  // reset base time in case of wrap-around
  if (current_time.tv_sec < base_time.tv_sec)
    base_time = current_time;

  counter =
    ((double)(current_time.tv_sec  - base_time.tv_sec)) * 1000000 +
    ((double)(current_time.tv_usec - base_time.tv_usec));

  return counter;		// return microseconds since last init
}
#endif

static char *debug_print_timestamp_get_padding(int padding_size)
{
  static char *padding = NULL;
  int max_padding_size = 100;

  if (padding == NULL)
  {
    padding = checked_calloc(max_padding_size + 1);
    memset(padding, ' ', max_padding_size);
  }

  return &padding[MAX(0, max_padding_size - padding_size)];
}

void debug_print_timestamp(int counter_nr, char *message)
{
  int indent_size = 8;
  int padding_size = 40;
  float timestamp_interval;

  if (counter_nr < 0)
    Fail("debugging: invalid negative counter");
  else if (counter_nr >= DEBUG_NUM_TIMESTAMPS)
    Fail("debugging: increase DEBUG_NUM_TIMESTAMPS in misc.c");

#if DEBUG_TIME_IN_MICROSECONDS
  static double counter[DEBUG_NUM_TIMESTAMPS][2];
  char *unit = "ms";

  counter[counter_nr][0] = Counter_Microseconds();
#else
  static int counter[DEBUG_NUM_TIMESTAMPS][2];
  char *unit = "s";

  counter[counter_nr][0] = Counter();
#endif

  timestamp_interval = counter[counter_nr][0] - counter[counter_nr][1];
  counter[counter_nr][1] = counter[counter_nr][0];

  if (message)
    Debug("time:init", "%s%s%s %.3f %s",
	  debug_print_timestamp_get_padding(counter_nr * indent_size),
	  message,
	  debug_print_timestamp_get_padding(padding_size - strlen(message)),
	  timestamp_interval / 1000,
	  unit);
}

#if 0
static void debug_print_parent_only(char *format, ...)
{
  if (!IS_PARENT_PROCESS())
    return;

  if (format)
  {
    va_list ap;

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);

    printf("\n");
  }
}
#endif

#endif	// DEBUG

static void print_timestamp_ext(char *message, char *mode)
{
#if DEBUG_PRINT_INIT_TIMESTAMPS
  static char *debug_message = NULL;
  static char *last_message = NULL;
  static int counter_nr = 0;
  int max_depth = DEBUG_PRINT_INIT_TIMESTAMPS_DEPTH;

  checked_free(debug_message);
  debug_message = getStringCat3(mode, " ", message);

  if (strEqual(mode, "INIT"))
  {
    debug_print_timestamp(counter_nr, NULL);

    if (counter_nr + 1 < max_depth)
      debug_print_timestamp(counter_nr, debug_message);

    counter_nr++;

    debug_print_timestamp(counter_nr, NULL);
  }
  else if (strEqual(mode, "DONE"))
  {
    counter_nr--;

    if (counter_nr + 1 < max_depth ||
	(counter_nr == 0 && max_depth == 1))
    {
      last_message = message;

      if (counter_nr == 0 && max_depth == 1)
      {
	checked_free(debug_message);
	debug_message = getStringCat3("TIME", " ", message);
      }

      debug_print_timestamp(counter_nr, debug_message);
    }
  }
  else if (!strEqual(mode, "TIME") ||
	   !strEqual(message, last_message))
  {
    if (counter_nr < max_depth)
      debug_print_timestamp(counter_nr, debug_message);
  }
#endif
}

void print_timestamp_init(char *message)
{
  print_timestamp_ext(message, "INIT");
}

void print_timestamp_time(char *message)
{
  print_timestamp_ext(message, "TIME");
}

void print_timestamp_done(char *message)
{
  print_timestamp_ext(message, "DONE");
}
