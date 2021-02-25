(load "iterators.l" '(std \ if nil? cons let car match cons cdr))
(use '(std \ if nil? cons let car match cons cdr))

#
# The comparator is expected to return a pair (CMD . VAL) where CMD is:
# * APPEND: append VAL to the accumulator
# * REPLACE: replace the head of the accumulator by VAL
# * SKIP: skip VAL
#
(def dedup (cmp lst)
	"Deduplicate list LST using comparator CMP."
	(foldr (\ (elt acc)
					 (if (nil? acc)
						 (cons elt acc)
						 (let (((cmd . val) . (cmp elt (car acc))))
							 (match cmd
								 (APPEND	. (cons val acc))
								 (REPLACE . (cons val (cdr acc)))
								 (SKIP		. acc)))))
		lst NIL))
