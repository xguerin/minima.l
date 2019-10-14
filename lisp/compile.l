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

(def cc:genprologue ()
  "Generate C code prologue."
  (prog
    (prinl "#include <mnml/closure.h>")
    (prinl "#include <mnml/lisp.h>")
    (prinl "#include <mnml/maker.h>")
    (prinl "#include <mnml/plugin.h>")
    (prinl "#include <mnml/slab.h>")
    (prinl "#include <mnml/types.h>")
    (prinl)
    (prinl "#define NO_INLINE __attribute__((noinline))")
    (prinl)
    #
    # Generate the closure cleaner.
    #
    (prinl "static void NO_INLINE cleanup(closure_t C) {")
    (prinl "  for (size_t i = 0; i < CLOSURE_SIZE; i += 1) {")
    (prinl "    if (C->V[i] != NULL) {")
    (prinl "      X(C->V[i]);")
    (prinl "      C->V[i] = NULL;")
    (prinl "    }")
    (prinl "  }")
    (prinl "  lisp_closure_deallocate(C);")
    (prinl "}")
    (prinl)
    #
    # Generate the identity continuation.
    #
    (prinl "static atom_t identity(closure_t C, atom_t V) {")
    (prinl "  return V;")
    (prinl "}")
    (prinl)))

(def cc:genprototype (ARGS)
  "Generate C code function prototype."
  (prog
    (prin "static atom_t fun(");
    (iter (\ (e) (prin "atom_t " e ", ")) ARGS)
    (prinl "closure_t C, callback_t K);");
    (prinl)))

(def cc:genfunction (ARGS kvar)
  "Generate C code for the main function."
  (prog
    (prin "static atom_t fun(")
    (cc:>)
    (iter (\ (e) (prin "atom_t " e ", ")) ARGS)
    (prinl "closure_t C, callback_t K) {");
    (prinl (cc:|) "register closure_t _C = lisp_closure_allocate(C);")
    (foldl (\ (acc e) (prog
                        (prinl (cc:|) "_C->V[" acc "] = " e ";")
                        (+ acc 1)))
                      0 ARGS)
    (prinl (cc:|) "register atom_t R = (atom_t)K;");
    (cc:gennext ARGS kvar)
    (prinl (cc:|) "return _K(_C, R);")
    (cc:<)
    (prinl "}")
    (prinl)))

(def cc:genplugin (name args)
  "Generate C code for the plugin."
  (prog
    (prinl "static atom_t lisp_function_" name "(const atom_t closure, const atom_t cell) {")
    (cc:>)
    (prinl (cc:|) "atom_t cur = cell, cdr;")
    (iter (\ (e) (prog
                   (prinl (cc:|) "atom_t " e " = lisp_car(cur);")
                   (prinl (cc:|) "cdr = lisp_cdr(cur);")
                   (prinl (cc:|) "X(cur); cur = cdr;")))
      args)
    (prinl (cc:|) "X(cur);")
    (prinl (cc:|) "closure_t _C = NULL;")
    (prinl (cc:|) "callback_t _K = identity;")
    (prin (cc:|) "return fun(")
    (iter (\ (e) (prin e ", ")) args)
    (prinl "_C, _K);")
    (cc:<)
    (prinl "}")
    (prinl)
    (prinl "LISP_PLUGIN_REGISTER(" name ", " name ");")))

(def cc:binnumop (ARGS kvar body)
  "Generate C code for binary numeric operations."
  (let (((op arg1 arg2 cont)  . body))
    (prog
      (prin (cc:|) "register atom_t R = lisp_make_number(")
      (cc:numrvalue ARGS arg1)
      (match op
        (+  . (prin " + "))
        (-  . (prin " - "))
        (=  . (prin " == "))
        (<= . (prin " <= "))
        (_  . (prin " /* " op " */ ")))
      (cc:numrvalue ARGS arg2)
      (prinl ");")
      )))

(def cc:recop (ARGS kvar args prms)
  "Generate C code for recursive operations."
  (if (and (not (nil? args)) (not (nil? prms)))
    (let (((acar . acdr) . args)
          ((pcar . pcdr) . prms))
      (if (<> acar kvar)
        (prog
          (prin (cc:|) "register atom_t " acar " = ")
          (cc:refrvalue ARGS pcar)
          (cc:recop ARGS acdr pcdr))))))

(def cc:kvarop (ARGS kvar body)
  "Generate C code for immediate KVAR funcall."
  (prog
    (prin (cc:|) "register atom_t R = ")
    (cc:refrvalue ARGS (car (cdr body)))))

(def cc:numrvalue (ARGS arg)
  "Generate the C code for the number R-value ARG."
  (cond arg
    (sym? . (let ((idx . (cc:getrefidx ARGS arg)))
              (prin "C->V[" idx "]->number")))
    (num? . (prin arg))
    (_    . (prin "abort()"))))

(def cc:refrvalue (ARGS arg)
  "Generate the C code for the ref R-value ARG."
  (cond arg
    (num? . (prinl "lisp_make_number(" arg ");"))
    (sym? . (let ((idx . (cc:getrefidx ARGS arg)))
              (prinl "UP(C->V[" idx "]);")))
    (_    . (prinl "abort();"))))

(def cc:genclosure (ARGS next)
  "Generate a default closure capture."
  (if (= (last ARGS) next)
    (prinl (cc:|) "register closure_t _C = C->C;")
    (prinl (cc:|) "register closure_t _C = C;")))

(def cc:gennext (ARGS ref)
  "Generate the continuation capture for REF."
  (if (<> (last ARGS) ref)
    (prinl (cc:|) "register callback_t _K = fn" ref ";")
    (prog
      (prinl (cc:|) "register callback_t _K = (callback_t)C->V[" (cdr (str ref)) "];")
      (prinl (cc:|) "C->V[" (cdr (str ref)) "] = NULL;")
      (prinl (cc:|) "cleanup(C);"))))

(def cc:genblock (NAME ARGS kvar body)
  "Generate the basic block for BODY."
  (match body
    #
    # Binary operations.
    #
    ((+ _ _ _)  . (let ((next . (last body)))
                    (prog
                      (cc:binnumop ARGS kvar body)
                      (cc:genclosure ARGS next)
                      (cc:gennext ARGS next)
                      (prinl (cc:|) "return _K(_C, R);"))))
    ((- _ _ _)  . (let ((next . (last body)))
                    (prog
                      (cc:binnumop ARGS kvar body)
                      (cc:genclosure ARGS next)
                      (cc:gennext ARGS next)
                      (prinl (cc:|) "return _K(_C, R);"))))
    ((= _ _ _)  . (let ((next . (last body)))
                    (prog
                      (cc:binnumop ARGS kvar body)
                      (cc:genclosure ARGS next)
                      (cc:gennext ARGS next)
                      (prinl (cc:|) "return _K(_C, R);"))))
    ((<= _ _ _) . (let ((next . (last body)))
                    (prog
                      (cc:binnumop ARGS kvar body)
                      (cc:genclosure ARGS next)
                      (cc:gennext ARGS next)
                      (prinl (cc:|) "return _K(_C, R);"))))
    #
    # IF/THEN/ELSE
    #
    ((if _ _ _) . (let (((_ _ then else) . body))
                    (prog
                      (prinl (cc:|) "if (V->number) {")
                      (cc:>)
                      (cc:genblock NAME ARGS kvar then)
                      (cc:<)
                      (prinl (cc:|) "} else {")
                      (cc:>)
                      (cc:genblock NAME ARGS kvar else)
                      (cc:<)
                      (prinl (cc:|) "}")
                      )))
    #
    # KVAR funcall.
    #
    ((_ _)  . (let ((next . (car body)))
                (prog
                  (cc:genclosure ARGS next)
                  (cc:kvarop ARGS kvar body)
                  (cc:gennext ARGS next)
                  (prinl (cc:|) "return _K(_C, R);"))))
    #
    # Recursive behavior
    #
    ((_ . _) . (let (((symb . prms) . body)
                     (next          . (last body)))
                 (if (= symb NAME)
                   (prog
                     (cc:genclosure ARGS next)
                     (cc:recop ARGS kvar ARGS prms)
                     (cc:gennext ARGS next)
                     (prin (cc:|) "return fun(")
                     (iter (\ (e) (prin e ", ")) (rev (cdr (rev ARGS))))
                     (prinl "_C, _K);"))
                   (prinl (cc:|) "abort(); /* " body " */"))))
    #
    # Unknown behavior.
    #
    (_  . (prinl (cc:|) "abort(); /* " body " */"))
    ))

(def cc:genfunc (NAME ARGS kvar body)
  "Generate a continuation KVAR with BODY."
  (prog
    (prinl "static atom_t fn" kvar "(closure_t C, atom_t V) {")
    (cc:>)
    (prinl (cc:|) "C->V[" (cdr (str kvar)) "] = V;")
    (cc:genblock NAME ARGS kvar body)
    (cc:<)
    (prinl "}")
    (prinl)))

(def cc:gencont-any (NAME ARGS kvar fun)
  "Generate the C code for any type of funcall FUN."
  (let ((cont          . (last fun))
        ((_ args body) . cont)
        (nvar          . (last args)))
    (if (lst? cont)
      (prog
        (cc:gencont NAME ARGS nvar body)
        (cc:genfunc NAME ARGS kvar (cc:replace nvar fun)))
      (cc:genfunc NAME ARGS kvar fun))))

(def cc:gencont-ite (NAME ARGS kvar fun)
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
    (prog
      (iter (\ ((_ args body))
              (cc:gencont-any NAME ARGS (last args) body)) conts)
      (cc:genfunc NAME ARGS kvar (cons symb (cons cnd first))))))

(def cc:gencont (NAME ARGS kvar fun)
  "Generate the C code for the continuations in FUN."
  (match fun
    #
    # IF/THEN/ELSE
    #
    ((if _ . _) . (cc:gencont-ite NAME ARGS kvar fun))
    #
    # Default behavior.
    #
    (_  .  (cc:gencont-any NAME ARGS kvar fun))))

(def >! (NAME)
  "Compile the CPS function NAME."
  (let ((cps            . (>& NAME))
        ((args _ body)  . cps)
        (ARGS           . (rev (cdr (rev args))))
        (kvar           . (last args)))
    (prog
      (cc:genprologue)
      (cc:genprototype ARGS)
      (cc:gencont NAME args kvar body)
      (cc:genfunction ARGS kvar)
      (cc:genplugin NAME ARGS))))
