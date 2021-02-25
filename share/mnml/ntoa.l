(load '(logic =) '(math + % /) '(std let \ if cons chr))
(use '(logic =) '(math + % /) '(std let \ if cons chr))

(def ntoa (n)
	"Convert a number N into a list of character."
	(let ((descend . (\ (n res)
										 (if (= n 0)
											 res
											 (descend (/ n 10) (cons (chr (+ (% n 10) ^0)) res))))))
		(if (= n 0) "0" (descend n NIL))))
