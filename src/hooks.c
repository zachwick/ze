/**
 * @file hooks.c
 * @brief Scheme hook invocation implementations.
 * @ingroup hooks
 */
#include <libguile.h>
#include <string.h>

#include "ze.h"
#include "status.h"
#include "fileio.h"

extern struct editorConfig E;

/**
 * @brief Invoke the Scheme @c preDirOpenHook with the directory path.
 * @ingroup hooks
 *
 * Calls the Guile function @c preDirOpenHook with @c E.filename as a Scheme
 * string and displays its returned message in the status bar.
 *
 * @post Status message is updated via editorSetStatusMessage().
 * @sa postDirOpenHook(), preFileOpenHook(), editorOpen()
 */
void preDirOpenHook(void) {
  SCM preDirOpenHook;
  SCM results_scm;
  char* results;
  preDirOpenHook = scm_variable_ref(scm_c_lookup("preDirOpenHook"));
  results_scm = scm_call_1(preDirOpenHook, scm_from_locale_string(E.filename));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

/**
 * @brief Invoke the Scheme @c postDirOpenHook with the file count.
 * @ingroup hooks
 *
 * Calls @c postDirOpenHook with the number of directory entries scanned and
 * displays its returned message.
 *
 * @param[in] num_files Number of entries returned by scandir(). Must be >= 0.
 * @post Status message is updated.
 * @sa preDirOpenHook(), editorOpen()
 */
void postDirOpenHook(int num_files) {
  SCM postDirOpenHook;
  SCM results_scm;
  char *results;
  postDirOpenHook = scm_variable_ref(scm_c_lookup("postDirOpenHook"));
  results_scm = scm_call_1(postDirOpenHook, scm_from_int(num_files));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

/**
 * @brief Invoke the Scheme @c preFileOpenHook with the filename.
 * @ingroup hooks
 *
 * Calls @c preFileOpenHook with @c E.filename and displays the returned
 * message.
 *
 * @post Status message is updated.
 * @sa postFileOpenHook(), editorOpen()
 */
void preFileOpenHook(void) {
  SCM preFileOpenHook;
  SCM results_scm;
  char *results;
  preFileOpenHook = scm_variable_ref(scm_c_lookup("preFileOpenHook"));
  results_scm = scm_call_1(preFileOpenHook, scm_from_locale_string(E.filename));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

/**
 * @brief Invoke the Scheme @c postFileOpenHook with current buffer contents.
 * @ingroup hooks
 *
 * Serializes the buffer with editorRowsToString() and passes it to
 * @c postFileOpenHook. The returned Scheme string is shown in the status bar.
 *
 * Ownership: the temporary buffer from editorRowsToString() is freed in Guile
 * after conversion; no NUL terminator is guaranteed—length is respected.
 *
 * @post Status message is updated.
 * @sa preFileOpenHook(), editorRowsToString(), editorOpen()
 */
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

/**
 * @brief Invoke the Scheme @c preSaveHook with current buffer contents.
 * @ingroup hooks
 *
 * Serializes the buffer and passes it to @c preSaveHook, displaying the
 * returned message.
 *
 * @post Status message is updated.
 * @sa editorRowsToString(), editorSave(), editorPostSaveHook()
 */
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

/**
 * @brief Invoke the Scheme @c postSaveHook with current buffer contents.
 * @ingroup hooks
 *
 * Serializes the buffer and passes it to @c postSaveHook, displaying the
 * returned message.
 *
 * @post Status message is updated.
 * @sa editorRowsToString(), editorSave(), editorPreSaveHook()
 */
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


