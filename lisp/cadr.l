(load '(std car cdr))

(def cadr (x)
  "Call CDR CAR on X."
  (car (cdr x)))
