(load
	"@lib/test.l"
	'(math + - * /))

(test:run
	"Arithmetic operations"
	#
	# Addition.
	#
	("add_1" . (assert:equal 2 (+ 1 1)))
	("add_2" . (assert:equal 4 (+ (+ 1 2) 1)))
	("add_3" . (assert:equal 5 (+ (+ 1 2) (+ 1 1))))
	#
	# Subtraction.
	#
	("sub_1" . (assert:equal  0 (- 1 1)))
	("sub_2" . (assert:equal -1 (- (- 1 1) 1)))
	("sub_3" . (assert:equal -1 (- (- 2 1) (- 3 1))))
	#
	# Multiplication.
	#
	("mul_1" . (assert:equal 1 (* 1 1)))
	("mul_2" . (assert:equal 1 (* (* 1 1) 1)))
	("mul_3" . (assert:equal 6 (* (* 2 1) (* 3 1))))
	#
	# Division.
	#
	("div_1" . (assert:equal 1 (/ 1 1)))
	("div_2" . (assert:equal 1 (/ (* 1 1) 1)))
	("div_3" . (assert:equal 2 (/ (/ 2 1) (/ 2 2))))
	#
	)
