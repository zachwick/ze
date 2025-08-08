(define notes_template '"/Users/zach/.ze/templates/notes")
(define readme_template '"/Users/zach/.ze/templates/readme")

(define ze_config
  (lambda ()
    (display "Loading ze config") (newline)))

(define (preSaveHook contents)
  (number->string (string-length contents)))

(define (postSaveHook contents)
  (string-append (string-append "Wrote " (number->string (string-length contents)) " bytes to file")))

(define (preDirOpenHook dirname)
  (string-append "Opening directory " dirname))

(define (postDirOpenHook numFiles)
  (string-append (number->string numFiles) " files"))

(define (preFileOpenHook filename)
  (string-append "Opening file " filename))

(define (postFileOpenHook contents)
  (string-append (string-append "Read " (number->string (string-length contents)) " bytes from file")))