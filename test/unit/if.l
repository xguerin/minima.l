(load '(logic =) '(std if let))

(let ((A . 1)
			(REM . 1)
			(B . (if (= A 1) REM A)))
	(= A B))
