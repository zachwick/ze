/**
 * @file terminal.c
 * @brief Raw terminal mode helpers and key decoding.
 * @ingroup terminal
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "terminal.h"

extern struct editorConfig E;

/**
 * @brief Print an error, reset terminal, and exit the process.
 * @ingroup terminal
 *
 * Clears the screen, moves the cursor home, prints a perror() message for @p s,
 * and exits with code 1.
 *
 * @param[in] s Message passed to perror(). Must be a NUL-terminated string.
 */
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

/**
 * @brief Restore cooked terminal mode.
 * @ingroup terminal
 *
 * Restores original termios captured in @c E.orig_termios. On failure, calls die().
 *
 * @sa enableRawMode()
 */
void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("tcsetattr");
  }
}

/**
 * @brief Enable raw terminal mode with minimal processing.
 * @ingroup terminal
 *
 * Captures current termios into @c E.orig_termios, registers atexit handler to
 * restore it, and configures raw input/output settings.
 *
 * @post Raw mode active; on failure, terminates via die().
 * @sa disableRawMode()
 */
void enableRawMode(void) {
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

/**
 * @brief Read a single key from stdin, interpreting escape sequences.
 * @ingroup terminal
 *
 * Blocks until one byte is read; if the byte begins an escape sequence,
 * attempts to parse known sequences into control codes.
 *
 * @return ASCII char or one of the custom key codes (e.g., ARROW_*).
 */
char editorReadKey(void) {
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

/**
 * @brief Query the terminal for the current cursor position.
 * @ingroup terminal
 *
 * Sends CSI 6n and parses the response.
 *
 * @param[out] rows Receives 1-based row number.
 * @param[out] cols Receives 1-based column number.
 * @return 0 on success; -1 on failure.
 */
int getCursorPosition(int *rows, int *cols) {
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

/**
 * @brief Determine the terminal window size in characters.
 * @ingroup terminal
 *
 * Uses ioctl(TIOCGWINSZ) and falls back to cursor probing if needed.
 *
 * @param[out] rows Receives number of rows.
 * @param[out] cols Receives number of columns.
 * @return 0 on success; -1 on failure.
 * @sa getCursorPosition()
 */
int getWindowSize(int *rows, int *cols) {
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


