(load '(logic and) '(std if lst? car cdr))

(def map2 (fun lst1 lst2)
  "Apply FUN to all elements of both LST1 and LST2."
  (if (and (and lst1 (lst? lst1)) (and lst2 (lst? lst2)))
    (cons
      (fun (car lst1) (car lst2))
      (map2 fun (cdr lst1) (cdr lst2)))))
