(display "ze: loaded plugin hello.scm") (newline)

;; Example: define a command that can be invoked from Guile REPL inside ze
(define (ze-hello)
  (set-editor-status "Hello from ze plugin!"))


