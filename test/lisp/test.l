(load "../../lisp/foldl.l")

(def test:run @
  "Run all tests specified as a ((NAME . TEST)) list."
  (foldl (\ (acc (name . test))
           (and acc
             (cond (eval test)
               (tru?  . (prog (prinl "✓ " name ": passed") T))
               (_     . (prog (prinl "! " name ": failed") NIL)))))
    T @))

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
