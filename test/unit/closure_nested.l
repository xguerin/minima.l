(load '(logic =) '(std \ let))

(let ((A . 1)
			(F . (\ () A))
			(R . (F)))
	(= R 1))
