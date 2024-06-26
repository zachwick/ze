/**
 * ze - zwick's editor of choice
 *
 * Copyright 2018, 2019, 2020 zach wick <zach@zachwick.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <libguile.h>

/*** defines ***/

#define ZE_VERSION "0.0.1"
#define ZE_TAB_STOP 2
#define ZE_QUIT_TIMES 1
#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
  ARROW_LEFT = CTRL_KEY('b'),
  ARROW_RIGHT = CTRL_KEY('f'),
  ARROW_UP = CTRL_KEY('p'),
  ARROW_DOWN = CTRL_KEY('n'),
  PAGE_UP = CTRL_KEY('g'),
  PAGE_DOWN = CTRL_KEY('v'),
  HOME_KEY = CTRL_KEY('a'),
  END_KEY = CTRL_KEY('e'),
  BACKSPACE = 127
  //DEL_KEY
};

enum editorHighlight {
  HL_NORMAL = 0,
  HL_COMMENT,
  HL_MLCOMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_STRING,
  HL_NUMBER,
  HL_MATCH
};

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

static int
_true (const struct dirent *empty) {
	// In order to fix a compiler warning, we need to make sure that the
	// signature of our _true function matches what the scandir function
	// expects. Therefore, we have to accept an argument that we don't use.
  (void)empty;
	return 1;
}

/*** data ***/

// If you are adding additional templates, make sure to define a new variable
// here, and load the relevant Guile variable from your zerc.scm config file
// by adding the needed two lines in the `main` method.
char* notes_template = "";
char* readme_template = "";

struct editorSyntax {
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;
  char *multiline_comment_start;
  char *multiline_comment_end;
  int flags;
};

typedef struct erow {
  int idx;
  int size;
  int rsize;
  char *chars;
  char *render;
  unsigned char *hl;
  int hl_open_comment;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  erow *row;
  int dirty;
  char *filename;
  char statusmsg[150];
  time_t statusmsg_time;
  struct editorSyntax *syntax;
  struct termios orig_termios;
};

struct editorConfig E;

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}
/*** filetypes ***/

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
char *Python_HL_extensions[] = {".py", NULL};
char *Ruby_HL_extensions[] = {".rb", ".erb", NULL};
char *PHP_HL_extensions[] = {".php", NULL};
char *Rust_HL_extensions[] = {".rs", NULL};
char *APL_HL_extensions[] = {".apl", NULL};
char *Swift_HL_extensions[] = {".swift", NULL};

char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case", "NULL",

  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", "#define|", "#include|", NULL
};
char *Python_HL_keywords[] = {
  "and", "as", "assert", "break", "class", "continue", "def", "del", "elif",
  "else", "except", "finally", "for", "from", "global", "if", "import", "in",
  "is", "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
  "while", "with", "yield",

  "False|", "None|", "True|", NULL
};
char *Ruby_HL_keywords[] = {
  "alias", "and", "begin", "break", "case", "class", "def", "defined?", "do",
  "else", "elsif", "end", "ensure", "false", "for", "if", "in", "module", "next",
  "nil", "not", "or", "redo", "rescue", "retry", "return", "self", "super", "then",
  "true", "undef", "unless", "until", "when", "while", "yield",

  "__ENCODING__|", "__LINE__|", "__FILE__|", "BEGIN|", "END|", NULL
};
char *PHP_HL_keywords[] = {
  "__halt_compiler", "abstract", "and", "array", "as", "break", "callable",
  "case", "catch", "class", "clone", "const", "continue", "declare", "default",
  "die", "do", "echo", "else", "elseif", "empty", "enddeclare", "endfor",
  "endforeach", "endif", "endswitch", "endwhile", "eval", "exit", "extends",
  "final", "for", "foreach", "function", "global", "goto", "if", "implements",
  "include", "include_once", "instanceof", "insteadof", "interface", "isset",
  "list", "namespace", "new", "or", "print", "private", "protected", "public",
  "require", "require_once", "return", "static", "switch", "throw", "trait",
  "try", "unset", "use", "var", "while", "xor",

  "__CLASS__|", "__DIR__|", "__FILE__|", "__FUNCTION__|", "__LINE__|",
  "__METHOD__|", "__NAMESPACE__|", "__TRAIT__|", NULL
};
char *Rust_HL_keywords[] = {
  "_", "abstract", "alignof", "as", "become", "box", "break", "const", "continue",
  "crate", "do", "else", "enum", "extern", "false", "final", "fn", "for", "if",
  "impl", "in", "let", "loop", "macro", "match", "mod", "move", "mut", "offset",
  "override", "priv", "proc", "pub", "pure", "ref", "return", "Self", "self",
  "sizeof", "static", "struct", "super", "trait", "true", "type", "typeof", "unsafe",
  "unsized", "use", "virtual", "where", "while", "yield",

  "derive|", "println!|", "Some|", "unwrap()|", "value_of()|", "next()|", "to_string()|", NULL

};
char *APL_HL_keywords[] = {
  "Public", "Shared",

  ":Class|", ":Access|", ":For|", ":In|", ":EndFor|", ":If|", ":AndIf|", ":EndIf|",
  ":EndClass|", NULL
};

char *Swift_HL_keywords[] = {
  "associatedtype", "class", "deinit", "enum", "extension", "fileprivate", "func", "import",
  "init", "inout", "internal", "let", "open", "operator", "private", "protocol", "public",
  "static", "struct", "subscript", "typealias", "var","break", "case", "continue", "default",
  "defer", "do", "else", "fallthrough", "for", "guard", "if", "in", "repeat", "return",
  "switch", "where", "while,as", "Any", "catch", "false", "is", "nil", "rethrows", "super",
  "self", "Self", "throw", "throws", "true", "try",

  "#available|", "#colorLiteral|", "#column|", "#else|", "#elseif|", "#endif|", "#file|",
  "#fileLiteral|", "#function|", "#if|", "#imageLiteral|", "#line|", "#selector|",
  "#sourceLocation|", "associativity|", "convenience|", "dynamic|", "didSet|", "final|",
  "get|", "infix|", "indirect|", "lazy|", "left|", "mutating|", "none|", "nonmutating|",
  "optional|", "override|", "postfix|", "precedence|", "prefix|", "Protocol|", "required|",
  "right|", "set|", "Type|", "unowned|", "weak|", "willSet|", NULL

};

struct editorSyntax HLDB[] = {
  {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "python",
    Python_HL_extensions,
    Python_HL_keywords,
    "#","'''","'''",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "ruby",
    Ruby_HL_extensions,
    Ruby_HL_keywords,
    "#", "=begin", "=end",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "PHP",
    PHP_HL_extensions,
    PHP_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "Rust",
    Rust_HL_extensions,
    Rust_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "APL",
    APL_HL_extensions,
    APL_HL_keywords,
    "⍝", "⍝", "⍝",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "Swift",
    Swift_HL_extensions,
    Swift_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/*** prototypes ***/

void editorSetStatusMessage(const char *fmt, ...);
void scmEditorSetStatusMessage(SCM message);
void editorRefreshScreen(void);
char *editorPrompt(char *prompt, void (*callback)(char *, int));
void initEditor(void);
void die(const char *s);
void disableRawMode(void);
void enableRawMode(void);
char editorReadKey(void);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight(void);
int editorRowCxToRx(erow *row, int cx);
int editorRowRxToCx(erow *row, int rx);
void editorUpdateRow(erow *row);
void editorInsertRow(int at, char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowAppendString(erow *row, char *s, size_t len);
void editorRowDelChar(erow *row, int at);
void editorDelRowAtChar(erow *row, int at);
void editorInsertChar(int c);
void editorInsertTimestamp(void);
void editorInsertNewline(void);
void editorDelChar(void);
char* editorRowsToString(int *buflen);
void editorCloneTemplate(void);
void preDirOpenHook(void);
void postDirOpenHook(int num_files);
void preFileOpenHook(void);
void editorOpen(char *filename);
void postFileOpenHook(void);
void editorPreSaveHook(void);
void editorPostSaveHook(void);
void editorSave(void);
void editorExec(void);
void editorFindCallback(char *query, int key);
void editorFind(void);
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void editorScroll(void);
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorMoveCursor(char key);
void editorProcessKeypress(void);

/*** terminal ***/

void
die(const char *s)
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void
disableRawMode(void)
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("tcsetattr");
  }
}

void
enableRawMode(void)
{
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
    die("tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

char
editorReadKey(void)
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) {
      die("read");
    }
  }

  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return '\x1b';
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return '\x1b';
    }

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
	if (read(STDIN_FILENO, &seq[2], 1) != 1) {
	  return '\x1b';
	}
	if (seq[2] == '~') {
	  switch (seq[1]) {
	  case '1': return HOME_KEY;
	    /*case '3': return DEL_KEY;*/
	  case '4': return END_KEY;
	  case '5': return PAGE_UP;
	  case '6': return PAGE_DOWN;
	  case '7': return HOME_KEY;
	  case '8': return END_KEY;
	  }
	}
      } else {
	switch (seq[1]) {
	case 'A': return ARROW_UP;
	case 'B': return ARROW_DOWN;
	case 'C': return ARROW_RIGHT;
	case 'D': return ARROW_LEFT;
	case 'H': return HOME_KEY;
	case 'F': return END_KEY;
	}
      }
    } else if (seq[0] == '0') {
      switch (seq[1]) {
      case 'H': return HOME_KEY;
      case 'F': return END_KEY;
      }
    }
    return '\x1b';
  } else {
    return c;
  }
}

int
getCursorPosition(int *rows, int *cols)
{
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    return -1;
  }
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) {
      break;
    }
    if (buf[i] == 'R') {
      break;
    }
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') {
    return -1;
  }
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
    return -1;
  }
  return 0;
}

int
getWindowSize(int *rows, int *cols)
{
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
      return -1;
    }
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** syntax highlighting ***/

int
is_separator(int c)
{
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void
editorUpdateSyntax(erow *row)
{
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);

  if (E.syntax == NULL) {
    return;
  }

  char **keywords = E.syntax->keywords;

  char *scs = E.syntax->singleline_comment_start;
  char *mcs = E.syntax->multiline_comment_start;
  char *mce = E.syntax->multiline_comment_end;

  int scs_len = scs ? strlen(scs) : 0;
  int mcs_len = mcs ? strlen(mcs) : 0;
  int mce_len = mce ? strlen(mce) : 0;

  int prev_sep = 1;
  int in_string = 0;
  int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);

  int i = 0;
  while (i < row->rsize) {
    char c = row->render[i];
    unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

    if (scs_len && !in_string && !in_comment) {
      if (!strncmp(&row->render[i], scs, scs_len)) {
	memset(&row->hl[i], HL_COMMENT, row->rsize - i);
	break;
      }
    }

    if (mcs_len && mce_len && !in_string) {
      if (in_comment) {
	row->hl[i] = HL_MLCOMMENT;
	if (!strncmp(&row->render[i], mce, mce_len)) {
	  memset(&row->hl[i], HL_MLCOMMENT, mce_len);
	  i += mce_len;
	  in_comment = 0;
	  prev_sep = 1;
	  continue;
	} else {
	  i++;
	  continue;
	}
      } else if (!strncmp(&row->render[i], mcs, mcs_len)) {
	memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
	i += mcs_len;
	in_comment = 1;
	continue;
      }
    }

    if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
      if (in_string) {
	row->hl[i] = HL_STRING;
	if (c == '\\' && i + 1 < row->rsize) {
	  row->hl[i + 1] = HL_STRING;
	  i += 2;
	  continue;
	}
	if (c == in_string) {
	  in_string = 0;
	}
	i++;
	prev_sep = 1;
	continue;
      } else {
	if (c == '"' || c == '\'') {
	  in_string = c;
	  row->hl[i] = HL_STRING;
	  i++;
	  continue;
	}
      }
    }

    if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
      if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
	  (c == '.' && prev_hl == HL_NUMBER)) {
	row->hl[i] = HL_NUMBER;
	i++;
	prev_sep = 0;
	continue;
      }
    }

    if (prev_sep) {
      int j;
      for (j = 0; keywords[j]; j++) {
	int klen = strlen(keywords[j]);
	int kw2 = keywords[j][klen - 1] == '|';
	if (kw2) {
	  klen--;
	}

	if (!strncmp(&row->render[i], keywords[j], klen) &&
	    is_separator(row->render[i + klen])) {
	  memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
	  i += klen;
	  break;
	}
      }
      if (keywords[j] != NULL) {
	prev_sep = 0;
	continue;
      }
    }

    prev_sep = is_separator(c);
    i++;
  }

  int changed = (row->hl_open_comment != in_comment);
  row->hl_open_comment = in_comment;
  if (changed && row->idx + 1 < E.numrows) {
    editorUpdateSyntax(&E.row[row->idx + 1]);
  }
}

int
editorSyntaxToColor(int hl)
{
  switch (hl) {
  case HL_COMMENT:
  case HL_MLCOMMENT:
    return 36;
  case HL_KEYWORD1:
    return 33;
  case HL_KEYWORD2:
    return 32;
  case HL_STRING:
    return 35;
  case HL_NUMBER:
    return 31;
  case HL_MATCH:
    return 34;
  default:
    return 37;
  }
}

void
editorSelectSyntaxHighlight(void)
{
  E.syntax = NULL;
  if (E.filename == NULL) {
    return;
  }

  char *ext = strrchr(E.filename, '.');

  for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
    struct editorSyntax *s = &HLDB[j];
    unsigned int i = 0;
    while (s->filematch[i]) {
      int is_ext = (s->filematch[i][0] == '.');
      if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
	  (!is_ext && strstr(E.filename, s->filematch[i]))) {
	E.syntax = s;

	int filerow;
	for (filerow = 0; filerow < E.numrows; filerow++) {
	  editorUpdateSyntax(&E.row[filerow]);
	}

	return;
      }
      i++;
    }
  }
}

/*** row operations ***/

int
editorRowCxToRx(erow *row, int cx)
{
  int rx = 0;
  int j;

  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      rx += (ZE_TAB_STOP - 1) - (rx % ZE_TAB_STOP);
    }
    rx++;
  }

  return rx;
}

int
editorRowRxToCx(erow *row, int rx)
{
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t') {
      cur_rx += (ZE_TAB_STOP - 1) - (cur_rx % ZE_TAB_STOP);
    }
    cur_rx++;

    if (cur_rx > rx) {
      return cx;
    }
  }
  return cx;
}

void
editorUpdateRow(erow *row)
{
  int tabs = 0;
  int j;

  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      tabs++;
    }
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (ZE_TAB_STOP - 1) + 1);


  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % ZE_TAB_STOP != 0) {
	row->render[idx++] = ' ';
      }
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;

  editorUpdateSyntax(row);
}

void
editorInsertRow(int at, char *s, size_t len)
{
  if (at < 0 || at > E.numrows) {
    return;
  }
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));

  for (int j = at + 1; j <= E.numrows; j++) {
    E.row[j].idx++;
  }

  E.row[at].idx = at;

  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  E.row[at].hl = NULL;
  E.row[at].hl_open_comment = 0;
  editorUpdateRow(&E.row[at]);

  E.numrows++;
  E.dirty++;
}

void
editorFreeRow(erow *row)
{
  free(row->render);
  free(row->chars);
  free(row->hl);
}

void
editorDelRow(int at)
{
  if (at < 0 || at >= E.numrows) {
    return;
  }
  editorFreeRow(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
  for (int j = 0; j < E.numrows - 1; j++) {
    E.row[j].idx--;
  }
  E.numrows--;
  E.dirty++;
}

void
editorRowInsertChar(erow *row, int at, int c)
{
  if (at < 0 || at > row->size) {
    at = row->size;
  }
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  editorUpdateRow(row);
  E.dirty++;
}

void
editorRowAppendString(erow *row, char *s, size_t len)
{
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  editorUpdateRow(row);
  E.dirty++;
}

void
editorRowDelChar(erow *row, int at)
{
  if (at < 0 || at >= row->size) {
    return;
  }
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}

void
editorDelRowAtChar(erow *row, int at)
{
  if (at < 0 || at >= row->size) {
    return;
  }
  int len_diff = row->size - at - 1;
  for (int i = at - 1; i < len_diff; i++)
    {
      editorRowDelChar(row, i);
    }
  row->size = at;
  editorUpdateRow(row);
  E.dirty++;
}

/*** editor operations ***/

void
editorInsertChar(int c)
{
  if (E.cy == E.numrows) {
    editorInsertRow(E.numrows, "", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}

void
editorInsertTimestamp(void)
{
  time_t timer;
  char buffer[26];
  struct tm* tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);
  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  int len = strlen(buffer);

  for (int i = 0; i < len; i++) {
    editorInsertChar(buffer[i]);
  }
}

void
editorInsertNewline(void)
{
  if (E.cx == 0) {
    editorInsertRow(E.cy, "", 0);
  } else {
    erow *row = &E.row[E.cy];
    editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }
  E.cy++;
  E.cx = 0;
}

void
editorDelChar(void)
{
  if (E.cy == E.numrows) {
    return;
  }
  if (E.cx == 0 && E.cy == 0) {
    return;
  }

  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    editorRowDelChar(row, E.cx - 1);
    E.cx--;
  } else {
    E.cx = E.row[E.cy - 1].size;
    editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
    editorDelRow(E.cy);
    E.cy--;
  }
}

/*** file i/o ***/

char*
editorRowsToString(int *buflen)
{
  int totlen = 0;
  int j;
  for (j = 0; j < E.numrows; j++) {
    totlen += E.row[j].size + 1;
  }
  *buflen = totlen;

  char *buf = malloc(totlen);
  char *p = buf;
  for (j = 0; j < E.numrows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }
  return buf;
}

void
editorCloneTemplate(void) {
  FILE *templateFile = NULL;
  char *template = editorPrompt("Select Template: (N)otes | (R)eadme %s", NULL);

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
    editorInsertRow(E.numrows, line, linelen);
  }
  free(line);
  fclose(templateFile);
  E.dirty = 0;
}

void
preDirOpenHook(void) {
  SCM preDirOpenHook;
  SCM results_scm;
  char* results;
  preDirOpenHook = scm_variable_ref(scm_c_lookup("preDirOpenHook"));
  results_scm = scm_call_1(preDirOpenHook, scm_from_locale_string(E.filename));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
  return;
}

void
postDirOpenHook(int num_files) {
  SCM postDirOpenHook;
  SCM results_scm;
  char *results;
  postDirOpenHook = scm_variable_ref(scm_c_lookup("postDirOpenHook"));
  results_scm = scm_call_1(postDirOpenHook, scm_from_int(num_files));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void
preFileOpenHook(void) {
  SCM preFileOpenHook;
  SCM results_scm;
  char *results;
  preFileOpenHook = scm_variable_ref(scm_c_lookup("preFileOpenHook"));
  results_scm = scm_call_1(preFileOpenHook, scm_from_locale_string(E.filename));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
}

void
postFileOpenHook(void) {
  SCM postFileOpenHook;
  SCM results_scm;
  char* results;
  int len;
  char *contents = editorRowsToString(&len);
  postFileOpenHook = scm_variable_ref(scm_c_lookup("postFileOpenHook"));
  results_scm = scm_call_1(postFileOpenHook, scm_from_locale_string(contents));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
  return;
}

void
editorOpen(char *filename) {
  if (filename == NULL) {
    filename = editorPrompt("Path to open: (ESC to cancel) %s", NULL);
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

  if( stat(E.filename,&s) == 0 ) {
    if( s.st_mode & S_IFDIR ) {
      // Call the scandir function.
      // The 3rd param is a filter/selctor function; we want all children of the
      // directory, so we use our `_true` function to accept all children.
      // The 4th param is a sort function. We are using alphasort, which is
      // provided by the GNU stdlib
      preDirOpenHook();
      struct dirent **dits;
      int num_files = 0;
      num_files = scandir (E.filename, &dits, _true, alphasort);
      if (num_files >= 0) {
        int count;
        for (count = 0; count < num_files; ++count) {
          editorInsertRow(count, dits[count]->d_name, strlen(dits[count]->d_name));
        }
        // Clean up the memory allocatted by scandir
        for (count = 0; count < num_files; ++count) {
          // Free each dirent before freeing dits as a whole
          free(dits[count]);
        }
      free(dits);
      } else {
        perror ("Error opening directory");
      }
      postDirOpenHook(num_files);
      return;
    }
    else if( s.st_mode & S_IFREG ) {
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
        editorInsertRow(E.numrows, line, linelen);
      }
      postFileOpenHook();
    }
    else {
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

void
editorPreSaveHook(void) {
  SCM preSaveHook;
  SCM results_scm;
  char* results;
  int len;
  char *contents = editorRowsToString(&len);
  preSaveHook = scm_variable_ref(scm_c_lookup("preSaveHook"));
  results_scm = scm_call_1(preSaveHook, scm_from_locale_string(contents));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
  return;
}

void
editorPostSaveHook(void) {
  SCM postSaveHook;
  SCM results_scm;
  char* results;
  int len;
  char *contents = editorRowsToString(&len);
  postSaveHook = scm_variable_ref(scm_c_lookup("postSaveHook"));
  results_scm = scm_call_1(postSaveHook, scm_from_locale_string(contents));
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);
  return;
}

void
editorSave(void)
{
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

void
editorExec(void)
{
  char *command;
  SCM results_scm;
  char *results;

  command = editorPrompt("scheme@(guile-user)> %s", NULL);
  results_scm = scm_c_eval_string(command);
  results = scm_to_locale_string(results_scm);
  editorSetStatusMessage(results);  
}

/*** find ***/

void
editorFindCallback(char *query, int key)
{
  static int last_match = -1;
  static int direction = 1;

  static int saved_hl_line;
  static char *saved_hl = NULL;

  if (saved_hl) {
    memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
    free(saved_hl);
    saved_hl = NULL;
  }

  if (key == '\r' || key == '\x1b') {
    last_match = -1;
    direction = 1;
    return;
  } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
    direction = 1;
  } else if (key == ARROW_LEFT || key == ARROW_UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }

  if (last_match == -1) {
    direction = 1;
  }
  int current = last_match;
  int i;
  for (i = 0; i < E.numrows; i++) {
    current += direction;
    if (current == -1) {
      current = E.numrows - 1;
    } else if (current == E.numrows) {
      current = 0;
    }
    erow *row = &E.row[current];
    char *match = strstr(row->render, query);
    if (match) {
      last_match = current;
      E.cy = current;
      E.cx = editorRowRxToCx(row, match - row->render);
      E.rowoff = E.numrows;

      saved_hl_line = current;
      saved_hl = malloc(row->rsize);
      memcpy(saved_hl, row->hl, row->rsize);
      memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
      break;
    }
  }
}

void
editorFind(void)
{
  int saved_cx = E.cx;
  int saved_cy = E.cy;
  int saved_coloff = E.coloff;
  int saved_rowoff = E.rowoff;

  char *query = editorPrompt("Search: %s", editorFindCallback);
  if (query) {
    free(query);
  } else {
    E.cx = saved_cx;
    E.cy = saved_cy;
    E.coloff = saved_coloff;
    E.rowoff = saved_rowoff;
  }
}

/** append buffer ***/

void
abAppend(struct abuf *ab, const char *s, int len)
{
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL) {
    return;
  }

  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void
abFree(struct abuf *ab)
{
  free(ab->b);
}

/*** output ***/

void
editorScroll(void)
{
  E.rx = 0;
  if (E.cy < E.numrows) {
    E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
  }

  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

void
editorDrawRows(struct abuf *ab)
{
  int y;
  for (y = 0; y < E.screenrows; y++) {
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows) {
      if (E.numrows == 0 && y == E.screenrows / 3) {
	char welcome[80];
	int welcomelen = snprintf(welcome, sizeof(welcome),
				  "ze -- version %s", ZE_VERSION);
	if (welcomelen > E.screencols) {
	  welcomelen = E.screencols;
	}
	int padding = (E.screencols - welcomelen) / 2;
	if (padding) {
	  abAppend(ab, "~", 1);
	  padding--;
	}
	while (padding--) {
	  abAppend(ab, " ", 1);
	}
	abAppend(ab, welcome, welcomelen);
      } else {
	abAppend(ab, "~", 1);
      }
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0) {
	len = 0;
      }
      if (len > E.screencols) {
	len = E.screencols;
      }
      char *c = &E.row[filerow].render[E.coloff];
      unsigned char *hl = &E.row[filerow].hl[E.coloff];
      int current_color = -1;
      int j;
      for (j = 0; j < len; j++) {
	if (iscntrl(c[j])) {
	  char sym = (c[j] <= 26) ? '@' + c[j] : '?';
	  abAppend(ab, "\x1b[7m", 4);
	  abAppend(ab, &sym, 1);
	  abAppend(ab, "\x1b[m", 3);
	  if (current_color != -1) {
	    char buf[16];
	    int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
	    abAppend(ab, buf, clen);
	  }
	} else if (hl[j] == HL_NORMAL) {
	  if (current_color != -1) {
	    abAppend(ab, "\x1b[39m", 5);
	    current_color = -1;
	  }
	  abAppend(ab, &c[j], 1);
	} else {
	  int color = editorSyntaxToColor(hl[j]);
	  if (color != current_color) {
	    current_color = color;
	    char buf[16];
	    int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
	    abAppend(ab, buf, clen);
	  }
	  abAppend(ab, &c[j], 1);
	}
      }
      abAppend(ab, "\x1b[39m", 5);
    }

    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}

void
editorDrawStatusBar(struct abuf *ab)
{
  abAppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
		     E.filename ? E.filename : "[No Name]", E.numrows,
		     E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
		      E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);
  if (len > E.screencols) {
    len = E.screencols;
  }
  abAppend(ab, status, len);
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void
editorDrawMessageBar(struct abuf *ab) {
  abAppend(ab, "\x1b[K", 3);
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) {
    msglen = E.screencols;
  }
  if (msglen && time(NULL) - E.statusmsg_time < 1) {
    abAppend(ab, E.statusmsg, msglen);
  }
}

void
editorRefreshScreen(void)
{
  editorScroll();

  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff)+ 1, (E.rx - E.coloff) + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

void
editorSetStatusMessage(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

void
scmEditorSetStatusMessage(SCM message)
{
  va_list ap;
  char *fmt = scm_to_locale_string(message);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

/*** input ***/

char*
editorPrompt(char *prompt, void (*callback)(char *, int))
{
  size_t bufsize = 128;
  char *buf = malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();

    int c = editorReadKey();
    if (/*c == DEL_KEY ||*/ c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0) {
	      buf[--buflen] = '\0';
      }
    } else if (c == '\x1b') {
      editorSetStatusMessage("");
      if (callback) {
	      callback(buf, c);
      }
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
	      editorSetStatusMessage("");
	      if (callback) {
	        callback(buf, c);
	      }
	      return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
	       bufsize *= 2;
	       buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
    if (callback) {
      callback(buf, c);
    }
  }
}

void
editorMoveCursor(char key)
{
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  switch (key) {
  case ARROW_LEFT:
    if (E.cx != 0) {
      E.cx--;
    } else if (E.cy > 0) {
      E.cy--;
      E.cx = E.row[E.cy].size;
    }
    break;
  case ARROW_RIGHT:
    if (row && E.cx < row->size) {
      E.cx++;
    } else if (row && E.cx == row->size) {
      E.cy++;
      E.cx = 0;
    }
    break;
  case ARROW_UP:
    if (E.cy != 0) {
      E.cy--;
    }
    break;
  case ARROW_DOWN:
    if (E.cy < E.numrows) {
      E.cy++;
    }
    break;
  }
  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;
  if (E.cx > rowlen) {
    E.cx = rowlen;
  }
}

void
editorProcessKeypress(void)
{
  static int quit_times = ZE_QUIT_TIMES;
  char c = editorReadKey();

  switch (c) {
  case '\r':
    editorInsertNewline();
    break;
  case CTRL_KEY('q'):
    if (E.dirty && quit_times > 0) {
      editorSetStatusMessage("WARNING!! File has unsaved changes. "
			     "Press C-q %d more time to quit.", quit_times);
      quit_times--;
      return;
    }
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  case CTRL_KEY('o'):
    editorOpen(NULL);
    break;
  case CTRL_KEY('t'):
    editorCloneTemplate();
    break;
	case CTRL_KEY('i'):
		editorInsertTimestamp();
		break;
  case CTRL_KEY('w'):
    editorSave();
    break;
  case CTRL_KEY('x'):
    editorExec();
    break;
  case HOME_KEY:
    E.cx = 0;
    break;
  case END_KEY:
    if (E.cy < E.numrows) {
      E.cx = E.row[E.cy].size;
    }
    break;
  case CTRL_KEY('s'):
    editorFind();
    break;
  case CTRL_KEY('d'):
    editorDelRow(E.cy);
    break;
  case CTRL_KEY('k'):
    editorDelRowAtChar(&E.row[E.cy], E.cx);
    break;
  case BACKSPACE:
  case CTRL_KEY('h'):
    /*case DEL_KEY:*/
    /*if (c == DEL_KEY) {
      editorMoveCursor(ARROW_RIGHT);
      }*/
    editorDelChar();
    break;
  case PAGE_UP:
  case PAGE_DOWN:
    {
      if (c == PAGE_UP) {
	E.cy = E.rowoff;
      } else if (c == PAGE_DOWN) {
	E.cy = E.rowoff + E.screenrows - 1;
	if (E.cy > E.numrows) {
	  E.cy = E.numrows;
	}
      }
      int times = E.screenrows;
      while (times --) {
	editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
    }
    break;
  case ARROW_UP:
  case ARROW_DOWN:
  case ARROW_LEFT:
  case ARROW_RIGHT:
    editorMoveCursor(c);
    break;
  case CTRL_KEY('l'):
  case '\x1b':
    break;
  default:
		editorInsertChar(c);
    break;
  }
  quit_times = ZE_QUIT_TIMES;
}

/*** init ***/

void
initEditor(void)
{
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

int
main(int argc, char *argv[])
{
  enableRawMode();
  initEditor();

  editorSetStatusMessage("HELP: C-o = open a file | C-t = clone a template | C-w = write to disk | C-s = search | C-x guile | C-q = quit");

  // Initialize Guile
  scm_init_guile();

  // Load configuration script file
  SCM init_func;
  SCM notes_template_scm;
  SCM readme_template_scm;

  scm_c_primitive_load("/Users/zach/.ze/zerc.scm");
  init_func = scm_variable_ref(scm_c_lookup("ze_config"));
  scm_call_0(init_func);
  scm_c_define_gsubr("set-editor-status", 1, 0, 0, &scmEditorSetStatusMessage);

  // If you want to add additional templates, you'll want to make sure to add
  // the appropriate lines here, and add your new template to the selection
  // list in the editorCloneTemplate method and define a global variable of
  // type char* similar to how notes_template and readme_template are defined.
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
