#pragma once

#include "ze.h"
#include <libguile.h>

void editorSetStatusMessage(const char *fmt, ...);
void scmEditorSetStatusMessage(SCM message);


