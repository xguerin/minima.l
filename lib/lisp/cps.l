(load "@lib/append.l")
(load "@lib/assoc.l")
(load "@lib/foldl.l")
(load "@lib/foldr.l")
(load "@lib/last.l")
(load "@lib/len.l")
(load "@lib/ntoa.l")
(load "@lib/replc.l")
(load "@lib/rev.l")

#
# Make a symbol placeholder
#
(def cps:symbol (id)
  "Generate a symbol placeholder using ID."
  (sym (cons '_' (ntoa id))))

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
  (let (((_ stmt then)           . body)
        ((then:next . then:body) . (cps:lift kvar else:next then))
        (nbody                   . (list then:body))
        ((stmt:next . stmt:body) . (cps:lift nbody then:next (list 'if stmt))))
    (cons else:next stmt:body)))

(def cps:lift-ite (kvar counter body)
  "Lift IF-BODY arguments into lambdas; use KVAR as external continuation."
  (let (((_ stmt then else)      . body)
        ((else:next . else:body) . (cps:lift kvar counter else))
        ((then:next . then:body) . (cps:lift kvar else:next then))
        (nbody                   . (list then:body else:body))
        ((stmt:next . stmt:body) . (cps:lift nbody then:next (list 'if stmt))))
    (cons else:next stmt:body)))

#
# Lift a generic funcall into wrapping lambdas.
#
(def cps:lift-any (kvar counter body)
  "Lift BODY arguments into lambdas; use KVAR as external continuation."
  (let (((count . swapped) . (cps:swap kvar counter body)))
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
    (lst? .  (match body
               #
               # Control forms,
               #
               ((if _ _  ) . (cps:lift-ith kvar counter body))
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
  (let (((args closure body) . (eval fsym))
        (arglen              . (len args))
        (kvar                . (list (sym (cons '_' (ntoa arglen)))))
        (fargs               . (append args kvar))
        ((count . lifted)    . (cps:lift kvar (+ arglen 1) body)))
    (cons fargs (cons closure (list lifted)))))

(def <->& (fsym)
  "Convert FSYM into its continuation-passing style and store it back into FSYM."
  (<- fsym (>& fsym)))

#
# Collect the usage count for each placeholder variable. The first symbol in the
# funcall is skipped unless it is the current external continuation variable.
# Local symbol (eg. local continuations) are collected with a count of 0.
#
(def cps:getrefs (xvar kvar acc expr)
  "Accumulate in ACC references found in EXPR."
  (let (((op . _) . expr)
        (params . (if (= op xvar) expr (cdr expr))))
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

(def cps:refcount_r (xvar kvar fun acc)
  "Recursively collect placeholder reference count in a CPS funcall FUN."
  (match fun
    ((if _ . _) . (let (((_ _ . ops) . fun)
                        (red . (\ (e acm) (cps:refcount_r xvar kvar e acm))))
                    (cps:getrefs xvar kvar (foldr red ops acc) fun)))
    ((_ . _) . (let (((_ args body) . (last fun))
                     (nvar . (last args)))
                 (cps:getrefs xvar kvar (cps:refcount_r xvar nvar body acc) fun)))
    (_ . acc)))

(def cps:refcount ((args _ body))
  "Collect placeholder reference count in a CPS function FUN."
  (let ((xvar . (last args)))
    (if body (cps:refcount_r xvar xvar body NIL))))

#
# Infer type signature from a CPS funcall.
#
(def cps:addtype (key type acc)
  "Add TYPE to KEY in ACC."
  (let ((value . (assoc key acc)))
    (if value
      (replc key (cons type value) acc)
      (cons (cons key (cons type NIL)) acc))))

(def cps:replace (nvar args)
  "Replace the last argument by the symbol of the next continuation."
  (if nvar
    (rev (cons nvar (cdr (rev args))))
    args))

(def cps:scantypes (kvar body type alst)
  "Scan references in BODY and mark them as TYPE in ALST."
  (let (((_ . args) . body)
        ((_ (nvar) next) . (last body)))
    (foldl
      (\ (acc e) (if (sym? e) (cps:addtype e type acc) acc))
      (cps:comptypes name nvar next alst)
      (cps:replace nvar args))))

(def cps:comptypes (name kvar body alst)
  "Compute the types involved in a funcall."
  (match body
    #
    # Generate the binary numeric operations
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons (list e _ _ _)
                                '(cps:scantypes kvar body 'NUMBER alst))) acc))
       NIL '(+ - * / = <> < <= > >=))
    #
    # Generate the list unary operations
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons (list e _ _)
                                '(cps:scantypes kvar body 'LIST alst))) acc))
       NIL '(car cdr))
    #
    # Generate the list binary operations
    #
    ~(foldl
       (\ (acc e) (cons (conc (cons (list e _ _ _)
                                '(cps:scantypes kvar body 'LIST alst))) acc))
       NIL '(cons conc))
    #
    # Predicates.
    #
    ((nil? _ _) . (let (((_ arg lst) . body)
                        ((_ (nvar) next) . lst))
                    (cps:comptypes name nvar next
                                   (cps:addtype nvar 'NUMBER
                                                (cps:addtype arg 'LIST alst)))))
    #
    #
    # IF/THEN/ELSE.
    #
    ((if _ . _) . (let (((_ e . ops) . body))
                    (foldr (\ (e acc) (cps:comptypes name kvar e acc))
                      ops (cps:addtype e 'NUMBER alst))))
    #
    # KVAR funcall.
    #
    ((_ _) . (let (((xvar arg) . body)
                   (atype . (assoc arg alst))
                   (xtype . (assoc xvar alst)))
               (if xtype
                 (cps:addtype arg (cps:reducetype xtype) alst)
                 (cps:addtype arg 'UNDEF alst))))
    #
    # Generic method invocation case.
    #
    ((_ . _)  . (let (((symb . prms) . body)
                      ((_ (nvar) next) . (last body)))
                  (cps:comptypes name nvar next alst)))
    #
    # Default case, return the association list.
    #
    (_ . alst)))

(def cps:reducetype (lst)
  "Reduce the types in LST and check for errors."
  (foldr
    (\ (e acc)
      (if (or (nil? acc) (or (= acc e) (= acc 'UNDEF))) e 'ERROR))
    lst NIL))

(def cps:reducetypes (alst)
  "Reduce the type list ALST and check for errors."
  (foldr
    (\ ((symb . types) acc0) (cons (cons symb (cps:reducetype types)) acc0))
    alst NIL))

(def cps:fixupkvar (kvar alst)
  "Mark KVAR in ALST as LIST if not found."
  (if (nil? (assoc kvar alst))
    (cps:addtype kvar 'LIST alst)
    alst))

(def cps:fixupundef (kvar alst)
  "Scan for UNDEF references in ALST and mark them as KVAR."
  (let ((ktype . (assoc kvar alst)))
    (foldr
      (\ ((k . type) acc)
        (if (= type 'UNDEF)
          (cons (cons k ktype) acc)
          (cons (cons k type) acc)))
      alst NIL)))

(def cps:infer (name (args _ body))
  "Infer the types of a CPS function FUN with name NAME."
  (let ((kvar . (last args)))
    #
    # NOTE(xrg) don't ever consider removing the fluent operator again.
    #
    (|> NIL
        (cps:comptypes name kvar body)
        (cps:fixupkvar kvar)
        cps:reducetypes
        (cps:fixupundef kvar))))
