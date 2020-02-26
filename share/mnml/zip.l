(load "iterators.l" '(std cons))

(def zip (lst1 lst2)
	"Sequentially pair-up elements from LST1 and LST2."
	(map2 cons lst1 lst2))
