(load "rev.l" "truncl.l" '(std |>))

(def truncr (n lst)
  "Truncate a list LST by N elements from the right."
  (|> lst rev (truncl n) rev))
