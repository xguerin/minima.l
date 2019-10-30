(def truncl (n lst)
  "Truncate a list LST by N elements from the left."
  (when (> n 0)
    (cons (car lst) (truncl (- n 1) (cdr lst)))))
