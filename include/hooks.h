/**
 * @file hooks.h
 * @brief Scheme hook integration points invoked during I/O events.
 * @defgroup hooks Hooks
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"
#include <libguile.h>

/** Called before reading a directory listing into the buffer. */
void preDirOpenHook(void);
/** Called after reading a directory listing; provides file count. */
void postDirOpenHook(int num_files);
/** Called before opening a regular file. */
void preFileOpenHook(void);
/** Called after a file has been read into the buffer. */
void postFileOpenHook(void);
/** Called before saving the current buffer to disk. */
void editorPreSaveHook(void);
/** Called after saving the current buffer to disk. */
void editorPostSaveHook(void);

/** @} */


