(load '(logic =) '(math + -) '(std if))
(use '(logic =) '(math + -) '(std if))

(def accumulate (V)
	(if (= 0 V)
		42
		(accumulate (- V 1)))
		))

(= (accumulate 3) 42)
