(load "iterators.l" '(logic = or) '(std \))

(def has (LST X)
	"Check if X is in LST"
	(foldl (\ (acc e) (or (= e X) acc)) NIL LST))
