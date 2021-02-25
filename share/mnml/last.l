(load '(std if car cdr))
(use '(std if car cdr))

(def last (LST)
	"Return the last element of a list."
	(if (cdr LST)
		(last (cdr LST))
		(car LST)))
