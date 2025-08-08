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


