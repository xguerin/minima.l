(def truncl (n lst)
  "Truncate a list LST by N elements from the left."
  (if (> n 0)
    (cons (car lst) (truncl (- n 1) (cdr lst)))))
