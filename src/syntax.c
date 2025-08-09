/**
 * @file syntax.c
 * @brief Syntax highlighting engine implementation.
 * @ingroup syntax
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "ze.h"

extern struct editorConfig E;

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
char *Python_HL_extensions[] = {".py", NULL};
char *Ruby_HL_extensions[] = {".rb", ".erb", NULL};
char *PHP_HL_extensions[] = {".php", NULL};
char *Rust_HL_extensions[] = {".rs", NULL};
char *APL_HL_extensions[] = {".apl", NULL};
char *Swift_HL_extensions[] = {".swift", NULL};
char *TypeScript_HL_extensions[] = {".ts", ".tsx", NULL};

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
char *TypeScript_HL_keywords[] = {
  "abstract", "any", "as", "async", "await", "boolean", "break", "case", "catch", "class",
  "const", "constructor", "continue", "debugger", "declare", "default", "delete", "do",
  "else", "enum", "export", "extends", "false", "finally", "for", "from", "function",
  "get", "if", "implements", "import", "in", "instanceof", "interface", "let", "module",
  "namespace", "new", "null", "number", "of", "package", "private", "protected", "public",
  "return", "set", "static", "string", "super", "switch", "symbol", "this", "throw",
  "true", "try", "type", "typeof", "undefined", "var", "void", "while", "with", "yield",
  "Array|", "Boolean|", "Date|", "Error|", "Function|", "JSON|", "Math|", "Number|",
  "Object|", "Promise|", "Proxy|", "RegExp|", "Set|", "String|", "Symbol|", "TypeError|",
  "URIError|", "WeakMap|", "WeakSet|", "console|", "document|", "window|", "global|",
  "process|", "require|", "module|", "exports|", "__dirname|", "__filename|", NULL
};

struct editorSyntax HLDB[] = {
  { "c", C_HL_extensions, C_HL_keywords, "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "python", Python_HL_extensions, Python_HL_keywords, "#", "'''", "'''",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "ruby", Ruby_HL_extensions, Ruby_HL_keywords, "#", "=begin", "=end",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "PHP", PHP_HL_extensions, PHP_HL_keywords, "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "Rust", Rust_HL_extensions, Rust_HL_keywords, "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "APL", APL_HL_extensions, APL_HL_keywords, "⍝", "⍝", "⍝",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "Swift", Swift_HL_extensions, Swift_HL_keywords, "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
  { "TypeScript", TypeScript_HL_extensions, TypeScript_HL_keywords, "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/**
 * @brief Determine whether a byte is a token separator for highlighting.
 * @ingroup syntax
 *
 * Separators include whitespace, NUL, and common punctuation.
 *
 * @param[in] c Byte value to test.
 * @return Non-zero if @p c is a separator; 0 otherwise.
 */
int is_separator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

/**
 * @brief Compute syntax highlighting classes for a row.
 * @ingroup syntax
 *
 * Updates @c row->hl based on the current filetype rules in @c E.syntax,
 * marking comments, strings, numbers, and keywords. Propagates multi-line
 * comment state to the following row when it changes.
 *
 * @param[in,out] row Row whose render buffer has been prepared.
 * @sa editorUpdateRow(), editorSelectSyntaxHighlight()
 */
void editorUpdateSyntax(erow *row) {
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);
  if (E.syntax == NULL) {
    return;
  }
  char **keywords = E.syntax->keywords;
  char *scs = E.syntax->singleline_comment_start;
  char *mcs = E.syntax->multiline_comment_start;
  char *mce = E.syntax->multiline_comment_end;

  int scs_len = scs ? (int)strlen(scs) : 0;
  int mcs_len = mcs ? (int)strlen(mcs) : 0;
  int mce_len = mce ? (int)strlen(mce) : 0;

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
        int klen = (int)strlen(keywords[j]);
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

/**
 * @brief Map a highlight class to an ANSI color code.
 * @ingroup syntax
 *
 * @param[in] hl One of HL_* constants.
 * @return ANSI color code suitable for 30–37 range.
 */
int editorSyntaxToColor(int hl) {
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

/**
 * @brief Select the syntax highlighting rules based on the filename.
 * @ingroup syntax
 *
 * Sets @c E.syntax to a matching entry in @c HLDB by extension or substring,
 * and recomputes highlighting for all rows.
 *
 * @post @c E.syntax may change; row highlights are updated accordingly.
 */
void editorSelectSyntaxHighlight(void) {
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
        for (int filerow = 0; filerow < E.numrows; filerow++) {
          editorUpdateSyntax(&E.row[filerow]);
        }
        return;
      }
      i++;
    }
  }
}


