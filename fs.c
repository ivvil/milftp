#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#inclide <windows.h>
#else
#include <dirent.h>
#endif

#define CHAR_BUFFER_INITIAL_SIZE 256

/**
 *  @brief Creates a directory listing
 *
 *  Creates a directory listing for the given directory
 *
 *  @param dirname The pathname from where the listing will be created
 *  @return The directory listing as a dynamically allocated string, or NULL on
 *  error
 */
char *dirlist(const char *dirname) {
  char* res = NULL;
  size_t resSize = CHAR_BUFFER_INITIAL_SIZE;
  size_t used = 0;

  DIR *dir = opendir(dirname);
  if (!dir) {
    return NULL;
  }

  res = malloc(CHAR_BUFFER_INITIAL_SIZE);
  if (!res) goto posix_error;
  res[0] = '\0';

  struct dirent *entry;
  while ((entry = readdir(dir))) {
	const char *fname = entry->d_name;
	if (strcmp(fname, ".") == 0 || strcmp(fname, "..") == 0) continue;

	size_t len = strlen(fname);
	size_t req = used + len + 2;

	if (req > resSize) {
	  resSize *= 2;
	  if (resSize < req) resSize = req;

	  char *tmp = realloc(res, resSize);
	  if (!tmp) goto posix_error;
	  res = tmp;
	}

	memcpy(res + used, fname, len);
	
	used += len;
	res[used++] = '\n';
	res[used] = '\0';
  }
  closedir(dir);

  char *final = realloc(res, used + 1);
  return final ? final : res;

 posix_error:
  closedir(dir);
  free(dir);
  return NULL;
}
