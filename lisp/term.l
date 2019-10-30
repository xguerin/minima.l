(load "append.l" "iter.l" "len.l" "rep.l" "truncl.l" "truncr.l")

#
# Escape STR with CODE.
#
(def term:esc (CODE STR)
  "Escape STR with CODE."
  (|>
    "[0m"
    (cons (chr 27))
    ($+ STR)
    (cons ^m)
    ($+ CODE)
    (cons ^[)
    (cons (chr 27))))

#
# Generate formatting escape definitions.
#
(iter (\ ((name . code))
        (let ((symb . (sym (append "term:" name)))
              (body . (list 'term:esc code 'STR)))
         (eval (list 'def symb '(STR) body))))
   '(("normal"     . "0")
     ("bold"       . "1")
     ("italic"     . "3")
     ("underlined" . "4")
     ("reversed"   . "7")
     ("striked"    . "9")
     #
     ("black"   . "0;30")
     ("red"     . "0;31")
     ("green"   . "0;32")
     ("orange"  . "0;33")
     ("blue"    . "0;34")
     ("purple"  . "0;35")
     ("cyan"    . "0;36")
     ("gray+"   . "0;37")
     ("gray-"   . "1;30")
     ("red+"    . "1;31")
     ("green+"  . "1;32")
     ("yellow"  . "1;33")
     ("blue+"   . "1;34")
     ("purple+" . "1;35")
     ("cyan+"   . "1;36")
     ("white"   . "1;37")
     ))

#
# Padding operations.
#
(def term:left (WLEN STR)
  "Pad STR to the left using WLEN length."
  (let ((delta . (- WLEN (len STR))))
    (if (>= delta 0)
      ($+ STR (rep delta ^ ))
      (truncl WLEN STR))))

(def term:right (WLEN STR)
  "Pad STR to the right using WLEN length."
  (let ((delta . (- WLEN (len STR))))
    (if (>= delta 0)
      ($+ (rep delta ^ ) STR)
      (truncr WLEN STR))))
