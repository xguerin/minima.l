(load "foldl.l")

(def aton (STR)
  "Convert a string STR to a (positive) number."
  (foldl (\ (acc v) (+ (* 10 acc) (- v ^0))) 0 STR))
