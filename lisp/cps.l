(load
  "append.l" "assoc.l" "foldl.l" "foldr.l" "has.l" "last.l" "len.l" "ntoa.l"
  "orelse.l" "replc.l" "rev.l" "when.l")

#
# Make a symbol placeholder
#
(def cps:symbol (id)
  "Generate a symbol placeholder using ID."
  (sym (cons ^_ (ntoa id))))

#
# Reverse-scan a function body and replace the funcall arguments with
# numbered placeholders like _0, _1, or _2.
#
(def cps:swap (kvar counter body)
  "Swap BODY funcall arguments with placeholders; append KVAR."
  (foldr (\ (e (id . result))
           (if (lst? e)
             (let ((larg . (cps:symbol id))
                   ((next . _) . (cps:swap kvar (+ id 1) e)))
               (cons next (cons larg result)))
             (cons id (cons e result))))
    body
    (cons counter kvar)))

#
# Lift an if-then-else funcall into wrapping lambdas for the statement and
# the then/else parts separately.
#
(def cps:lift-ith (kvar counter body)
  "Lift IF-BODY arguments into lambdas; use KVAR as external continuation."
  (let (((op stmt then)          . body)
        ((then:next . then:body) . (cps:lift kvar counter then))
        (nbody                   . (list then:body))
        ((stmt:next . stmt:body) . (cps:lift nbody then:next (list op stmt))))
    (cons stmt:next stmt:body)))

(def cps:lift-ite (kvar counter body)
  "Lift IF-BODY arguments into lambdas; use KVAR as external continuation."
  (let (((_ stmt then else)      . body)
        ((else:next . else:body) . (cps:lift kvar counter else))
        ((then:next . then:body) . (cps:lift kvar else:next then))
        (nbody                   . (list then:body else:body))
        ((stmt:next . stmt:body) . (cps:lift nbody then:next (list 'if stmt))))
    (cons stmt:next stmt:body)))

#
# Lift a generic funcall into wrapping lambdas.
#
(def cps:lift-any (kvar counter body)
  "Lift BODY arguments into lambdas; use KVAR as external continuation."
  (let (((_ . swapped) . (cps:swap kvar counter body)))
    (foldr (\ (e (id . res))
             (if (lst? e)
               (let ((largs . (list (cps:symbol id)))
                     (lfunc . (list (list '\ largs res)))
                     ((next . nbody) . (cps:lift lfunc (+ id 1) e)))
                 (cons next nbody))
               (cons id res)))
      (cdr body)
      (cons counter swapped))))

#
# Lift a funcall into wrapping lambdas.
#
(def cps:lift (kvar counter body)
  "Lift BODY arguments into lambdas; use KVAR as external continuation."
  (cond body
    (lst? . (match body
              #
              # IF/THEN forms,
              #
              ~(foldl
                 (\ (acc e) (cons (cons (list e _ _)
                                    '(cps:lift-ith kvar counter body))
                              acc))
                 NIL '(if unless when))
              #
              # IF/THEN/ELSE form.
              #
              ((if _ _ _) . (cps:lift-ite kvar counter body))
              #
              # Generic form.
              #
              (_ . (cps:lift-any kvar counter body))))
    (_  . (cons counter (append kvar (list body))))))

#
# Convert a function in direct style into CPS.
#
(def >& (fsym)
  "Convert FSYM into its continuation-passing style."
  (let (((args closure clst body) . (eval fsym))
        (arglen                   . (len args))
        (kvar                     . (list (sym (cons ^_ (ntoa arglen)))))
        (fargs                    . (append args kvar))
        ((_ . lifted)             . (cps:lift kvar (+ arglen 1) body)))
    (cons fargs (cons closure (cons clst (list lifted))))))

#
# Collect the usage count for each placeholder variable. The first symbol in the
# funcall is skipped unless it is the current external continuation variable.
# Local symbol (eg. local continuations) are collected with a count of 0.
#
(def cps:getrefs (ARGS kvar acc expr)
  "Accumulate in ACC references found in EXPR."
  (let (((op . _) . expr)
        (params . (if (has ARGS op) expr (cdr expr))))
    (foldl
      (\ (acc e)
        (if (sym? e)
          (let ((cur . (assoc e acc))
                (inc . (if (= e kvar) 0 1)))
            (if cur
              (replc e (+ cur inc) acc)
              (cons (cons e inc) acc)))
          acc))
      acc params)))

(def cps:getrefs-any (ARGS kvar fun acc)
  "Recursively collect placeholder reference count in any CPS form."
  (let (((_ args body) . (last fun))
        (nvar          . (last args)))
    (cps:getrefs ARGS kvar (cps:refcount_r ARGS nvar body acc) fun)))

(def cps:getrefs-ite (ARGS kvar fun acc)
  "Recursively collect placeholder reference count in a CPS ITE form."
  (let (((_ _ . ops) . fun))
    (cps:getrefs ARGS kvar
                 (foldr (\ (e acm) (cps:refcount_r ARGS kvar e acm)) ops acc)
                 fun)))

(def cps:refcount_r (ARGS kvar fun acc)
  "Recursively collect placeholder reference count in a CPS funcall FUN."
  (match fun
    #
    # Control forms
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ . _) '(cps:getrefs-ite ARGS kvar fun acc)) acc))
       NIL '(if unless when))
    #
    # Any forms.
    #
    ((_ . _) . (cps:getrefs-any ARGS kvar fun acc))
    #
    # Default.
    #
    (_ . acc)))

(def cps:refcount ((ARGS _ _ body))
  "Collect placeholder reference count in a CPS function FUN."
  (if (lst? body)
    (let ((xvar . (last ARGS)))
      (if body (cps:refcount_r ARGS xvar body NIL)))))

#
# Infer type signature from a CPS funcall.
#

(def cps:replace (nvar args)
  "Replace the last argument by the symbol of the next continuation."
  (if nvar
    (rev (cons nvar (cdr (rev args))))
    args))

(def cps:type:get (key acc)
  "Get the type of KEY in ACC."
  (cond key
    (nil? . 'ATOM)
    (num? . 'NUMBER)
    (str? . 'STRING)
    (sym? .  (<|> (cps:type:fold1 (assoc key acc)) 'ATOM))
    (lst? . 'ATOM)
    ( _   . 'ATOM)))

(def cps:type:mark (key type acc)
  "Add TYPE to KEY in ACC."
  (if (sym? key)
    (let ((value . (assoc key acc)))
      (if value
        (replc key (cons type value) acc)
        (cons (cons key (cons type NIL)) acc)))
    acc))

(def cps:type:next (NAME ARGS CONT next type alst)
  "Follow the next continuation and mark it as TYPE."
  (if (sym? next)
    (cps:type:mark next type alst)
    (let (((_ (nvar) body) . next))
      (|> alst
          (cps:type:mark nvar type)
          (cps:type:findr NAME ARGS CONT nvar body)
          ))))

(def cps:type:func (NAME ARGS symb params alst)
  "Check if SYMB is a function from ARGS."
  (if (and (has ARGS symb) (<> NAME symb))
    (cps:type:mark symb 'ATOM
                   (foldl (\ (acc e) (cps:type:mark e 'ATOM acc)) alst params))
    alst))

(def cps:type:findr (NAME ARGS CONT kvar body alst)
  "Compute the types involved in a funcall."
  (match body
    #
    # Generate the comparators.
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _ _)
                          '(let (((_ arg0 arg1 next) . body))
                             (|> alst
                                 (cps:type:mark arg0 'UNDEF)
                                 (cps:type:mark arg1 'UNDEF)
                                 (cps:type:next NAME ARGS CONT next 'NUMBER)
                                 )))
                    acc))
       NIL '(= <>))
    #
    # Generate the unary numeric operations.
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _)
                          '(let (((_ arg next) . body))
                             (|> alst
                                 (cps:type:mark arg 'NUMBER)
                                 (cps:type:next NAME ARGS CONT next 'NUMBER)
                                 )))
                    acc))
       NIL '(not))
    #
    # Generate the binary numeric operations.
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _ _)
                          '(let (((_ arg0 arg1 next) . body))
                             (|> alst
                                 (cps:type:mark arg0 'NUMBER)
                                 (cps:type:mark arg1 'NUMBER)
                                 (cps:type:next NAME ARGS CONT next 'NUMBER)
                                 )))
                    acc))
       NIL '(+ - * / < <= > >= and or))
    #
    # Generate the list unary operations
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _)
                          '(let (((_ arg next) . body))
                             (|> alst
                                 (cps:type:mark arg 'ATOM)
                                 (cps:type:next NAME ARGS CONT next 'ATOM)
                                 )))
                    acc))
       NIL '(car cdr))
    #
    # Predicates.
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _)
                          '(let (((_ arg next) . body))
                             (|> alst
                                 (cps:type:mark arg 'ATOM)
                                 (cps:type:next NAME ARGS CONT next 'NUMBER)
                                 )))
                    acc))
       NIL '(lst? nil?))
    #
    # Generate the list binary operations.
    #
    ((conc _ _ _) . (let (((_ arg0 arg1 next) . body))
                      (|> alst
                          (cps:type:mark arg0 'ATOM)
                          (cps:type:mark arg1 'ATOM)
                          (cps:type:next NAME ARGS CONT next 'ATOM)
                          )))
    ((cons _ _ _) . (let (((_ arg0 arg1 next) . body))
                      (|> alst
                          (cps:type:mark arg0 'ATOM)
                          (cps:type:mark arg1 'ATOM)
                          (cps:type:next NAME ARGS CONT next 'ATOM)
                          )))
    #
    # IF/THEN, UNLESS, WHEN.
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _)
                          '(let (((_ e thn) . body))
                             (|> alst
                                 (cps:type:mark e 'NUMBER)
                                 (cps:type:findr NAME ARGS CONT kvar thn)
                                 (cps:type:mark CONT 'ATOM)
                                 )))
                    acc))
       NIL '(if unless when))
    #
    # IF/THEN/ELSE.
    #
    ((if _ _ _) . (let (((_ e thn els) . body))
                    (|> alst
                        (cps:type:mark e 'NUMBER)
                        (cps:type:findr NAME ARGS CONT kvar thn)
                        (cps:type:findr NAME ARGS CONT kvar els)
                        )))
    #
    # KVAR funcall.
    #
    ((_ _) . (let (((var arg) . body)
                   (xtype     . (cps:type:fold1 (assoc var alst)))
                   (atype     . (cps:type:get arg alst)))
               (match (cons xtype atype)
                 ((NIL . NIL) . (|> alst
                                    (cps:type:mark var 'UNDEF)
                                    (cps:type:mark arg 'UNDEF)
                                    ))
                 ((_   . NIL) . (cps:type:mark arg xtype alst))
                 ((NIL . _  ) . (cps:type:mark var atype alst))
                 (_           .  alst))))
    #
    # Generic method invocation case.
    #
    ((_ . _)  . (let (((symb . args)   . body)
                      ((_ (nvar) next) . (last body)))
                  (|> alst
                      (cps:type:func NAME ARGS symb (cps:replace nvar args))
                      (cps:type:findr NAME ARGS CONT nvar next)
                      )))
    #
    # Default case, return the association list.
    #
    (_ . alst)))

(def cps:type:fold1 (lst)
  "Fold the types in LST and check for errors."
  (foldr
    (\ (e acc)
      (if (|> (nil? acc) (or (= e acc)) (or (= acc 'UNDEF))) e 'ERROR))
    lst NIL))

(def cps:type:fold (alst)
  "Fold the type list ALST and check for errors."
  (foldr
    (\ ((symb . types) acc0) (cons (cons symb (cps:type:fold1 types)) acc0))
    alst NIL))

(def cps:type:findl (kvar body alst)
  "Resolve the type for KVAR if it does not have one."
  (match body
    #
    # IF/THEN, UNLESS, WHEN.
    #
    ~(foldl
       (\ (acc e) (cons (cons (list e _ _)
                          '(let (((_ _ thn) . body))
                             (|> alst
                                 (cps:type:mark kvar 'ATOM)
                                 (cps:type:findl kvar thn)
                                 )))
                    acc))
       NIL '(if unless when))
    #
    # IF/THEN/ELSE.
    #
    ((if _ _ _) . (let (((_ _ thn els) . body))
                    (|> alst
                        (cps:type:findl kvar thn)
                        (cps:type:findl kvar els)
                        )))
    #
    # KVAR.
    #
    ((_ _)  . (let (((op arg) . body)
                    (atype    . (cps:type:get arg alst)))
                (cps:type:mark kvar atype alst)))
    #
    # Other.
    #
    ((_ . _)  . (let (((_ _ next) . (last body)))
                  (cps:type:findl kvar next alst)))
    #
    # Default.
    #
    (_ . alst)))

(def cps:infer (name (args _ _ body))
  "Infer the types of a CPS function FUN with name NAME."
  (let ((kvar . (last args)))
    #
    # NOTE(xrg) don't ever consider removing the fluent operator again.
    #
    (|> NIL
        (cps:type:findr name args kvar kvar body)
        (cps:type:findl kvar body)
        cps:type:fold
        )))
