/**
 * @file edit.c
 * @brief Editing primitives implementation.
 * @ingroup edit
 */
#include <time.h>
#include <string.h>

#include "ze.h"
#include "row.h"

extern struct editorConfig E;

/**
 * @brief Insert a character at the current cursor position.
 * @ingroup edit
 *
 * If the cursor is positioned at the end of the buffer (on the phantom line
 * equal to @c E.numrows), a new empty line is created first. The character is
 * inserted into the current row at column @c E.cx and the cursor advances by 1.
 * Dirty state and render cache are updated via row helpers.
 *
 * @param[in] c Character code to insert. Only the low 8 bits are used.
 * @post On return, @c E.cx may increase, @c E.numrows may increase on first insert,
 *       and the buffer contents are modified.
 * @sa editorRowInsertChar(), editorInsertNewline(), editorDelChar()
 */
void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    editorInsertRow(E.numrows, "", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}

/**
 * @brief Insert a timestamp string at the cursor.
 * @ingroup edit
 *
 * Inserts the current local time formatted as "%Y-%m-%d %H:%M:%S" by calling
 * editorInsertChar() for each character.
 *
 * @post Buffer is modified; cursor advances by the length of the timestamp.
 * @sa editorInsertChar()
 */
void editorInsertTimestamp(void) {
  time_t timer;
  char buffer[26];
  struct tm* tm_info;
  timer = time(NULL);
  tm_info = localtime(&timer);
  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  int len = (int)strlen(buffer);
  for (int i = 0; i < len; i++) {
    editorInsertChar(buffer[i]);
  }
}

/**
 * @brief Insert a newline at the cursor position, splitting the line if needed.
 * @ingroup edit
 *
 * If @c E.cx is 0, inserts an empty line above the current line. Otherwise,
 * splits the current row at @c E.cx, moving the tail to the next line.
 * Advances the cursor to the beginning of the new line.
 *
 * @post @c E.cy increments, @c E.cx becomes 0. Buffer rows are re-rendered.
 * @sa editorInsertChar(), editorDelChar(), editorInsertRow()
 */
void editorInsertNewline(void) {
  if (E.cx == 0) {
    editorInsertRow(E.cy, "", 0);
  } else {
    erow *row = &E.row[E.cy];
    editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }
  E.cy++;
  E.cx = 0;
}

/**
 * @brief Delete the character to the left of the cursor or join lines.
 * @ingroup edit
 *
 * If at the beginning of a line (column 0) and not at the top of the file,
 * joins the current line into the end of the previous line. Otherwise deletes
 * the character before the cursor. Cursor and dirty state are updated.
 *
 * @post Buffer and cursor position are modified.
 * @sa editorRowDelChar(), editorRowAppendString(), editorDelRow()
 */
void editorDelChar(void) {
  if (E.cy == E.numrows) {
    return;
  }
  if (E.cx == 0 && E.cy == 0) {
    return;
  }
  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    editorRowDelChar(row, E.cx - 1);
    E.cx--;
  } else {
    E.cx = E.row[E.cy - 1].size;
    editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
    editorDelRow(E.cy);
    E.cy--;
  }
}


