// Core editor types, macros, and global state
#pragma once

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stddef.h>
#include <time.h>
#include <termios.h>

#define ZE_VERSION "1.0.0"
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

// Global state defined in src/main.c
extern struct editorConfig E;


