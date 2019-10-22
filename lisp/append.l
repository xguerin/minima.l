(load "foldl.l" "foldr.l")

(def append (x . y)
  "Non-destructively concatenate X and Y."
  (foldl (\ (acc0 e0)
           (foldr (\ (e1 acc1) (cons e1 acc1)) acc0 (eval e0)))
    x y))
