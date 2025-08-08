/**
 * @file syntax.h
 * @brief Syntax highlighting engine and language selection.
 * @defgroup syntax Syntax
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/** Check whether a character is a token separator. */
int is_separator(int c);
/** Compute highlighting for a row based on current filetype. */
void editorUpdateSyntax(erow *row);
/** Map highlight class to ANSI color code. */
int editorSyntaxToColor(int hl);
/** Select appropriate language rules based on `E.filename`. */
void editorSelectSyntaxHighlight(void);

/** @} */


