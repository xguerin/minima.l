(load "test.l" "../../lisp/cps.l")

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
                    (exp . '((a . NUMBER) (b . NUMBER) (_2 . NUMBER) (_3 . NUMBER) (_4 . NUMBER)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  # Recursive function.
  #
  ("recursive" . (let ((fun . (\ (N) (if (<= N 1) N (+ (fun (- N 1)) (fun (- N 2))))))
                       (exp . '((_5 . NUMBER) (_3 . NUMBER) (N . NUMBER) (_1 . NUMBER) (_2 . NUMBER) (_4 . NUMBER) (_6 . NUMBER)))
                       (act . (cps:infer 'fun (>& 'fun))))
                   (assert:equal exp act)))
  #
  # List arguments.
  #
  ("list_0" . (let ((fun . (\ (x) (car (car (car x)))))
                    (exp . '((x . LIST) (_3 . LIST) (_1 . LIST) (_2 . LIST)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  # Type mismatch.
  #
  ("mismatch" . (let ((fun . (\ (x y) (+ y (car (car (car x))))))
                      (exp . '((x . LIST) (_5 . LIST) (_4 . LIST) (_2 . NUMBER) (_3 . ERROR) (y . NUMBER)))
                      (act . (cps:infer 'fun (>& 'fun))))
                  (assert:equal exp act)))
  #
  # Unused argument.
  #
  ("unused" . (let ((fun . (\ (x) (car (car (car '(1 2))))))
                    (exp . '((_4 . LIST) (_3 . LIST) (_1 . LIST) (_2 . LIST)))
                    (act . (cps:infer 'fun (>& 'fun))))
                (assert:equal exp act)))
  #
  )
