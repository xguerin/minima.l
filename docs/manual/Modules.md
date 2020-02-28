# Modules

Modules are collections of function definitions. They are loaded using the
`load` function. They can be implemented in in any language that compiles into
shared libraries with a C interface (binary modules). 

## Binary interface

A module is a shared library that must export the following interface:
```c
const char * lisp_module_name();
atom_t lisp_module_setup(const lisp_t lisp);
```
The `lisp_module_setup` function returns an `atom_t` value that points to a
list definition as follows:
```lisp
(ARGUMENTS NIL . INTEGER)
```
The integer part represents a pointer to a function with the following
signature:
```c
atom_t
lisp_function_NAME(const lisp_t lisp, const atom_t closure, const atom_t args);
```
## Example

Let's take the following code written for an `add` function and save it in a
file called `add.c`:
```c
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>

static atom_t
lisp_function_add(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(x, closure, X);
  LISP_LOOKUP(y, closure, Y);
  /*
   * Make sure the arguments are numbers.
   */
  if (!(IS_NUMB(x) && IS_NUMB(y))) {
    X(x, y);
    return UP(NIL);
  }
  /*
   * Return the new value.
   */
  atom_t result = lisp_make_number(x->number + y->number);
  X(x, y);
  return result;
}

LISP_MODULE_SETUP(add, add, X, Y, NIL)
```
Then, let's compile it:
```bash
$ cc -I${PREFIX}/include -L${PREFIX}/lib -lminimal -o libminimal_function_add.${LD_EXT} add.c
```
Where `${PREFIX}` is Minima.l's installation prefix and `${LD_EXT}` the linker
extension for your platform (`.so` on most, `.dylib` on Mac).

Finally, place this shared object in your Minima.l cache (`$HOME/.mnml`) and
fire the interpreter:
```lisp
: (load 'add)
> add
: (add 1 2)
> 3
```
## Programming interface

### Atom type

The basic type in Minima.l is `atom_t`. This type represents numbers,
characters, symbols, and pairs. Some symbols are defined as singletons, like
`NIL`, `TRUE`, `QUOTE`, and `WILDCARD`.

#### Allocation

Atom values are allocated using functions in the _maker_ interface:
```c
atom_t lisp_make_char(const char c);
atom_t lisp_make_number(const int64_t num);
atom_t lisp_make_string(const char * const s, const size_t len);
atom_t lisp_make_symbol(const symbol_t sym);
```
Pairs are created by using the `cons` and `conc` standard functions in the
`lisp` interface:
```c
atom_t lisp_cons(const atom_t a, const atom_t b);
atom_t lisp_conc(const atom_t a, const atom_t b);
```
#### Deallocation

Atom values must be manually deallocated using the `X(...)` macro defined in the
`slab` interface. Atom values are reference counted, so great care must be taken
when handling them.

### Registration

The `LISP_MODULE_SETUP` macro is used to register a new module:
```c
#define LISP_MODULE_SETUP(__s, __n, ...)
```
The first argument `__s` is the suffix of the function. The second argument
`__n` is the name of the symbol. The variadic arguments represent the function
argument symbols. For instance, the function `add` is defined as follow:
```c
static atom_t
lisp_function_add(const lisp_t lisp, const atom_t closure)
{
  /* ... */
}

LISP_MODULE_SETUP(add, add, X, Y, NIL)
```
### Argument processing

The argument symbols are defined as follows:

| C version                              | Minima.l equivalent       |
|:---------------------------------------|:--------------------------| 
| `LISP_MODULE_SETUP(fun, fun)`            | `(def fun () ...)`          |
| `LISP_MODULE_SETUP(fun, fun, A)`         | `(def fun A ...)`           |
| `LISP_MODULE_SETUP(fun, fun, A, NIL)`    | `(def fun (A) ...)`         |
| `LISP_MODULE_SETUP(fun, fun, A, REM)`    | `(def fun (A . REM) ... )`  |
| `LISP_MODULE_SETUP(fun, fun, A, B, NIL)` | `(def fun (A B) ...)`       |
| `LISP_MODULE_SETUP(fun, fun, A, B, REM)` | `(def fun (A B . REM) ...)` |

Values for the declared symbols are passed to the module by the interpreter
through the `args` parameter. They can be retrieved using the `LISP_LOOKUP`
macro:
```c
#define LISP_LOOKUP(_v, _c, _x)
```
The first argument is a name to use for the variable to be assigned the value.
The second argument is the name of the `args` parameter. The last argument
is the name of the symbol to look up.
