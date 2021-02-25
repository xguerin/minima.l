(load
	"append.l"
	"iterators.l"
	"rev.l"
	'(std let \ car cdr list))

(use '(std let \ car cdr list))

(def join (lst chr)
	(let ((rlst . (rev lst)))
		(foldl (\ (acc e) (append e (list chr) acc)) (car rlst) (cdr rlst))))
