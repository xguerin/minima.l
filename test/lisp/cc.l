(load "test.l" "../../lisp/cc.l")

(test:run
  "Compiler operations"
  #
  # Basic functions.
  #
  ("add"  . (prog
              (def _add (a b) (+ a b))
              (let ((res . (cc:build (cc:compile '_add)))
                    (bod . (|> _add cdr cdr cdr)))
                (|> T
                    (and (assert:equal '_add res))
                    (and (assert:predicate 'num? bod))
                    (and (assert:equal 2 (_add 1 1)))))))
  ("sub"  . (prog
              (def _sub (a b) (- a b))
              (let ((res . (cc:build (cc:compile '_sub)))
                    (bod . (|> _sub cdr cdr cdr)))
                (|> T
                    (and (assert:equal '_sub res))
                    (and (assert:predicate 'num? bod))
                    (and (assert:equal 0 (_sub 1 1)))))))
  ("mul"  . (prog
              (def _mul (a b) (* a b))
              (let ((res . (cc:build (cc:compile '_mul)))
                    (bod . (|> _mul cdr cdr cdr)))
                (|> T
                    (and (assert:equal '_mul res))
                    (and (assert:predicate 'num? bod))
                    (and (assert:equal 8 (_mul 2 4)))))))
  ("div"  . (prog
              (def _div (a b) (/ a b))
              (let ((res . (cc:build (cc:compile '_div)))
                    (bod . (|> _div cdr cdr cdr)))
                (|> T
                    (and (assert:equal '_div res))
                    (and (assert:predicate 'num? bod))
                    (and (assert:equal 3 (_div 9 3)))))))
  #
  # IF/THEN/ELSE
  #
  ("ite"  . (prog
              (def _ite (a b) (if (< a b) a b))
              (let ((res . (cc:build (cc:compile '_ite)))
                    (bod . (|> _ite cdr cdr cdr)))
                (|> T
                    (and (assert:equal '_ite res))
                    (and (assert:predicate 'num? bod))
                    (and (assert:equal 1 (_ite 1 3)))
                    (and (assert:equal 1 (_ite 3 1)))
                    ))))
  #
  # Fibonacci.
  #
  ("fib"  . (prog
              (def _fib (N) (if (<= N 1) N (+ (_fib (- N 1)) (_fib (- N 2)))))
              (let ((res . (cc:build (cc:compile '_fib)))
                    (bod . (|> _fib cdr cdr cdr)))
                (|> T
                    (and (assert:equal '_fib res))
                    (and (assert:predicate 'num? bod))
                    (and (assert:equal 55 (_fib 10)))))))
  #
  )
