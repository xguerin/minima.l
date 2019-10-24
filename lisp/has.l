(load "foldl.l")

(def has (LST X)
  "Check if X is in LST"
  (foldl (\ (acc e) (or (= e X) acc)) NIL LST))
