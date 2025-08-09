/**
 * @file input.c
 * @brief Prompt and keypress handling implementations.
 * @ingroup input
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ze.h"
#include "status.h"
#include "terminal.h"
#include "render.h"
#include "edit.h"
#include "fileio.h"
#include "search.h"
#include "row.h"
#include "plugins.h"

extern struct editorConfig E;

/**
 * @brief Display a prompt and capture input with optional incremental callback.
 * @ingroup input
 *
 * Displays a prompt on the status line, appending the current input buffer,
 * and reads keypresses until Enter (returns the input) or Escape (returns NULL).
 * If a callback is provided, it is invoked after each keypress with the
 * current buffer and the last key code, enabling live behaviors (e.g., search).
 *
 * Allocation/ownership: returns a heap-allocated, NUL-terminated C string on
 * success; caller must free(). On cancel (Escape), returns NULL.
 *
 * @post Status message is cleared on completion; screen is refreshed during input.
 * @sa editorFind(), editorOpen(), editorSave(), editorSetStatusMessage()
 */
char* editorPrompt(char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[0] = '\0';
  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();
    int c = editorReadKey();
    if (/*c == DEL_KEY ||*/ c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0) { buf[--buflen] = '\0'; }
    } else if (c == '\x1b') {
      editorSetStatusMessage("");
      if (callback) { callback(buf, c); }
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
        editorSetStatusMessage("");
        if (callback) { callback(buf, c); }
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = (char)c;
      buf[buflen] = '\0';
    }
    if (callback) { callback(buf, c); }
  }
}

/**
 * @brief Move the cursor one step in the given direction, clamped to content.
 * @ingroup input
 *
 * Updates @c E.cx and @c E.cy according to @p key. When moving left from the
 * beginning of a line, moves to the end of the previous line. When moving right
 * past the end of a line, moves to the start of the next line. Ensures @c E.cx
 * is not beyond the line length.
 *
 * @param[in] key One of ARROW_LEFT, ARROW_RIGHT, ARROW_UP, ARROW_DOWN.
 * @post Cursor position is updated; no modifications to buffer contents.
 * @sa editorMoveCursor() (Scheme binding), editorInsertNewline()
 */
void editorMoveCursor(char key) {
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  switch (key) {
  case ARROW_LEFT:
    if (E.cx != 0) {
      E.cx--;
    } else if (E.cy > 0) {
      E.cy--;
      E.cx = E.row[E.cy].size;
    }
    break;
  case ARROW_RIGHT:
    if (row && E.cx < row->size) {
      E.cx++;
    } else if (row && E.cx == row->size) {
      E.cy++;
      E.cx = 0;
    }
    break;
  case ARROW_UP:
    if (E.cy != 0) { E.cy--; }
    break;
  case ARROW_DOWN:
    if (E.cy < E.numrows) { E.cy++; }
    break;
  }
  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;
  if (E.cx > rowlen) { E.cx = rowlen; }
}

/**
 * @brief Decode a keypress and execute the corresponding editor action.
 * @ingroup input
 *
 * Reads one key via editorReadKey(), dispatches plugin key handlers first, and
 * falls back to built-in controls (insert/delete/newline/save/open/search/etc.).
 * May modify the buffer, cursor, dirty state, and status message.
 *
 * @post Editor state may change; screen will be refreshed by the main loop.
 * @sa editorReadKey(), pluginsHandleKey(), editorRefreshScreen()
 */
void editorProcessKeypress(void) {
  static int quit_times = ZE_QUIT_TIMES;
  char c = editorReadKey();
  if (pluginsHandleKey((unsigned char)c)) {
    quit_times = ZE_QUIT_TIMES;
    return;
  }
  switch (c) {
  case '\r':
    editorInsertNewline();
    break;
  case CTRL_KEY('q'):
    if (E.dirty && quit_times > 0) {
      editorSetStatusMessage("WARNING!! File has unsaved changes. Press C-q %d more time to quit.", quit_times);
      quit_times--;
      return;
    }
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  case CTRL_KEY('o'):
    editorOpen(NULL);
    break;
  case CTRL_KEY('t'):
    editorCloneTemplate();
    break;
  case CTRL_KEY('i'):
    editorInsertTimestamp();
    break;
  case CTRL_KEY('w'):
    editorSave();
    break;
  case CTRL_KEY('x'):
    editorExec();
    break;
  case HOME_KEY:
    E.cx = 0;
    break;
  case END_KEY:
    if (E.cy < E.numrows) { E.cx = E.row[E.cy].size; }
    break;
  case CTRL_KEY('s'):
    editorFind();
    break;
  case CTRL_KEY('d'):
    editorDelRow(E.cy);
    break;
  case CTRL_KEY('k'):
    editorDelRowAtChar(&E.row[E.cy], E.cx);
    break;
  case BACKSPACE:
  case CTRL_KEY('h'):
    editorDelChar();
    break;
  case PAGE_UP:
  case PAGE_DOWN: {
    if (c == PAGE_UP) { E.cy = E.rowoff; }
    else if (c == PAGE_DOWN) {
      E.cy = E.rowoff + E.screenrows - 1;
      if (E.cy > E.numrows) { E.cy = E.numrows; }
    }
    int times = E.screenrows;
    while (times--) { editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN); }
  } break;
  case ARROW_UP:
  case ARROW_DOWN:
  case ARROW_LEFT:
  case ARROW_RIGHT:
    editorMoveCursor(c);
    break;
  case CTRL_KEY('l'):
  case '\x1b':
    break;
  default:
    editorInsertChar(c);
    break;
  }
  quit_times = ZE_QUIT_TIMES;
}


