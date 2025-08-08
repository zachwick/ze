;; Example: augment post-save to show a different status
(define (postSaveHook contents)
  (string-append "Post-save plugin says: wrote " (number->string (string-length contents)) " bytes"))


