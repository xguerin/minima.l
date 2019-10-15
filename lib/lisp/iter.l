(def iter (fun lst)
  "Iterate over list LST and call FUN on each item."
  (if lst
    (prog
      (fun (car lst))
      (iter fun (cdr lst)))))
