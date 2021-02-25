(load '(logic =) '(math +) '(std eval list))
(use '(logic =) '(math +) '(std eval list))

(def add (A B) (+ A B))

(= (eval (list 'add 1 1)) 2)
