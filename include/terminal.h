/**
 * @file terminal.h
 * @brief Raw terminal mode, key reading, and screen size utilities.
 * @defgroup terminal Terminal
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

/** Abort the program after resetting the screen and printing perror. */
void die(const char *s);
/** Restore cooked terminal mode. */
void disableRawMode(void);
/** Enable raw terminal mode and register an atexit handler. */
void enableRawMode(void);
/** Read a key, decoding escape sequences into editorKey values. */
char editorReadKey(void);
/** Report the current cursor position (1-based). */
int getCursorPosition(int *rows, int *cols);
/** Obtain terminal window size in rows and columns. */
int getWindowSize(int *rows, int *cols);

/** @} */


