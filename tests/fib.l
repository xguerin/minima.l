(load "../lib/lisp/map.l")

(def fib (N) (if (<= N 1) N (+ (fib (- N 1)) (fib (- N 2)))))
(printl (map fib '(0 1 2 3 4 5 6 7 8 9 10)))
