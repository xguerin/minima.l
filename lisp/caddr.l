(def caddr (x)
  "Call CDR CDR CAR on X."
  (|> (\ () x) cdr cdr car))
