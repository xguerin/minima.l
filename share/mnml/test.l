(load
	"args.l"
	"has.l"
	"iter.l"
	"iterators.l"
	"split.l"
	"term.l"
	'(io prin printl prinl)
	'(logic = and)
	'(math >)
	'(std len let if eval cond prog car setq tru?)
	'(sys slabinfo))

(use
	'(io prin printl prinl)
	'(logic = and)
	'(math >)
	'(std len let if eval cond prog car setq tru?)
	'(sys slabinfo))

(def usage ()
  "Print out a usage string for the testsuite."
  (let ((arg0 . (car (cdr ARGV)))
        (prgn . (car (rev (split arg0 ^/)))))
    (prinl "Usage: " prgn " [-l|-t test1,test2,...|-h] ")
    (prinl)
    (prinl "  -l  List available tests")
    (prinl "  -t  Comma-separated list of tests")
    (prinl "  -h  Print this help")
    ))

(setq options
  '(("-l" . (\ ()			(setq LST T)))
    ("-t" . (\ (LST)	(setq TST (map (\ (e) (cons e NIL)) (split LST ^,)))))
    ("-h" . usage)))

(def test:run (SUITE . TESTS)
	"Run all tests specified as a ((NAME . TEST)) list."
	(let ((args . (args:parse options))
				(hdr	. (\ () (prinl (term:bold "[") (term:bold SUITE) (term:bold "]"))))
				(pad	. (\ (LST) (foldl (\ (acc (name . _))
																	(let ((nlen . (len name)))
																		(if (> nlen acc) nlen acc)))
													 0 LST)))
				(exe	. (\ (acc (name . _)) (let ((test . (assoc name TESTS)))
									 (if test
										 (and acc
											 (cond (eval test)
												 (tru?	. (prog (prinl "✓ " (|<- name) ": " (term:green "passed")) T))
												 (_			. (prog (prinl "! " (|<- name) ": " (term:red "failed")) NIL))))
										 (prog
											 (prinl "! " (|<- name) ": " (term:red "not found"))
											 NIL))))))
		(if (has args "-t")
			(let ((|<- . (term:left (pad TST))))
				(hdr)
				(foldl exe T TST))
			(let ((|<- . (term:left (pad TESTS))))
				(if (has args "-l")
					(prog
						(hdr)
						(iter (\ ((name . _)) (prinl "- " (|<- name))) TESTS))
					(when (not (has args "-h"))
						(hdr)
						(foldl exe T TESTS))
					)))))

(def test:slabdelta ()
	"Return the slab allocator position."
	(let (((alloc . free) . (slabinfo))) (- alloc free)))

(def assert:equal (EXPECT . ACTUAL)
	"Assert that EXPECT and ACTUAL are equal."
	(let ((OP . (car ACTUAL))
				(EVAL . (eval OP)))
		(if (= EXPECT EVAL)
			T
			(prog
				(prinl (term:red "> FAILED "))
				(prin "! " (term:underlined "Expect") ": ")
				(printl EXPECT)
				(prin "! " (term:underlined "Actual") ": ")
				(printl EVAL)
				NIL
				))))

(def assert:predicate (PREDICATE . VALUE)
	"Assert that VALUE matches the PREDICATE."
	(let ((OP . (car VALUE))
				(EVAL . (eval OP)))
		(if (PREDICATE EVAL)
			T
			(prog
				(prinl (term:red "> FAILED "))
				(prin "! " (term:underlined "Predicate") ": ")
				(printl PREDICATE)
				(prin "! " (term:underlined "Value") ": ")
				(printl EVAL)
				NIL
				))))
