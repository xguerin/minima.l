(load "test.l")

(def deconstruct (LST)
  "Deconstruct LST."
  (let (((a (b . c) . d) . '(1 (2 . 3) 4 5)))
    (cons a (cons (cons b c) d))))

(assert:equal
  '(1 (2 . 3) 4 5)
  (deconstruct '(1 (2 . 3) 4 5)))
