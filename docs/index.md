# ze

ze - a simple extensible text editor

## Installation

(This section needs fleshed out, but at a high level, the steps are:)
- Install dependencies (guile)
- clone the repo
- edit zerc.scm path
- copy zerc.scm to ~/.ze
- make install

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
