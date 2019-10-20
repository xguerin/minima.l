(def spliN (lst chr end)
  (if lst
    (if (= (car lst) chr)
      (cdr lst)
      (spliN (cdr lst) chr (cdr (conc end (list (car lst)))))
      )))

(def split (lst chr)
  (if lst
    (if (= (car lst) chr)
      (split (cdr lst) chr)
      (let ((beg . (list (car lst)))
            (nxt . (spliN (cdr lst) chr beg)))
        (cons beg (split nxt chr))
        ))))
