#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libguile.h>

#include "ze.h"

extern struct editorConfig E;

void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

void scmEditorSetStatusMessage(SCM message) {
  char *fmt = scm_to_locale_string(message);
  strncpy(E.statusmsg, fmt, sizeof(E.statusmsg) - 1);
  E.statusmsg[sizeof(E.statusmsg) - 1] = '\0';
  free(fmt);
  E.statusmsg_time = time(NULL);
}


