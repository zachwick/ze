/**
 * @file edit.h
 * @brief Editing primitives (insert/delete/newline) operating on the buffer.
 * @defgroup edit Editing
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/** Insert character at the current cursor position. */
void editorInsertChar(int c);
/** Insert a formatted local timestamp string at the cursor. */
void editorInsertTimestamp(void);
/** Insert a newline, potentially splitting the current row. */
void editorInsertNewline(void);
/** Delete the character left of the cursor or join with previous row. */
void editorDelChar(void);

/** @} */


