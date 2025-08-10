/**
 * @file plugins.c
 * @brief Guile/Scheme plugin and keybinding implementation.
 * @ingroup plugins
 */
#include "ze.h"

#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libguile.h>
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

/**
 * @brief Fetch a registered hook by name.
 * @ingroup plugins
 *
 * Recognized names: "preDirOpenHook", "postDirOpenHook",
 * "preFileOpenHook", "postFileOpenHook", "preSaveHook", "postSaveHook".
 *
 * @param name C string name of the hook.
 * @return Scheme object for the hook or \c SCM_BOOL_F if unset/unknown.
 */
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

/**
 * @brief If a Scheme binding exists for the key, invoke it.
 * @ingroup plugins
 * @param code Key code.
 * @return 1 if a binding was invoked, 0 otherwise.
 */
int pluginsHandleKey(unsigned char code) {
  SCM proc = key_bindings[code];
  if (scm_is_true(proc) && scm_is_true(scm_procedure_p(proc))) {
    scm_call_0(proc);
    return 1;
  }
  return 0;
}

/**
 * @brief Initialize the in-memory key binding table.
 * @ingroup plugins
 */
void initKeyBindings(void) {
  for (int i = 0; i < 256; i++) {
    key_bindings[i] = SCM_BOOL_F;
    key_specs[i] = NULL;
  }
}

/**
 * @brief Load Scheme plugins from "$HOME/.ze/plugins" (all .scm files).
 * @ingroup plugins
 */
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

/**
 * @brief Bind a key specification to a Scheme procedure.
 * @ingroup plugins
 * @note Scheme procedure: bind-key key-spec proc
 * @param keySpec Scheme string like "C-x", "C-s", or "a".
 * @param proc Scheme procedure to call when the key is pressed.
 * @return \c SCM_BOOL_T on success, \c SCM_BOOL_F on failure.
 */
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

/**
 * @brief Prompt for and evaluate a single Scheme expression (mini-REPL).
 * @ingroup plugins
 */
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

/**
 * @brief Return the entire buffer as a Scheme string.
 * @ingroup plugins
 * @note Scheme procedure: buffer->string
 * @return Scheme string containing the full buffer contents.
 */
SCM scmBufferToString(void) {
  int len = 0;
  char *buf = editorRowsToString(&len);
  if (!buf) return scm_from_locale_string("");
  SCM s = scm_from_locale_stringn(buf, (size_t)len);
  free(buf);
  return s;
}

/**
 * @brief Get the number of lines in the current buffer.
 * @ingroup plugins
 * @note Scheme procedure: buffer-line-count
 * @return Scheme integer line count.
 */
SCM scmBufferLineCount(void) {
  return scm_from_int(E.numrows);
}

/**
 * @brief Get the contents of a line by index.
 * @ingroup plugins
 * @note Scheme procedure: get-line idx
 * @param idx_scm Scheme integer line index (0-based).
 * @return Scheme string for the line, or \c SCM_BOOL_F if out of range.
 */
SCM scmGetLine(SCM idx_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0 || idx >= E.numrows) return SCM_BOOL_F;
  erow *row = &E.row[idx];
  return scm_from_locale_stringn(row->chars, (size_t)row->size);
}

/**
 * @brief Replace the contents of a line.
 * @ingroup plugins
 * @note Scheme procedure: set-line! idx text
 * @param idx_scm Scheme integer line index (0-based).
 * @param str_scm Scheme string new contents.
 * @return \c SCM_BOOL_T on success, \c SCM_BOOL_F if index is out of range.
 */
SCM scmSetLine(SCM idx_scm, SCM str_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0 || idx >= E.numrows) return SCM_BOOL_F;
  char *text = scm_to_locale_string(str_scm);
  replace_row_text(&E.row[idx], text, strlen(text));
  free(text);
  return SCM_BOOL_T;
}

/**
 * @brief Insert a new line at index.
 * @ingroup plugins
 * @note Scheme procedure: insert-line! idx text
 * @param idx_scm Scheme integer index (clamped to [0, line-count]).
 * @param str_scm Scheme string new line text (without trailing newline).
 * @return \c SCM_BOOL_T.
 */
SCM scmInsertLine(SCM idx_scm, SCM str_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0) idx = 0;
  if (idx > E.numrows) idx = E.numrows;
  char *text = scm_to_locale_string(str_scm);
  editorInsertRow(idx, text, strlen(text));
  free(text);
  return SCM_BOOL_T;
}

/**
 * @brief Append a new line at the end of the buffer.
 * @ingroup plugins
 * @note Scheme procedure: append-line! text
 * @param str_scm Scheme string new line text.
 * @return \c SCM_BOOL_T.
 */
SCM scmAppendLine(SCM str_scm) {
  char *text = scm_to_locale_string(str_scm);
  editorInsertRow(E.numrows, text, strlen(text));
  free(text);
  return SCM_BOOL_T;
}

/**
 * @brief Delete a line by index.
 * @ingroup plugins
 * @note Scheme procedure: delete-line! idx
 * @param idx_scm Scheme integer line index (0-based).
 * @return \c SCM_BOOL_T on success, \c SCM_BOOL_F if out of range.
 */
SCM scmDeleteLine(SCM idx_scm) {
  int idx = scm_to_int(idx_scm);
  if (idx < 0 || idx >= E.numrows) return SCM_BOOL_F;
  editorDelRow(idx);
  return SCM_BOOL_T;
}

/**
 * @brief Insert text at the cursor position, interpreting newlines.
 * @ingroup plugins
 * @note Scheme procedure: insert-text! text
 * @param str_scm Scheme string text (may contain newlines).
 * @return \c SCM_BOOL_T.
 */
SCM scmInsertText(SCM str_scm) {
  char *text = scm_to_locale_string(str_scm);
  for (size_t i = 0; text[i] != '\0'; i++) {
    if (text[i] == '\n') editorInsertNewline();
    else editorInsertChar((unsigned char)text[i]);
  }
  free(text);
  return SCM_BOOL_T;
}

/**
 * @brief Insert a single character at the cursor.
 * @ingroup plugins
 * @note Scheme procedure: insert-char! ch
 * @param ch_scm Either a Scheme integer char code or a one-char string.
 * @return \c SCM_BOOL_T.
 */
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

/**
 * @brief Insert a newline at the cursor position.
 * @ingroup plugins
 * @note Scheme procedure: insert-newline!
 * @return \c SCM_BOOL_T.
 */
SCM scmInsertNewline(void) {
  editorInsertNewline();
  return SCM_BOOL_T;
}

/**
 * @brief Delete the character to the left of the cursor (backspace).
 * @ingroup plugins
 * @note Scheme procedure: delete-char!
 * @return \c SCM_BOOL_T.
 */
SCM scmDeleteChar(void) {
  editorDelChar();
  return SCM_BOOL_T;
}

// ===== Cursor and viewport =====

/**
 * @brief Get current cursor position.
 * @ingroup plugins
 * @note Scheme procedure: get-cursor
 * @return Scheme pair (x . y) in editor column/line coordinates.
 */
SCM scmGetCursor(void) {
  SCM x = scm_from_int(E.cx);
  SCM y = scm_from_int(E.cy);
  return scm_list_2(x, y);
}

/**
 * @brief Set cursor position, clamped to the current buffer.
 * @ingroup plugins
 * @note Scheme procedure: set-cursor! x y
 * @param x_scm Scheme integer column.
 * @param y_scm Scheme integer line.
 * @return \c SCM_BOOL_T.
 */
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

/**
 * @brief Move the cursor in a direction.
 * @ingroup plugins
 * @note Scheme procedure: move-cursor! dir
 * @param dir_scm Scheme string: "left", "right", "up", or "down".
 * @return \c SCM_BOOL_T.
 */
SCM scmMoveCursor(SCM dir_scm) {
  char *dir = scm_to_locale_string(dir_scm);
  if (strcasecmp(dir, "left") == 0) editorMoveCursor(ARROW_LEFT);
  else if (strcasecmp(dir, "right") == 0) editorMoveCursor(ARROW_RIGHT);
  else if (strcasecmp(dir, "up") == 0) editorMoveCursor(ARROW_UP);
  else if (strcasecmp(dir, "down") == 0) editorMoveCursor(ARROW_DOWN);
  free(dir);
  return SCM_BOOL_T;
}

/**
 * @brief Get the editor screen size in rows and columns.
 * @ingroup plugins
 * @note Scheme procedure: screen-size
 * @return Scheme pair (rows . cols).
 */
SCM scmScreenSize(void) {
  return scm_list_2(scm_from_int(E.screenrows), scm_from_int(E.screencols));
}

// ===== File I/O and filenames =====

/**
 * @brief Open a file into the editor.
 * @ingroup plugins
 * @note Scheme procedure: open-file! path
 * @param path_scm Scheme string path.
 * @return \c SCM_BOOL_T.
 */
SCM scmOpenFile(SCM path_scm) {
  char *path = scm_to_locale_string(path_scm);
  editorOpen(path);
  free(path);
  return SCM_BOOL_T;
}

/**
 * @brief Save the current buffer to disk.
 * @ingroup plugins
 * @note Scheme procedure: save-file!
 * @return \c SCM_BOOL_T.
 */
SCM scmSaveFile(void) {
  editorSave();
  return SCM_BOOL_T;
}

/**
 * @brief Get the current filename, if any.
 * @ingroup plugins
 * @note Scheme procedure: get-filename
 * @return Scheme string filename, or \c SCM_BOOL_F if unnamed.
 */
SCM scmGetFilename(void) {
  if (E.filename == NULL) return SCM_BOOL_F;
  return scm_from_locale_string(E.filename);
}

/**
 * @brief Set the current filename and reselect syntax highlighting.
 * @ingroup plugins
 * @note Scheme procedure: set-filename! path
 * @param path_scm Scheme string path.
 * @return \c SCM_BOOL_T.
 */
SCM scmSetFilename(SCM path_scm) {
  char *path = scm_to_locale_string(path_scm);
  if (E.filename) free(E.filename);
  E.filename = strdup(path);
  editorSelectSyntaxHighlight();
  free(path);
  return SCM_BOOL_T;
}

// ===== Prompts and UI =====

/**
 * @brief Prompt the user and return their input.
 * @ingroup plugins
 * @note Scheme procedure: prompt message
 * @param msg_scm Scheme string prompt message (printf-style not supported).
 * @return Scheme string response, or \c SCM_BOOL_F on cancel.
 */
SCM scmPrompt(SCM msg_scm) {
  char *msg = scm_to_locale_string(msg_scm);
  char *resp = editorPrompt(msg, NULL);
  free(msg);
  if (resp == NULL) return SCM_BOOL_F;
  SCM out = scm_from_locale_string(resp);
  free(resp);
  return out;
}

/**
 * @brief Force a screen refresh.
 * @ingroup plugins
 * @note Scheme procedure: refresh-screen!
 * @return \c SCM_BOOL_T.
 */
SCM scmRefreshScreen(void) {
  editorRefreshScreen();
  return SCM_BOOL_T;
}

// ===== Search (non-interactive) =====

/**
 * @brief Search forward for a substring and jump to the next match.
 * @ingroup plugins
 * @note Scheme procedure: search-forward! query
 * @param query_scm Scheme string to search for.
 * @return Pair (y . x) of the match position, or \c SCM_BOOL_F if not found.
 */
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

/**
 * @brief Select syntax based on a filename, without opening the file.
 * @ingroup plugins
 * @note Scheme procedure: select-syntax-for-filename! path
 * @param path_scm Scheme string path whose extension drives syntax.
 * @return \c SCM_BOOL_T.
 */
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

/**
 * @brief Get the current filetype name from syntax highlighting.
 * @ingroup plugins
 * @note Scheme procedure: get-filetype
 * @return Scheme string filetype, or \c SCM_BOOL_F if none.
 */
SCM scmGetFiletype(void) {
  if (E.syntax == NULL || E.syntax->filetype == NULL) return SCM_BOOL_F;
  return scm_from_locale_string(E.syntax->filetype);
}

// ===== Keymap management =====

/**
 * @brief Remove a key binding.
 * @ingroup plugins
 * @note Scheme procedure: unbind-key key-spec
 * @param keySpec Scheme string key specification (e.g., "C-x", "a").
 * @return \c SCM_BOOL_T.
 */
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

/**
 * @brief List all key bindings.
 * @ingroup plugins
 * @note Scheme procedure: list-bindings
 * @return Scheme list of pairs (key . procedure).
 */
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

/**
 * @brief Check whether the current buffer has unsaved changes.
 * @ingroup plugins
 * @note Scheme procedure: buffer-dirty?
 * @return Scheme boolean.
 */
SCM scmBufferDirty(void) {
  return scm_from_bool(E.dirty != 0);
}

/**
 * @brief Mark the current buffer dirty/clean.
 * @ingroup plugins
 * @note Scheme procedure: set-buffer-dirty! bool
 * @param bool_scm Scheme boolean.
 * @return \c SCM_BOOL_T.
 */
SCM scmSetBufferDirty(SCM bool_scm) {
  E.dirty = scm_is_true(bool_scm) ? 1 : 0;
  return SCM_BOOL_T;
}

// ===== Templates (simple wrapper) =====

/**
 * @brief Clone a template into the current buffer (interactive selection).
 * @ingroup plugins
 * @note Scheme procedure: clone-template!
 * @return \c SCM_BOOL_T.
 */
SCM scmCloneTemplate(void) {
  editorCloneTemplate();
  return SCM_BOOL_T;
}


