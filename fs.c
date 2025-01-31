#include "fs.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  DIR *dir;
  struct dirent *ent;
  size_t buf_size = CHAR_BUFFER_INITIAL_SIZE;
  size_t buf_used = 0;
  char *buf = (char *)calloc(buf_size, sizeof(char));

  if (buf == NULL) {
    perror("Failed to allocate memory");
    return NULL;
  }

  if ((dir = opendir(dirname)) == NULL) {
    perror("Can't open directory");
    free(buf);
    return NULL;
  }

  while ((ent = readdir(dir)) != NULL) {
    const char *name = ent->d_name;

    // Calculate the required space for this entry
    size_t name_len = strlen(name);
    size_t entry_len = name_len + 2; // Space for "name  "

    // Resize the buffer if necessary
    if (buf_used + entry_len + 1 > buf_size) {
      buf_size = buf_used + entry_len + CHAR_BUFFER_INITIAL_SIZE;
      char *new_buf = realloc(buf, buf_size);
      if (new_buf == NULL) {
        perror("Failed to reallocate memory");
        free(buf);
        closedir(dir);
        return NULL;
      }
      buf = new_buf;
    }

    // Append the entry to the buffer
    strcat(buf + buf_used, name);
    strcat(buf + buf_used + name_len, "  ");
    buf_used += entry_len;
  }

  closedir(dir);
  return buf;
}
