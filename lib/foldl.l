(def foldl (fun acc lst)
  "Left-fold FUN over LST with ACC as the default accumulator value."
  (?: (lst? lst)
      (?: (nil? lst)
          acc
          (foldl fun (fun acc (car lst)) (cdr lst)))
      (fun acc lst)))
