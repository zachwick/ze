#pragma once

#include "ze.h"
#include <libguile.h>

void initKeyBindings(void);
void loadPlugins(void);
SCM scmBindKey(SCM keySpec, SCM proc);
int pluginsHandleKey(unsigned char code);
void editorExec(void);

// Scheme bindings
SCM scmBufferToString(void);
SCM scmBufferLineCount(void);
SCM scmGetLine(SCM idx_scm);
SCM scmSetLine(SCM idx_scm, SCM str_scm);
SCM scmInsertLine(SCM idx_scm, SCM str_scm);
SCM scmAppendLine(SCM str_scm);
SCM scmDeleteLine(SCM idx_scm);
SCM scmInsertText(SCM str_scm);
SCM scmInsertChar(SCM ch_scm);
SCM scmInsertNewline(void);
SCM scmDeleteChar(void);
SCM scmGetCursor(void);
SCM scmSetCursor(SCM x_scm, SCM y_scm);
SCM scmMoveCursor(SCM dir_scm);
SCM scmScreenSize(void);
SCM scmOpenFile(SCM path_scm);
SCM scmSaveFile(void);
SCM scmGetFilename(void);
SCM scmSetFilename(SCM path_scm);
SCM scmPrompt(SCM msg_scm);
SCM scmRefreshScreen(void);
SCM scmSearchForward(SCM query_scm);
SCM scmSelectSyntaxForFilename(SCM path_scm);
SCM scmGetFiletype(void);
SCM scmUnbindKey(SCM keySpec);
SCM scmListBindings(void);
SCM scmBufferDirty(void);
SCM scmSetBufferDirty(SCM bool_scm);
SCM scmCloneTemplate(void);

// Optional hook registry accessors
SCM pluginsGetHook(const char *name);


