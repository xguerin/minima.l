(load "rev.l" '(logic = not) '(math > -) '(std |> car cdr cons if nil? when))

(def assoc (k lst)
	"Return the value for K in the association list LST."
	(if (not (nil? lst))
		(if (= (caar lst) k)
			(cdar lst)
			(assoc k (cdr lst)))))

(def caar (x)
	"Call CAR CAR on X."
	(car (car x)))

(def cddr (x)
	"Call CDR CDR on X."
	(cdr (cdr x)))

(def cadar (x)
	"Call CAR CDR CAR on X."
	(car (cdr (car x))))

(def caddr (x)
	"Call CDR CDR CAR on X."
	(car (cdr (cdr x))))

(def cadr (x)
	"Call CDR CAR on X."
	(car (cdr x)))

(def cdar (x)
	"Call CAR CDR on X."
	(cdr (car x)))

(def last (lst)
	"Return the last item of LST."
	(if (nil? (cadr lst))
		(car lst)
		(last (cdr lst))))

(def rep (n atm)
	"Create a list of ATM repeated N times"
	(if (> n 0)
		(cons atm (rep (- n 1) atm))))

(def truncl (n lst)
	"Truncate a list LST by N elements from the left."
	(when (> n 0)
		(cons (car lst) (truncl (- n 1) (cdr lst)))))
