(load "@lib/test.l" "@lib/append.l" "@lib/ntoa.l" 'time 'unlink)

(test:run
  "I/O operations"
  #
  # In and OUT.
  #
  ("outin" . (let ((ts . (time))
                   (fname . (append "/tmp/out." (ntoa ts))))
               (out fname (print '(1 2 3 4 5)))
               (let ((data . (in fname (read))))
                 (unlink fname)
                 (assert:equal '(1 2 3 4 5) data))))
  #
  )
