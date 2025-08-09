/**
 * @file status.c
 * @brief Status bar message helpers.
 * @ingroup status
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libguile.h>

#include "ze.h"

extern struct editorConfig E;

/**
 * @brief Set the transient status message using printf-style formatting.
 * @ingroup status
 *
 * Formats into @c E.statusmsg and records the current time for display in the
 * message bar.
 *
 * @param[in] fmt printf-style format string.
 * @param[in] ... Format arguments.
 * @post Status message and timestamp are updated.
 * @sa editorDrawMessageBar()
 */
void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

/**
 * @brief Set the status message from a Scheme string.
 * @ingroup status
 *
 * Converts @p message to a C string, copies into @c E.statusmsg (clamped to
 * buffer size), and updates the timestamp.
 *
 * Ownership: the temporary C string is freed before return; @p message remains owned by Guile.
 *
 * @param[in] message Scheme string to display.
 * @post Status message and timestamp are updated.
 * @sa editorSetStatusMessage()
 */
void scmEditorSetStatusMessage(SCM message) {
  char *fmt = scm_to_locale_string(message);
  strncpy(E.statusmsg, fmt, sizeof(E.statusmsg) - 1);
  E.statusmsg[sizeof(E.statusmsg) - 1] = '\0';
  free(fmt);
  E.statusmsg_time = time(NULL);
}


