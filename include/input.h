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
 * @param prompt printf-style prompt format including one %s for current buffer
 * @param callback Optional callback receiving (buffer, last_key)
 * @return Newly allocated string or NULL if cancelled
 */
char* editorPrompt(char *prompt, void (*callback)(char *, int));

void editorMoveCursor(char key);

void editorProcessKeypress(void);

/** @} */


