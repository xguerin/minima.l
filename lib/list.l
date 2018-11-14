(def cadr  (x) (|> (\ () x) cdr car))

(def caddr (x) (|> (\ () x) cdr cdr car))

(def foldl (fun acc lst)
  (?: (lst? lst)
      (?: (nil? lst)
          acc
          (foldl fun (fun acc (car lst)) (cdr lst)))
      (fun acc lst)))

(def foldr (fun lst acc)
  (?: (lst? lst)
      (?: (nil? lst)
          acc
          (fun (car lst) (foldr fun (cdr lst) acc)))
      (fun lst acc)))

(def map (fun lst)
  (? (and (lst? lst) (not (nil? lst)))
    (cons
      (fun (car lst))
      (map fun (cdr lst)))))

(def map2 (fun lst1 lst2)
  (? (and (lst? lst1) (lst? lst2))
    (?! (and (nil? lst1) (nil? lst2))
      (cons
        (fun (car lst1) (car lst2))
        (map2 fun (cdr lst1) (cdr lst2))))))

(def zip (lst1 lst2)
  (map2 cons lst1 lst2))

(def filter (fun lst)
  (foldr
    (\ (e acc) (?: (fun e) acc (cons e acc)))
    lst NIL))

(def flatten (lst)
  (?! (nil? lst)
      (?: (lst? lst)
          (foldr (\ (lst acc) (conc (flatten lst) acc)) lst NIL)
          (list lst))))

(def rev (lst)
  (foldl (\ (acc e) (cons e acc)) NIL lst))

(def len (lst)
  (foldl (\ (acc _) (+ acc 1)) 0 lst))
