(load
	"append.l"
	"aton.l"
	"cps.l"
	"filter.l"
	"iter.l"
	"iter2.l"
	"iterators.l"
	"join.l"
	"manips.l"
	"rev.l"
	"run.l"
	'io 'logic 'math 'std 'sys 'unix)

#
# Indentation management.
#

(setq cc:level 0)

(def cc:indent_r (lvl)
	"Recursively build indentation string."
	(if (<> lvl 0)
		(cons " " (cc:indent_r (- lvl 1)))))

(def o| ()
	"Return the indentation string."
	(cc:indent_r cc:level))

(def o> ()
	"Increment the indentation level."
	(setq cc:level (+ cc:level 2)))

(def <o ()
	"Decrement the indentation level."
	(setq cc:level (- cc:level 2)))

#
# Helper functions.
#

(def /**/ @
	"Generate a C comment."
	(prinl (o|) "/*")
	(prin	(o|) " * ")
	(iter (\ (e) (prin (eval e))) @)
	(prinl)
	(prinl (o|) " */"))

(def cc:replace (kvar body)
	"Replace the continuation in BODY by KVAR."
	(if (cdr body)
		(cons (car body) (cc:replace kvar (cdr body)))
		(list kvar)))

(def cc:arg:idx (ARGS ref idx)
	"Get the index of REF in the closure based on ARGS."
	(if ARGS
		(if (= (car ARGS) ref)
			idx
			(cc:arg:idx (cdr ARGS) ref (+ idx 1)))))

(def cc:arg:get (ARGS)
	"Remove the continuation argument from ARGS."
	(rev (cdr (rev ARGS))))

(def cc:ref:idx (ARGS ref)
	"Get the index of REF in the closure."
	(let ((tostr . (str ref)))
		(if (= (car tostr) ^_)
			(aton (cdr tostr))
			(cc:arg:idx ARGS ref 0))))

#
# Operation generators.
#

(def cc:op:predicate (ARGS TYPES kvar body)
	"Generate C code for unary predicate operations."
	(let (((op arg cont) . body))
		(prin (o|) "value_t R = { .number = ")
		(match op
			(lst? . (prog
								(prin "IS_PAIR(")
								(cc:value:atm ARGS kvar arg)
								(prin ") || IS_NULL(")
								(cc:value:atm ARGS kvar arg)
								(prinl ") };")))
			(nil? . (prog
								(prin "IS_NULL(")
								(cc:value:atm ARGS kvar arg)
								(prinl ") };")))
			(_	 . (prinl " 0 }; /* Unsupported predicate " op " */")))))

(def cc:op:num:one (ARGS TYPES kvar body)
	"Generate C code for binary numeric operations."
	(let (((op arg cont)	. body))
		(prin (o|) "value_t R = { .number = ")
		(match op
			(not . (prin "!"))
			(_	 . (prin " /* Unsupported operation " op " */ ")))
		(cc:value:num ARGS kvar arg)
		(prinl " };")))

(def cc:op:num:two (ARGS TYPES kvar body)
	"Generate C code for binary numeric operations."
	(let (((op arg1 arg2 cont)	. body))
		(prin (o|) "value_t R = { .number = ")
		(cc:value:num ARGS kvar arg1)
		(match op
			#
			# Immediate translation to C.
			#
			~(foldl
				 (\ (acc e) (cons (cons e (list 'prin " " (str e) " ")) acc))
				 NIL '(+ - * / < <= > >=))
			#
			# Special cases.
			#
			(=		. (prin " == "))
			(<>		. (prin " != "))
			(and	. (prin " && "))
			(or		. (prin " || "))
			#
			# Unsupported cases.
			#
			(_		. (prin " /* Unsupported operation " op " */ ")))
		(cc:value:num ARGS kvar arg2)
		(prinl " };")))

(def cc:op:lst:one (ARGS TYPES kvar body)
	"Generate C code for unary list operations."
	(let (((op arg cont)	. body))
		(prin (o|) "value_t R = { .atom = ")
		(match op
			(car . (prin "lisp_car("))
			(cdr . (prin "lisp_cdr("))
			(_	 . (prin " /* Unsupported operation " op " */ ")))
		(cc:value:atm ARGS kvar arg)
		(prinl ") };")
		(when (= kvar arg) (cc:gen:x ARGS kvar arg))))

(def cc:op:lst:two (ARGS TYPES kvar body)
	"Generate C code for binary list operations."
	(let (((op arg1 arg2 cont)	. body))
		(prin (o|) "value_t R = { .atom = ")
		(match op
			(conc . (prin "lisp_conc(UP("))
			(cons . (prin "lisp_cons(UP("))
			(_		.	(prin " /* Unsupported operation " op " */ ")))
		(cc:value:atm ARGS kvar arg1)
		(prin "), UP(")
		(cc:value:atm ARGS kvar arg2)
		(prinl ")) };")
		(when (= kvar arg1) (cc:gen:x ARGS kvar arg1))
		(when (= kvar arg2) (cc:gen:x ARGS kvar arg2))))

(def cc:op:atm:two (ARGS TYPES kvar body)
	"Generate C code for binary ATOM operations."
	(let (((op arg1 arg2 cont)	. body)
				(typ1 . (assoc arg1 TYPES))
				(typ2 . (assoc arg2 TYPES)))
		(prin (o|) "value_t R = { .number = ")
		(match (cons typ1 typ2)
			((NUMBER . NUMBER)	. (prog
															(cc:value:num ARGS kvar arg1)
															(match op
																(=	. (prin " == "))
																(<>	. (prin " != "))
																(_	. (prin " /* Unsupported operation " op " */ ")))
															(cc:value:num ARGS kvar arg2)
															))
			((ATOM . ATOM)	.	(match op
													(=	. (prog
																	(prin "lisp_equ(")
																	(cc:value:atm ARGS kvar arg1)
																	(prin ", ")
																	(cc:value:atm ARGS kvar arg2)
																	(prin ")")
																	))
													(<>	. (prog
																	(prin "lisp_neq(")
																	(cc:value:atm ARGS kvar arg1)
																	(prin ", ")
																	(cc:value:atm ARGS kvar arg2)
																	(prin ")")
																	))
													(_	. (prin " 0 /* Unsupported operation " op " */ "))
													))
			(_	. (prin " 0 /* Unsupported types (" typ1 ", " typ2 ") */ ")))
		(prinl " };")
		(when (= kvar arg1) (cc:gen:x ARGS kvar arg1))
		(when (= kvar arg2) (cc:gen:x ARGS kvar arg2))))

(def cc:op:extcall (ARGS TYPES kvar (_ arg))
	"Generate C code for immediate KVAR funcall."
	(prin (o|) "value_t R = ")
	(cc:value:get ARGS kvar arg)
	(when (= 'ATOM (assoc arg TYPES))
		(prinl (o|) "UP(R.atom);")))

(def cc:op:funcall (ARGS TYPES kvar args)
	"Generate C code for external function calls."
	(let ((nvar			. (last args))
				(effargs	. (cc:arg:get args))
				(genquot	. (\ (arg)
											(prog
												(prin (o|) "atom_t " arg " = lisp_cons(lisp_make_quote(), UP(")
												(cc:value:atm ARGS kvar arg)
												(prinl "));")
												(when (= kvar arg) (cc:gen:x ARGS kvar arg))
												)))
				(gensymb	. (\ (arg)
											(prinl (o|) "MAKE_SYMBOL_STATIC(s_" arg ", \"" arg "\", " (len (str arg)) ");")
											(prinl (o|) "atom_t " arg " = lisp_make_symbol(s_" arg ");")
											))
				(genargs	. (\ (arg)
											(let ((type . (assoc arg TYPES)))
												(match type
													(NUMBER . (prog
																			(prin (o|) "atom_t " arg " = lisp_make_number(")
																			(cc:value:num ARGS kvar arg)
																			(prinl ");")))
													#
													# NOTE We must QUOTE atoms to prevent them from being
													# reevaluated because the what we manipulates are
													# immediate values and not symbols.
													#
													(ATOM . (if (cc:ref:idx ARGS arg)
																		(genquot arg)
																		(gensymb arg)))
													#
													# NOTE If no type was found for the ATOM, assume it's
													# a function symbol.
													#
													(NIL	. (gensymb arg))
													(_		. (prinl (o|) "atom_t " arg " = lisp_make_nil(); /* Unsupported type " (str type) " */"))
													))))
				(gencall	. (\ (arg)
											(prinl (o|) "_T = lisp_cons(" arg ", _S);")
											(prinl (o|) "_S = _T;")))
				(type			. (assoc nvar TYPES)))
		(iter genargs effargs)
		(/**/ "Build the expression to evaluate.")
		(prinl (o|) "atom_t _S = lisp_make_nil(), _T;")
		(iter gencall (rev effargs))
		(/**/ "Evaluate the expression.")
		(prinl (o|) "atom_t _R = lisp_eval(lisp, E, _S);")
		(/**/ "Convert the atom_t result to value_t.")
		(cc:gen:value type T 'R)))

(def cc:op:reccall (ARGS TYPES kvar body)
	"Generate C code for recursive calls."
	(iter2 (\ (arg prm)
					 (unless (= arg kvar)
						 (prin (o|) "value_t " arg " = ")
						 (cc:value:get ARGS kvar prm)
						 (when (and (<> kvar prm) (= 'ATOM (assoc arg TYPES)))
							 (prinl (o|) "UP(" arg ".atom);"))))
		ARGS (cdr body)))

#
# Value generators.
#

(def cc:gen:ref (ARGS kvar ref)
	"Generate the code for REF."
	(if (= kvar ref)
		(prin "V")
		(prin "C->V[" (cc:ref:idx ARGS ref) "]")))

(def cc:gen:value (type x arg)
	"Generate conversion from atom_t to value_t."
	(prin (o|) "value_t " arg " = ")
	(match type
		(NUMBER . (prinl "{ .number = _" arg "->number };"))
		(ATOM		. (prinl "{ .atom = UP(_" arg ") };"))
		(_			. (prinl "{ .number = 0 }; /* Unsupported type " (str atype) " */")))
	(if x (prinl (o|) "X(_" arg ");")))

(def cc:gen:x (ARGS kvar ref)
	"Generate X(REF)."
	(prin (o|) "X(") (cc:value:atm ARGS kvar ref) (prinl ");"))

(def cc:value:get (ARGS kvar arg)
	"Generate the C code for the ref R-value ARG."
	(cond arg
		(sym? . (cc:gen:ref ARGS kvar arg))
		(num? . (prin "{ .number = " arg " }"))
		(_		. (prin "abort()")))
	(prinl ";"))

(def cc:value:atm (ARGS kvar arg)
	"Generate the C code for the list R-value ARG."
	(cond arg
		(sym? . (prog (cc:gen:ref ARGS kvar arg) (prin ".atom")))
		(_		. (prin "abort()"))))

(def cc:value:num (ARGS kvar arg)
	"Generate the C code for the number R-value ARG."
	(cond arg
		(sym? . (prog (cc:gen:ref ARGS kvar arg) (prin ".number")))
		(num? . (prin arg))
		(_		. (prin "abort()"))))

#
# Intermediate continuations.
#

(def cc:gen:closure (CONT next)
	"Generate a default closure capture."
	(if (= CONT next)
		(prinl (o|) "closure_t _C = C->C;")
		(prinl (o|) "closure_t _C = C;")))

(def cc:gen:callback (NAME REFS CONT ref)
	"Generate the callback capture for REF."
	(if (= CONT ref)
		(if (= (assoc ref REFS) 0)
			(prinl (o|) "callback_t _K = V.callback;")
			(prinl (o|) "callback_t _K = C->V[" (cdr (str ref)) "].callback;"))
		(prinl (o|) "callback_t _K = _mnml_" NAME ref ";")))

(def cc:gen:cleanup (ARGS REFS TYPES CONT kvar ref)
	"Generate the variable clean-up."
	(let ((cnt . (len ARGS))
				(pos . (cc:ref:idx ARGS kvar)))
		(when (= CONT ref)
			(/**/ "Clean-up the placeholders.")
			(|> REFS
					(filter (\ ((_	 . count)) (= count 0)))
					(filter (\ ((arg . _		)) (= arg CONT)))
					(filter (\ ((arg . _		)) (let ((idx . (cc:ref:idx ARGS arg)))
																			 (and (>= idx cnt) (< idx pos)))))
					(filter (\ ((arg . _		)) (<> 'ATOM (assoc arg TYPES))))
					(filter (\ ((arg . _		)) (not (cc:ref:idx ARGS arg))))
					(iter	 (\ ((arg . _		)) (cc:gen:x ARGS NIL arg))))
			(/**/ "Clean-up the closure.")
			(prinl (o|) "lisp_closure_put(&cache, C);"))))

(def cc:gen:block-any (NAME ARGS REFS TYPES CONT kvar body succ op)
	"Common generation routing for block items of any form."
	(let ((next . (succ body)))
		(/**/ "Build the closure.")
		(cc:gen:closure CONT next)
		(/**/ "Collect the arguments.")
		(op ARGS TYPES kvar body)
		(/**/ "Build the continuation.")
		(cc:gen:callback NAME REFS CONT next)
		(cc:gen:cleanup ARGS REFS TYPES CONT kvar next)
		(/**/ "Call the continuation.")
		(if (= NAME (car body))
			(prog
				(prin (o|) "return _mnml_" NAME "(lisp, E, ")
				(iter (\ (e) (prin e ", ")) ARGS)
				(prinl "_C, _K);"))
			(prinl (o|) "return _K(lisp, E, _C, R);"))))

(def cc:gen:block-ite (NAME ARGS REFS TYPES CONT kvar body then else)
	"Common generation routing for ITE block items."
	(let (((op _ thn els) . body)
				(neg . (= op 'unless)))
		(prinl (o|) "if (" (when neg "!") "V.number) {")
		(o>)
		(then NAME ARGS REFS TYPES CONT kvar thn)
		(<o)
		(prinl (o|) "} else {")
		(o>)
		(else NAME ARGS REFS TYPES CONT kvar els)
		(<o)
		(prinl (o|) "}")))

(def cc:gen:default (NAME ARGS REFS TYPES CONT kvar body)
	"Generate KVAR call with NIL."
	(/**/ "Build the closure.")
	(cc:gen:closure CONT CONT)
	(/**/ "Collect the arguments.")
	(prinl (o|) "value_t R = { .atom = lisp_make_nil() };")
	(/**/ "Build the continuation.")
	(cc:gen:callback NAME REFS CONT CONT)
	(cc:gen:cleanup ARGS REFS TYPES CONT kvar CONT)
	(/**/ "Call the continuation.")
	(prinl (o|) "return _K(lisp, E, _C, R);"))

(def cc:gen:block (NAME ARGS REFS TYPES CONT kvar body)
	"Generate the basic block for BODY."
	(let ((make-any . (cc:gen:block-any NAME ARGS REFS TYPES CONT kvar body))
				(make-ite . (cc:gen:block-ite NAME ARGS REFS TYPES CONT kvar body)))
		(match body
			#
			# Predicate operations.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _) '(make-any last cc:op:predicate)) acc))
				 NIL '(lst? nil?))
			#
			# Unary operations.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _) '(make-any last cc:op:num:one)) acc))
				 NIL '(not))
			#
			# Binary operations.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _ _) '(make-any last cc:op:num:two)) acc))
				 NIL '(+ - * / < <= > >= and or))
			#
			# Equality operations.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _ _) '(make-any last cc:op:atm:two)) acc))
				 NIL '(= <>))
			#
			# Unary list operations.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _) '(make-any last cc:op:lst:one)) acc))
				 NIL '(car cdr))
			#
			# Binary list operations.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _ _) '(make-any last cc:op:lst:two)) acc))
				 NIL '(conc cons))
			#
			# Quote operation.
			#
			((quote . _) . ((prinl (o|) "abort(); /* " body " */")))
			#
			# IF/THEN, UNLESS, WHEN.
			#
			~(foldl
				 (\ (acc e) (cons (cons (list e _ _) '(make-ite cc:gen:block cc:gen:default)) acc))
				 NIL '(if unless when))
			#
			# IF/THEN/ELSE.
			#
			((if _ _ _) . (make-ite cc:gen:block cc:gen:block))
			#
			# KVAR funcall.
			#
			((_ _) . (make-any car cc:op:extcall))
			#
			# Recursive behavior
			#
			((_ . _) . (if (= NAME (car body))
									 (make-any last cc:op:reccall)
									 (make-any last cc:op:funcall)))
			#
			# Unknown behavior.
			#
			(_	. (prinl (o|) "abort(); /* " body " */")))))

(def cc:gen:function (NAME ARGS REFS TYPES CONT kvar body)
	"Generate a continuation KVAR with BODY."
	(prinl "static atom_t _mnml_" NAME kvar "(const lisp_t lisp, const atom_t E, closure_t C, value_t V) {")
	(o>)
	(if (> (assoc kvar REFS) 0)
		(prog
			(/**/ "Save the current argument (refcount=" (assoc kvar REFS) ")")
			(prinl (o|) "C->V[" (cdr (str kvar)) "] = V;")))
	(cc:gen:block NAME ARGS REFS TYPES CONT kvar body)
	(<o)
	(prinl "}")
	(prinl))

(def cc:gen:cont-any (NAME ARGS REFS TYPES CONT kvar fun)
	"Generate the C code for any type of funcall FUN."
	(let ((cont						. (last fun))
				((_ args body)	. cont)
				(nvar						. (last args)))
		(if (lst? cont)
			(prog
				(cc:gen:cont NAME ARGS REFS TYPES CONT nvar body)
				(cc:gen:function NAME ARGS REFS TYPES CONT kvar (cc:replace nvar fun)))
			(cc:gen:function NAME ARGS REFS TYPES CONT kvar fun))))

(def cc:gen:cont-ite (NAME ARGS REFS TYPES CONT kvar fun)
	"Generate the C code for the IF/THEN/ELSE funcall FUN."
	(let (((symb cnd . ite) . fun)
				(first	. (map (\ (e) (let ((cont						. (last e))
																		((_ args body)	. cont)
																		(nvar						. (last args)))
																(if (lst? cont) (cc:replace nvar e) e)))
										ite))
				(clist	. (map (\ (e) (let ((cont . (last e)))
																(if (lst? cont) cont)))
										ite))
				(conts	. (filter nil? clist)))
		(iter (\ ((_ args body))
						(cc:gen:cont NAME ARGS REFS TYPES CONT (last args) body)) conts)
		(cc:gen:function NAME ARGS REFS TYPES CONT kvar (cons symb (cons cnd first)))))

(def cc:gen:cont (NAME ARGS REFS TYPES CONT kvar fun)
	"Generate the C code for the continuations in FUN."
	(match fun
		#
		# IF/THEN/ELSE
		#
		~(foldl
			 (\ (acc e) (cons (cons (list e _ . _)
													'(cc:gen:cont-ite NAME ARGS REFS TYPES CONT kvar fun))
										acc))
			 NIL '(if unless when))
		#
		# Default behavior.
		#
		(_	.	(cc:gen:cont-any NAME ARGS REFS TYPES CONT kvar fun))))

#
# Prologue and identity.
#

(def cc:gen:prologue ()
	"Generate C code prologue."
	(prinl "#include <mnml/closure.h>")
	(prinl "#include <mnml/lisp.h>")
	(prinl "#include <mnml/maker.h>")
	(prinl "#include <mnml/module.h>")
	(prinl "#include <mnml/slab.h>")
	(prinl "#include <mnml/types.h>")
	(prinl)
	(prinl "static closure_t cache = NULL;")
	(prinl))

(def cc:gen:retval (TYPES kvar)
	"Generate the return value conversion."
	(let ((rtype . (assoc kvar TYPES)))
		(match rtype
			(NUMBER	. (prinl (o|) "return lisp_make_number(V.number);"))
			(ATOM		. (prinl (o|) "return V.atom;"))
			(_			. (prinl (o|) "return lisp_make_nil(); /* Unsupported type " (str rtype) " */"))
			)))

(def cc:gen:identity (NAME TYPES kvar)
	"Generate C code for the identity continuation."
	(prinl "static atom_t _mnml_" NAME "_identity(const lisp_t lisp, const atom_t E, closure_t C, value_t V) {")
	(o>)
	(cc:gen:retval TYPES kvar)
	(<o)
	(prinl "}")
	(prinl))

#
# Main function.
#

(def cc:gen:info (SYMB CPS TYPES)
	"Generate CPS code information."
	(prinl "/*")
	(prin	" * FUNCTION   : ") (printl SYMB)
	(prin	" * DEFINITION : ") (printl (eval SYMB))
	(prin	" * CPS        : ") (printl CPS)
	(prin	" * TYPES      : ") (printl TYPES)
	(prinl " */")
	(prinl))

(def cc:gen:prototype (NAME ARGS)
	"Generate C code function prototype."
	(prin "static atom_t _mnml_" NAME "(const lisp_t lisp, const atom_t E, ");
	(iter (\ (e) (prin "value_t " e ", ")) ARGS)
	(prinl "closure_t C, callback_t K);");
	(prinl))

(def cc:gen:main (NAME ARGS REFS kvar)
	"Generate C code for the main function."
	(prin "static atom_t _mnml_" NAME "(const lisp_t lisp, const atom_t E, ")
	(o>)
	(iter (\ (e) (prin "value_t " e ", ")) ARGS)
	(prinl "closure_t C, callback_t K) {");
	(/**/ "Allocate a new closure.") 
	(prinl (o|) "closure_t _C = lisp_closure_get(&cache, C, " (len REFS) ");")
	(/**/ "Save the arguments.")
	(foldl
		(\ (acc e) (prinl (o|) "_C->V[" acc "] = " e ";") (+ acc 1)) 0 ARGS)
	(/**/ "Generate the callback.")
	(cc:gen:callback NAME REFS ARGS kvar)
	(/**/ "Generate the continuation argument.")
	(prinl (o|) "value_t R = { .callback = K };");
	(/**/ "Call the continuation.")
	(prinl (o|) "return _K(lisp, E, _C, R);")
	(<o)
	(prinl "}")
	(prinl))

#
# Module.
#

(def cc:gen:argument (TYPES ARGS)
	"Generate the argument extraction."
	(prin (o|) "LISP_ARGS(closure, C")
	(iter (\ (e) (prin ", _" e)) ARGS)
	(prinl ");")
	(iter (\ (e) (cc:gen:value (assoc e TYPES) NIL e)) ARGS))

(def cc:gen:entry (NAME ARGS TYPES)
	"Generate C code for the NAME entry point."
	(prinl "static atom_t lisp_function_" NAME "(const lisp_t lisp, const atom_t closure) {")
	(o>)
	(/**/ "Grab the arguments.")
	(cc:gen:argument TYPES ARGS)
	(/**/ "Define the default closure and callback.")
	(prinl (o|) "closure_t _C = NULL;")
	(prinl (o|) "callback_t _K = _mnml_" NAME "_identity;")
	(/**/ "Call the main function.")
	(prin (o|) "atom_t R = _mnml_" NAME "(lisp, closure, ")
	(iter (\ (e) (prin e ", ")) ARGS)
	(prinl "_C, _K);")
	(/**/ "Clear the closure cache and return.")
	(prinl (o|) "lisp_closure_clear(&cache);")
	(prinl (o|) "return R;")
	(<o)
	(prinl "}")
	(prinl)
	(prin "LISP_MODULE_SETUP(" NAME ", " NAME)
	(iter (\ (e) (prin ", _" e)) ARGS)
	(prinl ", NIL);")
	(prinl))

(def cc:gen::module(MODULE NAMES)
	"Generate C code for the module."
	(prinl "module_entry_t ENTRIES[] = {")
	(iter (\ (NAME) (prinl "	LISP_MODULE_REGISTER(" NAME "),")) NAMES)
	(prinl "	{ NULL, NULL }")
	(prinl "};")
	(prinl)
	(prinl "const char * lisp_module_name() { return \"" MODULE "\"; }")
	(prinl)
	(prinl "const module_entry_t * lisp_module_entries() {")
	(prinl "	return ENTRIES;")
	(prinl "}"))

#
# Main compiler entrypoint.
#

(def cc:compile1 (SYMB)
	"Generate the C code for the CPS function SYMB."
	(let ((cps						. (>& SYMB))
				((args _ body)	. cps)
				(ARGS						. (cc:arg:get args))
				(REFS						. (cps:refcount cps))
				(TYPES					. (cps:infer NAME cps))
				(isvalid				. (foldl (\ (acc (_ . t)) (and (<> t 'ERROR) acc)) T TYPES))
				(kvar						. (last args)))
		(when isvalid
			#
			# Generate the C file
			#
			(cc:gen:info SYMB cps TYPES)
			(cc:gen:identity SYMB TYPES kvar)
			(cc:gen:prototype SYMB ARGS)
			(cc:gen:cont SYMB ARGS REFS TYPES kvar kvar body)
			(cc:gen:main SYMB ARGS REFS kvar)
			(cc:gen:entry SYMB ARGS TYPES)
			#
			# Return then symbol passed as argument.
			#
			SYMB
			)))

(def cc:compile (MODULE SYMBS . TARGET)
	"Generate the C code for the CPS functions SYMBS."
	(let ((trgt . (append (assoc "HOME" ENV) "/.mnml"))
				(cout . (<|> (eval (car TARGET)) (append trgt "/" (str MODULE) ".c"))))
		#
		# Remove the C file
		#
		(unlink cout)
		#
		# Generate the C file
		#
		(out cout
			(cc:gen:prologue)
			(let ((result . (filter nil? (map cc:compile1 SYMBS))))
				(cc:gen::module MODULE result)
				result))
		))

(def cc:ext ()
	"Get the library extension depending on the OS name."
	(let (((_ osname) . (run "/usr/bin/env" '("uname" "-s") ENV)))
		(if (= osname "Darwin") ".dylib" ".so")))

(def cc:flags ()
	"Generate the build flags based on the OS name."
	(let (((_ osname) . (run "/usr/bin/env" '("uname" "-s") ENV))
				(incd				. (append (assoc 'PREFIX CONFIG) "/include"))
				(libd				. (append (assoc 'PREFIX CONFIG) "/lib"))
				(flags			. (list
												"-shared" "-fPIC" "-fomit-frame-pointer"
												"-I" incd "-L" libd "-lminimal"
												"-O3" "-g3" "-o" oout cout)))
		(if (or (= osname "Linux") (= osname "Darwin"))
			($+ flags '("-flto"))
			flags
			)))

(def cc:build (MODULE SYMBS . TARGET)
	"Build the module for the CPS function SYMB."
	(let ((trgt	. (append (assoc "HOME" ENV) "/.mnml"))
				(cout . (append trgt "/" (str MODULE) ".c"))
				(oout . (<|> (eval (car TARGET)) (append trgt "/lib" (str MODULE) (cc:ext)))))
		(when SYMBS
			#
			# Remove the SO file
			#
			(unlink oout)
			#
			# Build the SO file
			#
			(let (((status . messages) . (run "/usr/bin/cc" (cc:flags) ENV)))
				(if (= 0 status) (load (cons MODULE SYMBS)))))))
