(def spliN (lst chr beg end)
  "Stage 3: stable recursion."
  (if (nil? lst)
    (cons beg NIL)
    (if (= (car lst) chr)
      (cons beg (cdr lst))
      (spliN (cdr lst) chr beg (cdr (conc end (list (car lst)))))
      )))

(def spliY (lst chr beg)
  "Stage 2: use the beginning as both beginning and end."
  (spliN lst chr beg beg))

(def spli1 (lst chr)
  "Stage 1: create the first character list and call stage 2."
  (if (nil? lst)
    (cons beg NIL)
    (if (= (car lst) chr)
      (cons beg (cdr lst))
      (spliY (cdr lst) chr (list (car lst)))
      )))

(def split (lst chr)
  (if (not (nil? lst))
    (let (((acc . rst) . (spli1 lst chr)))
      (cons acc (split rst chr)))))
