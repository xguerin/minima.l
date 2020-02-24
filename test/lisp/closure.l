(load "@lib/test.l" '(logic and) '(std let |> <- \))

(test:run
  "Closures"
  #
  ("set"  . (let ((a . 1))
              (|>
                (assert:equal 1 a)
                (and (let ((b . 1))
                       (<- 'a 2)
                       (and
                         (assert:equal 2 a)
                         (assert:equal 1 b))))
                (and (assert:equal 2 a)))))
  ("nested_let" . (let ((a . 1) (b . 2))
                    (let ((c . 3) (d . 4))
                      (|> T
                          (and (assert:equal 1 a))
                          (and (assert:equal 2 b))
                          (and (assert:equal 3 c))
                          (and (assert:equal 4 d))))))
  ("nested_lambda_0" . (let ((fn . (\ () (|> T
                                             (and (assert:equal 1 a))
                                             (and (assert:equal 2 b))
                                             ))))
                         (let ((a . 1)) (let ((b . 2)) (fn)))))
  ("nested_lambda_1" . (let ((a . 1)
                             (fn . (\ (x) (|> T
                                              (and (assert:equal 1 a))
                                              (and (assert:equal 2 x))
                                              ))))
                         (let ((a . 2)) (fn a))))
  ("nested_lambda_2" . (let ((a . 1) (b . 1) (fn0 . (\ (fn) (assert:equal 3 (fn)))))
                         (let ((a . 2) (fn1 . (\ () (+ a b)))) (fn0 fn1))))
  #
  )
