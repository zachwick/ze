/**
 * @file buffer.h
 * @brief Simple append-only string buffer used for terminal drawing.
 * @defgroup buffer Append buffer
 * @ingroup core
 * @{
 */
#pragma once

/**
 * Append-only string buffer used to accumulate terminal escape sequences
 * and text prior to a single write to STDOUT.
 */
struct abuf {
  char *b;   /**< Pointer to allocated buffer memory. */
  int len;   /**< Current length of valid data in `b`. */
};

/** Initializer macro for an empty append buffer. */
#define ABUF_INIT {NULL, 0}

/**
 * @param ab Target buffer
 * @param s  String data to append
 * @param len Length of `s`
 */
void abAppend(struct abuf *ab, const char *s, int len);

void abFree(struct abuf *ab);

/** @} */


