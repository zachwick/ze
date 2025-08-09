/**
 * @file terminal.h
 * @brief Raw terminal mode, key reading, and screen size utilities.
 * @defgroup terminal Terminal
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"

void die(const char *s);
void disableRawMode(void);
void enableRawMode(void);
char editorReadKey(void);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);

/** @} */


