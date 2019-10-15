(load "../lisp/compile.l")

(def add (a b) (if (= a b) (+ 1 (+ 2 a)) 1))
(>! 'add)

(quit)
