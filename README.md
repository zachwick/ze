# ze

**ze** - a simple extensible text editor

ze is a very simple text editor that uses GNU Guile as its extension language. It supports running guile code as pre- and post- hooks upon opening a file, opening a directory, or saving a file. It supports highlighting for C, Python, Ruby, PHP, Rust, APL, and Swift.

See [the docs](https://zachwick.github.io/ze/) for more information.

## Features

- **Simple and lightweight**: Minimal text editor with essential functionality
- **Extensible**: Uses GNU Guile for scripting and customization
- **Syntax highlighting**: Supports C, Python, Ruby, PHP, Rust, APL, and Swift
- **Hooks system**: Pre- and post- hooks for file operations
- **Template system**: Clone predefined templates for common file types
- **Search functionality**: Forward search with navigation
- **Cross-platform**: Works on macOS, Linux, and other Unix-like systems

## Installation

### Prerequisites

First, you must install GNU Guile (and its related libraries) on your system. You can follow the [official installation instructions](https://www.gnu.org/software/guile/download/) or by simply doing:

```bash
brew install guile
```

if you are using macOS.

### Building from Source

1. **Clone the repository**:
   ```bash
   git clone git@github.com:zachwick/ze
   cd ze
   ```

2. **Set up configuration**:
   ```bash
   mkdir ~/.ze
   cp zerc.scm ~/.ze/
   cp -R templates ~/.ze/templates
   ```

3. **Edit the configuration path**:
   You must edit the path supplied to `scm_c_primitive_load` in the `main` method to match wherever your local `zerc.scm` configuration file is located. A common practice is to copy the example configuration file from the repo into a `.ze` directory in your home directory.

4. **Build and install**:
   ```bash
   make install
   ```

## Usage

### Basic Usage

- Start ze: `ze`
- Open a file: `ze <filename>`

### Default Keybindings

#### File Operations
| Key | Action | Description |
|-----|--------|-------------|
| `Ctrl+o` | Open path | Prompts the user for a path to open in ze |
| `Ctrl+w` | Write to file | Write/Save the current buffer to a file |
| `Ctrl+t` | Clone template | Prompts the user for a template to clone into the current buffer |

#### Search
| Key | Action | Description |
|-----|--------|-------------|
| `Ctrl+s` | Forward search | Prompt for a string to search forward in the document |
| `Ctrl+n` | Next match | Move to the next search match (while searching) |
| `Ctrl+b` | Previous match | Move to the previous search match (while searching) |
| `ESC` | Quit search | Quit searching and return cursor to original position |
| `Enter` | Accept match | Quit searching and leave cursor at current match |

#### Editing
| Key | Action | Description |
|-----|--------|-------------|
| `Ctrl+i` | Insert timestamp | Inserts current timestamp at cursor |
| `Ctrl+d` | Delete line | Deletes the entire line that the cursor is currently on |
| `Ctrl+k` | Delete rest of line | Deletes from cursor position to end of line |
| `Ctrl+h` | Delete character | Deletes the character at cursor position |

#### Navigation
| Key | Action | Description |
|-----|--------|-------------|
| `Ctrl+p` | Move up | Move cursor up one line |
| `Ctrl+n` | Move down | Move cursor down one line |
| `Ctrl+f` | Move forward | Move cursor one column to the right |
| `Ctrl+b` | Move backward | Move cursor one column to the left |
| `Ctrl+v` | Page down | Move cursor to beginning of next page |
| `Ctrl+g` | Page up | Move cursor to beginning of previous page |

## Advanced Usage

### Plugin System

ze supports Guile Scheme plugins that are automatically loaded at startup.

- **Location**: Plugins are `.scm` files placed in `~/.ze/plugins`. All non-hidden `.scm` files in that directory are loaded at startup.
- **Examples**: `make install` creates `~/.ze/plugins` and copies the example plugins from the repo.
- **API exposed to Scheme**:
  - `set-editor-status(string)` — set the status line message
  - `bind-key(key-spec, procedure)` — bind a key to a Scheme procedure
- **Key specs**: Either a single character (e.g., `"y"`) or Control chords like `"C-y"`.
- **Input handling**: When a key is pressed, ze checks plugin bindings first. If a bound procedure runs, the built-in handler is skipped for that keypress.
- **REPL**: Press `Ctrl+x` to open a one-line Scheme prompt inside ze to evaluate expressions in the current environment.

#### Minimal examples

Bind a key and set a status message (`~/.ze/plugins/hello.scm`):

```scheme
(display "ze: loaded plugin hello.scm") (newline)

(define (ze-hello)
  (set-editor-status "Hello from ze plugin!"))

(bind-key "C-y" ze-hello)
```

Override a hook from a plugin (`~/.ze/plugins/hooks.scm`):

```scheme
(define (postSaveHook contents)
  (string-append "Post-save plugin says: wrote "
                 (number->string (string-length contents))
                 " bytes"))
```

See the Hooks section below for the list of available hooks and their return values.

### Guile Hooks

ze supports various Guile hooks for customization:

| Hook Function | Returns | Description |
|---------------|---------|-------------|
| `preSaveHook` | string | Called prior to writing buffer to file |
| `postSaveHook` | string | Called after writing buffer to file |
| `preDirOpenHook` | string | Called prior to opening a directory in ze |
| `postDirOpenHook` | string | Called after opening a directory in ze |
| `preFileOpenHook` | string | Called prior to opening a file into a buffer |
| `postFileOpenHook` | string | Called after opening a file into a buffer |

### Templates

To add a new template, you must make changes in three places in `ze.c` and one place in your `zerc.scm` configuration file:

1. **Define global variable** in `ze.c` (around line 90):
   ```c
   char* email_template = "";
   ```

2. **Add template option** in `editorCloneTemplate` method:
   ```c
   char *template = editorPrompt("Select Template: (N)otes | (R)eadme | (E)mail", NULL);
   ```

3. **Add template handling** in the if/else block:
   ```c
   else if (strcasecmp(template, "e") == 0) {
     editorSetStatusMessage("Load Email template");
     templateFile = fopen(email_template, "r");
   }
   ```

4. **Define template path** in `zerc.scm`:
   ```scheme
   (define email_template '"/Users/zwick/.ze/templates/email")
   ```

5. **Read template in main method**:
   ```c
   SCM email_template_scm;
   email_template_scm = scm_variable_ref(scm_c_lookup("email_template"));
   email_template = scm_to_locale_string(email_template_scm);
   ```

After making these changes, run `make install` to build and install your new version of ze.

## Configuration

The main configuration file is `~/.ze/zerc.scm`. This file contains:
- Template file paths
- Custom Guile functions and hooks
- Editor behavior customization

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

ze is copyright 2018, 2019, 2020 zach wick <zach@zachwick.com> and is licensed under the GNU GPLv3 or later. You can find a copy of the GNU GPLv3 included in the project as the file named [LICENSE.txt](LICENSE.txt).

## Author

- **zach wick** - <zach@zachwick.com>

## Acknowledgments

- Built with GNU Guile for extensibility
- Inspired by simple, efficient text editors such as [kilo](https://github.com/antirez/kilo).