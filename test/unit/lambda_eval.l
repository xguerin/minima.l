(load '(logic =) '(math +) '(std \ eval))
(use '(logic =) '(math +) '(std \ eval))

(= (eval '((\ (A B) (+ A B)) 1 1)) 2)
