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

Put something about the various guile hooks and how to use them here.

## License

ze is copyright 2018, 2019, 2020 zach wick <zach@zachwick.com> and is licensed
under the GNU GPLv3 or later. You can find a copy of the GNU GPLv3
included in the project as the file named [License](https://github.com/zachwick/genie/blob/master/LICENSE).
