(load
	"@lib/test.l"
	'(std match cond num? str? sym? chr?))

(test:run
	"Control operations"
	#
	# Match.
	#
	("match_integer"	. (let ((result . (match 1 (1 . T) (SYMBOL) ("STRING") (^c) (_))))
												assert:equal result T))
	("match_symbol"		. (let ((result . (match 'SYMBOL (1) (SYMBOL . T) ("STRING") (^c) (_))))
												assert:equal result T))
	("match_string"		. (let ((result	. (match "STRING" (1) (SYMBOL) ("STRING" . T) (^c) (_))))
												assert:equal result T))
	("match_char"			. (let ((result	. (match ^c (1) (SYMBOL) ("STRING") (^c . T) (_))))
												assert:equal result T))
	("match_any"			. (let ((result	. (match 'any (1) (SYMBOL) ("STRING") (^c) (_ . T))))
												assert:equal result T))
	#
	# Match with deconstruction.
	#
	("match_deconstruct_0"	. (let ((result . (match '(1) ((1) . T) ((1 . 2)) ((1 2)) (_))))
															assert:equal result T))
	("match_deconstruct_1"	. (let ((result . (match '(1 . 2) ((1)) ((1 . 2) . T) ((1 2)) (_))))
															assert:equal result T))
	("match_deconstruct_2"	. (let ((result . (match '(1 2) ((1)) ((1 . 2)) ((1 2) . T) (_))))
															assert:equal result T))
	("match_deconstruct_3"	. (let ((result . (match '(1 2 3) ((1)) ((1 . 2)) ((1 2)) (_ . T))))
															assert:equal result T))
	#
	# Cond.
	#
	("cond_integer"	. (let ((result	. (cond 1 (num? . T) (sym?) (str?) (chr?) (_))))
											assert:equal result T))
	("cond_symbol"	. (let ((result	. (cond 'SYMBOL (num?) (sym? . T) (str?) (chr?) (_))))
											assert:equal result T))
	("cond_string"	. (let ((result	. (cond "STRING" (num?) (sym?) (str? . T) (chr?) (_))))
											assert:equal result T))
	("cond_char"		. (let ((result	. (cond ^c (num?) (sym?) (str?) (chr? . T) (_))))
											assert:equal result T))
	("cond_any"			. (let ((result	. (cond T (num?) (sym?) (str?) (chr?) (_ . T))))
											assert:equal result T))
	#
	)
