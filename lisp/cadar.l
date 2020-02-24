(load '(std car cdr))

(def cadar (x)
  "Call CAR CDR CAR on X."
  (car (cdr (car x))))
