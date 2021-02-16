(load '(logic =) '(math +) '(std \ let))

(let ((fun . (\ (A B) (+ A B))))
	(= (fun 1 1) 2))
