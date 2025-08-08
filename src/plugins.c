#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <libguile.h>

#include "ze.h"
#include "status.h"
#include "input.h"
#include "row.h"
#include "edit.h"
#include "fileio.h"
#include "render.h"
#include "syntax.h"

static SCM key_bindings[256];
static char *key_specs[256];

// Hook registry (optional; falls back to global Scheme names if not set)
static SCM hook_preDirOpen = SCM_BOOL_F;
static SCM hook_postDirOpen = SCM_BOOL_F;
static SCM hook_preFileOpen = SCM_BOOL_F;
static SCM hook_postFileOpen = SCM_BOOL_F;
static SCM hook_preSave = SCM_BOOL_F;
static SCM hook_postSave = SCM_BOOL_F;

SCM pluginsGetHook(const char *name) {
  if (name == NULL) return SCM_BOOL_F;
  if (strcmp(name, "preDirOpenHook") == 0) return hook_preDirOpen;
  if (strcmp(name, "postDirOpenHook") == 0) return hook_postDirOpen;
  if (strcmp(name, "preFileOpenHook") == 0) return hook_preFileOpen;
  if (strcmp(name, "postFileOpenHook") == 0) return hook_postFileOpen;
  if (strcmp(name, "preSaveHook") == 0) return hook_preSave;
  if (strcmp(name, "postSaveHook") == 0) return hook_postSave;
  return SCM_BOOL_F;
}

static int parse_keyspec(const char *spec, unsigned char *out_code) {
  if (spec == NULL || out_code == NULL) return 0;
  size_t len = strlen(spec);
  if (len == 3 && (spec[0] == 'C' || spec[0] == 'c') && spec[1] == '-') {
    unsigned char letter = (unsigned char)spec[2];
    if (isalpha(letter)) {
      *out_code = (unsigned char)CTRL_KEY(tolower(letter));
      return 1;
    }
  } else if (len == 1) {
    *out_code = (unsigned char)spec[0];
    return 1;
  }
  return 0;
}

int pluginsHandleKey(unsigned char code) {
  SCM proc = key_bindings[code];
  if (scm_is_true(proc) && scm_is_true(scm_procedure_p(proc))) {
    scm_call_0(proc);
    return 1;
  }
  return 0;
}

void initKeyBindings(void) {
  for (int i = 0; i < 256; i++) {
    key_bindings[i] = SCM_BOOL_F;
    key_specs[i] = NULL;
  }
}

void loadPlugins(void) {
  const char *home_dir = getenv("HOME");
  if (home_dir == NULL) return;
  char plugins_dir[PATH_MAX];
  snprintf(plugins_dir, sizeof(plugins_dir), "%s/.ze/plugins", home_dir);
  struct stat st;
  if (stat(plugins_dir, &st) != 0 || !(st.st_mode & S_IFDIR)) {
    return;
  }
  struct dirent **entries;
  int num_entries = scandir(plugins_dir, &entries, NULL, alphasort);
  if (num_entries < 0) return;
  for (int i = 0; i < num_entries; i++) {
    struct dirent *entry = entries[i];
    if (!entry) continue;
    const char *name = entry->d_name;
    if (name[0] == '.') { free(entry); continue; }
    const char *ext = strrchr(name, '.');
    if (ext == NULL || strcmp(ext, ".scm") != 0) { free(entry); continue; }
    char plugin_path[PATH_MAX];
    snprintf(plugin_path, sizeof(plugin_path), "%s/%s", plugins_dir, name);
    scm_c_primitive_load(plugin_path);
    free(entry);
  }
  free(entries);
}

SCM scmBindKey(SCM keySpec, SCM proc) {
  char *spec = scm_to_locale_string(keySpec);
  unsigned char code = 0;
  if (!parse_keyspec(spec, &code)) {
    free(spec);
    return SCM_BOOL_F;
  }
  if (!scm_is_true(scm_procedure_p(proc))) {
    free(spec);
    return SCM_BOOL_F;
  }
  if (scm_is_true(key_bindings[code])) {
    scm_gc_unprotect_object(key_bindings[code]);
  }
  key_bindings[code] = proc;
  scm_gc_protect_object(proc);
  if (key_specs[code]) { free(key_specs[code]); }
  key_specs[code] = strdup(spec);
  free(spec);
  return SCM_BOOL_T;
}

// Utility: convert any Scheme object to a freshly-allocated C string using display semantics
static char *scm_to_display_c_string(SCM obj) {
  SCM port = scm_open_output_string();
  scm_display(obj, port);
  SCM str = scm_get_output_string(port);
  char *out = scm_to_locale_string(str);
  return out; // caller must free
}

// Catch body for evaluating a Scheme string (data is a char* pointing to the code)
static SCM eval_body(void *data) {
  const char *code = (const char *)data;
  return scm_c_eval_string(code);
}

// Catch handler to turn exceptions into a human-readable string result
static SCM eval_handler(void *data, SCM tag, SCM throw_args) {
  (void)data;
  (void)tag;
  SCM port = scm_open_output_string();
  scm_display(scm_from_locale_string("Error: "), port);
  scm_display(throw_args, port);
  return scm_get_output_string(port);
}

void editorExec(void) {
  char *command;
  SCM result_scm;
  char *results;
  command = editorPrompt("scheme@(guile-user)> %s", NULL);
  if (command == NULL) {
    editorSetStatusMessage("Command cancelled");
    return;
  }
  // Evaluate with error handling
  result_scm = scm_c_catch(SCM_BOOL_T, eval_body, (void *)command,
                           eval_handler, NULL, NULL, NULL);
  // Convert result to display string and show it
  results = scm_to_display_c_string(result_scm);
  editorSetStatusMessage(results);
  free(results);
  free(command);
}

// ===== Buffer inspection and mutation =====

extern struct editorConfig E;

static void replace_row_text(erow *row, const char *text, size_t len) {
  if (row == NULL) return;
  free(row->chars);
  row->chars = malloc(len + 1);
  memcpy(row->chars, text, len);
  row->chars[len] = '\0';
  row->size = (int)len;
  editorUpdateRow(row);
  E.dirty++;
}

SCM scmBufferToString(void) {
  int len = 0;
  char *buf = editorRowsToString(&len);
  if (!buf) return scm_from_locale_string("");
  SCM s = scm_from_locale_stringn(buf, (size_t)len);
  free(buf);
  return s;
}

SCM scmBufferLineCount(void) {
  return scm_from_int(E.numrows);
}

SCM scmGetLine(SCM idx_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0 || idx >= E.numrows) return SCM_BOOL_F;
  erow *row = &E.row[idx];
  return scm_from_locale_stringn(row->chars, (size_t)row->size);
}

SCM scmSetLine(SCM idx_scm, SCM str_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0 || idx >= E.numrows) return SCM_BOOL_F;
  char *text = scm_to_locale_string(str_scm);
  replace_row_text(&E.row[idx], text, strlen(text));
  free(text);
  return SCM_BOOL_T;
}

SCM scmInsertLine(SCM idx_scm, SCM str_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0) idx = 0;
  if (idx > E.numrows) idx = E.numrows;
  char *text = scm_to_locale_string(str_scm);
  editorInsertRow(idx, text, strlen(text));
  free(text);
  return SCM_BOOL_T;
}

SCM scmAppendLine(SCM str_scm) {
  char *text = scm_to_locale_string(str_scm);
  editorInsertRow(E.numrows, text, strlen(text));
  free(text);
  return SCM_BOOL_T;
}

SCM scmDeleteLine(SCM idx_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0 || idx >= E.numrows) return SCM_BOOL_F;
  editorDelRow(idx);
  return SCM_BOOL_T;
}

SCM scmInsertText(SCM str_scm) {
  char *text = scm_to_locale_string(str_scm);
  for (size_t i = 0; text[i] != '\0'; i++) {
    if (text[i] == '\n') editorInsertNewline();
    else editorInsertChar((unsigned char)text[i]);
  }
  free(text);
  return SCM_BOOL_T;
}

SCM scmInsertChar(SCM ch_scm) {
  if (scm_is_integer(ch_scm)) {
    int c = scm_to_int(ch_scm);
    editorInsertChar(c);
    return SCM_BOOL_T;
  }
  char *s = scm_to_locale_string(ch_scm);
  if (s[0] != '\0') editorInsertChar((unsigned char)s[0]);
  free(s);
  return SCM_BOOL_T;
}

SCM scmInsertNewline(void) {
  editorInsertNewline();
  return SCM_BOOL_T;
}

SCM scmDeleteChar(void) {
  editorDelChar();
  return SCM_BOOL_T;
}

// ===== Cursor and viewport =====

SCM scmGetCursor(void) {
  SCM x = scm_from_int(E.cx);
  SCM y = scm_from_int(E.cy);
  return scm_list_2(x, y);
}

SCM scmSetCursor(SCM x_scm, SCM y_scm) {
  int x = scm_to_int(x_scm);
  int y = scm_to_int(y_scm);
  if (y < 0) y = 0;
  if (y > E.numrows) y = E.numrows;
  E.cy = y;
  int rowlen = (E.cy >= E.numrows) ? 0 : E.row[E.cy].size;
  if (x < 0) x = 0;
  if (x > rowlen) x = rowlen;
  E.cx = x;
  return SCM_BOOL_T;
}

SCM scmMoveCursor(SCM dir_scm) {
  char *dir = scm_to_locale_string(dir_scm);
  if (strcasecmp(dir, "left") == 0) editorMoveCursor(ARROW_LEFT);
  else if (strcasecmp(dir, "right") == 0) editorMoveCursor(ARROW_RIGHT);
  else if (strcasecmp(dir, "up") == 0) editorMoveCursor(ARROW_UP);
  else if (strcasecmp(dir, "down") == 0) editorMoveCursor(ARROW_DOWN);
  free(dir);
  return SCM_BOOL_T;
}

SCM scmScreenSize(void) {
  return scm_list_2(scm_from_int(E.screenrows), scm_from_int(E.screencols));
}

// ===== File I/O and filenames =====

SCM scmOpenFile(SCM path_scm) {
  char *path = scm_to_locale_string(path_scm);
  editorOpen(path);
  free(path);
  return SCM_BOOL_T;
}

SCM scmSaveFile(void) {
  editorSave();
  return SCM_BOOL_T;
}

SCM scmGetFilename(void) {
  if (E.filename == NULL) return SCM_BOOL_F;
  return scm_from_locale_string(E.filename);
}

SCM scmSetFilename(SCM path_scm) {
  char *path = scm_to_locale_string(path_scm);
  if (E.filename) free(E.filename);
  E.filename = strdup(path);
  editorSelectSyntaxHighlight();
  free(path);
  return SCM_BOOL_T;
}

// ===== Prompts and UI =====

SCM scmPrompt(SCM msg_scm) {
  char *msg = scm_to_locale_string(msg_scm);
  char *resp = editorPrompt(msg, NULL);
  free(msg);
  if (resp == NULL) return SCM_BOOL_F;
  SCM out = scm_from_locale_string(resp);
  free(resp);
  return out;
}

SCM scmRefreshScreen(void) {
  editorRefreshScreen();
  return SCM_BOOL_T;
}

// ===== Search (non-interactive) =====

SCM scmSearchForward(SCM query_scm) {
  char *query = scm_to_locale_string(query_scm);
  if (!query || query[0] == '\0') { if (query) free(query); return SCM_BOOL_F; }
  int start = E.cy;
  int current = start;
  for (int i = 0; i < E.numrows; i++) {
    current++;
    if (current >= E.numrows) current = 0;
    erow *row = &E.row[current];
    if (!row->render) editorUpdateRow(row);
    char *match = strstr(row->render, query);
    if (match) {
      E.cy = current;
      E.cx = editorRowRxToCx(row, (int)(match - row->render));
      E.rowoff = E.numrows; // force scroll to center-ish on next refresh
      SCM result = scm_list_2(scm_from_int(E.cy), scm_from_int(E.cx));
      free(query);
      return result;
    }
  }
  free(query);
  return SCM_BOOL_F;
}

// ===== Syntax highlighting =====

SCM scmSelectSyntaxForFilename(SCM path_scm) {
  char *path = scm_to_locale_string(path_scm);
  char *old = E.filename ? strdup(E.filename) : NULL;
  if (E.filename) { free(E.filename); }
  E.filename = strdup(path);
  editorSelectSyntaxHighlight();
  if (old) { free(old); }
  free(path);
  return SCM_BOOL_T;
}

SCM scmGetFiletype(void) {
  if (E.syntax == NULL || E.syntax->filetype == NULL) return SCM_BOOL_F;
  return scm_from_locale_string(E.syntax->filetype);
}

// ===== Keymap management =====

SCM scmUnbindKey(SCM keySpec) {
  char *spec = scm_to_locale_string(keySpec);
  unsigned char code = 0;
  if (!parse_keyspec(spec, &code)) { free(spec); return SCM_BOOL_F; }
  if (scm_is_true(key_bindings[code])) {
    scm_gc_unprotect_object(key_bindings[code]);
  }
  key_bindings[code] = SCM_BOOL_F;
  if (key_specs[code]) { free(key_specs[code]); key_specs[code] = NULL; }
  free(spec);
  return SCM_BOOL_T;
}

SCM scmListBindings(void) {
  SCM list = SCM_EOL;
  for (int i = 255; i >= 0; i--) {
    if (scm_is_true(key_bindings[i])) {
      SCM key = key_specs[i] ? scm_from_locale_string(key_specs[i]) : scm_from_int(i);
      SCM pair = scm_cons(key, key_bindings[i]);
      list = scm_cons(pair, list);
    }
  }
  return list;
}

// ===== Dirty state =====

SCM scmBufferDirty(void) {
  return scm_from_bool(E.dirty != 0);
}

SCM scmSetBufferDirty(SCM bool_scm) {
  E.dirty = scm_is_true(bool_scm) ? 1 : 0;
  return SCM_BOOL_T;
}

// ===== Templates (simple wrapper) =====

SCM scmCloneTemplate(void) {
  editorCloneTemplate();
  return SCM_BOOL_T;
}


