#include <libguile.h>
#include <string.h>

#include "ze.h"
#include "status.h"
#include "fileio.h"

extern struct editorConfig E;

void preDirOpenHook(void) {
  SCM preDirOpenHook;
  SCM results_scm;
  char* results;
  preDirOpenHook = scm_variable_ref(scm_c_lookup("preDirOpenHook"));
  results_scm = scm_call_1(preDirOpenHook, scm_from_locale_string(E.filename));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void postDirOpenHook(int num_files) {
  SCM postDirOpenHook;
  SCM results_scm;
  char *results;
  postDirOpenHook = scm_variable_ref(scm_c_lookup("postDirOpenHook"));
  results_scm = scm_call_1(postDirOpenHook, scm_from_int(num_files));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void preFileOpenHook(void) {
  SCM preFileOpenHook;
  SCM results_scm;
  char *results;
  preFileOpenHook = scm_variable_ref(scm_c_lookup("preFileOpenHook"));
  results_scm = scm_call_1(preFileOpenHook, scm_from_locale_string(E.filename));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void postFileOpenHook(void) {
  SCM postFileOpenHook;
  SCM results_scm;
  char* results;
  int len;
  char *contents = editorRowsToString(&len);
  postFileOpenHook = scm_variable_ref(scm_c_lookup("postFileOpenHook"));
  results_scm = scm_call_1(postFileOpenHook, scm_from_locale_string(contents));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void editorPreSaveHook(void) {
  SCM preSaveHook;
  SCM results_scm;
  char* results;
  int len;
  char *contents = editorRowsToString(&len);
  preSaveHook = scm_variable_ref(scm_c_lookup("preSaveHook"));
  results_scm = scm_call_1(preSaveHook, scm_from_locale_string(contents));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void editorPostSaveHook(void) {
  SCM postSaveHook;
  SCM results_scm;
  char* results;
  int len;
  char *contents = editorRowsToString(&len);
  postSaveHook = scm_variable_ref(scm_c_lookup("postSaveHook"));
  results_scm = scm_call_1(postSaveHook, scm_from_locale_string(contents));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}


