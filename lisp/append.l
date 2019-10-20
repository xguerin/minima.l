(def append (x y)
  "Non-destructively concatenate X and Y."
  (if (nil? x)
    y
    (cons (car x) (append (cdr x) y))))
