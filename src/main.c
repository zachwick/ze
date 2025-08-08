#include <libguile.h>
#include <stdio.h>
#include <stdlib.h>

#include "ze.h"
#include "terminal.h"
#include "status.h"
#include "render.h"
#include "plugins.h"
#include "search.h"
#include "fileio.h"
#include "init.h"
#include "templates.h"
#include "input.h"

struct editorConfig E;

char* notes_template = "";
char* readme_template = "";

void initEditor(void) {
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.row = NULL;
  E.dirty = 0;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;
  E.syntax = NULL;
  if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
    die("getWindowSize");
  }
  E.screenrows -= 2;
}

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();
  initKeyBindings();
  editorSetStatusMessage("HELP: C-o = open a file | C-t = clone a template | C-w = write to disk | C-s = search | C-x guile | C-q = quit");
  scm_init_guile();
  SCM init_func;
  SCM notes_template_scm;
  SCM readme_template_scm;
  scm_c_primitive_load("/Users/zach/.ze/zerc.scm");
  init_func = scm_variable_ref(scm_c_lookup("ze_config"));
  scm_call_0(init_func);
  scm_c_define_gsubr("set-editor-status", 1, 0, 0, (scm_t_subr)&scmEditorSetStatusMessage);
  scm_c_define_gsubr("bind-key", 2, 0, 0, (scm_t_subr)&scmBindKey);
  scm_c_define_gsubr("buffer->string", 0, 0, 0, (scm_t_subr)&scmBufferToString);
  scm_c_define_gsubr("buffer-line-count", 0, 0, 0, (scm_t_subr)&scmBufferLineCount);
  scm_c_define_gsubr("get-line", 1, 0, 0, (scm_t_subr)&scmGetLine);
  scm_c_define_gsubr("set-line!", 2, 0, 0, (scm_t_subr)&scmSetLine);
  scm_c_define_gsubr("insert-line!", 2, 0, 0, (scm_t_subr)&scmInsertLine);
  scm_c_define_gsubr("append-line!", 1, 0, 0, (scm_t_subr)&scmAppendLine);
  scm_c_define_gsubr("delete-line!", 1, 0, 0, (scm_t_subr)&scmDeleteLine);
  scm_c_define_gsubr("insert-text!", 1, 0, 0, (scm_t_subr)&scmInsertText);
  scm_c_define_gsubr("insert-char!", 1, 0, 0, (scm_t_subr)&scmInsertChar);
  scm_c_define_gsubr("insert-newline!", 0, 0, 0, (scm_t_subr)&scmInsertNewline);
  scm_c_define_gsubr("delete-char!", 0, 0, 0, (scm_t_subr)&scmDeleteChar);
  scm_c_define_gsubr("get-cursor", 0, 0, 0, (scm_t_subr)&scmGetCursor);
  scm_c_define_gsubr("set-cursor!", 2, 0, 0, (scm_t_subr)&scmSetCursor);
  scm_c_define_gsubr("move-cursor!", 1, 0, 0, (scm_t_subr)&scmMoveCursor);
  scm_c_define_gsubr("screen-size", 0, 0, 0, (scm_t_subr)&scmScreenSize);
  scm_c_define_gsubr("open-file!", 1, 0, 0, (scm_t_subr)&scmOpenFile);
  scm_c_define_gsubr("save-file!", 0, 0, 0, (scm_t_subr)&scmSaveFile);
  scm_c_define_gsubr("get-filename", 0, 0, 0, (scm_t_subr)&scmGetFilename);
  scm_c_define_gsubr("set-filename!", 1, 0, 0, (scm_t_subr)&scmSetFilename);
  scm_c_define_gsubr("prompt", 1, 0, 0, (scm_t_subr)&scmPrompt);
  scm_c_define_gsubr("refresh-screen!", 0, 0, 0, (scm_t_subr)&scmRefreshScreen);
  scm_c_define_gsubr("search-forward!", 1, 0, 0, (scm_t_subr)&scmSearchForward);
  scm_c_define_gsubr("select-syntax-for-filename!", 1, 0, 0, (scm_t_subr)&scmSelectSyntaxForFilename);
  scm_c_define_gsubr("get-filetype", 0, 0, 0, (scm_t_subr)&scmGetFiletype);
  scm_c_define_gsubr("unbind-key", 1, 0, 0, (scm_t_subr)&scmUnbindKey);
  scm_c_define_gsubr("list-bindings", 0, 0, 0, (scm_t_subr)&scmListBindings);
  scm_c_define_gsubr("buffer-dirty?", 0, 0, 0, (scm_t_subr)&scmBufferDirty);
  scm_c_define_gsubr("set-buffer-dirty!", 1, 0, 0, (scm_t_subr)&scmSetBufferDirty);
  scm_c_define_gsubr("clone-template!", 0, 0, 0, (scm_t_subr)&scmCloneTemplate);
  loadPlugins();
  notes_template_scm = scm_variable_ref(scm_c_lookup("notes_template"));
  notes_template = scm_to_locale_string(notes_template_scm);
  readme_template_scm = scm_variable_ref(scm_c_lookup("readme_template"));
  readme_template = scm_to_locale_string(readme_template_scm);
  if (argc >= 2) {
    editorOpen(argv[1]);
  }
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  return EXIT_SUCCESS;
}


