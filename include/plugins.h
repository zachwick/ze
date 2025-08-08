#pragma once

#include "ze.h"
#include <libguile.h>

void initKeyBindings(void);
void loadPlugins(void);
SCM scmBindKey(SCM keySpec, SCM proc);
int pluginsHandleKey(unsigned char code);
void editorExec(void);


