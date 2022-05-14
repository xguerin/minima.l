(load
	'(iterators foldl)
	'(logic or)
	'(std \ car cdr cons if let nil?))

(def isort (LESS LST)
	"Insertion-sort of list LST using comparator LESS"
	(let ((ins . (\ (LST E)
								 "Insert E in LST using LESS"
								 (if (or (nil? LST) (LESS E (car LST)))
									 (cons E LST)
									 (cons (car LST) (ins (cdr LST) E))))))
		(foldl ins NIL LST)))
