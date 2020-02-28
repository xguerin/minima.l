(load
	"@lib/test.l" "@lib/cps.l"
	'(math + - * <=)
	'(std let \ car))

(test:run
	"CPS reference counting"
	#
	# Basic function.
	#
	("basic"	. (let ((fun	. (\ (a b) (+ a b)))
										(exp	. '((_2 . 0) (b . 1) (a . 1)))
										(act	. (cps:refcount (>& 'fun))))
								(assert:equal exp act)))
	#
	# Nested function.
	#
	("nested"	. (let ((fun	. (\ (a b) (+ (- a 1) (* 2 b))))
										(exp	. '((a . 1) (b . 1) (_2 . 1) (_3 . 0) (_4 . 1)))
										(act	. (cps:refcount (>& 'fun))))
								(assert:equal exp act)))
	#
	# Recursive function.
	#
	("recursive"	. (let ((fun	. (\ (N) (if (<= N 1) N (+ (fun (- N 1)) (fun (- N 2))))))
												(exp 	. '((_6 . 0) (_5 . 0) (N . 4) (_3 . 0) (_1 . 2) (_2 . 0) (_4 . 1)))
												(act 	. (cps:refcount (>& 'fun))))
										(assert:equal exp act)))
	#
	# List arguments.
	#
	("list"	. (let ((fun	. (\ (x) (car (car (car x)))))
									(exp	. '((x . 1) (_3 . 0) (_1 . 1) (_2 . 0)))
									(act	. (cps:refcount (>& 'fun))))
							(assert:equal exp act)))
	#
	# Unused argument.
	#
	("unused"	. (let ((fun	. (\ (x) (car (car (car '(1 2))))))
										(exp	. '((_4 . 0) (_3 . 0) (_1 . 1) (_2 . 0)))
										(act	. (cps:refcount (>& 'fun))))
								(assert:equal exp act)))
	#
	)
