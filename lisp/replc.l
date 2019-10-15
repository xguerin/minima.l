(load "@lib/foldr.l")

(def replc (k v lst)
  (foldr
    (\ ((_k . _v) acc)
      (if (= k _k) (cons (cons k v) acc) (cons (cons _k _v) acc)))
    lst NIL))
