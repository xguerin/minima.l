(load '(logic =) '(math +) '(std \))

(def exec (FN A B) (FN A B))

(= (exec (\ (A B) (+ A B)) 1 1) 2)
