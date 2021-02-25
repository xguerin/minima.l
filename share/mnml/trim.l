(load
	"iterators.l"
	"rev.l"
	'(logic = or)
	'(std \ |> car cdr cons if))

(use
	'(logic = or)
	'(std \ |> car cdr cons if))

(def whitespace? (chr)
	"Return T if CHR is a white space."
	(|> (= chr ^ ) (or (= chr ^\t)) (or (= chr ^\n)) (or (= chr ^\r))))

(def triml (strn)
	"Trim STRN of leading white spaces."
	(if (whitespace? (car strn))
		(triml (cdr strn))
		strn))

(def trimr (strn)
	"Trim STRN of trailing white spaces."
	(rev (triml (rev strn))))

(def trim (strn)
	"Trim STRN of leading and trailing white spaces."
	(trimr (triml strn)))
