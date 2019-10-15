(load "@lib/foldl.l")

(def rev (lst)
  "Reverse LST."
  (foldl (\ (acc e) (cons e acc)) NIL lst))
