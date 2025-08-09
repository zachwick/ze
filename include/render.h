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

void editorScroll(void);
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen(void);

/** @} */


