(load "@lib/test.l" "@lib/map.l" '(math <= + -) '(std if list))

(def fib (N)
	(if (<= N 1) N (+ (fib (- N 1)) (fib (- N 2)))))

(test:run
	"Fibonacci"
	("fib_sequence"	. (assert:equal
											(list 0 1 1 2 3 5 8 13 21 34 55)
											(map fib '(0 1 2 3 4 5 6 7 8 9 10)))
	 ))
