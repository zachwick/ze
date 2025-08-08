/**
 * @file fileio.h
 * @brief File and directory loading/saving utilities.
 * @defgroup fileio File I/O
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/**
 * Serialize the current buffer into a single string with newlines.
 * @param buflen [out] length of returned string
 * @return Newly allocated string containing the buffer contents
 */
char* editorRowsToString(int *buflen);

/** Prompt for and clone a template into the current buffer. */
void editorCloneTemplate(void);

/** Open a file or directory by path (prompts if NULL). */
void editorOpen(char *filename);

/** Save the current buffer to `E.filename`, prompting if necessary. */
void editorSave(void);

/** @} */


