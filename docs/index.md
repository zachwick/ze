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

### Guile Hooks

Put something about the various guile hooks and how to use them here.

### Templates

To add a new template, you must make changes in three places in `ze.c` and one place in your `zerc.scm` configuration file.

First, in `ze.c`, you must define a global variable of type char* that is initialized to an empty string. This variable should probably have a name like "my_new_template" or something similarly self-documenting. If we're adding an email template for instance, I'd add the following around line 90 in `ze.c` (where `notes_template` and `readme_template` are defined)

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

The "else if" block that needs to be added after the last "else if" but before the final "else" blockk would then look like the following:

```c
 else if (strcasecmp(template, "e") == 0) {
    editorSetStatusMessage("Load Email template");
    templateFile = fopen(email_template, "r");
  }
```

Note that `email_template` here should match whatever you named your global varible in the first step.

The actual filepath to your new template file is defined in your `zerc.scm` configuration file. You'll need to define a Guile variable that is a string of the path of your template file. To do this, you need to add a line like the following to youz `zerc.scm` file and ensure that the stringified path is correct.

```
(define email_template '"/Users/zwick/.ze/templates/email")
```

Finally, the last edit to make will be in the `main` method defined in `ze.c`. You'll need to write the three lines of C code that read in the Guile variable that you defined in your `zerc.scm` configuration file which contains the filepath of your new template. To do this, you'll first need to define a new variable of type `SCM` to hold Guile variable read from your `zerc.scm` configuration file.

```c
SCM email_template_scm;
```

You'll then need to write the two lines of C code that can read the Guile code and convert it into the correct C type.

```c
email_template_scm = scm_variable_ref(scm_c_lookup("email_template"));email_template = scm_to_locale_string(email_template_scm);  
```

After you've made these four changes, you can simply invoke `make install` to build and install your new version of ze that contains your new template.

## License

ze is copyright 2018, 2019, 2020 zach wick <zach@zachwick.com> and is licensed
under the GNU GPLv3 or later. You can find a copy of the GNU GPLv3
included in the project as the file named [License](https://github.com/zachwick/genie/blob/master/LICENSE).
