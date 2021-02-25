(load "iterators.l" '(std \ cons))
(use '(std \ cons))

(def rev (lst)
	"Reverse LST."
	(foldl (\ (acc e) (cons e acc)) NIL lst))
