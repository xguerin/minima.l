(def pretty (e)
  (?: (atm? e)
    (prin e)
    (prog
      (prin '(')
      (let ((hd . (car e))
            (tl . (cdr e)))
        (prog
          (pretty hd)
          (? (sym? hd) (prinl))
          (pretty-list tl)
          )))))

(def pretty-list (lst)
  (?: (nil? lst)
    (prin ')')
    (prog
      (prin ' ')
      (pretty (car lst))
      (pretty-list (cdr lst))
      )))
