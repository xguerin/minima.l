(load "test.l")

(test:run
  ("test_1" . (assert:equal 1 (+ 0 1)))
  ("test_3" . (assert:equal 2 (+ 1 1)))
  )
