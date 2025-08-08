(display "ze: loaded plugin format.scm") (newline)

;; Trim trailing spaces/tabs on all lines
(define (rstrip s)
  (let* ((len (string-length s))
         (i (let loop ((j (- len 1)))
               (if (< j 0) -1
                   (let ((c (string-ref s j)))
                     (if (or (char=? c #\space) (char=? c #\tab))
                         (loop (- j 1))
                         j))))))
    (if (< i 0) "" (substring s 0 (+ i 1)))))

(define (format-trailing-whitespace)
  (let ((n (buffer-line-count))
        (changed 0))
    (let loop ((i 0) (chg 0))
      (if (>= i n)
          (begin
            (set-editor-status (string-append "Trimmed trailing whitespace on " (number->string chg) " line(s)"))
            chg)
          (let* ((line (or (get-line i) ""))
                 (trimmed (rstrip line)))
            (if (not (string=? line trimmed))
                (begin (set-line! i trimmed) (loop (+ i 1) (+ chg 1)))
                (loop (+ i 1) chg)))))))

;; Bind to C-l (overrides built-in no-op for C-l)
(bind-key "C-l" format-trailing-whitespace)


