(load "@lib/caar.l")
(load "@lib/cdar.l")

(def assoc (k lst)
  (if lst
    (if (= (caar lst) k)
      (cdar lst)
      (assoc k (cdr lst)))))
