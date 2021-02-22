# Language

## Source encoding

The source encoding is UTF-8.

## Comment
```lisp
# This is a comment
```
## Types

### Table

| Name      | Description                                           |
|:----------|:------------------------------------------------------|
| List      | `( ... )`                                               |
| Number    | Positive and negative 64-bit integers                 |
| Symbol    | 16-character string                                   |
| Character | A `^`-prefixed printable character                      |
| `T`         | Stands for `true`                                       |
| `NIL`       | The empty list, also stands for `false`                 |
| `_`         | Wildcard, used as a placeholder during deconstruction |

### Symbols

Symbols are bound to values. Undefined symbols are unbound and resolve to `NIL`.
Symbols can be created or altered globally using the `def` and `setq` functions.
Symbols can also be defined locally using the `let` function.

### Strings

The grammar supports the _string_ type. Represented as a `"`-delimited string of
characters, it is internally stored as a list of characters. For example:
```lisp
: "hello"
> (^h ^e ^l ^l ^o)
```
## Expression evaluation

### Generic rules

* Numbers, strings, `NIL`, `T`, and `_` evaluate to themselves
* Symbols are dereferenced to their bound values
* Lists are evaluated as expressions

### Expressions

Expression take the following form: `(FUNCTION ARGS ...)`. The `FUNCTION` can be:

1. A symbol that must resolve to a function definition
2. A quoted function definition
3. A lambda definition

The forms 1, 2, and 3 are mutually interchangeable:
```lisp
: (def add (a b) (+ a b))
> ((a b) NIL (+ a b))
: (add 1 2) # Form 1
> 3
: ('((a b) NIL (+ a b)) 1 2) # Form 2
> 3
: ((\ (a b) (+ a b)) 1 2) # Form 3
> 3
```
## Functions

Functions are represented internally as the following 3-uple:
```lisp
(ARGUMENTS CLOSURE . BODY)
```
The `ARGUMENTS` element is a list of symbols representing the arguments of the
functions. The `CLOSURE` element is an association list that contains the
context of the function at the definition site. Lastly, the `BODY` element is
the expression of the function. When defined through plugins, the body may also
be a number representing the memory address of the native implementation of the
function.

### Bindings

The `BINDINGS` list may be one of these form:

* `NIL` or `()`: the function takes no argument
* `SYMBOL`: the unevaluated list of arguments is associated with `SYMBOL`
* `LIST`: arguments are evaluated and assigned to the symbols in `LIST`

### Lambdas

Lambda functions are defined using the `\` keyword. Invocation of `\` is
similar to `def`:
```lisp
: ((\ (x y) (+ x y)) 1 1)
> 2
: ((\ (x) (map (\ (n) (+ n 1)) x)) '(1 2 3 4))
> (2 3 4 5)
```
The closure available at the time a lambda is defined is placed in its
signature. This is the lambda's _define-site closure_. When the lambda is called,
its closure is extended with the environment available where the call is
performed. This is the lambda's _call-site closure_.

### Currying

Minima.l supports currying. It uses the function's closure to store the curried
arguments. Curryring is available to all `lisp` and native functions. For example:
```lisp
: (def add (a b) (+ a b))
> ((a b) NIL (+ a b))
: (setq +1 (add 1))
> ((b) ((a . 1)) (+ a b))
: (+1 2)
> 3
```
### Recursion

A function defined using `def` can be recursive. When functions are defined,
symbols with their names are not resolved in their closure and are resolved
dynamically in the symbol domain instead.

A lambda defined using `let` can also be recursive. Similarly as above, the
recursive calls are resolved dynamically. However, the resolution process
first looks into the _define-site closure_ of the lambda, then its _call-site
closure_, and finally in the symbol domain.

There is a caveat however: since the function symbols are resolved dynamically,
redefining these symbols will lead to undefined behavior. For instance, the
following is a valid recursive lambda and returns `0`:
```lisp
(let ((fn . (\ (A) (if (= A 0) 0 (fn (- A 1))))))
  (fn 10))
```
However, the following is not and returns `10`:
```lisp
(let ((fn . (\ (A) (+ A 1))))
  (let ((fn . (\ (A) (unless (= A 0) (fn (- A 1))))))
    (fn 10)))
```
### Tail-call optimization

When functions are defined with `def` or `\` in `let`, the function's body is
scanned for potential tail calls. During evaluation, when a tail call is
encountered, its argument are evaluated at its call-site and returned to the
parent. These arguments are then used at the root of the function in a new
application of that function, executed in a tight loop.

### Value deconstruction

Assignation of arguments in `def`, `lamda`, or `let` functions support
deconstruction. For instance, with `def`:
```lisp
: (def sum3 ((a b c)) (+ (+ a b) c))
> sum3
: (sum3 (list 1 2 3))
> 6
```
Or with a lambda:
```lisp
: (setq data '(("hello" . 1) ("world" . 2)))
> (("hello" . 1) ("world" . 2))
: (foldl (\ (acc (_ . v))(+ acc v)) 0 data)
> 3
```
## Global variables

### ARGV

When `mnml` is executed as a `#!` interpreter, the `ARGV` global contains the
argument vector  of the script.

### CONFIG

The `CONFIG` global contains the current runtime of the interpreter. It contains
the installation `PREFIX`, the interpreter `VERSION`, its build timestamp
`BUILD_TS`, and the name `COMPNAME` and version `COMPVER` of the compiler used to
build it.

### ENV

The `ENV` global contains the environment at the time of the invocation of the
interpreter.


