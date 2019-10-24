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
> ((a b) NIL NIL (+ a b))
: (add 1 2) # Form 1
> 3
: ('((a b) NIL NIL (+ a b)) 1 2) # Form 2
> 3
: ((\ (a b) (+ a b)) 1 2) # Form 3
> 3
```
## Functions

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

### Lambdas

Lambda functions are defined using the `\\` keyword. Invocation of `\\` is
similar to `def`:
```lisp
: ((\ (x y) (+ x y)) 1 1)
> 2
: ((\ (x) (map (\ (n) (+ n 1)) x)) '(1 2 3 4))
> (2 3 4 5)
```
### Currying

Minima.l supports currying. It uses the function's closure to store the curried
arguments. Curryring is available to all `lisp` and native functions. For example:
```lisp
: (def add (a b) (+ a b))
> ((a b) NIL NIL (+ a b))
: (setq +1 (add 1))
> ((b) NIL NIL ((a . 1)) (+ a b))
: (+1 2)
> 3
```
### Native functions

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
### Writing a plugin

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

### Recursion

A function defined using `def` can be recursive. When functions are defined,
symbols with their name are not resolved in their closure and are resolved
dynamically in the symbol domain instead. There is a caveat however: since the
function symbols are resolved dynamically, redefining these symbols will lead to
undefined behavior.

### Argument assignation

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


