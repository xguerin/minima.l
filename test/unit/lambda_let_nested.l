(load '(logic =) '(math + -) '(std \ let))

(let ((fun . (\ (A B) (+ A B)))
			(res . (let ((fun . (\ (A B) (- A B))))
							 (fun 3 1))))
	(= (fun 1 1) res))
