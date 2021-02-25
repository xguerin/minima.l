(load "@lib/test.l" '(std let cons))
(use '(std let cons))

(test:run
	"List deconstruction"
	#
	("basic_0"	. (let (((a (b . c) d . e) . '(1 (2 . 3) 4 . 5)))
									(assert:equal 1 a)
									(assert:equal 2 b)
									(assert:equal 3 c)
									(assert:equal 4 d)
									(assert:equal 5 e)))
	("basic_1"	. (let (((a (b . c) d . e) . '(1 (2) 4)))
									(assert:equal 1		a)
									(assert:equal 2 	b)
									(assert:equal NIL c)
									(assert:equal 4		d)
									(assert:equal NIL e)))
	#
	)
