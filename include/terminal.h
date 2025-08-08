#pragma once

#include "ze.h"

void die(const char *s);
void disableRawMode(void);
void enableRawMode(void);
char editorReadKey(void);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);


