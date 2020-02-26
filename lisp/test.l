(load
	"len.l"
	"term.l"
	'(io prin printl prinl)
	'(iterators foldl)
	'(logic = and)
	'(math >)
	'(std let if eval cond prog car tru?)
	'(sys slabinfo))

(def test:run (SUITE . TESTS)
	"Run all tests specified as a ((NAME . TEST)) list."
	(prinl (term:bold "[") (term:bold SUITE) (term:bold "]"))
	(let ((pad . (foldl (\ (acc (name . _))
												(let ((nlen . (len name)))
													(if (> nlen acc) nlen acc)))
								 0 TESTS))
				(|<- . (term:left pad)))
		(foldl (\ (acc (name . test))
						 (and acc
							 (cond (eval test)
								 (tru?	. (prog (prinl "✓ " (|<- name) ": " (term:green "passed")) T))
								 (_		 . (prog (prinl "! " (|<- name) ": " (term:red "failed")) NIL)))))
			T TESTS)))

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
