(load "@lib/test.l" '(logic =))
(use '(logic =))

(test:run
	"Comparison operations"
	#
	# =
	#
	("equ_1"	. (assert:equal T (= 1 1)))
	("equ_2"	. (assert:equal T (= T T)))
	("equ_3"	. (assert:equal T (= NIL NIL)))
	("equ_4"	. (assert:equal T (= NIL '())))
	("equ_5"	. (assert:equal T (= "hello" "hello")))
	#
	)
