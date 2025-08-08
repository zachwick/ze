#pragma once

#include "ze.h"

char* editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(char key);
void editorProcessKeypress(void);


