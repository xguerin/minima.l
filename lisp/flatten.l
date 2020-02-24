(load "foldr.l" '(logic and) '(std if lst? \ conc list))

(def flatten (lst)
  "Flatten k-deep LST into a 1-deep list."
  (if (and lst (lst? lst))
    (foldr (\ (lst acc) (conc (flatten lst) acc)) lst NIL)
    (list lst)))
