(def rep (n atm)
  "Create a list of ATM repeated N times"
  (if (> n 0)
    (cons atm (rep (- n 1) atm))))
