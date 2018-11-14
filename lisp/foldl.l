(def foldl (fun acc lst)
  "Left-fold FUN over LST with ACC as the default accumulator value."
  (if (lst? lst)
      (if (nil? lst)
          acc
          (foldl fun (fun acc (car lst)) (cdr lst)))
      (fun acc lst)))
