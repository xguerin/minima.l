(load
	'(logic = and not or)
	'(math <)
	'(std car cdr let nil?))

(def strlt (STRA STRB)
	"Strict less operator for strings"
	(and
		(not (nil? STRB))
		(or
			(nil? STRA)
			(let ((A . (car STRA))
						(B . (car STRB)))
				(or
					(< A B)
					(and
						(= A B)
						(strlt (cdr STRA) (cdr STRB))))))))
