(display "ze: loaded plugin save-and-format.scm (postSaveHook) ") (newline)

;; Post-save hook: format trailing whitespace; only re-save if changes were made
(define (postSaveHook contents)
  (let* ((n (buffer-line-count))
         (changes
          (let loop ((i 0) (chg 0))
            (if (>= i n)
                chg
                (let* ((line (or (get-line i) ""))
                       (trimmed (let* ((len (string-length line))
                                       (j (let rec ((k (- len 1)))
                                            (if (< k 0) -1
                                                (let ((c (string-ref line k)))
                                                  (if (or (char=? c #\space) (char=? c #\tab))
                                                      (rec (- k 1))
                                                      k))))))
                                  (if (< j 0) "" (substring line 0 (+ j 1))))))
                  (if (not (string=? line trimmed))
                      (begin (set-line! i trimmed) (loop (+ i 1) (+ chg 1)))
                      (loop (+ i 1) chg)))))))
    (when (> changes 0)
      (save-file!)
      (refresh-screen!))
    (string-append "Formatted lines: " (number->string changes))) )


