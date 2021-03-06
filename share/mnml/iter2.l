(load '(logic and) '(std if car cdr))

(def iter2 (fun lst1 lst2)
	"Iterate over list LST1 and LST1 and call FUN on each pair of item."
	(if (and lst1 lst2)
		(let ((car1 . (car lst1))
					(car2 . (car lst2))
					(cdr1 . (cdr lst1))
					(cdr2 . (cdr lst2))
					(rslt . (fun car1 car2)))
			(if (and cdr1 cdr2)
				(iter2 fun cdr1 cdr2)
				rslt))))
