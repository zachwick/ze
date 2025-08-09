/**
 * @file buffer.c
 * @brief Implementation of the append-only output buffer.
 * @ingroup buffer
 */
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/**
 * @brief Append raw bytes to the dynamic append buffer.
 * @ingroup buffer
 *
 * Reallocates the buffer as needed and copies the requested number of bytes to
 * the end of the buffer. The resulting buffer is not NUL-terminated; use the
 * maintained length field @c ab->len.
 *
 * @post On success, @c ab->b may be reallocated and @c ab->len increases by the number of appended bytes.
 * @note On allocation failure, the function returns early and leaves the buffer unchanged.
 * @sa abFree(), editorDrawRows(), editorRefreshScreen()
 */
void abAppend(struct abuf *ab, const char *s, int len) {
  char *newb = realloc(ab->b, (size_t)ab->len + (size_t)len);
  if (newb == NULL) {
    return;
  }
  memcpy(&newb[ab->len], s, len);
  ab->b = newb;
  ab->len += len;
}

/**
 * @brief Free the memory held by an append buffer.
 * @ingroup buffer
 *
 * Releases @c ab->b if non-NULL. Does not reset @c ab->len to 0.
 *
 * @param[in,out] ab Append buffer whose storage should be freed.
 * @sa abAppend()
 */
void abFree(struct abuf *ab) { free(ab->b); }


