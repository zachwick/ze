#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "ze.h"
#include "row.h"
#include "status.h"
#include "syntax.h"
#include "hooks.h"
#include "templates.h"
#include "input.h"
#include "init.h"

extern struct editorConfig E;

char* editorRowsToString(int *buflen) {
  int totlen = 0;
  for (int j = 0; j < E.numrows; j++) {
    totlen += E.row[j].size + 1;
  }
  *buflen = totlen;
  char *buf = malloc(totlen);
  char *p = buf;
  for (int j = 0; j < E.numrows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }
  return buf;
}

void editorCloneTemplate(void) {
  FILE *templateFile = NULL;
  char *template = editorPrompt("Select Template: (N)otes | (R)eadme %s", NULL);
  if (template == NULL) {
    editorSetStatusMessage("Template selection cancelled");
    return;
  }
  if (strcasecmp(template, "n") == 0) {
    editorSetStatusMessage("Load Notes template");
    templateFile = fopen(notes_template, "r");
  } else if (strcasecmp(template, "r") == 0) {
    editorSetStatusMessage("Load README template");
    templateFile = fopen(readme_template, "r");
  } else {
    editorSetStatusMessage("Template not found");
    return;
  }
  if (!templateFile) {
    editorSetStatusMessage("Error opening template");
    return;
  }
  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  while ((linelen = getline(&line, &linecap, templateFile)) != -1) {
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }
    editorInsertRow(E.numrows, line, (size_t)linelen);
  }
  free(line);
  fclose(templateFile);
  E.dirty = 0;
}

static int _true_selector(const struct dirent *empty) {
  (void)empty;
  return 1;
}

void editorOpen(char *filename) {
  if (filename == NULL) {
    filename = editorPrompt("Path to open: (ESC to cancel) %s", NULL);
    if (filename == NULL) {
      editorSetStatusMessage("Open cancelled");
      return;
    }
    initEditor();
  } else if (E.filename != NULL) {
    free(E.filename);
  }

  E.filename = strdup(filename);
  editorSelectSyntaxHighlight();

  FILE *fp = NULL;
  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  struct stat s;

  if (stat(E.filename, &s) == 0) {
    if (s.st_mode & S_IFDIR) {
      preDirOpenHook();
      struct dirent **dits;
      int num_files = scandir(E.filename, &dits, _true_selector, alphasort);
      if (num_files >= 0) {
        for (int count = 0; count < num_files; ++count) {
          editorInsertRow(count, dits[count]->d_name, strlen(dits[count]->d_name));
        }
        for (int count = 0; count < num_files; ++count) {
          free(dits[count]);
        }
        free(dits);
      } else {
        perror("Error opening directory");
      }
      postDirOpenHook(num_files);
      return;
    } else if (s.st_mode & S_IFREG) {
      preFileOpenHook();
      fp = fopen(filename, "r");
      if (!fp) {
        editorSetStatusMessage("Error opening specified file");
        return;
      }
      while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
          linelen--;
        }
        editorInsertRow(E.numrows, line, (size_t)linelen);
      }
      postFileOpenHook();
    } else {
      editorSetStatusMessage("Unknown object at filepath");
      return;
    }
  } else {
    editorSetStatusMessage("Error determining type of object at filepath");
    return;
  }

  free(line);
  fclose(fp);
  E.dirty = 0;
}

void editorSave(void) {
  if (E.filename == NULL) {
    E.filename = editorPrompt("Save as: (ESC to cancel) %s", NULL);
    if (E.filename == NULL) {
      editorSetStatusMessage("Save aborted");
      return;
    }
    editorSelectSyntaxHighlight();
  }
  editorPreSaveHook();
  int len;
  char *buf = editorRowsToString(&len);
  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) == len) {
        close(fd);
        free(buf);
        E.dirty = 0;
        editorSetStatusMessage("%d bytes written to disk", len);
        editorPostSaveHook();
        return;
      }
    }
    close(fd);
  }
  free(buf);
  editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}


