(load "foldl.l")

(def len (lst)
  "Return the length of LST."
  (foldl (\ (acc _) (+ acc 1)) 0 lst))
