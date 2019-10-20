(def foldr (fun lst acc)
  "Right-fold FUN over LST with ACC as the default accumulator value."
  (if (lst? lst)
      (if (nil? lst)
          acc
          (fun (car lst) (foldr fun (cdr lst) acc)))
      (fun lst acc)))
