(load "@lib/caar.l")
(load "@lib/cadar.l")

(def assoc (x y)
  (if (= (caar y) x)
    (cadar y)
    (assoc x (cdr y))))
