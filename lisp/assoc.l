(load "@lib/caar.l")
(load "@lib/cdar.l")

(def assoc (x y)
  (if (= (caar y) x)
    (cdar y)
    (assoc x (cdr y))))
