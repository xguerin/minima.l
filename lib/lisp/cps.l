(load "@lib/append.l")
(load "@lib/assoc.l")
(load "@lib/foldl.l")
(load "@lib/foldr.l")
(load "@lib/last.l")
(load "@lib/len.l")
(load "@lib/ntoa.l")
(load "@lib/replc.l")

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
#
# Collect the usage count for each placeholder variable.
#
(def cps:refcolct (kvar acc expr)
  "Accumulate in ACC references found in EXPR."
  (foldl
    (\ (acc e)
      (if (and (sym? e) (<> e kvar))
        (let ((cur . (assoc e acc)))
          (if cur
            (replc e (+ cur 1) acc)
            (cons (cons e 1) acc)))
        acc))
    acc expr))

(def cps:refcount_r (kvar fun acc)
  "Recursively collect placeholder reference count in a CPS funcall FUN."
  (match fun
    ((if _ _)   . (let (((_ _ thn) . fun))
                    (cps:refcolct kvar (cps:refcount_r kvar thn acc) fun)))
    ((if _ _ _) . (let (((_ _ thn els) . fun))
                    (cps:refcolct kvar (cps:refcount_r kvar els (cps:refcount_r kvar thn acc)) fun)))
    ((_ . _)    . (let (((_ args body) . (last fun)))
                    (cps:refcolct kvar (cps:refcount_r (last args) body acc) fun)))))

(def cps:refcount ((args clos body))
  "Collect placeholder reference count in a CPS function FUN."
  (if body (cps:refcount_r (last args) body NIL)))
