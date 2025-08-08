#pragma once

#include "ze.h"
#include <libguile.h>

void preDirOpenHook(void);
void postDirOpenHook(int num_files);
void preFileOpenHook(void);
void postFileOpenHook(void);
void editorPreSaveHook(void);
void editorPostSaveHook(void);


