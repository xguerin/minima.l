(load
  "assoc.l" "cps.l" "iter.l" "filter.l" "foldl.l" "foldr.l" "last.l" "map.l"
  "rev.l" "run.l" 'prefix 'unlink)

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
    (if (= (car tostr) ^_)
      (cdr tostr)
      (cc:getargidx ARGS ref 0))
    ))

(def cc:replace (kvar body)
  "Replace the continuation in BODY by KVAR."
  (if (cdr body)
    (cons (car body) (cc:replace kvar (cdr body)))
    (list kvar)))

#
# Operation handlers.
#

(def cc:op:predicate (ARGS kvar body)
  "Generate C code for unary predicate operations."
  (let (((op arg cont) . body))
    (prin (o|) "value_t R = { .integer = ")
    (match op
      (nil? . (prin "IS_NULL("))
      (_   . (prin " /* Unsupported predicate " op " */ ")))
    (cc:value:lst ARGS kvar arg)
    (prinl ") };")))

(def ccp:op:binnum (ARGS kvar body)
  "Generate C code for binary numeric operations."
  (let (((op arg1 arg2 cont)  . body))
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
      (=  . (prin " == "))
      (<> . (prin " != "))
      #
      # Unsupported cases.
      #
      (_  . (prin " /* Unsupported operation " op " */ ")))
    (cc:value:num ARGS kvar arg2)
    (prinl " };")))

(def cc:op:unqlst (ARGS kvar body)
  "Generate C code for unary list operations."
  (let (((op arg cont)  . body))
    (prin (o|) "value_t R = { .list = ")
    (match op
      (car . (prin "lisp_car("))
      (cdr . (prin "lisp_cdr("))
      (_   . (prin " /* Unsupported operation " op " */ ")))
    (cc:value:lst ARGS kvar arg)
    (prinl ") };")))

(def cc:op:binlst (ARGS kvar body)
  "Generate C code for binary list operations."
  (let (((op arg0 arg1 cont)  . body))
    (prin (o|) "value_t R = { .list = ")
    (match op
      (conc . (prin "lisp_conc("))
      (cons . (prin "lisp_cdr("))
      (_   .  (prin " /* Unsupported operation " op " */ ")))
    (cc:value:lst ARGS kvar arg0)
    (prin ", ")
    (cc:value:lst ARGS kvar arg1)
    (prinl ") };")))

(def cc:op:recur (ARGS kvar args prms)
  "Generate C code for recursive operations."
  (if (and (not (nil? args)) (not (nil? prms)))
    (let (((acar . acdr) . args)
          ((pcar . pcdr) . prms))
      (if (<> acar kvar)
        (prog
          (prin (o|) "value_t " acar " = ")
          (cc:value:ref ARGS kvar pcar)
          (cc:op:recur ARGS acdr pcdr))))))

(def cc:op:kvar (ARGS kvar (_ arg))
  "Generate C code for immediate KVAR funcall."
  (prin (o|) "value_t R = ")
  (cc:value:ref ARGS kvar arg))

#
# Value handlers.
#

(def cc:value:lst (ARGS kvar arg)
  "Generate the C code for the list R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prin "V.list")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prin "C->V[" idx "].list"))))
    (_    . (prin "abort()"))))

(def cc:value:num (ARGS kvar arg)
  "Generate the C code for the number R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prin "V.number")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prin "C->V[" idx "].number"))))
    (num? . (prin arg))
    (_    . (prin "abort()"))))

(def cc:value:ref (ARGS kvar arg)
  "Generate the C code for the ref R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prinl "V;")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prinl "C->V[" idx "];"))))
    (num? . (prinl "{ .number = " arg " };"))
    (_    . (prinl "abort();"))))

#
# Outer section generators.
#

(def cc:gen:retval (TYPES kvar)
  "Generate the return value conversion."
  (let ((rtype . (assoc kvar TYPES)))
    (match rtype
      (NUMBER . (prinl (o|) "return lisp_make_number(V.number);"))
      (LIST . (prinl (o|) "return V.list;"))
      (_ . (prinl (o|) "return UP(NIL); /* Unsupported type " (str rtype) " */"))
      )))

(def cc:gen:prologue (TYPES kvar)
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
  (o>)
  (cc:gen:retval TYPES kvar)
  (<o)
  (prinl "}")
  (prinl))

(def cc:gen:prototype (NAME ARGS)
  "Generate C code function prototype."
  (prin "static atom_t " NAME "(");
  (iter (\ (e) (prin "value_t " e ", ")) ARGS)
  (prinl "closure_t C, callback_t K);");
  (prinl))

(def cc:gen:main (NAME ARGS REFS kvar)
  "Generate C code for the main function."
  (prin "static atom_t " NAME "(")
  (o>)
  (iter (\ (e) (prin "value_t " e ", ")) ARGS)
  (prinl "closure_t C, callback_t K) {");
  (prinl (o|) "closure_t _C = lisp_closure_get(&cache, C, " (len REFS) ");")
  (foldl
    (\ (acc e) (prinl (o|) "_C->V[" acc "] = " e ";") (+ acc 1)) 0 ARGS)
  (prinl (o|) "value_t R = { .callback = K };");
  (cc:gen:next NAME ARGS kvar)
  (prinl (o|) "return _K(_C, R);")
  (<o)
  (prinl "}")
  (prinl))

(def cc:gen:argument (TYPES arg)
  "Generate the argument extraction."
  (prinl (o|) "LISP_LOOKUP(_" arg ", arguments, " arg ");")
  #
  # Determine how to extract the value.
  #
  (let ((atype . (assoc arg TYPES)))
    (match atype
      (NUMBER . (prinl (o|) "value_t " arg " = { .number = _" arg "->number };"))
      (LIST   . (prinl (o|) "value_t " arg " = { .list = UP(car) };"))
      (_      . (prinl (o|) "value_t " arg " = { .number = 0 }; /* Unsupported type " (str atype) " */"))
      ))
  #
  (prinl (o|) "X(_" arg ");"))

(def cc:gen:plugin (NAME ARGS TYPES)
  "Generate C code for the plugin."
  (prinl "static atom_t lisp_function_" NAME "(const atom_t closure, const atom_t arguments) {")
  (o>)
  (iter (\ (e) (cc:gen:argument TYPES e)) ARGS)
  (prinl (o|) "closure_t _C = NULL;")
  (prinl (o|) "callback_t _K = identity;")
  (prin (o|) "atom_t R = " NAME "(")
  (iter (\ (e) (prin e ", ")) ARGS)
  (prinl "_C, _K);")
  (prinl (o|) "lisp_closure_clear(&cache);")
  (prinl (o|) "return R;")
  (<o)
  (prinl "}")
  (prinl)
  (prin "LISP_PLUGIN_REGISTER(" NAME ", " NAME)
  (iter (\ (e) (prin ", " e)) ARGS)
  (prinl ");"))

#
# Inner section generators.
#

(def cc:gen:closure (CONT next)
  "Generate a default closure capture."
  (if (= CONT next)
    (prinl (o|) "closure_t _C = C->C;")
    (prinl (o|) "closure_t _C = C;")))

(def cc:gen:next (NAME CONT ref)
  "Generate the continuation capture for REF."
  (if (<> CONT ref)
    (prinl (o|) "callback_t _K = " NAME ref ";")
    (prog
      (prinl (o|) "callback_t _K = C->V[" (cdr (str ref)) "].callback;")
      (prinl (o|) "lisp_closure_put(&cache, C);"))))

(def cc:gen:block (NAME ARGS CONT kvar body)
  "Generate the basic block for BODY."
  (match body
    #
    # Predicate operations.
    #
    ~(foldl
       (\ (acc e) (cons (cons
                          (list e _ _)
                          (quote  . (let ((next . (last body)))
                                      (cc:op:predicate ARGS kvar body)
                                      (cc:gen:closure CONT next)
                                      (cc:gen:next NAME CONT next)
                                      (prinl (o|) "return _K(_C, R);"))))
                    acc))
       NIL '(nil?))
    #
    # Binary operations.
    #
    ~(foldl
       (\ (acc e) (cons (cons
                          (list e _ _ _)
                          (quote  . (let ((next . (last body)))
                                      (ccp:op:binnum ARGS kvar body)
                                      (cc:gen:closure CONT next)
                                      (cc:gen:next NAME CONT next)
                                      (prinl (o|) "return _K(_C, R);"))))
                    acc))
       NIL '(+ - * / = <> < <= > >=))
    #
    # Unary list operations.
    #
    ~(foldl
       (\ (acc e) (cons (cons
                          (list e _ _)
                          (quote .  (let ((next . (last body)))
                                      (cc:op:unqlst ARGS kvar body)
                                      (cc:gen:closure CONT next)
                                      (cc:gen:next NAME CONT next)
                                      (prinl (o|) "return _K(_C, R);"))))
                    acc))
       NIL '(car cdr))
    #
    # Binary list operations.
    #
    ~(foldl
       (\ (acc e) (cons (cons
                          (list e _ _ _)
                          (quote .  (let ((next . (last body)))
                                      (cc:op:binlst ARGS kvar body)
                                      (cc:gen:closure CONT next)
                                      (cc:gen:next NAME CONT next)
                                      (prinl (o|) "return _K(_C, R);"))))
                    acc))
       NIL '(conc cons))
    #
    # IF/THEN/ELSE
    #
    ((if _ _ _) . (let (((_ _ then else) . body))
                    (prinl (o|) "if (V.number) {")
                    (o>)
                    (cc:gen:block NAME ARGS CONT kvar then)
                    (<o)
                    (prinl (o|) "} else {")
                    (o>)
                    (cc:gen:block NAME ARGS CONT kvar else)
                    (<o)
                    (prinl (o|) "}")))
    #
    # KVAR funcall.
    #
    ((_ _)  . (let ((next . (car body)))
                (cc:gen:closure CONT next)
                (cc:op:kvar ARGS kvar body)
                (cc:gen:next NAME CONT next)
                (prinl (o|) "return _K(_C, R);")))
    #
    # Recursive behavior
    #
    ((_ . _) . (let (((symb . prms) . body)
                     (next          . (last body)))
                 (if (= symb NAME)
                   (prog
                     (cc:gen:closure CONT next)
                     (cc:op:recur ARGS kvar ARGS prms)
                     (cc:gen:next NAME CONT next)
                     (prin (o|) "return " NAME "(")
                     (iter (\ (e) (prin e ", ")) ARGS)
                     (prinl "_C, _K);"))
                   (prinl (o|) "abort(); /* Unsupported operation " symb " */"))))
    #
    # Unknown behavior.
    #
    (_  . (prinl (o|) "abort(); /* " body " */"))))

(def cc:gen:function (NAME ARGS REFS CONT kvar body)
  "Generate a continuation KVAR with BODY."
  (prinl "static atom_t " NAME kvar "(closure_t C, value_t V) {")
  (o>)
  (if (> (assoc kvar REFS) 0)
    (prinl (o|) "C->V[" (cdr (str kvar)) "] = V;"))
  (cc:gen:block NAME ARGS CONT kvar body)
  (<o)
  (prinl "}")
  (prinl))

(def cc:gen:cont-any (NAME ARGS REFS CONT kvar fun)
  "Generate the C code for any type of funcall FUN."
  (let ((cont          . (last fun))
        ((_ args body) . cont)
        (nvar          . (last args)))
    (if (lst? cont)
      (prog
        (cc:gen:cont NAME ARGS REFS CONT nvar body)
        (cc:gen:function NAME ARGS REFS CONT kvar (cc:replace nvar fun)))
      (cc:gen:function NAME ARGS REFS CONT kvar fun))))

(def cc:gen:cont-ite (NAME ARGS REFS CONT kvar fun)
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
            (cc:gen:cont-any NAME ARGS REFS CONT (last args) body)) conts)
    (cc:gen:function NAME ARGS REFS CONT kvar (cons symb (cons cnd first)))))

(def cc:gen:cont (NAME ARGS REFS CONT kvar fun)
  "Generate the C code for the continuations in FUN."
  (match fun
    #
    # IF/THEN/ELSE
    #
    ((if _ . _) . (cc:gen:cont-ite NAME ARGS REFS CONT kvar fun))
    #
    # Default behavior.
    #
    (_  .  (cc:gen:cont-any NAME ARGS REFS CONT kvar fun))))

#
# Main compiler entrypoint.
#

(def cc:transpile (SYMB)
  "Generate the C code for the CPS function SYMB."
  (let ((cps             . (>& SYMB))
        ((args _ _ body) . cps)
        (ARGS            . (rev (cdr (rev args))))
        (REFS            . (cps:refcount cps))
        (TYPES           . (cps:infer NAME cps))
        (kvar            . (last args))
        (cout            . (append (str SYMB) ".c")))
    #
    # Remove the C file
    #
    (unlink cout)
    #
    # Generate the C file
    #
    (out cout
      (cc:gen:prologue TYPES kvar)
      (cc:gen:prototype SYMB ARGS)
      (cc:gen:cont SYMB ARGS REFS kvar kvar body)
      (cc:gen:main SYMB ARGS REFS kvar)
      (cc:gen:plugin SYMB ARGS TYPES))
    #
    # Return then symbol passed as argument.
    #
    SYMB
    ))

(def cc:build (SYMB . WHERE)
  "Build the plugin for the CPS function SYMB."
  (let ((wopt . (eval (car WHERE)))
        (trgt . (if wopt wopt (append (assoc "HOME" ENV) "/.mnml")))
        (cout . (append (str SYMB) ".c"))
        (oout . (append trgt "/libminimal_function_" (str SYMB) ".so")))
    #
    # Remove the SO file
    #
    (unlink oout)
    #
    # Build the SO file
    #
    (let ((incd . (append (prefix) "/include"))
          (libd . (append (prefix) "/lib"))
          (args . (list "-O3" "-shared" "-fPIC" "-fomit-frame-pointer" "-flto" "-o" oout cout "-I" incd "-L" libd "-lminimal"))
          ((status . messages) . (run "/usr/bin/gcc" args ENV)))
      (= 0 status)
      )))
