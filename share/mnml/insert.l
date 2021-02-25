(load '(std if nil? list let car cdr cons))
(use '(std if nil? list let car cdr cons))

(def insert (cmp a lst)
	"Insert A into ordered list LST using comparator CMP."
	(if (nil? lst)
		(list a)
		(let ((cur . (car lst))
					(nxt . (cdr lst)))
			(if (cmp a cur)
				(cons a (cons cur nxt))
				(cons cur (insert cmp a nxt))
				))))
