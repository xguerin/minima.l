(def iter (fun lst)
  "Iterate over list LST and call FUN on each item."
  (if lst
    (let ((rslt . (fun (car lst)))
          (tail . (cdr lst)))
      (if tail
        (iter fun (cdr lst))
        rslt))))
