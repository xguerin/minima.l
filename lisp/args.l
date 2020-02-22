(load "@lib/assoc.l" "@lib/len.l")

(def args:skip (LST LEN)
  "Extract sublist of LEN from LST."
  (if (or (= 0 LEN) (nil? LST))
    LST
    (args:skip (cdr LST) (- LEN 1))))

(def args:sublist (LST LEN)
  "Extract sublist of LEN from LST."
  (if (or (= 0 LEN) (nil? LST))
    NIL
    (cons (car LST) (args:sublist (cdr LST) (- LEN 1)))))

(def args:parse_r (USAGE SCHEMA LST)
  "Recursively match flags in LST with SCHEMA."
  (let (((hd . tl) . LST)
        (fun . (eval (assoc hd SCHEMA)))
        (sze . (len (car fun)))
        (nxt . (args:skip tl sze)))
    (if (nil? fun)
      (prog
        (prinl "Unknown flag: " hd)
        (USAGE)
        NIL)
      (if (> sze (len tl))
        (prog
          (prinl "Not enough values for flag: " hd)
          (USAGE)
          NIL)
        (prog
          (eval (cons (cons 'quote fun) (args:sublist tl sze)))
          (if (nil? nxt)
            T
            (args:parse_r USAGE SCHEMA nxt))
          )))))

(def args:parse (USAGE SCHEMA)
  "Parse ARGV according to SCHEMA."
  (args:parse_r USAGE SCHEMA (cdr (cdr ARGV))))
