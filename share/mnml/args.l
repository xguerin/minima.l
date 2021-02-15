(load
	"len.l" "manips.l"
	'(logic or =)
	'(io prinl)
	'(math - >)
	'(std if nil? cdr cons car eval let prog))

(def args:skip (LST LEN)
	"Extract sublist of LEN from LST."
	(if (or (= 0 LEN) (nil? LST))
		LST
		(args:skip (cdr LST) (- LEN 1))))

(def args:parse_r (SCHEMA LST RES)
	"Recursively match flags in LST with SCHEMA."
	(if (nil? LST)
		RES
		(let (((hd . tl) . LST)
					(fun . (eval (assoc hd SCHEMA)))
					(sze . (len (car fun)))
					(nxt . (args:skip tl sze)))
			(if (nil? fun)
				(prog
					(prinl "Unknown flag: " hd)
					NIL)
				(if (> sze (len tl))
					(prog
						(prinl "Not enough values for flag: " hd)
						NIL)
					(prog
						(eval (cons (cons 'quote fun) (truncl sze tl)))
						(args:parse_r SCHEMA nxt (cons hd RES)))
					)))))

(def args:parse (SCHEMA)
	"Parse ARGV according to SCHEMA."
	(args:parse_r SCHEMA (cdr ARGV) NIL))
