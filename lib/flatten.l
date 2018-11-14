(load "@lib/foldr.l")

(def flatten (lst)
  "Flatten k-deep LST into a 1-deep list."
  (?! (nil? lst)
      (?: (lst? lst)
          (foldr (\ (lst acc) (conc (flatten lst) acc)) lst NIL)
          (list lst))))
