(load "@lib/cps.l")
(load "@lib/iter.l")
(load "@lib/filter.l")
(load "@lib/foldl.l")
(load "@lib/foldr.l")
(load "@lib/last.l")
(load "@lib/map.l")
(load "@lib/rev.l")

#
# Indentation management
#

(setq cc:level 0)

(def cc:indent_r (lvl)
  "Recursively build indentation string."
  (if (<> lvl 0)
    (cons " " (cc:indent_r (- lvl 1)))))

(def cc:| ()
  "Return the indentation string."
  (cc:indent_r cc:level))

(def cc:> ()
  "Increment the indentation level."
  (setq cc:level (+ cc:level 2)))

(def cc:< ()
  "Decrement the indentation level."
  (setq cc:level (- cc:level 2)))

#
# Helper functions.
#

(def cc:getref (fun)
  "Return the current symbol of the function FUN."
  (let (((_ (symb) _) . fun)) symb))

(def cc:getargidx (ARGS ref idx)
  "Get the index of REF in the closure based on ARGS."
  (if ARGS
    (if (= (car ARGS) ref)
      idx
      (cc:getargidx (cdr ARGS) ref (+ idx 1)))))

(def cc:getrefidx (ARGS ref)
  "Get the index of REF in the closure."
  (let ((tostr  . (str ref)))
    (if (= (car tostr) '_')
      (cdr tostr)
      (cc:getargidx ARGS ref 0))))

(def cc:replace (kvar body)
  "Replace the continuation in BODY by KVAR."
  (if (cdr body)
    (cons (car body) (cc:replace kvar (cdr body)))
    (list kvar)))

#
# Code generators.
#

(def cc:genretval (TYPES kvar)
  "Generate the return value conversion."
  (let ((rtype . (assoc kvar TYPES)))
    (match rtype
      (NUMBER . (prinl (cc:|) "return lisp_make_number(V.number);"))
      (LIST . (prinl (cc:|) "return V.list;"))
      (_ . (prinl (cc:|) "return UP(NIL); /* Unsupported type " (str rtype) " */"))
      )))

(def cc:genprologue (TYPES kvar)
  "Generate C code prologue."
  (prinl "#include <mnml/closure.h>")
  (prinl "#include <mnml/lisp.h>")
  (prinl "#include <mnml/maker.h>")
  (prinl "#include <mnml/plugin.h>")
  (prinl "#include <mnml/slab.h>")
  (prinl "#include <mnml/types.h>")
  (prinl)
  #
  # Generate the closure cache.
  #
  (prinl "static closure_t cache = NULL;")
  (prinl)
  #
  # Generate the identity continuation.
  #
  (prinl "static atom_t identity(closure_t C, value_t V) {")
  (cc:>)
  (cc:genretval TYPES kvar)
  (cc:<)
  (prinl "}")
  (prinl))

(def cc:genprototype (NAME ARGS)
  "Generate C code function prototype."
  (prin "static atom_t " NAME "(");
  (iter (\ (e) (prin "value_t " e ", ")) ARGS)
  (prinl "closure_t C, callback_t K);");
  (prinl))

(def cc:genmain (NAME ARGS REFS kvar)
  "Generate C code for the main function."
  (prin "static atom_t " NAME "(")
  (cc:>)
  (iter (\ (e) (prin "value_t " e ", ")) ARGS)
  (prinl "closure_t C, callback_t K) {");
  (prinl (cc:|) "closure_t _C = lisp_closure_get(&cache, C, " (len REFS) ");")
  (foldl
    (\ (acc e) (prinl (cc:|) "_C->V[" acc "] = " e ";") (+ acc 1))
    0 ARGS)
  (prinl (cc:|) "value_t R = { .callback = K };");
  (cc:gennext NAME ARGS kvar)
  (prinl (cc:|) "return _K(_C, R);")
  (cc:<)
  (prinl "}")
  (prinl))

(def cc:genargument (TYPES arg)
  "Generate the argument extraction."
  (prinl (cc:|) "car = lisp_eval(closure, lisp_car(cur));")
  #
  # Determine how to extract the value.
  #
  (let ((atype . (assoc arg TYPES)))
    (match atype
      (NUMBER .  (prinl (cc:|) "value_t " arg " = { .number = car->number };"))
      (LIST .  (prinl (cc:|) "value_t " arg " = { .list = UP(car) };"))
      (_ . (prinl (cc:|) "value_t " arg " = { .number = 0 }; /* Unsupported type " (str atype) " */"))
      ))
  #
  (prinl (cc:|) "X(car);")
  (prinl (cc:|) "cdr = lisp_cdr(cur);")
  (prinl (cc:|) "X(cur); cur = cdr;"))

(def cc:genplugin (NAME ARGS TYPES)
  "Generate C code for the plugin."
  (prinl "static atom_t lisp_function_" NAME "(const atom_t closure, const atom_t cell) {")
  (cc:>)
  (prinl (cc:|) "atom_t cur = cell, car, cdr;")
  (iter (\ (e) (cc:genargument TYPES e)) ARGS)
  (prinl (cc:|) "X(cur);")
  (prinl (cc:|) "closure_t _C = NULL;")
  (prinl (cc:|) "callback_t _K = identity;")
  (prin (cc:|) "atom_t R = " NAME "(")
  (iter (\ (e) (prin e ", ")) ARGS)
  (prinl "_C, _K);")
  (prinl (cc:|) "lisp_closure_clear(&cache);")
  (prinl (cc:|) "return R;")
  (cc:<)
  (prinl "}")
  (prinl)
  (prinl "LISP_PLUGIN_REGISTER(" NAME ", " NAME ");"))

(def cc:predicateop (ARGS kvar body)
  "Generate C code for unary predicate operations."
  (let (((op arg cont) . body))
    (prin (cc:|) "value_t R = { .integer = ")
    (match op
      (nil? . (prin "IS_NULL("))
      (_   . (prin " /* Unsupported predicate " op " */ ")))
    (cc:lstrvalue ARGS kvar arg)
    (prinl ") };")))

(def cc:binnumop (ARGS kvar body)
  "Generate C code for binary numeric operations."
  (let (((op arg1 arg2 cont)  . body))
    (prin (cc:|) "value_t R = { .number = ")
    (cc:numrvalue ARGS kvar arg1)
    (match op
      #
      # Immediate translation to C.
      #
      ~(foldl
         (\ (acc e) (cons (conc (cons e (list 'prin " " (str e) " "))) acc))
         NIL '(+ - * / < <= > >=))
      #
      # Special cases.
      #
      (=  . (prin " == "))
      (<> . (prin " != "))
      #
      # Unsupported cases.
      #
      (_  . (prin " /* Unsupported operation " op " */ ")))
    (cc:numrvalue ARGS kvar arg2)
    (prinl " };")))

(def cc:unqlstop (ARGS kvar body)
  "Generate C code for unary list operations."
  (let (((op arg cont)  . body))
    (prin (cc:|) "value_t R = { .list = ")
    (match op
      (car . (prin "lisp_car("))
      (cdr . (prin "lisp_cdr("))
      (_   . (prin " /* Unsupported operation " op " */ ")))
    (cc:lstrvalue ARGS kvar arg)
    (prinl ") };")))

(def cc:binlstop (ARGS kvar body)
  "Generate C code for binary list operations."
  (let (((op arg0 arg1 cont)  . body))
    (prin (cc:|) "value_t R = { .list = ")
    (match op
      (conc . (prin "lisp_conc("))
      (cons . (prin "lisp_cdr("))
      (_   .  (prin " /* Unsupported operation " op " */ ")))
    (cc:lstrvalue ARGS kvar arg0)
    (prin ", ")
    (cc:lstrvalue ARGS kvar arg1)
    (prinl ") };")))

(def cc:recop (ARGS kvar args prms)
  "Generate C code for recursive operations."
  (if (and (not (nil? args)) (not (nil? prms)))
    (let (((acar . acdr) . args)
          ((pcar . pcdr) . prms))
      (if (<> acar kvar)
        (prog
          (prin (cc:|) "value_t " acar " = ")
          (cc:refrvalue ARGS kvar pcar)
          (cc:recop ARGS acdr pcdr))))))

(def cc:kvarop (ARGS kvar (_ arg))
  "Generate C code for immediate KVAR funcall."
    (prin (cc:|) "value_t R = ")
    (cc:refrvalue ARGS kvar arg))

(def cc:lstrvalue (ARGS kvar arg)
  "Generate the C code for the list R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prin "V.list")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prin "C->V[" idx "].list"))))
    (_    . (prin "abort()"))))

(def cc:numrvalue (ARGS kvar arg)
  "Generate the C code for the number R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prin "V.number")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prin "C->V[" idx "].number"))))
    (num? . (prin arg))
    (_    . (prin "abort()"))))

(def cc:refrvalue (ARGS kvar arg)
  "Generate the C code for the ref R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prinl "V;")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prinl "C->V[" idx "];"))))
    (num? . (prinl "{ .number = " arg " };"))
    (_    . (prinl "abort();"))))

(def cc:genclosure (CONT next)
  "Generate a default closure capture."
  (if (= CONT next)
    (prinl (cc:|) "closure_t _C = C->C;")
    (prinl (cc:|) "closure_t _C = C;")))

(def cc:gennext (NAME CONT ref)
  "Generate the continuation capture for REF."
  (if (<> CONT ref)
    (prinl (cc:|) "callback_t _K = " NAME ref ";")
    (prog
      (prinl (cc:|) "callback_t _K = C->V[" (cdr (str ref)) "].callback;")
      (prinl (cc:|) "lisp_closure_put(&cache, C);"))))

(def cc:genblock (NAME ARGS CONT kvar body)
  "Generate the basic block for BODY."
  (match body
    #
    # Predicate operations.
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons
                                (list e _ _)
                                '(let ((next . (last body)))
                                   (cc:predicateop ARGS kvar body)
                                   (cc:genclosure CONT next)
                                   (cc:gennext NAME CONT next)
                                   (prinl (cc:|) "return _K(_C, R);"))))
                    acc))
       NIL '(nil?))
    #
    # Binary operations.
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons
                                (list e _ _ _)
                                '(let ((next . (last body)))
                                   (cc:binnumop ARGS kvar body)
                                   (cc:genclosure CONT next)
                                   (cc:gennext NAME CONT next)
                                   (prinl (cc:|) "return _K(_C, R);"))))
                    acc))
       NIL '(+ - * / = <> < <= > >=))
    #
    # Unary list operations.
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons
                                (list e _ _)
                                '(let ((next . (last body)))
                                   (cc:unqlstop ARGS kvar body)
                                   (cc:genclosure CONT next)
                                   (cc:gennext NAME CONT next)
                                   (prinl (cc:|) "return _K(_C, R);"))))
                    acc))
       NIL '(car cdr))
    #
    # Binary list operations.
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons
                                (list e _ _ _)
                                '(let ((next . (last body)))
                                   (cc:binlstop ARGS kvar body)
                                   (cc:genclosure CONT next)
                                   (cc:gennext NAME CONT next)
                                   (prinl (cc:|) "return _K(_C, R);"))))
                    acc))
       NIL '(conc cons))
    #
    # IF/THEN/ELSE
    #
    ((if _ _ _) . (let (((_ _ then else) . body))
                    (prinl (cc:|) "if (V.number) {")
                    (cc:>)
                    (cc:genblock NAME ARGS CONT kvar then)
                    (cc:<)
                    (prinl (cc:|) "} else {")
                    (cc:>)
                    (cc:genblock NAME ARGS CONT kvar else)
                    (cc:<)
                    (prinl (cc:|) "}")))
    #
    # KVAR funcall.
    #
    ((_ _)  . (let ((next . (car body)))
                (cc:genclosure CONT next)
                (cc:kvarop ARGS kvar body)
                (cc:gennext NAME CONT next)
                (prinl (cc:|) "return _K(_C, R);")))
    #
    # Recursive behavior
    #
    ((_ . _) . (let (((symb . prms) . body)
                     (next          . (last body)))
                 (if (= symb NAME)
                   (prog
                     (cc:genclosure CONT next)
                     (cc:recop ARGS kvar ARGS prms)
                     (cc:gennext NAME CONT next)
                     (prin (cc:|) "return " NAME "(")
                     (iter (\ (e) (prin e ", ")) ARGS)
                     (prinl "_C, _K);"))
                   (prinl (cc:|) "abort(); /* Unsupported operation " symb " */"))))
    #
    # Unknown behavior.
    #
    (_  . (prinl (cc:|) "abort(); /* " body " */"))))

(def cc:genfunction (NAME ARGS REFS CONT kvar body)
  "Generate a continuation KVAR with BODY."
  (prinl "static atom_t " NAME kvar "(closure_t C, value_t V) {")
  (cc:>)
  (if (> (assoc kvar REFS) 0)
    (prinl (cc:|) "C->V[" (cdr (str kvar)) "] = V;"))
  (cc:genblock NAME ARGS CONT kvar body)
  (cc:<)
  (prinl "}")
  (prinl))

(def cc:gencont-any (NAME ARGS REFS CONT kvar fun)
  "Generate the C code for any type of funcall FUN."
  (let ((cont          . (last fun))
        ((_ args body) . cont)
        (nvar          . (last args)))
    (if (lst? cont)
      (prog
        (cc:gencont NAME ARGS REFS CONT nvar body)
        (cc:genfunction NAME ARGS REFS CONT kvar (cc:replace nvar fun)))
      (cc:genfunction NAME ARGS REFS CONT kvar fun))))

(def cc:gencont-ite (NAME ARGS REFS CONT kvar fun)
  "Generate the C code for the IF/THEN/ELSE funcall FUN."
  (let (((symb cnd . ite) . fun)
        (first  . (map (\ (e) (let ((cont           . (last e))
                                    ((_ args body)  . cont)
                                    (nvar           . (last args)))
                                (if (lst? cont) (cc:replace nvar e) e)))
                    ite))
        (clist  . (map (\ (e) (let ((cont . (last e)))
                                (if (lst? cont) cont)))
                    ite))
        (conts  . (filter nil? clist)))
    (iter (\ ((_ args body))
            (cc:gencont-any NAME ARGS REFS CONT (last args) body)) conts)
    (cc:genfunction NAME ARGS REFS CONT kvar (cons symb (cons cnd first)))))

(def cc:gencont (NAME ARGS REFS CONT kvar fun)
  "Generate the C code for the continuations in FUN."
  (match fun
    #
    # IF/THEN/ELSE
    #
    ((if _ . _) . (cc:gencont-ite NAME ARGS REFS CONT kvar fun))
    #
    # Default behavior.
    #
    (_  .  (cc:gencont-any NAME ARGS REFS CONT kvar fun))))

(def >! (NAME)
  "Compile the CPS function NAME."
  (let ((cps           . (>& NAME))
        ((args _ body) . cps)
        (ARGS          . (rev (cdr (rev args))))
        (REFS          . (cps:refcount cps))
        (TYPES         . (cps:infer NAME cps))
        (kvar          . (last args)))
    (cc:genprologue TYPES kvar)
    (cc:genprototype NAME ARGS)
    (cc:gencont NAME ARGS REFS kvar kvar body)
    (cc:genmain NAME ARGS REFS kvar)
    (cc:genplugin NAME ARGS TYPES)))