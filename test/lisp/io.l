(load "test.l" "../../lisp/append.l" "../../lisp/ntoa.l" 'time)

(test:run
  "I/O operations"
  #
  # In and OUT.
  #
  ("outin" . (let ((ts . (time))
                   (fname . (append "/tmp/out." (ntoa ts))))
               (out fname (print '(1 2 3 4 5)))
               (assert:equal '(1 2 3 4 5) (in fname (read)))))
  #
  )
