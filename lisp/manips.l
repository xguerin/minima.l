(load "rev.l" '(logic =) '(math > -) '(std |> car cdr cons if when))

(def caar (x)
  "Call CAR CAR on X."
  (car (car x)))

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

(def rep (n atm)
  "Create a list of ATM repeated N times"
  (if (> n 0)
    (cons atm (rep (- n 1) atm))))

(def truncl (n lst)
  "Truncate a list LST by N elements from the left."
  (when (> n 0)
    (cons (car lst) (truncl (- n 1) (cdr lst)))))
