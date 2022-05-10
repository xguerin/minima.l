# Deconstruction

Deconstruction, or structural matching, is a mechanism that structurally match
lisp values with patterns. For instance:
```
A       (1 . 2) -> A = (1 . 2)
(A . B) (1 . 2) -> A = 1, B = 2
```
Deconstruction works by first matching a lisp value with a pattern, and then
associating named variables with the corresponding element in the value. Named
variables that cannot be matched are set to `NIL`.

Matched variables are made available in the local closure of the execution
context. As such, the following statements are functionally equivalent:
```minimal
(def ((A . B)) (prinl A B))

(def (X)
  (let (((A . B) . X))
    (prinl A B)))
```
