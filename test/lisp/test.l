(load "../../lisp/foldl.l" "../../lisp/term.l")

(def test:run (SUITE . TESTS)
  "Run all tests specified as a ((NAME . TEST)) list."
  (prinl (term:bold "[") (term:bold SUITE) (term:bold "]"))
  (foldl (\ (acc (name . test))
           (and acc
             (cond (eval test)
               (tru?  . (prog (prinl "✓ " name ": " (term:green "passed")) T))
               (_     . (prog (prinl "! " name ": " (term:red "failed")) NIL)))))
    T TESTS))

(def assert:equal (EXPECT . ACTUAL)
  "Assert that EXPECT and ACTUAL are equal."
  (let ((OP . (car ACTUAL))
        (EVAL . (eval OP)))
    (if (= EXPECT EVAL)
      T
      (prog
        (print '! EXPECT '= OP)
        (prin " FAILED ")
        (print EXPECT)
        (prin " <> ")
        (printl EVAL)
        ))))
