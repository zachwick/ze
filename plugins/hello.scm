(display "ze: loaded plugin hello.scm") (newline)

;; Example: define a command that can be invoked from Guile REPL inside ze
(define (ze-hello)
  (set-editor-status "Hello from ze plugin!"))

;; Bind Ctrl-y to our hello command
(bind-key "C-y" ze-hello)


