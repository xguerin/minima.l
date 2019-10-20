(def ntoa:descend (n res)
  (if (= n 0)
    res
    (ntoa:descend (/ n 10) (cons (chr (+ (% n 10) ^0)) res))))

(def ntoa (n)
  "Convert a number N into a list of character."
  (if (= n 0)
    "0"
    (ntoa:descend n NIL)))
