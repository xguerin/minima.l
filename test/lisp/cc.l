(load "@lib/test.l" "@lib/cc.l"
	'(logic and not)
	'(math + - * / <=)
	'(std cdr prog let list |> num? nil? unless))

(def test:build (SYM)
	"Compile and build SYM."
	(cc:build SYM (cc:compile SYM (list SYM))))

(setq DELTA 27)

(test:run
	"Compiler operations"
	#
	# Basic functions.
	#
	("add"	. (prog
							(def _add (a b) (+ a b))
							(let ((res	. (test:build '_add))
										(bod 	. (|> _add cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_add) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal 2 (_add 1 1)))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	("sub"	. (prog
							(def _sub (a b) (- a b))
							(let ((res	. (test:build '_sub))
										(bod 	. (|> _sub cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_sub) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal 0 (_sub 1 1)))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	("mul"	. (prog
							(def _mul (a b) (* a b))
							(let ((res	. (test:build '_mul))
										(bod 	. (|> _mul cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_mul) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal 8 (_mul 2 4)))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	("div"	. (prog
							(def _div (a b) (/ a b))
							(let ((res	. (test:build '_div))
										(bod 	. (|> _div cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_div) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal 3 (_div 9 3)))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	#
	# IF/THEN.
	#
	("ith"	. (prog
							(def _ith (a) (if (not (nil? a)) (cdr a)))
							(let ((res	. (test:build '_ith))
										(bod 	. (|> _ith cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_ith) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal NIL (_ith NIL)))
										(and (assert:equal '(2) (_ith '(1 2))))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	("unless" . (prog
								(def _unless (a) (unless (nil? a) (cdr a)))
								(let ((res	. (test:build '_unless))
											(bod 	. (|> _unless cdr cdr))
											(pre 	. (test:slabdelta)))
									(|> T
											(and (assert:equal '(_unless) res))
											(and (assert:predicate 'num? bod))
											(and (assert:equal NIL (_unless NIL)))
											(and (assert:equal '(2) (_unless '(1 2))))
											(and (assert:equal (+ pre DELTA) (test:slabdelta)))
											))))
	("when" . (prog
							(def _when (a) (if (not (nil? a)) (cdr a)))
							(let ((res	. (test:build '_when))
										(bod 	. (|> _when cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_when) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal NIL (_when NIL)))
										(and (assert:equal '(2) (_when '(1 2))))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	#
	# IF/THEN/ELSE.
	#
	("ite"	. (prog
							(def _ite (a b) (if (< a b) a b))
							(let ((res	. (test:build '_ite))
										(bod 	. (|> _ite cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_ite) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal 1 (_ite 1 3)))
										(and (assert:equal 1 (_ite 3 1)))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	("ite_nested" . (prog
										(def _ite_nested (a b c) (if (< a b) (if (> b c) b c) a))
										(let ((res	. (test:build '_ite_nested))
													(bod 	. (|> _ite_nested cdr cdr))
													(pre 	. (test:slabdelta)))
											(|> T
													(and (assert:equal '(_ite_nested) res))
													(and (assert:predicate 'num? bod))
													(and (assert:equal 3 (_ite_nested 3 2 1)))
													(and (assert:equal 3 (_ite_nested 2 3 1)))
													(and (assert:equal 4 (_ite_nested 2 3 4)))
													(and (assert:equal (+ pre DELTA) (test:slabdelta)))
													))))
	#
	# Fibonacci.
	#
	("fib"	. (prog
							(def _fib (N) (if (<= N 1) N (+ (_fib (- N 1)) (_fib (- N 2)))))
							(let ((res	. (test:build '_fib))
										(bod 	. (|> _fib cdr cdr))
										(pre 	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_fib) res))
										(and (assert:predicate 'num? bod))
										(and (assert:equal 55 (_fib 10)))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	#
	# Folds.
	#
	("foldl"	. (prog
								(def _foldl (fun acc lst)
									(if (lst? lst)
										(if (nil? lst) acc (_foldl fun (fun acc (car lst)) (cdr lst)))
										(fun acc lst)))
								(let ((res	. (test:build '_foldl))
											(bod 	. (|> _foldl cdr cdr))
											(pre 	. (test:slabdelta)))
									(|> T
											(and (assert:equal '(_foldl) res))
											(and (assert:predicate 'num? bod))
											(and (assert:equal 3 (_foldl (\ (acc _) (+ acc 1)) 0 '(1 2 3))))
											(and (assert:equal 4 (_foldl (\ (acc _) (+ acc 1)) 0 '(1 2 3 4))))
											(and (assert:equal 5 (_foldl (\ (acc _) (+ acc 1)) 0 '(1 2 3 4 5))))
											(and (assert:equal (+ pre DELTA) (test:slabdelta)))
											))))
	("foldr"	. (prog
								(def _foldr (fun lst acc)
									(if (lst? lst)
										(if (nil? lst) acc (fun (car lst) (_foldr fun (cdr lst) acc)))
										(fun lst acc)))
								(let ((res	. (test:build '_foldr))
											(bod 	. (|> _foldr cdr cdr))
											(pre 	. (test:slabdelta)))
									(|> T
											(and (assert:equal '(_foldr) res))
											(and (assert:predicate 'num? bod))
											(and (assert:equal 3 (_foldr (\ (_ acc) (+ acc 1)) '(1 2 3) 0)))
											(and (assert:equal 4 (_foldr (\ (_ acc) (+ acc 1)) '(1 2 3 4) 0)))
											(and (assert:equal 5 (_foldr (\ (_ acc) (+ acc 1)) '(1 2 3 4 5) 0)))
											(and (assert:equal (+ pre DELTA) (test:slabdelta)))
											))))
	#
	("map"	. (prog
								(def _map (fun lst)
									(if (and (not (nil? lst)) (lst? lst))
										(cons (fun (car lst)) (_map fun (cdr lst)))
										lst))
								(let ((res	. (test:build '_map))
											(bod 	. (|> _map cdr cdr))
											(pre 	. (test:slabdelta)))
									(|> T
											(and (assert:equal '(_map) res))
											(and (assert:equal '(2 3 4) (_map (\ (e) (+ e 1)) '(1 2 3))))
											(and (assert:equal '(0 1 2) (_map (\ (e) (- e 1)) '(1 2 3))))
											(and (assert:predicate 'num? bod))
											(and (assert:equal (+ pre DELTA) (test:slabdelta)))
											))))
	#
	# Func calls.
	#
	("func"	. (prog
							(def _add (A B) (+ A B))
							(def _fnc (A B) (_add A B))
							(let ((res	. (test:build '_fnc))
										(bod	. (|> _fnc cdr cdr))
										(pre	. (test:slabdelta)))
								(|> T
										(and (assert:equal '(_fnc) res))
										(and (assert:equal 2	(_fnc 1 1)))
										(and (assert:predicate 'num? bod))
										(and (assert:equal (+ pre DELTA) (test:slabdelta)))
										))))
	)
