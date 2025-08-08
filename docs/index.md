# ze

ze - a simple extensible text editor

ze is a very simple text editor that uses GNU Guile as its extension language. It supports running guile code as pre- and post- hooks upon opening a file, opening a directory, or saving a file. It supports highlighting for C, Python, Ruby, PHP, Rust, APL, and Swift. 

## Installation

First, you must install GNU Guile (and its related libraries) on your system. You can follow the [official installation instructions](https://www.gnu.org/software/guile/download/) or by simply doing 

```
brew install guile
```

if you are using macOS.

Next, you must clone the repo. This is usually done by something like

```
git clone git@github.com:zachwick/ze
```

Once you have a local clone of the repo, you must edit the path supplied to `scm_c_primitive_load` in the `main` method to match wherever your local `zerc.scm` configuration file is located. A common practice is to copy the example configuration file from the repo into a `.ze` directory in your home directory. You can do this by the following:

```
mkdir ~/.ze
cp path/to/local/clone/zerc.scm ~/.ze/
```

You should also copy the example file templates into this newly created `~/.ze` directory, and ensure that the paths for those templates in your `zerc.scm` match wherever you put the template files.

```
cp -R templates ~/.ze/templates
```

Finally, simply invoke `make install` to build ze and put the resulting binary in your PATH.

## CLI Usage

Use `ze` to start ze, or use `ze <filename>` to start ze and open `filename` all at once.

### Documentation

- [C API reference](api/)

## Default Keybindings

| chord | name | description |
| -- | -- | -- |
| CTRL-o | open path | Prompts the user for a path to open in ze. |
| CTRL-w | write to file | Write/Save the current buffer to a file. If the buffer does not have a filepath associated with it, the user will be prompted for a save location.|
| CTRL-t | clone a template | Prompts the user for a template to clone into the current buffer. |
| CTRL-s | forward search | Prompt the user for a string to search forward in the document for. While searching, CTRL-n moves to the next match and CTRL-b moves to the previous match. ESC quits searching and returns the cursor to the original position prior to beginning searching. ENTER quits searching and leaves the cursor at the current match. |
| CTRL-i | insert timestamp | Inserts the current timestamp at the cursor, using the format string "%Y-%m-%d %H:%M:%S" (which yields a timestamp such as "YYYY-MM-DD HH:MM:SS") |
| CTRL-d | delete line | Deletes the entire line that the cursor is currently on. |
| CTRL-k | delete rest of line | Deletes the line from the cursor's current position to the end of the current line. |
| CTRL-h | delete character | Deletes the character at the cursor's position. |
| CTRL-x | open REPL | Opens a Guile Scheme REPL |

### Moving the cursor

| chord | name | description |
| -- | -- | -- |
| CTRL-p | move cursor up | Moves the cursor up one line and either in the same column or the right-most column if the previous line does not contain a character in the original column. |
| CTRL-n | move cursor down | Moves the cursor down one line and either in the same column or the right-most column if the previous line does not contain a character in the original column. |
| CTRL-f | move cursor forward | Moves the cursor one column to the left. |
| CTRL-b | move cursor backward | Moves the cursor one column to the right. |
| CTRL-v | move page down | Moves the cursor to the beginning of the next page of content. |
| CTRL-g | move page up | Moves the cursor to the beginning of the previous page of content. |

## Advanced Usage

### Plugin System

ze supports Guile Scheme plugins that are automatically loaded at startup.

- **Location**: Place `.scm` files in `~/.ze/plugins`. All non-hidden `.scm` files in that directory are loaded when ze starts.
- **Examples**: `make install` will create `~/.ze/plugins` and copy the example plugins from this repo into it.
- **Key bindings**: Use `(bind-key key-spec procedure)` to bind keys from Scheme.
  - Key specs can be a single character (e.g., `"y"`) or a Control chord like `"C-y"`.
  - Plugin key bindings are checked before built-in handlers. If a plugin handles a key, ze skips the built-in action for that keypress.
- **Inline Scheme prompt (REPL)**: Press `CTRL-x` to open a one-line Scheme prompt (`scheme@(guile-user)>`) and evaluate expressions in the current editor environment.

Minimal plugin example (`~/.ze/plugins/hello.scm`):

```scheme
(display "ze: loaded plugin hello.scm") (newline)

(define (ze-hello)
  (set-editor-status "Hello from ze plugin!"))

(bind-key "C-y" ze-hello)
```

#### Example Plugins

- **hello.scm** (`C-y`): Sets a friendly status message to demonstrate key binding.
- **format.scm** (`C-l`): Trims trailing whitespace from all lines in the buffer and reports how many lines changed.
- **go-to-line.scm** (`C-g`): Prompts for a line number and moves the cursor there, refreshing the screen.
- **file-header.scm** (`C-,`): Inserts a simple 3-line header at the top with file name and detected type; moves cursor below header.
- **save-and-format.scm** (hook): Implements `postSaveHook` to trim trailing whitespace after saving; if changes were made it re-saves and refreshes the screen, returning a summary string.

You can remove or modify these by editing/deleting the corresponding files under `~/.ze/plugins`.

#### Guile Hooks

| guile function |  returns | description |
| -- | -- | -- |
| preSaveHook | string | called prior to writing buffer to file. |
| postSaveHook | string | called after writing buffer to file. |
| preDirOpenHook | string | called prior to opening a directory in ze. |
| postDirOpenHook | string | called after opening a directory in ze. |
| preFileOpenHook | string | called prior to opening a file into a buffer. |
| postFileOpenHook | string | called after opening a file into a buffer. |

#### Scheme API (bindings)

The following Scheme procedures are available to plugins. Return values are noted where relevant.

- **Status and key bindings**
  - `set-editor-status(string)` — set the status line message.
  - `bind-key(key-spec, procedure)` — bind a key to a Scheme procedure (e.g., `"C-y"`, `"g"`).
  - `unbind-key(key-spec)` — remove a key binding.
  - `list-bindings()` — returns a list of `(key . procedure)` pairs.

- **Buffer content**
  - `buffer->string()` → string of the entire buffer.
  - `buffer-line-count()` → number of lines.
  - `get-line(index)` → string at `index` (0-based) or `#f` if out of range.
  - `set-line!(index, string)` — replace contents of line at `index`.
  - `insert-line!(index, string)` — insert a new line at `index`.
  - `append-line!(string)` — append a new line to the end of the buffer.
  - `delete-line!(index)` — delete the line at `index`.
  - `insert-text!(string)` — insert text at the cursor (handles `\n`).
  - `insert-char!(char|string)` — insert a single character at the cursor.
  - `insert-newline!()` — insert a newline at the cursor.
  - `delete-char!()` — delete the character at the cursor.

- **Cursor and viewport**
  - `get-cursor()` → pair `(x y)` of cursor column and line (0-based).
  - `set-cursor!(x, y)` — move cursor to column `x`, line `y` (clamped to buffer bounds).
  - `move-cursor!(direction)` — move one step; `direction` is one of `"left"|"right"|"up"|"down"`.
  - `screen-size()` → pair `(rows cols)` of the current screen.
  - `refresh-screen!()` — force a screen refresh.

- **File I/O and filenames**
  - `open-file!(path)` — open file into the current buffer.
  - `save-file!()` — save current buffer to disk.
  - `get-filename()` → current filename string or `#f` if unsaved.
  - `set-filename!(path)` — set (or change) the current buffer filename and select syntax.
  - `prompt(message)` → read a line of input from the user or `#f` if cancelled.

- **Search**
  - `search-forward!(query)` → pair `(y x)` of the next match location or `#f` if none.

- **Syntax highlighting**
  - `select-syntax-for-filename!(path)` — set syntax by pretending the buffer is named `path`.
  - `get-filetype()` → current filetype string or `#f`.

- **Dirty state**
  - `buffer-dirty?()` → `#t` if the buffer has unsaved changes.
  - `set-buffer-dirty!(boolean)` — mark buffer as dirty/clean.

- **Templates**
  - `clone-template!()` — invoke the template cloning prompt.

### Templates

To add a new template, you must make changes in three places in `ze.c` and one place in your `zerc.scm` configuration file.

First, in `ze.c`, you must define a global variable of type `char*` that is initialized to an empty string. This variable should probably have a name like "my_new_template" or something similarly self-documenting. If we're adding an email template for instance, I'd add the following around line 90 in `ze.c` (where `notes_template` and `readme_template` are defined)

```c
char* email_template = "";
```

Next, you must also add your new template as an option in the `editorCloneTemplate` method in `ze.c`. You should do this by adding your new template as an option in the prompt, and by adding a new "else if" block to the large if/elseif/else block in that method. Continuing the example of adding an email template, I'd replace the line around line 871 that currently reads

```c
  char *template = editorPrompt("Select Template: (N)otes | (R)eadme ", NULL);
```
with a new prompt that includes the new email template:

```c
  char *template = editorPrompt("Select Template: (N)otes | (R)eadme | (E)mail", NULL);
```

The "else if" block that needs to be added after the last "else if" but before the final "else" block would then look like the following:

```c
 else if (strcasecmp(template, "e") == 0) {
    editorSetStatusMessage("Load Email template");
    templateFile = fopen(email_template, "r");
  }
```

Note that `email_template` here should match whatever you named your global varible in the first step.

The actual filepath to your new template file is defined in your `zerc.scm` configuration file. You'll need to define a Guile variable that is a string of the path of your template file. To do this, you need to add a line like the following to your `zerc.scm` file and ensure that the stringified path is correct.

```
(define email_template '"/Users/zwick/.ze/templates/email")
```

Finally, the last edit to make will be in the `main` method defined in `ze.c`. You'll need to write the three lines of C code that read in the Guile variable that you defined in your `zerc.scm` configuration file which contains the filepath of your new template. To do this, you'll first need to define a new variable of type `SCM` to hold the Guile variable read from your `zerc.scm` configuration file.

```c
SCM email_template_scm;
```

You'll then need to write the two lines of C code that can read the Guile code and convert it into the correct C type.

```c
email_template_scm = scm_variable_ref(scm_c_lookup("email_template"));
email_template = scm_to_locale_string(email_template_scm);  
```

After you've made these four changes, you can simply invoke `make install` to build and install your new version of ze that contains your new template.

## License

ze is copyright 2018, 2019, 2020 zach wick <zach@zachwick.com> and is licensed
under the GNU GPLv3 or later. You can find a copy of the GNU GPLv3
included in the project as the file named [License](https://github.com/zachwick/genie/blob/master/LICENSE).
