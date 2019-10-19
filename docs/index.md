# Minima.l

Minima.l is an opinionated LISP heavily inspired by [Picolisp](http://picolisp.com). It is distributed
under the ISC license.

## Interpreter

The intepreter is called `mnml`. It can be run interactively:
```
$ mnml
>
```
It accepts a file name as a parameter:
```
$ mnml file.l
```
It can be used as a _shebang_ interpreter:
```
#!/usr/bin/env mnml

(+ 1 2)
```
By default, the interpreter preload most common [plugins](#native-functions) out of the box. What
is loaded can be altered by the `MNML_PRELOAD` variable.

### Environment variables

#### MNML_DEBUG

If Minima.l has been compiled with debug support, this variable controls which
class of debug output to generate:

* `CONS`: list construction operations
* `MAKE`: atom creation operations
`* PLUGIN`: plugin operations
* `RC`: reference counting operations
* `SLOT`: slot allocator operations
* `SLAB`: slab allocator operations

The variable can be left empty to restrict debug output to evaluation steps.

#### MNML_PRELOAD

This variable controls which plugin is preloaded by the interpreter. Although it
can be used empty, you may want to at least preload `load`.

#### MNML_PLUGIN_PATH

This variable controls where to look for plugins, the default being the
installation prefix.

## Language

### Example
```lisp
(def fib (N)
  (if (<= N 1)
    N
    (+ (fib (- N 1)) (fib (- N 2)))))

(prinl "Result: " (fib 30))
```
### Source encoding

The source encoding is UTF-8.

### Comment
```lisp
# This is a comment
```
### Types

#### Table

| Name      | Description                                           |
|:----------|:------------------------------------------------------|
| List      | `( ... )`                                               |
| Number    | Positive and negative 64-bit integers                 |
| Symbol    | 16-character string                                   |
| Character | A `^`-prefixed printable character                      |
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
```lisp
: "hello"
> (^h ^e ^l ^l ^o)
```
### Expression evaluation

#### Generic rules

* Numbers, strings, `NIL`, `T`, and `_` evaluate to themselves
* Symbols are dereferenced to their bound values
* Lists are evaluated as expressions

#### Expressions

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
### Functions

Functions are represented internally as the following 3-uple:
```lisp
(ARGUMENTS CLOSURE . BODY)
```
The `ARGUMENTS` element is a list of symbols representing the arguments of the
functions. The `CLOSURE` element is an association list that contains the
context of the function at the definition site. It is also used for currying.
Lastly, the `BODY` element is the expression of the function. When defined
through plugins, the body may also be a number representing the memory address
of the native implementation of the function.

#### Lambdas

Lambda functions are defined using the `\\` keyword. Invocation of `\\` is
similar to `def`:
```lisp
: ((\ (x y) (+ x y)) 1 1)
> 2

: ((\ (x) (map (\ (n) (+ n 1)) x)) '(1 2 3 4))
> (2 3 4 5)
```
#### Currying

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
#### Native functions

Native function (machine code functions) are supported through the _plugin_
mechanism. A _plugin_ is a shared library that must export the following
interface:
```c
const char * lisp_plugin_name();
atom_t lisp_plugin_register();
```
The `lisp_plugin_register` function returns an `atom_t` value that points to a
list definition as follows:
```lisp
(AGUMENTS NIL . INTEGER)
```
The integer part represents a pointer to a function with the following
signature:
```c
atom_t lisp_function_NAME(const atom_t closure);
```
#### Writing a plugin

The macro below is used to register a new plugin:
```c
#define LISP_PLUGIN_REGISTER(__s, __n, ...)
```
The first argument `__s` is the suffix of the function. The second argument
`__n` is the name of the symbol. The variadic arguments are intended to be
for the function argument symbols. For instance, the function `add` is
defined as follow:
```c
static atom_t
lisp_function_add(const atom_t closure)
{
  /* ... */
}
LISP_PLUGIN_REGISTER(add, add, X, Y, NIL)
```
The argument symbols can be defined as follows:

| C version                                 | Minimal equivalent      |
|:------------------------------------------|:------------------------|
| `LISP_PLUGIN_REGISTER(fun, fun)`            | `(def fun () ...)`        |
| `LISP_PLUGIN_REGISTER(fun, fun, @)`         | `(def fun @ ...)`         |
| `LISP_PLUGIN_REGISTER(fun, fun, A)`         | `(def fun (A) ...)`       |
| `LISP_PLUGIN_REGISTER(fun, fun, A, NIL)`    | `(def fun (A) ...)`       |
| `LISP_PLUGIN_REGISTER(fun, fun, A, B)`      | `(def fun (A . B) ...)`   |
| `LISP_PLUGIN_REGISTER(fun, fun, A, B, NIL)` | `(def fun (A B) ...)`     |
| `LISP_PLUGIN_REGISTER(fun, fun, A, B, C)`   | `(def fun (A B . C) ...)` |

Values for the declared symbols are passed to the plugin by the interpreter
through its closure. They can be retrieved using the following macro:
```c
#define LISP_LOOKUP(_v, _c, _x)
```
The first argument is a name to use for the variable to be assigned the value.
The second argument is the name of the closure. The last argument is the symbol
to look up.

#### Recursion

A function defined using `def` can be recursive. When functions are defined,
symbols with their name are not resolved in their closure and are resolved
dynamically in the symbol domain instead. There is a caveat however: since the
function symbols are resolved dynamically, redefining these symbols will lead to
undefined behavior.

#### Argument assignation

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
## Globals

### ARGV

When `mnml` is executed as a `#!` interpreter, the `ARGV` global contains the
argument vector  of the script.

### ENV

The `ENV` global contains the environment at the time of the invocation of the
interpreter.

## Functions

### Notation rules

1. Elements in `[]` are optional
2. A quoted symbol means that it is evaluated

### Summary

| Name      | Syntax                      | Plugin | Description |
|:----------|:----------------------------|:------:|:------------|
| **Structural comparison**                            ||||
| `=`         | `(=  'any 'any)`              | ✓      | Equality |
| `<>`        | `(<> 'any 'any)`              | ✓      | Inequality |
| **Numeric comparison**                      |        |||
| `<`         | `(<  'num 'num)`              | ✓      | Less-than |
| `<=`        | `(<= 'num 'num)`              | ✓      | Less-than-or-equal-to |
| `>`         | `(>  'num 'num)`              | ✓      | Greater-than |
| `>`         | `(>= 'num 'num)`              | ✓      | Greater-than-or-equal-to |
| **Arithmetic operations**                            ||||
| `+`         | `(+ 'num 'num)`               | ✓      | Addition |
| `-`         | `(- 'num 'num)`               | ✓      | Subtraction |
| `*`         | `(* 'num 'num)`               | ✓      | Multiplication |
| `/`         | `(/ 'num 'num)`               | ✓      | Division |
| `%`         | `(% 'num 'num)`               | ✓      | Modulo |
| **Logical operation**                                ||||
| `and`       | `(and 'any 'any)`             | ✓      | Logical AND |
| `not`       | `(not 'any)`                  | ✓      | Logical NOT |
| `or`        | `(or 'any 'any)`              | ✓      | Logical OR |
| **Predicates**                                       ||||
| `chr?`      | `(chr? 'any)`                 | ✓      | Return `T` if `any` is a character |
| `lst?`      | `(lst? 'any)`                 | ✓      | Return `T` if `any` is a list |
| `nil?`      | `(nil? 'any)`                 | ✓      | Return `T` if `any` is `NIL` |
| `num?`      | `(num? 'any)`                 | ✓      | Return `T` if `any` is a number |
| `str?`      | `(str? 'any)`                 | ✓      | Return `T` if `any` is a string |
| `sym?`      | `(sym? 'any)`                 | ✓      | Return `T` if `any` is a symbol |
| `tru?`      | `(tru? 'any)`                 | ✓      | Return `T` if `any` is `T` |
| **String operations**                                ||||
| `ntoa`      | `(ntoa 'num)`                 |        | Convert `num` into a string |
| `split`     | `(split 'str 'chr)`           |        | Split `str` of `chr`-separted tokens |
| `str`       | `(str 'sym)`                  | ✓      | Make a string out of `sym` |
| **Symbol definition**                                ||||
| `<-`        | `(<- sym 'any)`               | ✓      | [Set](#set) `sym` to `any` |
| `def`       | `(def sym args [str] prg)`    | ✓      | [Define](#def) a function |
| `let`       | `(let lst . prg)`             | ✓      | [Let](#let)-binding symbols |
| `setq`      | `(setq sym 'any)`             | ✓      | [Bind](#setq) a symbols |
| `sym`       | `(sym 'str)`                  | ✓      | Make a symbol out of `str` |
| **List manipulation**                                ||||
| `append`    | `(append 'lst 'any)`          |        | Append `any` to `lst` |
| `car`       | `(car 'lst)`                  | ✓      | Get the head element of `lst` |
| `cadr`      | `(cdr 'lst)`                  |        | Get the 2nd element of `lst` |
| `caddr`     | `(caddr 'lst)`                |        | Get the 3rd element of `lst` |
| `caar`      | `(caar 'lst)`                 |        | Get the 1st element of the head of `lst` |
| `cadar`     | `(cadar 'lst)`                |        | Get the 2nd element of the head of `lst` |
| `cdar`      | `(cdar 'lst)`                 |        | Get the tail of the head of `lst` |
| `cdr`       | `(cdr 'lst)`                  | ✓      | Get the tail of `lst` |
| `chr`       | `(chr 'num)`                  | ✓      | Get the character for ASCII numner `num` |
| `conc`      | `(conc 'lst 'lst)`            | ✓      | [Concatenate](#conc) two lists into one |
| `cons`      | `(cons 'any 'any)`            | ✓      | [Construct](#cons) a pair |
| `filter`    | `(filter 'fun 'lst)`          |        | Filter `lst` using `fun` |
| `flatten`   | `(flatten 'lst)`              |        | Flatten a nested `lst` |
| `foldl`     | `(foldl 'fun 'acc 'lst)`      |        | Left-fold a `lst` |
| `foldr`     | `(foldr 'fun 'lst 'acc)`      |        | Right-fold a `lst` |
| `insert`    | `(insert 'fun 'any 'lst)`     |        | Insert `any` into a sorted `lst` using `fun` |
| `iter`      | `(iter 'fun 'lst)`            |        | Iterate over the elements of a list |
| `last`      | `(last 'lst)`                 |        | Return the last element of a list |
| `len`       | `(len 'lst)`                  |        | Compute the length `lst` |
| `list`      | `(list 'any ...)`             | ✓      | [Create](#list) a list with `any` |
| `map`       | `(map 'fun 'lst)`             |        | Map the content of `lst` |
| `map2`      | `(map2 'fun 'lst 'lst)`       |        | Map the content of a two lists |
| `merge`     | `(merge 'fun 'fun 'lst 'lst)` |        | Sorted and deduped merge of two lists |
| `rev`       | `(rev 'lst)`                  |        | Reverse `lst` |
| `zip`       | `(zip 'lst 'lst)`             |        | Sequentially pair-up elements from two lists |
| **Assoc-list operations**                            ||||
| `assoc`     | `(assoc 'any 'lst)`           |        | [Query](#assoc) an association list |
| `erase`     | `(erase 'any 'lst)`           |        | Remove an entry in an association list |
| `replc`     | `(replc 'any 'any 'lst)`      |        | Replace an entry in an association list |
| **Control flow**                            |        |||
| `|>`        | `(|> any0 [any1] ...)`        | ✓      | [Fluent](#stream) composition |
| `cond`      | `(cond 'any ...)`             | ✓      | [Predicate](#cond) matching |
| `if`        | `(if 'any then [else])`       | ✓      | [If](#if) construct |
| `match`     | `(match 'any ...)`            | ✓      | [Structural](#match) matching |
| `prog`      | `(prog any0 [any1] ...)`      | ✓      | [Sequential](#prog) composition |
| **Input/output operations**                          ||||
| `in`        | `(in 'any . prg)`             | ✓      | [In](#if) stream |
| `out`       | `(out 'any . prg)`            | ✓      | [Out](#if) stream |
| `prin`      | `(prin 'any ...)`             | ✓      | [Symbolic print](#prin) of a list of `any` |
| `prinl`     | `(prinl 'any ...)`            | ✓      | [Symbolic print](#prinl) of a list of `any`, with new line |
| `print`     | `(print 'any ...)`            | ✓      | [Literal print](#print) of a list of `any` |
| `printl`    | `(printl 'any ...)`           | ✓      | [Literal print](#print) of a list of `any`, with new line |
| `read`      | `(read)`                      | ✓      | Read a token from the current input stream |
| `readlines` | `(readlines)`                 | ✓      | Read all lines from the current input stream |
| **Core operations**                |        |||
| `eval`      | `(eval 'any)`                 | ✓      | [Evaluate](#eval) `any` |
| `load`      | `(load str)`                  | ✓      | [Load](#load) an external asset |
| `time`      | `(time prg)`                  | ✓      | Time the execution of `prg` |
| `quit`      | `(quit)`                      | ✓      | Quit the interpreter loop |
| `quote`     | `(quote . any)`               | ✓      | Quote `any` |
| **System functions**                                 ||||
| `close`     | `(dup 'num)`                  | ✓      | Close afile descriptor `num` |
| `dup`       | `(dup 'num ['num])`           | ✓      | Duplicate a file descriptor `num` |
| `exec`      | `(exec 'str 'lst 'lst)`       | ✓      | Execute an image at path with arguments and environment |
| `fork`      | `(fork)`                      | ✓      | Fork the current process |
| `wait`      | `(wait 'num)`                 | ✓      | Wait for PID `num` |

### Description

#### ASSOC
```lisp
(assoc 'any 'lst)
```
Return the value for `any` in the association list `lst`. Return `NIL` if the
symbol is not present.
```lisp
: (assoc 'hello '((hello . world)))
> world
: (assoc 'foo '((hello . world)))
> NIL
```
#### CONC
```lisp
(conc 'lst1 'lst2)
```
Destructively concatenate two lists into one.
```lisp
: (setq A '(1 2))
> (1 2)
: (conc A '(3 4))
> (1 2 3 4)
: A
> (1 2 3 4)
```
#### COND
```lisp
(cond 'any (any . prg) (any . prg) ...)
```
Evaluate `any` and use the `car` of the remaining arguments as a predicate over
the result. Return the evaluation of the first positive match. The _default_ or
_catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.
```lisp
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
```lisp
(cons 'any1 'any2)
```
Construct a new list cell using the first argument for `car` and the second
argument for `cdr`.
```lisp
: (cons 1 2)
> (1 . 2)
: (cons 1 (cons 2 3))
> (1 2 . 3)
```
#### DEF
```lisp
(def sym args [str] prg ...)
```
Define a function with arguments `args` and body `prg` and associate it with
the symbol `sym`. An optional `str` can be specified as a documentation string
and is ignored by the interpreter.
```lisp
: (def add (x y) (+ x y))
> add
```
Function defined with the `def` keyword are simply lambda functions assigned to
symbol. The following expressions are equivalent:
```lisp
: (def add (a b) (+ a b))
> add
: (setq add (\ (a b) (+ a b)))
> (\ (a b) (+ a b))
```
#### EVAL
```lisp
(eval 'any)
```
Evaluate `any`.
```lisp
: (eval '(+ 1 1))
> 2
```
#### IF
```lisp
(if 'any then [else])
```
When `any` evaluates to `T`, return the evaluation of `then`. Return the
evaluation of `then` otherwise. Return `NIL` if `then` is not specified.
```lisp
: (def test (v) (if (> v 10) (* v 2)))
> test
: (test 5)
> NIL 
: (test 20)
> 40
```
#### IN
```lisp
(in 'any . prg)
```
Create a new input channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdin`. When the
argument evaluates to a string, `in` assumes the string contains a file path and
tries to open that file.

#### LET
```lisp
(let lst . prg)
```
Evaluate `prg` within the context of the bind list `lst`. The bind list has the
following format:
```lisp
((any . 'any)(any . 'any)...)
```
For each element in the bind list, the `cdr` is evaluated and bound to its `car`
using the argument assignation process described above.
```lisp
: (let ((a . 1)(b . 2)) (printl a b))
1 2
> 2
```
#### LIST
```lisp
(list 'any ...)
```
Create a list with `any` arguments.
```lisp
: (list)
> (NIL)
: (list (+ 1 1) 3 "a")
> (2 3 "a")
```
#### LOAD
```lisp
(load 'str 'sym ...)
```
Load the `lisp` file pointed by `str` or the lisp symbol pointed by 'sym.  On
success, `load` returns the result of the last evaluated operation in the list.
Otherwise, `NIL` is returned.
```lisp
: (load "lib/lisp/cadr.l")
> ((x) NIL (car (cdr x)))
```
If the path is prefixed by `@lib`, `load` will look for the file in the library
directory of the installation prefix.
```lisp
: (load "@lib/cadr.l")
> ((x) NIL (car (cdr x)))
```
Symbols are loaded from plugins found in the installation prefix or in
the `MNML_PLUGIN_PATH` environment variable.
```lisp
: (load '+)
> 4425116848
```
#### MATCH
```lisp
(match 'any (any . prg) ...)
```
Evaluate `any` and use the `car` of the remaining arguments as a structural
template for the result. Return the evaluation of the first positive match. The
_default_ or _catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.

#### OUT
```lisp
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
```lisp
(prin 'any ...)
```
Print the string representation of `any`. When multiple arguments are printed,
no separator is used. The last argument is returned after evaluation.
```lisp
: (prin "hello, " "world!")
hello, world!> "world!"
```
#### PRINL
```lisp
(prinl 'any ...)
```
Calls `prin` and appends a new line.
```lisp
: (prinl "hello, " "world!")
hello, world!
> "world!"
```
##### PRINT
```lisp
(print 'any ...)
```
Print the lisp representation of `any`. When multiple arguments are printed, a
space separator is used. The last argument is returned after evaluation.
```lisp
: (print 'a 'b '(1 2 3))
a b (1 2 3)> (1 2 3)
```
##### PRINTL
```lisp
(printl 'any ...)
```
Calls `print` and appends a new line.
```lisp
: (print 'a 'b (1 2 3) +)
a b (1 2 3)
> (1 2 3)
```
#### PROG
```lisp
(prog prg1 prg2 ...)
```
Evaluate `prg1`, `prg2`, ..., in sequence and return the last evaluation.
```lisp
: (prog (+ 1 1) (+ 2 2))
> 4
```
#### SET
```lisp
(<- 'sym 'any)
```
Set an existing symbol `sym` to `any`.
```lisp
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
```lisp
(setq sym 'any)
```
Associate `any` with the symbol `sym`.
```lisp
: (setq A (+ 1 2))
> 3
```
#### STREAM
```lisp
(|> any0 [any1] ...)
```
Fluent composition operator. Evaluate `any0` and pass the result to `any1`, and
so on until the end of the list.
```lisp
: (|> '(1 2 3) cdr car)
> 2
```
