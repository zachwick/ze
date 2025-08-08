(display "ze: loaded plugin go-to-line.scm") (newline)

(define (goto-line)
  (let* ((resp (prompt "Go to line: %s"))
         (n (and resp (string->number resp))))
    (if (not n)
        (set-editor-status "Goto cancelled")
        (let* ((max (buffer-line-count))
               (target (max 0 (min (- max 1) (inexact->exact n)))))
          (set-cursor! 0 target)
          (refresh-screen!)
          (set-editor-status (string-append "Moved to line " (number->string target)))))))

;; Bind to C-g (safe remap from default page-up alias)
(bind-key "C-g" goto-line)


