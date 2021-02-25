(load "iterators.l" '(std \ eval cons))
(use '(std \ eval cons))

(def append (x . @)
	"Non-destructively concatenate multiple strings."
	(foldl (\ (acc0 e0)
					 (foldr (\ (e1 acc1) (cons e1 acc1)) acc0 (eval e0)))
		x @))

(def $+ (x y)
	"Non-destructively concatenate two strings."
	(append x y))
