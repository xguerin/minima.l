(load "foldr.l")

(def filter (fun lst)
  "Filter out element of LST that match FUN."
  (foldr
    (\ (e acc) (if (fun e) acc (cons e acc)))
    lst NIL))
