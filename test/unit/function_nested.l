(load '(logic =) '(math +))

(def add (A B) (+ A B))

(def exec (FN A B) (FN A B))

(= (exec add 1 1) 2)
