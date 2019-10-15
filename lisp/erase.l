(load "@lib/foldr.l")

(def erase (k lst)
  (foldr
    (\ ((_k . _v) acc)
      (if (= k _k) acc (cons (cons _k _v) acc)))
    lst NIL))
