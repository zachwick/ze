/**
 * @file search.h
 * @brief Incremental and interactive search utilities.
 * @defgroup search Search
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/** Highlight matches and move cursor during interactive search. */
void editorFindCallback(char *query, int key);
/** Prompt for a query and perform an interactive forward search. */
void editorFind(void);

/** @} */


