/**
 * @file input.h
 * @brief User input handling (prompt, cursor movement, key processing).
 * @defgroup input Input
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/**
 * Display a prompt and capture input with optional incremental callback.
 * @param prompt printf-style prompt format including one %s for current buffer
 * @param callback Optional callback receiving (buffer, last_key)
 * @return Newly allocated string or NULL if cancelled
 */
char* editorPrompt(char *prompt, void (*callback)(char *, int));

/** Move the cursor in response to an arrow key. */
void editorMoveCursor(char key);

/** Read a key and dispatch to editing or plugin commands. */
void editorProcessKeypress(void);

/** @} */


