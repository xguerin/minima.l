(load "iter.l")

(def unless (COND . PROG)
  "Execute PROG when !COND."
  (if (not COND) (iter eval PROG)))
