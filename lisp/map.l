(def map (fun lst)
  "Sequentially apply FUN to all elements of LST."
  (if (and lst (lst? lst))
    (cons
      (fun (car lst))
      (map fun (cdr lst)))))
