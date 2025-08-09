/**
 * @file edit.h
 * @brief Editing primitives (insert/delete/newline) operating on the buffer.
 * @defgroup edit Editing
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

void editorInsertChar(int c);
void editorInsertTimestamp(void);
void editorInsertNewline(void);
void editorDelChar(void);

/** @} */


