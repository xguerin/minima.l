(load '(std car cdr))

(def caddr (x)
  "Call CDR CDR CAR on X."
  (car (cdr (cdr x))))
