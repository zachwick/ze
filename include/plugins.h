/**
 * @file plugins.h
 * @brief Guile/Scheme plugin API and keybinding integration.
 * @defgroup plugins Plugins
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"
#include <libguile.h>

/** Initialize the in-memory key binding table. */
void initKeyBindings(void);
/** Load Scheme plugins from `~/.ze/plugins` (*.scm) if present. */
void loadPlugins(void);
/** Bind a key specification (e.g., "C-x" or "a") to a Scheme procedure. */
SCM scmBindKey(SCM keySpec, SCM proc);
/** If a binding exists for this key, invoke it and return non-zero. */
int pluginsHandleKey(unsigned char code);
/** Read, eval, print a Scheme expression entered via prompt. */
void editorExec(void);

/* Scheme bindings exposed to plugin authors */
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

/** Optional hook registry accessor by name. */
SCM pluginsGetHook(const char *name);

/** @} */


