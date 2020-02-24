(load "caar.l" "cdar.l" '(logic =) '(std cdr if))

(def assoc (k lst)
  "Return the value for K in the association list LST."
  (if lst
    (if (= (caar lst) k)
      (cdar lst)
      (assoc k (cdr lst)))))
