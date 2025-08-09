/**
 * @file render.c
 * @brief Terminal rendering of buffer contents and UI.
 * @ingroup render
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ze.h"
#include "row.h"
#include "buffer.h"
#include "syntax.h"

extern struct editorConfig E;

/**
 * @brief Recompute scroll offsets to keep the cursor visible.
 * @ingroup render
 *
 * Updates @c E.rowoff and @c E.coloff based on cursor location and render x
 * coordinate (rx) so that the cursor remains within the viewport.
 *
 * @post Viewport offsets may change.
 * @sa editorDrawRows(), editorRefreshScreen(), editorRowCxToRx()
 */
void editorScroll(void) {
  E.rx = 0;
  if (E.cy < E.numrows) {
    E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
  }
  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

/**
 * @brief Render visible rows to the append buffer.
 * @ingroup render
 *
 * Writes the visible portion of the buffer into @p ab using ANSI escapes and
 * syntax highlighting. Control characters are inverted for visibility.
 *
 * @param[in,out] ab Append buffer to receive terminal bytes.
 * @sa editorDrawStatusBar(), editorDrawMessageBar(), editorRefreshScreen()
 */
void editorDrawRows(struct abuf *ab) {
  for (int y = 0; y < E.screenrows; y++) {
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows) {
      if (E.numrows == 0 && y == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
                                  "ze -- version %s", ZE_VERSION);
        if (welcomelen > E.screencols) {
          welcomelen = E.screencols;
        }
        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--) {
          abAppend(ab, " ", 1);
        }
        abAppend(ab, welcome, welcomelen);
      } else {
        abAppend(ab, "~", 1);
      }
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0) len = 0;
      if (len > E.screencols) len = E.screencols;
      char *c = &E.row[filerow].render[E.coloff];
      unsigned char *hl = &E.row[filerow].hl[E.coloff];
      int current_color = -1;
      for (int j = 0; j < len; j++) {
        if (iscntrl(c[j])) {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abAppend(ab, "\x1b[7m", 4);
          abAppend(ab, &sym, 1);
          abAppend(ab, "\x1b[m", 3);
          if (current_color != -1) {
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
            abAppend(ab, buf, clen);
          }
        } else if (hl[j] == HL_NORMAL) {
          if (current_color != -1) {
            abAppend(ab, "\x1b[39m", 5);
            current_color = -1;
          }
          abAppend(ab, &c[j], 1);
        } else {
          int color = editorSyntaxToColor(hl[j]);
          if (color != current_color) {
            current_color = color;
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
            abAppend(ab, buf, clen);
          }
          abAppend(ab, &c[j], 1);
        }
      }
      abAppend(ab, "\x1b[39m", 5);
    }
    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}

/**
 * @brief Draw the status bar (filename, ft, position).
 * @ingroup render
 *
 * Shows filename, line count, modified flag, and right-aligned filetype and
 * cursor position.
 *
 * @param[in,out] ab Append buffer to receive terminal bytes.
 * @sa editorDrawMessageBar()
 */
void editorDrawStatusBar(struct abuf *ab) {
  abAppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                     E.filename ? E.filename : "[No Name]", E.numrows,
                     E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
                      E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);
  if (len > E.screencols) len = E.screencols;
  abAppend(ab, status, len);
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

/**
 * @brief Render the transient message bar.
 * @ingroup render
 *
 * Clears the line and draws the current status message if set recently.
 *
 * @param[in,out] ab Append buffer to receive terminal bytes.
 */
void editorDrawMessageBar(struct abuf *ab) {
  abAppend(ab, "\x1b[K", 3);
  int msglen = (int)strlen(E.statusmsg);
  if (msglen > E.screencols) msglen = E.screencols;
  if (msglen && time(NULL) - E.statusmsg_time < 1) {
    abAppend(ab, E.statusmsg, msglen);
  }
}

/**
 * @brief Update viewport offsets to keep the cursor visible.
 * @ingroup render
 *
 * Scrolls the viewport, composes rows, status, and message bars into a dynamic
 * buffer, restores the cursor position, and writes to STDOUT.
 *
 * @post Terminal output is written; the append buffer is freed by abFree().
 * @sa editorScroll(), editorDrawRows(), abAppend(), abFree()
 */
void editorRefreshScreen(void) {
  editorScroll();
  struct abuf ab = ABUF_INIT;
  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);
  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
  abAppend(&ab, buf, strlen(buf));
  abAppend(&ab, "\x1b[?25h", 6);
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}


