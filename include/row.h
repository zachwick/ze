/**
 * @file row.h
 * @brief Row data structure manipulation and conversions.
 * @defgroup row Rows
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/** Convert a character index to a render index accounting for tabs. */
int editorRowCxToRx(erow *row, int cx);
/** Convert a render index back to a character index. */
int editorRowRxToCx(erow *row, int rx);
/** Recompute `render`, `rsize`, and syntax highlighting for a row. */
void editorUpdateRow(erow *row);
/** Insert a new row at position `at` initialized from a string. */
void editorInsertRow(int at, char *s, size_t len);
/** Free memory owned by a row. */
void editorFreeRow(erow *row);
/** Delete row at index `at`. */
void editorDelRow(int at);
/** Insert a character into a row at index `at`. */
void editorRowInsertChar(erow *row, int at, int c);
/** Append a string to a row. */
void editorRowAppendString(erow *row, char *s, size_t len);
/** Delete a character at index `at` in a row. */
void editorRowDelChar(erow *row, int at);
/** Delete from character `at` to end of row (kill-line style). */
void editorDelRowAtChar(erow *row, int at);

/** @} */


