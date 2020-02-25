(load
  '(logic and not)
  '(std if lst? nil? car cdr cons))

(def foldl (fun acc lst)
  "Left-fold FUN over LST with ACC as the default accumulator value."
  (if (lst? lst)
    (if (nil? lst)
      acc
      (foldl fun (fun acc (car lst)) (cdr lst)))
    (fun acc lst)))

(def foldr (fun lst acc)
  "Right-fold FUN over LST with ACC as the default accumulator value."
  (if (lst? lst)
    (if (nil? lst)
      acc
      (fun (car lst) (foldr fun (cdr lst) acc)))
    (fun lst acc)))

(def map (fun lst)
  "Sequentially apply FUN to all elements of LST."
  (if (and (not (nil? lst)) (lst? lst))
    (cons
      (fun (car lst))
      (map fun (cdr lst)))
    lst))
