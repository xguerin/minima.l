(load '(logic =) '(math +) '(std let))
(use '(logic =) '(math +) '(std let))

(let ((A . 1)
			(B . (let ((A . 2)) A)))
	(= (+ A 1) B))
