(load "@lib/foldr.l")

(def split (lst chr)
  "Split the string LST by char CHR."
  (let ((push . (\ ((res . acc))
                  (if (not (nil? acc))
                    (list (cons acc res))
                    (cons res acc))))
        (rslt . (foldr (\ (elt (res . acc))
                 (if (= elt chr)
                   (push (cons res acc))
                   (cons res (cons elt acc))))
          lst NIL)))
    (|> rslt push car)))
