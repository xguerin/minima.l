(def foldr (fun lst acc)
  "Right-fold FUN over LST with ACC as the default accumulator value."
  (?: (lst? lst)
      (?: (nil? lst)
          acc
          (fun (car lst) (foldr fun (cdr lst) acc)))
      (fun lst acc)))
