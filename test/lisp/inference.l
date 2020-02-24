(load "@lib/test.l" "@lib/cps.l" '(math + - * < <=) '(std \ car let))

(test:run
  "Type inference"
  #
  # Basic function.
  #
  ("basic" . (let ((fun . (\ (a b) (+ a b)))
                   (exp . '((_2 . NUMBER) (b . NUMBER) (a . NUMBER)))
                   (act . (cps:infer 'fun (>& 'fun))))
               (assert:equal exp act)))
  #
  # Nested function.
  #
  ("nested" . (let ((fun . (\ (a b) (+ (- a 1) (* 2 b))))
                    (exp . '((_2 . NUMBER) (_3 . NUMBER) (b . NUMBER) (_4 . NUMBER) (a . NUMBER)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  # Recursive function.
  #
  ("fib"  . (let ((fun . (\ (N) (if (<= N 1) N (+ (fun (- N 1)) (fun (- N 2))))))
                  (exp . '((_2 . NUMBER) (_4 . NUMBER) (_3 . NUMBER) (_5 . NUMBER) (_1 . NUMBER) (_6 . NUMBER) (N . NUMBER)))
                  (act . (cps:infer 'fun (>& 'fun))))
              (assert:equal exp act)))
  #
  # List arguments.
  #
  ("list_0" . (let ((fun . (\ (x) (car (car (car x)))))
                    (exp . '((_1 . ATOM) (_2 . ATOM) (_3 . ATOM) (x . ATOM)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  # Type mismatch.
  #
  ("mismatch" . (let ((fun . (\ (x y) (+ y (car (car (car x))))))
                      (exp . '((_2 . NUMBER) (y . NUMBER) (_3 . ERROR) (_4 . ATOM) (_5 . ATOM) (x . ATOM)))
                      (act . (cps:infer 'fun (>& 'fun))))
                  (assert:equal exp act)))
  #
  # Unused argument.
  #
  ("unused" . (let ((fun . (\ (x y) (car (car (car x)))))
                    (exp . '((_2 . ATOM) (_3 . ATOM) (_4 . ATOM) (x . ATOM)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  # Single branch IF.
  #
  ("ifthen" . (let ((fun . (\ (x y) (if (< x y) (+ x y))))
                    (exp . '((_2 . ERROR) (_3 . NUMBER) (y . NUMBER) (x . NUMBER)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  # Functional argument
  #
  ("func" . (let ((fun . (\ (f y) (f y)))
                  (exp . '((f . ATOM) (_2 . ATOM) (y . ATOM)))
                  (act . (cps:infer 'fun (>& 'fun))))
              (assert:equal exp act)))
  #
  )
