/**
 * @file render.h
 * @brief Rendering of the editor buffer and UI components to the terminal.
 * @defgroup render Rendering
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"
#include "buffer.h"

/** Update viewport offsets to keep the cursor visible. */
void editorScroll(void);
/** Draw the buffer rows into the append buffer. */
void editorDrawRows(struct abuf *ab);
/** Draw the status bar (filename, ft, position). */
void editorDrawStatusBar(struct abuf *ab);
/** Draw the transient message bar. */
void editorDrawMessageBar(struct abuf *ab);
/** Re-render the full screen and place the cursor. */
void editorRefreshScreen(void);

/** @} */


