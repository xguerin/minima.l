(load "iterators.l" '(logic =) '(std \ if cons))
(use '(logic =) '(std \ if cons))

(def erase (k lst)
	(foldr
		(\ ((_k . _v) acc)
			(if (= k _k) acc (cons (cons _k _v) acc)))
		lst NIL))
