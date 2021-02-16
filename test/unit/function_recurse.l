(load '(logic =) '(math + -) '(std if))

(def accumulate (V)
	(if (= 0 V)
		V
		(+ V (accumulate (- V 1)))
		))

(= (accumulate 3) 6)
