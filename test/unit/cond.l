(load '(logic = and) '(std cond num? lst?))

(def test (v)
	(cond v
		(num? . 'number)
		(lst? . 'list)
		(_ . 'unknown)))

(and
	(= (test 1) 'number)
	(= (test '(a b)) 'list))
