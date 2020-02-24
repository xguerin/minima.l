(load '(std car))

(def caar (x)
  "Call CAR CAR on X."
  (car (car x)))
