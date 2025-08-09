/**
 * @file syntax.h
 * @brief Syntax highlighting engine and language selection.
 * @defgroup syntax Syntax
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight(void);

/** @} */


