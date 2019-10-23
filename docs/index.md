# Minima.l

Opinionated LISP dialect that takes some of its inspirations from [Picolisp](https://picolisp.com),
[CHICKEN Scheme](http://call-cc.org), and other great languages. It is released under the ISC
license. The source code is located [here](https://git.sr.ht/~xguerin/minima.l).

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
> ((a b) NIL NIL (+ a b))
: (add 1 2) # Form 1
> 3
: ('((a b) NIL NIL (+ a b)) 1 2) # Form 2
> 3
: ((\ (a b) (+ a b)) 1 2) # Form 3
> 3
```
### Functions

Functions are represented internally as the following 3-uple:
```lisp
(ARGUMENTS CLOSURE BINDINGS . BODY)
```
The `ARGUMENTS` element is a list of symbols representing the arguments of the
functions. The `CLOSURE` element is an association list that contains the
context of the function at the definition site. The `BINDINGS` element is used
for currying. Lastly, the `BODY` element is the expression of the function. When
defined through plugins, the body may also be a number representing the memory
address of the native implementation of the function.

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
> ((a b) NIL NIL (+ a b))
: (setq +1 (add 1))
> ((b) NIL ((a . 1)) (+ a b))
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
### Global variables

#### ARGV

When `mnml` is executed as a `#!` interpreter, the `ARGV` global contains the
argument vector  of the script.

#### ENV

The `ENV` global contains the environment at the time of the invocation of the
interpreter.

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
class of debug output to generate. It accepts a comma-separated list of category
names:

* `BIND`: argument binding operations
* `CHAN`: I/O channel operations
* `CONS`: list construction operations
* `MAKE`: atom creation operations
`* PLUG`: plugin operations
* `REFC`: reference counting operations
* `SLOT`: slot allocator operations
* `SLAB`: slab allocator operations
```
$ MNML_DEBUG=PLUG,SLOT mnml
```
The variable can be left empty to restrict debug output to `eval`.

#### MNML_PRELOAD

This variable controls which plugin is preloaded by the interpreter. It accepts
a comman-separated list of symbols:
```
$ MNML_PRELOAD=load,quit mnml
```
Although it can be used empty, you may want to at least preload `load`. 

#### MNML_PLUGIN_PATH

This variable controls where to look for plugins, the default being the
installation prefix. It accepts a colon-separated list of paths:
```
$ MNML_PLUGIN_PATH=/some/path:/some/other/path mnml
```
## Function list

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
| `append`    | `(append 'lst . any)`         |        | Recursively append `any` to `lst` |
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
| `read`      | `(read)`                      | ✓      | [Read a token](#read) from the current input stream |
| `readlines` | `(readlines)`                 | ✓      | [Read all lines](#readlines) from the current input stream |
| **Core operations**                |        |||
| `eval`      | `(eval 'any)`                 | ✓      | [Evaluate](#eval) `any` |
| `load`      | `(load str)`                  | ✓      | [Load](#load) an external asset |
| `time`      | `(time prg)`                  | ✓      | [Time](#time) the execution of `prg` |
| `quit`      | `(quit)`                      | ✓      | Quit the interpreter loop |
| `quote`     | `(quote . any)`               | ✓      | Quote `any` |
| **Interpreter functions**                            ||||
| `prefix`    | `(prefix)`                    | ✓      | Return `mnml` installation prefix |
| **System functions**                                 ||||
| `close`     | `(dup 'num)`                  | ✓      | Close afile descriptor `num` |
| `dup`       | `(dup 'num ['num])`           | ✓      | Duplicate a file descriptor `num` |
| `exec`      | `(exec 'str 'lst 'lst)`       | ✓      | Execute an image at path with arguments and environment |
| `fork`      | `(fork)`                      | ✓      | Fork the current process |
| `run`       | `(run 'str 'lst 'alst)`       |        | [Run](#run) a external program `str` |
| `unlink`    | `(unlink 'str)`               |        | Unlink the file pointed by `str` |
| `wait`      | `(wait 'num)`                 | ✓      | Wait for PID `num` |

### List

#### ASSOC

##### Invocation
```lisp
(assoc 'any 'lst)
```
##### Description

Look-up `any` in the association list `lst`.

##### Return value

If a value is bound to `any`, return that value. If not, return `NIL`.

##### Example
```lisp
: (assoc 'hello '((hello . world)))
> world
: (assoc 'foo '((hello . world)))
> NIL
```
#### CONC

##### Invocation
```lisp
(conc 'lst1 'lst2)
```
##### Description

Destructively concatenate two lists `lst1` and `lst2` into a single list.

##### Return value

If `lst1` is a list, return the concatenation of `lst1` and `lst2`. The value
pointed by `lst1` is actually modified. If `lst1` is not a list, return `NIL`.

##### Example
```lisp
: (setq A '(1 2))
> (1 2)
: (conc A '(3 4))
> (1 2 3 4)
: A
> (1 2 3 4)
```
#### COND

##### Invocation
```lisp
(cond 'any (any . prg) (any . prg) ...)
```
##### Description

Evaluate `any` and use the `car` of the remaining arguments as a predicate over
the result. Return the evaluation of the first positive match. The _default_ or
_catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.

##### Return value

If a match is made, returns the evaluation of the corresponding `prg`. If no
match is made, return `NIL`.

##### Example
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

##### Invocation
```lisp
(cons 'any1 'any2)
```
##### Description

Construct a new list using the first argument for `car` and the second argument
for `cdr`.

##### Return value

Return the newly constructed list without modifying the arguments.

##### Example
```lisp
: (cons 1 2)
> (1 . 2)
: (cons 1 (cons 2 3))
> (1 2 . 3)
```
#### DEF

##### Invocation
```lisp
(def sym lst [str] prg ...)
```
##### Description

Define a function with arguments `args` and body `prg` and associate it with
the symbol `sym`. An optional `str` can be specified as a documentation string
and is ignored by the interpreter.

Function defined with the `def` keyword are simply lambda functions assigned to
symbol. The following expressions are equivalent:
```lisp
: (def add (a b) (+ a b))
> add
: (setq add (\ (a b) (+ a b)))
> (\ (a b) (+ a b))
```
##### Return value

Return the S-expression of the newly defined function.

##### Example
```lisp
: (def add (x y) (+ x y))
> ((x y) NIL NIL (+ x y))
```
#### EVAL

##### Invocation
```lisp
(eval 'any)
```
##### Description

Evaluate `any`.

##### Return value

Return the result of the evaluation.

##### Example
```lisp
: (eval (list '+ 1 1))
> 2
```
#### IF

##### Invocation
```lisp
(if 'any . lst)
```
##### Description

When `any` evaluates to `T`, evaluate `(car lst)`. Otherwise, evaluate
`(car (cdr lst))`.

##### Return value

Return the result of the evaluation.

##### Example
```lisp
: (def test (v) (if (> v 10) (* v 2)))
> test
: (test 5)
> NIL 
: (test 20)
> 40
```
#### IN

##### Invocation
```lisp
(in 'any . prg)
```
##### Description

Create a new input channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdin`. When the
argument evaluates to a string, `in` assumes the string contains a file path and
tries to open that file.

#### LET

##### Invocation
```lisp
(let lst . prg)
```
##### Description

Evaluate `prg` within the context of the bind list `lst`. The bind list has the
following format:
```lisp
((any . 'any)(any . 'any)...)
```
##### Return value

Return the value of the evaluated `prg`.

##### Example
```lisp
: (let ((lhv . (+ 1 2)) (rhv . (+ 3 4))) (+ lhv rhv))
> 10
```
##### Description

For each element in the bind list, the `cdr` is evaluated and bound to its `car`
using the argument assignation process described above.
```lisp
: (let ((a . 1)(b . 2)) (printl a b))
1 2
> 2
```
#### LIST

##### Invocation
```lisp
(list 'any ...)
```
##### Description

Create a list with `any` arguments.

##### Return value

Return the newly created list.

##### Example
```lisp
: (list)
> NIL
: (list (+ 1 1) 3 "a")
> (2 3 "a")
```
#### LOAD

##### Invocation
```lisp
(load 'str 'sym ...)
```
##### Description

Load the `lisp` file pointed by `str` or the lisp symbol pointed by 'sym. If the
path is prefixed by `@lib`, `load` will look for the file in the `lib`
directory of the installation prefix.
```lisp
: (load "@lib/cadr.l")
> ((x) NIL NIL (car (cdr x)))
```
Symbols are loaded from plugins found in the installation prefix or in
the `MNML_PLUGIN_PATH` environment variable.
```lisp
: (load '+)
> 4425116848
```
##### Return value

Return the result of the last evaluated operation in the list. On error,
`NIL` is returned.

##### Example
```lisp
: (load "lisp/cadr.l")
> ((x) NIL NIL (car (cdr x)))
```
#### MATCH

##### Invocation
```lisp
(match 'any (any . prg) ...)
```
##### Description

Evaluate `any` and use the `car` of the remaining arguments as a structural
template for the result. The
_default_ or _catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.

##### Return value

Return the evaluation of the first positive match. Return `NIL` otherwise.

##### Example
```lisp
: (prinl (match '(1 2 3) ((1 _ _) . "OK") (_ . "KO")))
OK
> (^O ^K)
```
#### OUT

##### Invocation
```lisp
(out 'any . prg)
```
##### Description

Create a new output channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdout`. When the
argument evaluates to a string, `out` assumes the string contains a file path
and tries to open that file.

If the file does not exist, it is created. If the file exists, it is truncated.
If the file path is prepended with a `+` the file must exist and data will be
appended to it.

##### Return value

Return the evaluation of `prg`.

##### Example
```lisp
: (out "test.log" (prinl "Hello, world"))
> (^H ^e ^l ^l ^o ^, ^  ^w ^o ^r ^l ^d)
```
#### PRIN

##### Invocation
```lisp
(prin 'any ...)
```
##### Description

Print the string representation of `any`. When multiple arguments are printed,
no separator is used.

##### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

##### Example
```lisp
: (prin "hello, " "world!")
hello, world!> "world!"
```
#### PRINL

##### Invocation
```lisp
(prinl 'any ...)
```
##### Description

Calls `prin` and appends a new line.

##### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

##### Example
```lisp
: (prinl "hello, " "world!")
hello, world!
> "world!"
```
#### PRINT

##### Invocation
```lisp
(print 'any ...)
```
##### Description

Print the S-expression of `any`. When multiple arguments are printed, a
space separator is used.

##### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

##### Example
```lisp
: (print 'a 'b '(1 2 3))
a b (1 2 3)> (1 2 3)
```
#### PRINTL

##### Invocation
```lisp
(printl 'any ...)
```
##### Description

Calls `print` and appends a new line.

##### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

##### Example
```lisp
: (print 'a 'b (1 2 3))
a b (1 2 3)
> (1 2 3)
```
#### PROG

##### Invocation
```lisp
(prog prg1 prg2 ...)
```
##### Description

Evaluate `prg1`, `prg2`, ..., in sequence.

##### Return value

Return the the result of the last evaluation.

##### Example
```lisp
: (prog (+ 1 1) (+ 2 2))
> 4
```
#### READ

##### Invocation
```lisp
(read)
```
##### Description

Read a token from the current input stream. The current input stream differs
depending on the context:

* When run interactively, the current input stream is `stdin`
* When executing a file, or as a _shebang_ interpreter, the current input
  stream is that of the file

The current input stream is also altered by the [`in`](#in) command.

##### Return value

Return a valid token or `NIL` if the stream is closed or invalid.

#### READLINES

##### Invocation
```lisp
(readlines)
```
##### Description

Read all lines from the current input stream. Stop on `EOF`.

##### Return value

Return all lines as a list of string trimmed of any carry return.

#### RUN

##### Invocation
```lisp
(run 'str 'lst 'alst)
```
##### Description

Run an external program at path `str` with arguments `lst` and environment
`alst`.  If `alst` is `NIL`, the current environment `ENV` is used instead.

This function combines `fork`, `exec`, `dup`, `close`, `pipe`, and `wait` to
provide a handy way to run external binaries in a single shot.

##### Return value

Return a pair with `CAR` as the status code of the binary and `CDR` the
output of the command as list of lines (see [`readlines`](#readlines)).

##### Example
```lisp
: (run "/bin/hostname" NIL NIL)
> (0 (^E ^n ^c ^e ^l ^a ^d ^u ^s))
```
#### SET

##### Invocation
```lisp
(<- 'sym 'any)
```
##### Description

Set an existing symbol `sym` to `any`.

##### Return value

Return the value associated to the symbol.

##### Example
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

##### Invocation
```lisp
(setq sym 'any)
```
##### Description

Associate `any` with the symbol `sym`.

##### Return value

Return the value associated to the symbol.

##### Example
```lisp
: (setq A (+ 1 2))
> 3
```
#### STREAM

##### Invocation
```lisp
(|> any0 [any1] ...)
```
##### Description

Fluent composition operator. Evaluate `any0` and pass the result to `any1`, and
so on until the end of the list.

##### Return value

Return the result of the last `any` operation.

##### Example
```lisp
: (|> '(1 2 3) cdr car)
> 2
```
#### TIME

##### Invocation
```lisp
(time prg)
```
##### Description

Compute the execution time of `prg`.

##### Return value

Return the computed time, in nanoseconds. If `prg` is `NIL`, return the current
timestamp.

##### Example
```lisp
: (time (+ 1 2))
> 8433
: (time)
862596451378329
```
