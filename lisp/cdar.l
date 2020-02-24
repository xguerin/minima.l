(load '(std car cdr))

(def cdar (x)
  "Call CAR CDR on X."
  (cdr (car x)))
