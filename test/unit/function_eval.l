(load '(logic =) '(math +) '(std eval))
(use '(logic =) '(math +) '(std eval))

(def add (A B) (+ A B))

(= (eval '(add 1 1)) 2)
