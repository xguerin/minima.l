(load '(logic =) '(math + -) '(std \ let))
(use '(logic =) '(math + -) '(std \ let))

(let ((fun . (\ (A B) (+ A B)))
			(res . (let ((fun . (\ (A B) (fun A B))))
							 (fun 1 1))))
	(= (fun 1 1) res))
