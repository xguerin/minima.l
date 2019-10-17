# minima.l

## License

ISC. See `LICENSE.md`.

## Example
```minimal
(def fib (N)
  (if (<= N 1)
    N
    (+ (fib (- N 1)) (fib (- N 2)))))

(prinl "Result: " (fib 30))
```
## Language

### Source encoding

The source encoding is UTF-8.

### Comment
```minimal
# This is a comment
```
### Types

#### Table

| Name      | Description                                           |
|:----------|:------------------------------------------------------|
| List      | `( ... )`                                               |
| Number    | Positive and negative 64-bit integers                 |
| Symbol    | 16-character string                                   |
| Character | A `'`-delimited printable character                     |
| `T`         | Stands for `true`                                       |
| `NIL`       | The empty list, also stands for `false`                 |
| `_`         | Wildcard, used as a placeholder during deconstruction |

#### Symbols

Symbols are bound to values. Undefined symbols are unbound and resolve to `NIL`.
Symbols can be created or altered globally using the `def` and `setq` functions.
Symbols can also be defined locally using the `let` function.

#### Strings

The grammar supports the _string_ type. Represented as a `"`-delimited string of
characters, it is internally stored as a list of characters. For example:
```minimal
: "hello"
> ('h' 'e' 'l' 'l' 'o')
```
### Expression evaluation

#### Generic rules

* Numbers, strings, `NIL`, `T`, and `_` evaluate to themselves
* Symbols are dereferenced to their bound values
* Lists are evaluated as expressions

#### Expressions

Expression take the following form: `(FUNCTION ARGS...)`. The `FUNCTION` can be:

1. A symbol that must resolve to a function definition
2. A quoted function definition
3. A lambda definition
4. An integer

The forms 1, 2, and 3 are mutually interchangeable:
```minimal
: (def add (a b) (+ a b))
> ((a b) NIL (+ a b))
: (add 1 2) # Form 1
> 3
: ('((a b) NIL (+ a b)) 1 2) # Form 2
> 3
: ((\ (a b) (+ a b)) 1 2) # Form 3
> 3
```
The last form is used by the native function mechanism to call into dynamically
loaded plugins. The integer is casted as a function pointer and invoked by the
interpreter.

### Functions

Functions are represented as the following 3-uple: `(ARGUMENTS CLOSURE BODY)`.
The `ARGUMENTS` element is a list of symbols representing the arguments of the
functions. The `CLOSURE` element is an association list that contains the
context of the function at the definition site. It is also used for currying.
Lastly, the `BODY` element is the expression of the function.

#### Lambdas

Lambda functions are defined using the `\\` keyword. Invocation of `\\` is
similar to `def`:
```minimal
: ((\ (x y) (+ x y)) 1 1)
> 2

: ((\ (x) (map (\ (n) (+ n 1)) x)) '(1 2 3 4))
> (2 3 4 5)
```
#### Currying

Function can be curried. In the example below, the use of the closure for
currying is obvious:
```minimal
: (def add (a b) (+ a b))
> ((a b) NIL (+ a b))
: (setq +1 (add 1))
> ((b) ((a . 1)) (+ a b))
: (+1 2)
> 3
```
Curryring is available for all user-defined functions as well as for some
internal functions such as `+`, `-`, `and`, `cons`, and so on.

#### Native functions

Native function (machine code functions) are supported through the _plugin_
mechanism. A _plugin_ is a shared library that must export the following
interface:
```c
const char * lisp_plugin_name();
atom_t lisp_plugin_register();
```
The `lisp_plugin_register` function returns an `atom_t` value that contains an
integer representing a pointer to the following signature:
```c
atom_t lisp_function(const atom_t closure, const atom_t cell);
```
### Recursion

Function defined using `def` can be recursive. When functions are defined,
symbols with their name are not resolved in their closure and are resolved in
the symbol domain instead. However, there is a caveat: since the function
symbols are resolved dynamically, redefining these symbols will lead to
undefined behavior.

### Argument assignation

Assignation of arguments in `def`, `lamda`, or `let` functions support
deconstruction. For instance, with `def`:
```minimal
: (def sum3 ((a b c)) (+ (+ a b) c))
> sum3
: (sum3 (list 1 2 3))
> 6
```
Or with a lambda:
```minimal
: (setq data '(("hello" . 1) ("world" . 2)))
> (("hello" . 1) ("world" . 2))
: (foldl (\ (acc (_ . v))(+ acc v)) 0 data)
> 3
```
## Globals

### ARGV

When `mnml` is executed as a `#!` interpreter, the `ARGV` global contains the
argument vector  of the script.

### ENV

The `ENV` global contains the environment at the time of the invocation of the
interpreter.

## Functions

> Elements in `[]` are optional and a quoted symbol is evaluated.

### Summary

| Name | Syntax | Description |
|:-----|:-------|:------------|
| *Structural comparison*                   |||
| `=`         | `(=  'any 'any)`              | Equality |
| `<>`        | `(<> 'any 'any)`              | Inequality |
| *Numeric comparison*                      |||
| `<`         | `(<  'num 'num)`              | Less-than |
| `<=`        | `(<= 'num 'num)`              | Less-than-or-equal-to |
| `>`         | `(>  'num 'num)`              | Greater-than |
| `>`         | `(>= 'num 'num)`              | Greater-than-or-equal-to |
| *Arithmetic operations*                   |||
| `+`         | `(+ 'num 'num)`               | Addition |
| `-`         | `(- 'num 'num)`               | Subtraction |
| `*`         | `(* 'num 'num)`               | Multiplication |
| `/`         | `(/ 'num 'num)`               | Division |
| `%`         | `(% 'num 'num)`               | Modulo |
| *Logical operation*                       |||
| `and`       | `(and 'any 'any)`             | Logical AND |
| `not`       | `(not 'any)`                  | Logical NOT |
| `or`        | `(or 'any 'any)`              | Logical OR |
| *Predicates*                              |||
| `chr?`      | `(chr? 'any)`                 | Check if `any` is a character |
| `lst?`      | `(lst? 'any)`                 | Check if `any` is a list |
| `num?`      | `(num? 'any)`                 | Check if `any` is a number |
| `str?`      | `(str? 'any)`                 | Check if `any` is a string |
| `sym?`      | `(sym? 'any)`                 | Check if `any` is a symbol |
| `sym?`      | `(sym? 'any)`                 | Check if `any` is a symbol |
| *String operations*                       |||
| `str`       | `(str 'any)`                  | Make a string out of a symbol |
| *Symbol and function definition*          |||
| `<-`        | `(<- sym 'any)`               | [Set](#set) an existing symbol |
| `def`       | `(def sym args [str] prg)`    | [Define](#def) a function |
| `let`       | `(let lst . prg)`             | [Let](#let)-binding symbols |
| `setq`      | `(setq sym 'any)`             | [Bind](#setq) a symbols |
| `sym`       | `(sym 'any)`                  | Make a symbol out of a string |
| *Expression composition*                  |||
| `|>`        | `(|> any0 [any1] ...)`        | [Fluent](#stream) composition |
| `prog`      | `(prog any0 [any1] ...)`      | [Sequential](#prog) composition |
| *List manipulation*                       |||
| `append`    | `(append 'lst 'any)`          | Append `any` to `lst` |
| `assoc`     | `(assoc 'any 'lst)`           | [Query](#assoc) an association list |
| `car`       | `(car 'lst)`                  | Get the first element of `lst` |
| `cadr`      | `(cdr 'lst)`                  | Get the second element of `lst` |
| `caddr`     | `(caddr 'lst)`                | Get the third element of `lst` |
| `caar`      | `(caar 'lst)`                 | Get the first element of the head of `lst` |
| `cadar`     | `(cadar 'lst)`                | Get the second element of the head of `lst` |
| `cdar`      | `(cdar 'lst)`                 | Get the tail of the head of `lst` |
| `cdr`       | `(cdr 'lst)`                  | Get the tail of `lst` |
| `chr`       | `(chr 'num)`                  | Get the character for ASCII numner `num` |
| `conc`      | `(conc 'lst 'lst)`            | [Concatenate](#conc) two lists into one |
| `cond`      | `(cond 'any ...)`             | [Predicate](#cond) matching |
| `cons`      | `(cons 'any 'any)`            | [Construct](#cons) a pair |
| `list`      | `(list 'any ...)`             | [Create](#list) a list with `any` |
| `match`     | `(match 'any ...)`            | [Structural](#match) matching |
| *Control flow*                            |||
| `if`        | `(if 'any then [else])`       | [If](#if) construct |
| *Input/output operations*                 |||
| `in`        | `(in 'any . prg)`             | [In](#if) stream |
| `out`       | `(out 'any . prg)`            | [Out](#if) stream |
| `prin`      | `(prin 'any ...)`             | [Symbolic print](#prin) of a list of `any` |
| `prinl`     | `(prinl 'any ...)`            | [Symbolic print](#prinl) of a list of `any`, with new line |
| `print`     | `(print 'any ...)`            | [Literal print](#print) of a list of `any` |
| `printl`    | `(printl 'any ...)`           | [Literal print](#print) of a list of `any`, with new line |
| `read`      | `(read)`                      | Read a token from the current input stream |
| `readlines` | `(readlines)`                 | Read all lines from the current input stream |
| *Miscellaneous operations*                |||
| `eval`      | `(eval 'any)`                 | [Evaluate](#eval) `any` |
| `load`      | `(load str)`                  | [Load](#load) an external asset |
| `time`      | `(time prg)`                  | Time the execution of `prg` |
| `quit`      | `(quit)`                      | Quit the interpreter loop |
| `quote`     | `(quote . any)`               | Quote `any` |

### Description

#### ASSOC
```minimal
(assoc 'any 'lst)
```
Return the value for `any` in the association list `lst`. Return `NIL` if the
symbol is not present.
```minimal
: (assoc 'hello '((hello . world)))
> world
: (assoc 'foo '((hello . world)))
> NIL
```
#### CONC
```minimal
(conc 'lst1 'lst2)
```
Destructively concatenate two lists into one.
```minimal
: (setq A '(1 2))
> (1 2)
: (conc A '(3 4))
> (1 2 3 4)
: A
> (1 2 3 4)
```
#### COND
```minimal
(cond 'any (any . prg) (any . prg) ...)
```
Evaluate `any` and use the `car` of the remaining arguments as a predicate over
the result. Return the evaluation of the first positive match. The _default_ or
_catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.
```minimal
: (def test (v) (cond v (num? . 'number) (lst? . 'list) (_ . 'unknown)))
> test
: (test 1)
> number
: (test '(1 2))
> list
: (test T)
> unknown
```
#### CONS
```minimal
(cons 'any1 'any2)
```
Construct a new list cell using the first argument for `car` and the second
argument for `cdr`.
```minimal
: (cons 1 2)
> (1 . 2)
: (cons 1 (cons 2 3))
> (1 2 . 3)
```
#### DEF
```minimal
(def sym args [str] prg ...)
```
Define a function with arguments `args` and body `prg` and associate it with
the symbol `sym`. An optional `str` can be specified as a documentation string
and is ignored by the interpreter.
```minimal
: (def add (x y) (+ x y))
> add
```
Function defined with the `def` keyword are simply lambda functions assigned to
symbol. The following expressions are equivalent:
```minimal
: (def add (a b) (+ a b))
> add
: (setq add (\ (a b) (+ a b)))
> (\ (a b) (+ a b))
```
#### EVAL
```minimal
(eval 'any)
```
Evaluate `any`.
```minimal
: (eval '(+ 1 1))
> 2
```
#### IF
```minimal
(if 'any then [else])
```
When `any` evaluates to `T`, return the evaluation of `then`. Return the
evaluation of `then` otherwise. Return `NIL` if `then` is not specified.
```minimal
: (def test (v) (if (> v 10) (* v 2)))
> test
: (test 5)
> NIL 
: (test 20)
> 40
```
#### IN
```minimal
(in 'any . prg)
```
Create a new input channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdin`. When the
argument evaluates to a string, `in` assumes the string contains a file path and
tries to open that file.

#### LET
```minimal
(let lst . prg)
```
Evaluate `prg` within the context of the bind list `lst`. The bind list has the
following format:
```minimal
((any . 'any)(any . 'any)...)
```
For each element in the bind list, the `cdr` is evaluated and bound to its `car`
using the argument assignation process described above.
```minimal
: (let ((a . 1)(b . 2)) (printl a b))
1 2
> 2
```
#### LIST
```minimal
(list 'any ...)
```
Create a list with `any` arguments.
```minimal
: (list)
> (NIL)
: (list (+ 1 1) 3 "a")
> (2 3 "a")
```
#### LOAD
```minimal
(load str)
(load 'sym [str])
```
In the first form, load the `minimal` file pointed by `str`. On success, `load`
returns the result of the last evaluated operation in the file. Otherwise, `NIL`
is returned.
```minimal
: (load "lib/lisp/cadr.l")
> ((x) NIL (car (cdr x)))
```
If the path is prefixed by `@lib`, `load` will look for the file in the library
directory of the installation prefix.
```minimal
: (load "@lib/cadr.l")
> ((x) NIL (car (cdr x)))
```
In the second form, load the `minimal` symbol from plugins either from the
installation prefix, the `MNML_PLUGIN_PATH` environment variable, or the
optional `str` path.
```minimal
: (load '+)
> 4425116848
```
#### MATCH
```minimal
(match 'any (any . prg) ...)
```
Evaluate `any` and use the `car` of the remaining arguments as a structural
template for the result. Return the evaluation of the first positive match. The
_default_ or _catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.

#### OUT
```minimal
(out 'any . prg)
```
Create a new output channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdout`. When the
argument evaluates to a string, `out` assumes the string contains a file path
and tries to open that file.

If the file does not exist, it is created. If the file exists, it is truncated.
If the file path is prepended with a `+` the file must exist and data will be
appended to it.

#### PRIN
```minimal
(prin 'any ...)
```
Print the string representation of `any`. When multiple arguments are printed,
no separator is used. The last argument is returned after evaluation.
```minimal
: (prin "hello, " "world!")
hello, world!> "world!"
```
#### PRINL
```minimal
(prinl 'any ...)
```
Calls `prin` and appends a new line.
```minimal
: (prinl "hello, " "world!")
hello, world!
> "world!"
```
##### PRINT
```minimal
(print 'any ...)
```
Print the lisp representation of `any`. When multiple arguments are printed, a
space separator is used. The last argument is returned after evaluation.
```minimal
: (print 'a 'b '(1 2 3))
a b (1 2 3)> (1 2 3)
```
##### PRINTL
```minimal
(printl 'any ...)
```
Calls `print` and appends a new line.
```minimal
: (print 'a 'b (1 2 3) +)
a b (1 2 3)
> (1 2 3)
```
#### PROG
```minimal
(prog prg1 prg2 ...)
```
Evaluate `prg1`, `prg2`, ..., in sequence and return the last evaluation.
```minimal
: (prog (+ 1 1) (+ 2 2))
> 4
```
#### SET
```minimal
(<- 'sym 'any)
```
Set an existing symbol `sym` to `any`.
```minimal
: (<- A (+ 1 2))
> NIL 
: (setq A (+ 1 2))
> 3
: (<- A 4)
> 4
: A
> 4
```
#### SETQ
```minimal
(setq sym 'any)
```
Associate `any` with the symbol `sym`.
```minimal
: (setq A (+ 1 2))
> 3
```
#### STREAM
```minimal
(|> any0 [any1] ...)
```
Fluent composition operator. Evaluate `any0` and pass the result to `any1`, and
so on until the end of the list.
```minimal
: (|> '(1 2 3) cdr car)
> 2
```
