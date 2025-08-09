/**
 * @file search.c
 * @brief Interactive forward search implementation.
 * @ingroup search
 */
#include <stdlib.h>
#include <string.h>

#include "ze.h"
#include "row.h"
#include "input.h"

extern struct editorConfig E;

/**
 * @brief Live-search callback to update match highlighting and cursor.
 * @ingroup search
 *
 * Maintains search state across invocations. On arrow keys, changes search
 * direction; on other input, resets state. Highlights the next match in the
 * buffer and moves the cursor there, restoring previous highlight as needed.
 *
 * @param[in] query NUL-terminated search string (may be empty).
 * @param[in] key Last key pressed to drive search behavior.
 * @post Cursor, row highlighting, and scroll offset may change.
 * @sa editorFind(), editorRowRxToCx()
 */
void editorFindCallback(char *query, int key) {
  static int last_match = -1;
  static int direction = 1;

  static int saved_hl_line;
  static char *saved_hl = NULL;

  if (saved_hl) {
    memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
    free(saved_hl);
    saved_hl = NULL;
  }

  if (key == '\r' || key == '\x1b') {
    last_match = -1;
    direction = 1;
    return;
  } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
    direction = 1;
  } else if (key == ARROW_LEFT || key == ARROW_UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }

  if (last_match == -1) {
    direction = 1;
  }
  int current = last_match;
  for (int i = 0; i < E.numrows; i++) {
    current += direction;
    if (current == -1) {
      current = E.numrows - 1;
    } else if (current == E.numrows) {
      current = 0;
    }
    erow *row = &E.row[current];
    char *match = strstr(row->render, query);
    if (match) {
      last_match = current;
      E.cy = current;
      E.cx = editorRowRxToCx(row, (int)(match - row->render));
      E.rowoff = E.numrows;

      saved_hl_line = current;
      saved_hl = malloc(row->rsize);
      memcpy(saved_hl, row->hl, row->rsize);
      memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
      break;
    }
  }
}

/**
 * @brief Prompt for a search query and jump to matches interactively.
 * @ingroup search
 *
 * Prompts the user with "Search: " and uses editorFindCallback() to move to
 * matches as the query changes. Restores the original cursor and viewport if
 * the prompt is cancelled.
 *
 * @post May modify cursor and viewport on success; restores them on cancel.
 * @sa editorPrompt(), editorFindCallback()
 */
void editorFind(void) {
  int saved_cx = E.cx;
  int saved_cy = E.cy;
  int saved_coloff = E.coloff;
  int saved_rowoff = E.rowoff;
  char *query = editorPrompt("Search: %s", editorFindCallback);
  if (query) {
    free(query);
  } else {
    E.cx = saved_cx;
    E.cy = saved_cy;
    E.coloff = saved_coloff;
    E.rowoff = saved_rowoff;
  }
}


