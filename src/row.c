/**
 * @file row.c
 * @brief Row manipulation and conversion implementations.
 * @ingroup row
 */
#include <stdlib.h>
#include <string.h>

#include "ze.h"
#include "syntax.h"

extern struct editorConfig E;

/**
 * @brief Convert an index in characters (cx) to a render index (rx).
 * @ingroup row
 *
 * Accounts for tabs expanding to @c ZE_TAB_STOP columns.
 *
 * @param[in] row Row whose data to measure. Must be non-NULL.
 * @param[in] cx Character index within @p row (0..size).
 * @return Render column index corresponding to @p cx.
 * @sa editorRowRxToCx()
 */
int editorRowCxToRx(erow *row, int cx) {
  int rx = 0;
  for (int j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      rx += (ZE_TAB_STOP - 1) - (rx % ZE_TAB_STOP);
    }
    rx++;
  }
  return rx;
}

/**
 * @brief Convert a render index (rx) to a character index (cx).
 * @ingroup row
 *
 * Inverse of editorRowCxToRx(), handling tab expansion.
 *
 * @param[in] row Row whose data to measure. Must be non-NULL.
 * @param[in] rx Render column index (0..rsize).
 * @return Character index corresponding to @p rx.
 * @sa editorRowCxToRx()
 */
int editorRowRxToCx(erow *row, int rx) {
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t') {
      cur_rx += (ZE_TAB_STOP - 1) - (cur_rx % ZE_TAB_STOP);
    }
    cur_rx++;
    if (cur_rx > rx) {
      return cx;
    }
  }
  return cx;
}

/**
 * @brief Rebuild the render buffer and syntax highlights for a row.
 * @ingroup row
 *
 * Allocates/updates @c row->render from @c row->chars expanding tabs, updates
 * @c row->rsize, and recomputes syntax highlighting. Propagates multi-line
 * comment state to the next row when it changes.
 *
 * @param[in,out] row Row to update. Its render buffer is reallocated.
 * @sa editorUpdateSyntax(), editorRowInsertChar(), editorRowDelChar()
 */
void editorUpdateRow(erow *row) {
  int tabs = 0;
  for (int j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      tabs++;
    }
  }
  free(row->render);
  row->render = malloc(row->size + tabs * (ZE_TAB_STOP - 1) + 1);
  int idx = 0;
  for (int j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % ZE_TAB_STOP != 0) {
        row->render[idx++] = ' ';
      }
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;
  editorUpdateSyntax(row);
}

/**
 * @brief Insert a new row into the buffer at a given index.
 * @ingroup row
 *
 * Shifts rows after @p at, initializes the new row's fields, and updates
 * indexes and dirty state. Copies exactly @p len bytes from @p s and appends a
 * NUL terminator.
 *
 * Ownership: @p s is not owned and is not modified. The new row holds its own
 * NUL-terminated copy.
 *
 * @param[in] at Destination index in [0, E.numrows]. Out-of-range is ignored.
 * @param[in] s Pointer to bytes (may contain non-printables; no NUL required).
 * @param[in] len Number of bytes from @p s to copy.
 * @sa editorDelRow(), editorInsertNewline(), editorRowAppendString()
 */
void editorInsertRow(int at, char *s, size_t len) {
  if (at < 0 || at > E.numrows) {
    return;
  }
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
  for (int j = at + 1; j <= E.numrows; j++) {
    E.row[j].idx++;
  }
  E.row[at].idx = at;
  E.row[at].size = (int)len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  E.row[at].hl = NULL;
  E.row[at].hl_open_comment = 0;
  editorUpdateRow(&E.row[at]);
  E.numrows++;
  E.dirty++;
}

/**
 * @brief Free dynamic memory associated with a row.
 * @ingroup row
 *
 * Releases @c render, @c chars, and @c hl arrays if allocated.
 *
 * @param[in,out] row Row whose buffers to free.
 */
void editorFreeRow(erow *row) {
  free(row->render);
  free(row->chars);
  free(row->hl);
}

/**
 * @brief Delete a row from the buffer.
 * @ingroup row
 *
 * Frees the row, compacts the array, updates indices, and marks the buffer dirty.
 *
 * @param[in] at Index of the row to delete in [0, E.numrows).
 * @sa editorInsertRow()
 */
void editorDelRow(int at) {
  if (at < 0 || at >= E.numrows) {
    return;
  }
  editorFreeRow(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
  for (int j = 0; j < E.numrows - 1; j++) {
    E.row[j].idx--;
  }
  E.numrows--;
  E.dirty++;
}

/**
 * @brief Insert a character into a row at a position.
 * @ingroup row
 *
 * Reallocates the row text, shifts tail, inserts @p c, updates render and marks dirty.
 *
 * @param[in,out] row Target row. Must be non-NULL.
 * @param[in] at Insertion index; values outside [0, size] clamp to end.
 * @param[in] c Character code to insert (low 8 bits used).
 * @sa editorRowDelChar(), editorUpdateRow()
 */
void editorRowInsertChar(erow *row, int at, int c) {
  if (at < 0 || at > row->size) {
    at = row->size;
  }
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = (char)c;
  editorUpdateRow(row);
  E.dirty++;
}

/**
 * @brief Append a byte sequence to the end of a row.
 * @ingroup row
 *
 * Extends @p row->chars by @p len bytes from @p s, appends a NUL terminator,
 * updates render and dirty state.
 *
 * @param[in,out] row Target row.
 * @param[in] s Bytes to append; need not be NUL-terminated.
 * @param[in] len Number of bytes from @p s to append.
 * @sa editorRowInsertChar(), editorUpdateRow()
 */
void editorRowAppendString(erow *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += (int)len;
  row->chars[row->size] = '\0';
  editorUpdateRow(row);
  E.dirty++;
}

/**
 * @brief Delete a character from a row.
 * @ingroup row
 *
 * Removes the byte at @p at, shifts the tail left, updates render and marks dirty.
 *
 * @param[in,out] row Target row.
 * @param[in] at Index to delete in [0, size).
 * @sa editorRowInsertChar(), editorUpdateRow()
 */
void editorRowDelChar(erow *row, int at) {
  if (at < 0 || at >= row->size) {
    return;
  }
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}

/**
 * @brief Delete from a row starting at a character index to the end of the line.
 * @ingroup row
 *
 * Deletes characters from @p at (exclusive) to the end, effectively trimming the
 * line at @p at. Updates render and marks dirty.
 *
 * @param[in,out] row Target row.
 * @param[in] at Index after which characters are removed. Must be in range.
 */
void editorDelRowAtChar(erow *row, int at) {
  if (at < 0 || at >= row->size) {
    return;
  }
  int len_diff = row->size - at - 1;
  for (int i = at - 1; i < len_diff; i++) {
    editorRowDelChar(row, i);
  }
  row->size = at;
  editorUpdateRow(row);
  E.dirty++;
}


