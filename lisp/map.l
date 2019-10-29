(def map (fun lst)
  "Sequentially apply FUN to all elements of LST."
  (if (and (not (nil? lst)) (lst? lst))
    (cons
      (fun (car lst))
      (map fun (cdr lst)))
    lst))
