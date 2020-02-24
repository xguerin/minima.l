(load '(std let car cdr if))

(def last (lst)
  "Return the last item of LST."
  (let ((head . (car lst))
        (tail . (car (cdr lst))))
    (if tail
      (last (cdr lst))
      head)))
