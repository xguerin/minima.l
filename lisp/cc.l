(load
  "assoc.l" "cps.l" "iter.l" "filter.l" "foldl.l" "foldr.l" "last.l" "map.l"
  "rev.l" "run.l" "unless.l" "when.l" 'prefix 'unlink)

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
  (iter (\ (e) (prinl (o|) " * " e)) @)
  (prinl (o|) " */"))

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
      (lst? . (prin "IS_PAIR("))
      (nil? . (prin "IS_NULL("))
      (_   . (prin " /* Unsupported predicate " op " */ ")))
    (cc:value:lst ARGS kvar arg)
    (prinl ") };")))

(def cc:op:num:two (ARGS kvar body)
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

(def cc:op:lst:one (ARGS kvar body)
  "Generate C code for unary list operations."
  (let (((op arg cont)  . body))
    (prin (o|) "value_t R = { .atom = ")
    (match op
      (car . (prin "lisp_car("))
      (cdr . (prin "lisp_cdr("))
      (_   . (prin " /* Unsupported operation " op " */ ")))
    (cc:value:lst ARGS kvar arg)
    (prinl ") };")))

(def cc:op:lst:two (ARGS kvar body)
  "Generate C code for binary list operations."
  (let (((op arg0 arg1 cont)  . body))
    (prin (o|) "value_t R = { .atom = ")
    (match op
      (conc . (prin "lisp_conc("))
      (cons . (prin "lisp_cdr("))
      (_   .  (prin " /* Unsupported operation " op " */ ")))
    (cc:value:lst ARGS kvar arg0)
    (prin ", ")
    (cc:value:lst ARGS kvar arg1)
    (prinl ") };")))

(def cc:op:kvar (ARGS kvar (_ arg))
  "Generate C code for immediate KVAR funcall."
  (prin (o|) "value_t R = ")
  (cc:value:ref ARGS kvar arg))

(def cc:op:funcall (ARGS TYPES kvar args)
  "Generate C code for external function calls."
  (let ((genquot  . (\ (arg) (prog
                               (prin "lisp_cons(QUOTE, ")
                               (cc:value:lst ARGS kvar arg)
                               (prinl ");"))))
        (genargs  . (\ (arg)
                      (let ((type . (assoc arg TYPES)))
                        (unless (= arg kvar)
                          (prin (o|) "atom_t " arg " = ")
                          (match type
                            (NUMBER . (prog
                                        (prin "lisp_make_number(")
                                        (cc:value:num ARGS kvar arg)
                                        (prinl ");")))
                            (ATOM . (genquot arg))
                            (_    . (prinl "UP(NIL); /* Unsupported type " (str type) " */"))
                            )))))
        (gencall  . (\ (arg)
                      (unless (= arg kvar)
                        (prinl (o|) "__T = lisp_cons(" arg ", __S);")
                        (prinl (o|) "X(__S); __S = __T;"))))
        (type     . (assoc kvar TYPES)))
    (/**/ "Convert the value_t arguments to atom_t.")
    (iter genargs args)
    (/**/ "Build the expression to evaluate.")
    (prinl (o|) "atom_t __S = NIL, __T;")
    (iter gencall (rev args))
    (/**/ "Evaluate the expression.")
    (prinl (o|) "atom_t __R = lisp_eval(E, __S);")
    (/**/ "Convert the atom_t result to value_t.")
    (match type
      (NUMBER . (prinl (o|) "value_t R = { .number = __R->number };"))
      (ATOM   . (prinl (o|) "value_t R = { .atom = UP(__R) };"))
      (_      . (prinl (o|) "value_t R = { .number = 0 }; /* Unsupported type " (str atype) " */")))
    (prinl (o|) "X(__R);")
    ))

(def cc:op:reccall (ARGS kvar args prms)
  "Generate C code for recursive calls."
  (if (and (not (nil? args)) (not (nil? prms)))
    (let (((acar . acdr) . args)
          ((pcar . pcdr) . prms))
      (unless (= acar kvar)
        (prin (o|) "value_t " acar " = ")
        (cc:value:ref ARGS kvar pcar)
        (cc:op:reccall ARGS acdr pcdr)))))

#
# Value handlers.
#

(def cc:value:lst (ARGS kvar arg)
  "Generate the C code for the list R-value ARG."
  (cond arg
    (sym? . (if (= kvar arg)
              (prin "V.atom")
              (let ((idx . (cc:getrefidx ARGS arg)))
                (prin "C->V[" idx "].atom"))))
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
      (ATOM   . (prinl (o|) "return V.atom;"))
      (_ . (prinl (o|) "return UP(NIL); /* Unsupported type " (str rtype) " */"))
      )))

(def cc:gen:prologue (SYMB CPS TYPES kvar)
  "Generate C code prologue."
  (prinl "#include <mnml/closure.h>")
  (prinl "#include <mnml/lisp.h>")
  (prinl "#include <mnml/maker.h>")
  (prinl "#include <mnml/plugin.h>")
  (prinl "#include <mnml/slab.h>")
  (prinl "#include <mnml/types.h>")
  (prinl)
  #
  # Generate some information
  #
  (prinl "/*")
  (prin  " * FUNCTION   : ") (printl SYMB)
  (prin  " * DEFINITION : ") (printl (eval SYMB))
  (prin  " * CPS        : ") (printl CPS)
  (prin  " * TYPES      : ") (printl TYPES)
  (prinl " */")
  (prinl)
  #
  # Generate the closure cache.
  #
  (prinl "static closure_t cache = NULL;")
  (prinl))

(def cc:gen:identity (TYPES kvar)
  "Generate C code for the identity continuation."
  (prinl "static atom_t identity(const atom_t E, closure_t C, value_t V) {")
  (o>)
  (cc:gen:retval TYPES kvar)
  (<o)
  (prinl "}")
  (prinl))

(def cc:gen:prototype (NAME ARGS)
  "Generate C code function prototype."
  (prin "static atom_t _mnml_" NAME "(const atom_t E, ");
  (iter (\ (e) (prin "value_t " e ", ")) ARGS)
  (prinl "closure_t C, callback_t K);");
  (prinl))

(def cc:gen:main (NAME ARGS REFS kvar)
  "Generate C code for the main function."
  (prin "static atom_t _mnml_" NAME "(const atom_t E, ")
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
  (prinl (o|) "return _K(E, _C, R);")
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
      (ATOM   . (prinl (o|) "value_t " arg " = { .atom = _" arg " };"))
      (_      . (prinl (o|) "value_t " arg " = { .number = 0 }; /* Unsupported type " (str atype) " */"))
      ))
  #
  (prinl (o|) "X(_" arg ");"))

(def cc:gen:plugin (NAME ARGS TYPES)
  "Generate C code for the plugin."
  (prinl "static atom_t lisp_function_" NAME "(const atom_t env, const atom_t arguments) {")
  (o>)
  (/**/ "Grab the arguments.")
  (iter (\ (e) (cc:gen:argument TYPES e)) ARGS)
  (/**/ "Define the default closure and callback.")
  (prinl (o|) "closure_t _C = NULL;")
  (prinl (o|) "callback_t _K = identity;")
  (/**/ "Call the main function.")
  (prin (o|) "atom_t R = _mnml_" NAME "(env, ")
  (iter (\ (e) (prin e ", ")) ARGS)
  (prinl "_C, _K);")
  (/**/ "Clear the closure cache and return.")
  (prinl (o|) "lisp_closure_clear(&cache);")
  (prinl (o|) "return R;")
  (<o)
  (prinl "}")
  (prinl)
  (prin "LISP_PLUGIN_REGISTER(" NAME ", " NAME)
  (iter (\ (e) (prin ", " e)) ARGS)
  (if (= (len ARGS) 2) (prin ", NIL"))
  (prinl ");"))

#
# Inner section generators.
#

(def cc:gen:closure (CONT next)
  "Generate a default closure capture."
  (if (= CONT next)
    (prinl (o|) "closure_t _C = C->C;")
    (prinl (o|) "closure_t _C = C;")))

(def cc:gen:callback (NAME REFS CONT ref)
  "Generate the callback capture for REF."
  (if (= CONT ref)
    (prog
      (if (= (assoc ref REFS) 0)
        (prinl (o|) "callback_t _K = V.callback;")
        (prinl (o|) "callback_t _K = C->V[" (cdr (str ref)) "].callback;"))
      (prinl (o|) "lisp_closure_put(&cache, C);"))
    (prinl (o|) "callback_t _K = _mnml_" NAME ref ";")))

(def cc:gen:block (NAME ARGS REFS CONT kvar body)
  "Generate the basic block for BODY."
  (match body
    #
    # Predicate operations.
    #
    ~(foldl
       (\ (acc e)
         (cons (cons
                 (list e _ _)
                 (quote  . (let ((next . (last body)))
                             (/**/ "Build the closure.")
                             (cc:gen:closure CONT next)
                             (/**/ "Collect the argument.")
                             (cc:op:predicate ARGS kvar body)
                             (/**/ "Build the continuation.")
                             (cc:gen:callback NAME REFS CONT next)
                             (/**/ "Call the continuation.")
                             (prinl (o|) "return _K(E, _C, R);"))))
           acc))
       NIL '(lst? nil?))
    #
    # Binary operations.
    #
    ~(foldl
       (\ (acc e)
         (cons (cons
                 (list e _ _ _)
                 (quote  . (let ((next . (last body)))
                             (/**/ "Build the closure.")
                             (cc:gen:closure CONT next)
                             (/**/ "Collect the arguments.")
                             (cc:op:num:two ARGS kvar body)
                             (/**/ "Build the continuation.")
                             (cc:gen:callback NAME REFS CONT next)
                             (/**/ "Call the continuation.")
                             (prinl (o|) "return _K(E, _C, R);"))))
           acc))
       NIL '(+ - * / = <> < <= > >=))
    #
    # Unary list operations.
    #
    ~(foldl
       (\ (acc e)
         (cons (cons
                 (list e _ _)
                 (quote .  (let ((next . (last body)))
                             (/**/ "Build the closure.")
                             (cc:gen:closure CONT next)
                             (/**/ "Collect the argument.")
                             (cc:op:lst:one ARGS kvar body)
                             (/**/ "Build the continuation.")
                             (cc:gen:callback NAME REFS CONT next)
                             (/**/ "Call the continuation.")
                             (prinl (o|) "return _K(E, _C, R);"))))
           acc))
       NIL '(car cdr))
    #
    # Binary list operations.
    #
    ~(foldl
       (\ (acc e)
         (cons (cons
                 (list e _ _ _)
                 (quote .  (let ((next . (last body)))
                             (/**/ "Build the closure.")
                             (cc:gen:closure CONT next)
                             (/**/ "Collect the arguments.")
                             (cc:op:lst:two ARGS kvar body)
                             (/**/ "Build the continuation.")
                             (cc:gen:callback NAME REFS CONT next)
                             (/**/ "Call the continuation.")
                             (prinl (o|) "return _K(E, _C, R);"))))
           acc))
       NIL '(conc cons))
    #
    # IF/THEN/ELSE
    #
    ((if _ _ _) . (let (((_ _ then else) . body))
                    (prinl (o|) "if (V.number) {")
                    (o>)
                    (cc:gen:block NAME ARGS REFS CONT kvar then)
                    (<o)
                    (prinl (o|) "} else {")
                    (o>)
                    (cc:gen:block NAME ARGS REFS CONT kvar else)
                    (<o)
                    (prinl (o|) "}")))
    #
    # KVAR funcall.
    #
    ((_ _)  . (let ((next . (car body)))
                (/**/ "Build the closure.")
                (cc:gen:closure CONT next)
                (/**/ "Collect the argument for the external continuation.")
                (cc:op:kvar ARGS kvar body)
                (/**/ "Build the continuation.")
                (cc:gen:callback NAME REFS CONT next)
                (/**/ "Call the continuation.")
                (prinl (o|) "return _K(E, _C, R);")))
    #
    # Recursive behavior
    #
    ((_ . _) . (let (((symb . prms) . body)
                     (next          . (last body)))
                 (if (= symb NAME)
                   (prog
                     (/**/ "Build the closure.")
                     (cc:gen:closure CONT next)
                     (/**/ "Collect the arguments for recursion.")
                     (cc:op:reccall ARGS kvar ARGS prms)
                     (/**/ "Build the continuation.")
                     (cc:gen:callback NAME REFS CONT next)
                     (/**/ "Call the continuation.")
                     (prin (o|) "return _mnml_" NAME "(E, ")
                     (iter (\ (e) (prin e ", ")) ARGS)
                     (prinl "_C, _K);"))
                   (prog
                     (/**/ "Build the closure.")
                     (cc:gen:closure CONT next)
                     (cc:op:funcall ARGS TYPES kvar body)
                     (/**/ "Build the continuation.")
                     (cc:gen:callback NAME REFS CONT next)
                     (/**/ "Call the continuation.")
                     (prinl (o|) "return _K(E, _C, R);")))))
    #
    # Unknown behavior.
    #
    (_  . (prinl (o|) "abort(); /* " body " */"))))

(def cc:gen:function (NAME ARGS REFS CONT kvar body)
  "Generate a continuation KVAR with BODY."
  (prinl "static atom_t _mnml_" NAME kvar "(const atom_t E, closure_t C, value_t V) {")
  (o>)
  (if (> (assoc kvar REFS) 0)
    (prinl (o|) "C->V[" (cdr (str kvar)) "] = V;"))
  (cc:gen:block NAME ARGS REFS CONT kvar body)
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

(def cc:compile (SYMB)
  "Generate the C code for the CPS function SYMB."
  (let ((cps             . (>& SYMB))
        ((args _ _ body) . cps)
        (ARGS            . (rev (cdr (rev args))))
        (REFS            . (cps:refcount cps))
        (TYPES           . (cps:infer NAME cps))
        (isvalid         . (foldl (\ (acc (_ . t)) (and (<> t 'ERROR) acc)) T TYPES))
        (kvar            . (last args))
        (trgt            . (append (assoc "HOME" ENV) "/.mnml"))
        (cout            . (append trgt "/" (str SYMB) ".c")))
    (when isvalid
      #
      # Remove the C file
      #
      (unlink cout)
      #
      # Generate the C file
      #
      (out cout
        (cc:gen:prologue SYMB cps TYPES kvar)
        (cc:gen:prototype SYMB ARGS)
        (cc:gen:identity TYPES kvar)
        (cc:gen:cont SYMB ARGS REFS kvar kvar body)
        (cc:gen:main SYMB ARGS REFS kvar)
        (cc:gen:plugin SYMB ARGS TYPES))
      #
      # Return then symbol passed as argument.
      #
      SYMB
      )))

(def cc:flags ()
  "Generate the build flags based on the OS name."
  (let (((_ osname) . (run "/usr/bin/uname" '("-s") ENV))
        (incd . (append (assoc 'PREFIX CONFIG) "/include"))
        (libd . (append (assoc 'PREFIX CONFIG) "/lib"))
        (srcd . (append (assoc 'PREFIX CONFIG) "/src"))
        (flags . (list
                   "-g3" "-shared" "-fPIC" "-fomit-frame-pointer"
                   "-I" incd "-L" libd "-L" srcd "-lminimal"
                   "-o" oout cout)))
    (if (= osname "Linux") ($+ flags '("-flto")) flags)))

(def cc:build (SYMB)
  "Build the plugin for the CPS function SYMB."
  (let ((trgt . (append (assoc "HOME" ENV) "/.mnml"))
        (cout . (append trgt "/" (str SYMB) ".c"))
        (oout . (append trgt "/libminimal_function_" (str SYMB) ".so")))
    (when SYMB
      #
      # Remove the SO file
      #
      (unlink oout)
      #
      # Build the SO file
      #
      (let (((status . messages) . (run "/usr/bin/cc" (cc:flags) ENV)))
        (if (= 0 status) (load SYMB))))))
