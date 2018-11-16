(def cadar (x)
  "Call CAR CDR CAR on X."
  (|> 'x car cdr car))
