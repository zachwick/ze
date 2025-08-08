/**
 * @file buffer.c
 * @brief Implementation of the append-only output buffer.
 * @ingroup buffer
 */
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

void abAppend(struct abuf *ab, const char *s, int len) {
  char *newb = realloc(ab->b, (size_t)ab->len + (size_t)len);
  if (newb == NULL) {
    return;
  }
  memcpy(&newb[ab->len], s, len);
  ab->b = newb;
  ab->len += len;
}

void abFree(struct abuf *ab) { free(ab->b); }


