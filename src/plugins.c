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

static SCM key_bindings[256];

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
  free(spec);
  if (!scm_is_true(scm_procedure_p(proc))) {
    return SCM_BOOL_F;
  }
  key_bindings[code] = proc;
  return SCM_BOOL_T;
}

void editorExec(void) {
  char *command;
  SCM results_scm;
  char *results;
  command = editorPrompt("scheme@(guile-user)> %s", NULL);
  if (command == NULL) {
    editorSetStatusMessage("Command cancelled");
    return;
  }
  results_scm = scm_c_eval_string(command);
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}


