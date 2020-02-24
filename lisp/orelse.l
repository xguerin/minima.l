(load '(std if))

(def <|> (VAL DEF)
  "If VAL then return VAL or return DEF."
  (if VAL VAL DEF))
