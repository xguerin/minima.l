# Reference

## Notation rules

1. Elements in `[]` are optional
2. A quoted symbol means that it is evaluated

## Summary

#### Structural comparisons

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `=`         | `(=  'any 'any)`              | `logic`  | Equality |
| `<>`        | `(<> 'any 'any)`              | `logic`  | Inequality |

#### Numeric comparison

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `<`         | `(<  'num 'num)`              | `math`   | Less-than |
| `<=`        | `(<= 'num 'num)`              | `math`   | Less-than-or-equal-to |
| `>`         | `(>  'num 'num)`              | `math`   | Greater-than |
| `>`         | `(>= 'num 'num)`              | `math`   | Greater-than-or-equal-to |

#### Arithmetic operations

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `+`         | `(+ 'num 'num)`               | `math`   | Addition |
| `-`         | `(- 'num 'num)`               | `math`   | Subtraction |
| `*`         | `(* 'num 'num)`               | `math`   | Multiplication |
| `/`         | `(/ 'num 'num)`               | `math`   | Division |
| `%`         | `(% 'num 'num)`               | `math`   | Modulo |

#### Logical operations
| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `and`       | `(and 'any 'any)`             | `logic`  | Logical AND |
| `not`       | `(not 'any)`                  | `logic`  | Logical NOT |
| `or`        | `(or 'any 'any)`              | `logic`  | Logical OR |

#### Predicates

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `chr?`      | `(chr? 'any)`                 | `std`    | Return `T` if `any` is a character |
| `lst?`      | `(lst? 'any)`                 | `std`    | Return `T` if `any` is a list |
| `nil?`      | `(nil? 'any)`                 | `std`    | Return `T` if `any` is `NIL` |
| `num?`      | `(num? 'any)`                 | `std`    | Return `T` if `any` is a number |
| `str?`      | `(str? 'any)`                 | `std`    | Return `T` if `any` is a string |
| `sym?`      | `(sym? 'any)`                 | `std`    | Return `T` if `any` is a symbol |
| `tru?`      | `(tru? 'any)`                 | `std`    | Return `T` if `any` is `T` |

#### String operations

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `ntoa`      | `(ntoa 'num)`                 |        | Convert `num` into a string |
| `join`      | `(join 'lst 'chr)`            |        | Join `lst` of strings into a `chr`-separted string |
| `split`     | `(split 'str 'chr)`           |        | Split `str` of `chr`-separted tokens |
| `str`       | `(str 'sym)`                  | `std`    | Make a string out of `sym` |
| `trim`      | `(trim 'str)`                 |        | Trim `str` of leading and trailing white spaces |

#### Symbol definition

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `def`       | `(def sym args [str] prg)`    | `std`    | [Define](#def) a function |
| `let`       | `(let lst . prg)`             | `std`    | [Let](#let)-binding symbols |
| `<-`        | `(setq sym 'any)`             | `std`    | [Update](#set) an existing symbol |
| `setq`      | `(setq sym 'any)`             | `std`    | [Bind](#setq) a symbol |
| `sym`       | `(sym 'str)`                  | `std`    | Make a symbol out of `str` |

#### List manipulation

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `append`    | `(append 'lst . any)`         |        | Recursively append `any` to `lst` |
| `car`       | `(car 'lst)`                  | `std`    | Get the head element of `lst` |
| `cadr`      | `(cadr 'lst)`                 | `manips` | Get the 2nd element of `lst` |
| `cddr`      | `(cddr 'lst)`                 | `manips` | Get the tail of the tail of `lst` |
| `caddr`     | `(caddr 'lst)`                | `manips` | Get the 3rd element of `lst` |
| `cadddr`    | `(cadddr 'lst)`               | `manips` | Get the 4th element of `lst` |
| `caar`      | `(caar 'lst)`                 | `manips` | Get the 1st element of the head of `lst` |
| `cadar`     | `(cadar 'lst)`                | `manips` | Get the 2nd element of the head of `lst` |
| `cdar`      | `(cdar 'lst)`                 | `manips` | Get the tail of the head of `lst` |
| `cdr`       | `(cdr 'lst)`                  | `std`    | Get the tail of `lst` |
| `chr`       | `(chr 'num)`                  | `std`    | Get the character for ASCII numner `num` |
| `conc`      | `(conc 'lst 'lst)`            | `std`    | [Concatenate](#conc) two lists into one |
| `cons`      | `(cons 'any 'any)`            | `std`    | [Construct](#cons) a pair |
| `filter`    | `(filter 'fun 'lst)`          |        | Filter `lst` using `fun` |
| `flatten`   | `(flatten 'lst)`              |        | Flatten a nested `lst` |
| `foldl`     | `(foldl 'fun 'acc 'lst)`      |        | Left-fold a `lst` |
| `foldr`     | `(foldr 'fun 'lst 'acc)`      |        | Right-fold a `lst` |
| `insert`    | `(insert 'fun 'any 'lst)`     |        | Insert `any` into a sorted `lst` using `fun` |
| `iter`      | `(iter 'fun 'lst)`            |        | Iterate over the elements of a list |
| `last`      | `(last 'lst)`                 |        | Return the last element of a list |
| `len`       | `(len 'lst)`                  | `std`    | Compute the length `lst` |
| `list`      | `(list 'any ...)`             | `std`    | [Create](#list) a list with `any` |
| `map`       | `(map 'fun 'lst)`             |        | Map the content of `lst` |
| `map2`      | `(map2 'fun 'lst 'lst)`       |        | Map the content of a two lists |
| `merge`     | `(merge 'fun 'fun 'lst 'lst)` |        | Sorted and deduped merge of two lists |
| `rev`       | `(rev 'lst)`                  |        | Reverse `lst` |
| `zip`       | `(zip 'lst 'lst)`             |        | Sequentially pair-up elements from two lists |

#### Assoc-list operations

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `assoc`     | `(assoc 'any 'lst)`           |        | [Query](#assoc) an association list |
| `erase`     | `(erase 'any 'lst)`           |        | Remove an entry in an association list |
| `replc`     | `(replc 'any 'any 'lst)`      |        | Replace an entry in an association list |

#### Control flow

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `|>`        | `(|> any0 [any1] ...)`        | `std`    | [Fluent](#stream) composition |
| `cond`      | `(cond 'any ...)`             | `std`    | [Predicate](#cond) matching |
| `if`        | `(if 'any then [else])`       | `std`    | [If](#if) construct |
| `match`     | `(match 'any ...)`            | `std`    | [Structural](#match) matching |
| `prog`      | `(prog any0 [any1] ...)`      | `std`    | [Sequential](#prog) composition |
| `unless`    | `(unless 'any . prg)`         | `std`    | Execute `prg` unless `any` is not `NIL` |
| `when`      | `(when 'any . prg)`           | `std`    | Execute `prg` if `any` is not `NIL` |
| `while`     | `(while 'any . prg)`          | `std`    | Execute `prg` while `any` is not `NIL` |

#### Input/output operations

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `in`        | `(in 'any . prg)`             | `io`     | [In](#if) stream |
| `out`       | `(out 'any . prg)`            | `io`     | [Out](#if) stream |
| `prin`      | `(prin 'any ...)`             | `io`     | [Symbolic print](#prin) of a list of `any` |
| `prinl`     | `(prinl 'any ...)`            | `io`     | [Symbolic print](#prinl) of a list of `any`, with new line |
| `print`     | `(print 'any ...)`            | `io`     | [Literal print](#print) of a list of `any` |
| `printl`    | `(printl 'any ...)`           | `io`     | [Literal print](#print) of a list of `any`, with new line |
| `read`      | `(read)`                      | `io`     | [Read a token](#read) from the current input stream |
| `readline`  | `(readline)`                  | `io`     | [Read one line](#readline) from the current input stream |

#### Core operations

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `eval`      | `(eval 'any)`                 | `std`    | [Evaluate](#eval) `any` |
| `load`      | `(load str)`                  | `std`    | [Load](#load) an external asset |
| `time`      | `(time prg)`                  | `sys`    | [Time](#time) the execution of `prg` |
| `quit`      | `(quit)`                      | `std`    | Quit the interpreter loop |
| `quote`     | `(quote . any)`               | `std`    | Quote `any` |

#### Socket functions

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `accept`    | `(accept 'num)`             | `unix`   | Accept a connection from server descriptor `num` |
| `connect`   | `(connect 'dns 'svc)`       | `unix`   | Connect to `dns` on service port `svc` |
| `listen`    | `(listen 'num)`             | `unix`   | Listen for connections on port `num` |

#### System functions

| Name      | Syntax                      | Module | Description |
|:----------|:----------------------------|:------:|:------------|
| `close`     | `(dup 'num)`                  | `unix`   | Close a file descriptor `num` |
| `dup`       | `(dup 'num ['num])`           | `unix`   | Duplicate a file descriptor `num` |
| `exec`      | `(exec 'str 'lst 'lst)`       | `unix`   | Execute an image at path with arguments and environment |
| `fork`      | `(fork)`                      | `unix`   | Fork the current process |
| `run`       | `(run 'str 'lst 'alst)`       |        | [Run](#run) a external program `str` |
| `select`    | `(select 'fds 'rcb 'ecb)`     | `unix`   | Wait for available data on descriptors `fds` |
| `unlink`    | `(unlink 'str)`               | `unix`   | Unlink the file pointed by `str` |
| `wait`      | `(wait 'num)`                 | `unix`   | Wait for PID `num` |

## Detailed description

### ASSOC

#### Invocation
```lisp
(assoc 'any 'lst)
```
#### Description

Look-up `any` in the association list `lst`.

#### Return value

If a value is bound to `any`, return that value. If not, return `NIL`.

#### Example
```lisp
: (assoc 'hello '((hello . world)))
> world
: (assoc 'foo '((hello . world)))
> NIL
```
****
### CONC

#### Invocation
```lisp
(conc 'lst1 'lst2)
```
#### Description

Destructively concatenate two lists `lst1` and `lst2` into a single list.

#### Return value

If `lst1` is a list, return the concatenation of `lst1` and `lst2`. The value
pointed by `lst1` is actually modified. If `lst1` is not a list, return `NIL`.

#### Example
```lisp
: (setq A '(1 2))
> (1 2)
: (conc A '(3 4))
> (1 2 3 4)
: A
> (1 2 3 4)
```
****
### COND

#### Invocation
```lisp
(cond 'any (any . prg) (any . prg) ...)
```
#### Description

Evaluate `any` and use the `car` of the remaining arguments as a predicate over
the result. Return the evaluation of the first positive match. The _default_ or
_catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.

#### Return value

If a match is made, returns the evaluation of the corresponding `prg`. If no
match is made, return `NIL`.

#### Example
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
****
### CONS

#### Invocation
```lisp
(cons 'any1 'any2)
```
#### Description

Construct a new list using the first argument for `car` and the second argument
for `cdr`.

#### Return value

Return the newly constructed list without modifying the arguments.

#### Example
```lisp
: (cons 1 2)
> (1 . 2)
: (cons 1 (cons 2 3))
> (1 2 . 3)
```
****
### DEF

#### Invocation
```lisp
(def sym lst [str] prg ...)
```
#### Description

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
#### Return value

Return the S-expression of the newly defined function.

#### Example
```lisp
: (def add (x y) (+ x y))
> ((x y) NIL NIL (+ x y))
```
****
### EVAL

#### Invocation
```lisp
(eval 'any)
```
#### Description

Evaluate `any`.

#### Return value

Return the result of the evaluation.

#### Example
```lisp
: (eval (list '+ 1 1))
> 2
```
****
### IF

#### Invocation
```lisp
(if 'any . lst)
```
#### Description

When `any` evaluates to `T`, evaluate `(car lst)`. Otherwise, evaluate
`(car (cdr lst))`.

#### Return value

Return the result of the evaluation.

#### Example
```lisp
: (def test (v) (if (> v 10) (* v 2)))
> test
: (test 5)
> NIL 
: (test 20)
> 40
```
****
### IN

#### Invocation
```lisp
(in 'any . prg)
```
#### Description

Create a new input channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdin`. When the
argument evaluates to a string, `in` assumes the string contains a file path and
tries to open that file.

#### Return value

Return the evaluation of `prg`.

****
### LET

#### Invocation
```lisp
(let lst . prg)
```
#### Description

Evaluate `prg` within the context of the bind list `lst`. The bind list has the
following format:
```lisp
((any . 'any)(any . 'any)...)
```
#### Return value

Return the value of the evaluated `prg`.

#### Example
```lisp
: (let ((lhv . (+ 1 2)) (rhv . (+ 3 4))) (+ lhv rhv))
> 10
```
****
### LIST

#### Invocation
```lisp
(list 'any ...)
```
#### Description

Create a list with `any` arguments.

#### Return value

Return the newly created list.

#### Example
```lisp
: (list)
> NIL
: (list (+ 1 1) 3 "a")
> (2 3 "a")
```
****
### LOAD

#### Invocation
```lisp
(load 'str ...)
(load '(mod sym ...))
(load 'sym ...)
(load '(mod . T))
```
#### Description

In the first form, load the `lisp` file pointed by `str`. If the path is
prefixed by `@lib`, `load` will look for the file in the `lib` directory of the
installation prefix.
```lisp
: (load "@lib/cadr.l")
> ((x) NIL NIL (car (cdr x)))
```
In the second form, load `sym` in the module `mod`. In the third and fourth
forms, all symbols in the module `mod` are loaded.
```lisp
: (load '(math +))
> (+)
```
Modules are searched in the `lib/mnml` directory of the installation prefix or
in the `MNML_MODULE_PATH` environment variable.

#### Return value

In the first form, return the result of the last evaluated operation in the
list. In the second and third form, return the list of loaded symbols. On error,
`NIL` is returned.

#### Example
```lisp
: (load "lisp/cadr.l")
> ((x) NIL NIL (car (cdr x)))
```
****
### MATCH

#### Invocation
```lisp
(match 'any (any . prg) ...)
```
#### Description

Evaluate `any` and use the `car` of the remaining arguments as a structural
template for the result. The
_default_ or _catch all_ case is written using the special value `_` as `car`.

Order is important. If multiple match exist, the first one is evaluated. If `_`
is placed before a valid match, `_` is evaluated.

#### Return value

Return the evaluation of the first positive match. Return `NIL` otherwise.

#### Example
```lisp
: (prinl (match '(1 2 3) ((1 _ _) . "OK") (_ . "KO")))
OK
> (^O ^K)
```
****
### OUT

#### Invocation
```lisp
(out 'any . prg)
```
#### Description

Create a new output channel context and evaluate `prg` within that context. The
previous context is restored after the evaluation.

When the first argument evaluates to `NIL`, the context uses `stdout`. When the
argument evaluates to a string, `out` assumes the string contains a file path
and tries to open that file.

If the file does not exist, it is created. If the file exists, it is truncated.
If the file path is prepended with a `+` the file must exist and data will be
appended to it.

#### Return value

Return the evaluation of `prg`.

#### Example
```lisp
: (out "test.log" (prinl "Hello, world"))
> (^H ^e ^l ^l ^o ^, ^  ^w ^o ^r ^l ^d)
```
****
### PRIN

#### Invocation
```lisp
(prin 'any ...)
```
#### Description

Print the string representation of `any`. When multiple arguments are printed,
no separator is used.

#### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

#### Example
```lisp
: (prin "hello, " "world!")
hello, world!> "world!"
```
****
### PRINL

#### Invocation
```lisp
(prinl 'any ...)
```
#### Description

Calls `prin` and appends a new line.

#### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

#### Example
```lisp
: (prinl "hello, " "world!")
hello, world!
> "world!"
```
****
### PRINT

#### Invocation
```lisp
(print 'any ...)
```
#### Description

Print the S-expression of `any`. When multiple arguments are printed, a
space separator is used.

#### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

#### Example
```lisp
: (print 'a 'b '(1 2 3))
a b (1 2 3)> (1 2 3)
```
****
### PRINTL

#### Invocation
```lisp
(printl 'any ...)
```
#### Description

Calls `print` and appends a new line.

#### Return value

Return the result of the evaluation of the last argument. If there is no
argument, return `NIL`.

#### Example
```lisp
: (print 'a 'b (1 2 3))
a b (1 2 3)
> (1 2 3)
```
****
### PROG

#### Invocation
```lisp
(prog prg1 prg2 ...)
```
#### Description

Evaluate `prg1`, `prg2`, ..., in sequence.

#### Return value

Return the the result of the last evaluation.

#### Example
```lisp
: (prog (+ 1 1) (+ 2 2))
> 4
```
****
### READ

#### Invocation
```lisp
(read)
```
#### Description

Read a token from the current input stream. The current input stream differs
depending on the context:

* When run interactively, the current input stream is `stdin`
* When executing a file, or as a _shebang_ interpreter, the current input
  stream is that of the file

The current input stream is also altered by the [`in`](#in) command.

#### Return value

Return a valid token or `NIL` if the stream is closed or invalid.

****
### READLINE

#### Invocation
```lisp
(readline)
```
#### Description

Read one line from the current input stream. Stop on `EOF`.

#### Return value

Return a line as a string trimmed of any carry return. `NIL` in case of `EOF`.

****
### RUN

#### Invocation
```lisp
(run 'str 'lst 'alst)
```
#### Description

Run an external program at path `str` with arguments `lst` and environment
`alst`.  If `alst` is `NIL`, the current environment `ENV` is used instead.

This function combines `fork`, `exec`, `dup`, `close`, `pipe`, and `wait` to
provide a handy way to run external binaries in a single shot.

#### Return value

Return a pair with `CAR` as the status code of the binary and `CDR` the
output of the command as list of lines.

#### Example
```lisp
: (run "/bin/hostname" NIL NIL)
> (0 (^E ^n ^c ^e ^l ^a ^d ^u ^s))
```
****
### SET

#### Invocation
```lisp
(<- 'sym 'any)
```
#### Description

Associate `any` with the symbol `sym`. The symbol must exist.

#### Return value

Return the previous value associated to the symbol.

#### Example
```lisp
: (setq A (+ 1 2))
> 3
: (<- 'A 4)
> 3
: A
> 4
```
****
### SETQ

#### Invocation
```lisp
(setq sym 'any)
```
#### Description

Associate `any` with the symbol `sym`.

#### Return value

Return the value associated to the symbol.

#### Example
```lisp
: (setq A (+ 1 2))
> 3
```
****
### STREAM

#### Invocation
```lisp
(|> any0 [any1] ...)
```
#### Description

Fluent composition operator. Evaluate `any0` and pass the result to `any1`, and
so on until the end of the list.

#### Return value

Return the result of the last `any` operation.

#### Example
```lisp
: (|> '(1 2 3) cdr car)
> 2
```
****
### TIME

#### Invocation
```lisp
(time prg)
```
#### Description

Compute the execution time of `prg`.

#### Return value

Return the computed time, in nanoseconds. If `prg` is `NIL`, return the current
timestamp.

#### Example
```lisp
: (time (+ 1 2))
> 8433
: (time)
862596451378329
```
