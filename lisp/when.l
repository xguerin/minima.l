(load "iter.l")

(def when (COND . PROG)
  "Execute PROG when COND."
  (if COND (iter eval PROG)))
