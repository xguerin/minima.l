# Plugins

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
## Example

Let's take the following code written for an `add` function and save it in a
file called `add.c`:
```c
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>

static atom_t
lisp_function_add(const atom_t closure, const atom_t arguments)
{
  LISP_LOOKUP(x, arguments, X);
  LISP_LOOKUP(y, arguments, Y);
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

LISP_PLUGIN_REGISTER(add, add, X, Y, NIL)
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

The `LISP_PLUGIN_REGISTER` macro is used to register a new plugin:
```c
#define LISP_PLUGIN_REGISTER(__s, __n, ...)
```
The first argument `__s` is the suffix of the function. The second argument
`__n` is the name of the symbol. The variadic arguments represent the function
argument symbols. For instance, the function `add` is defined as follow:
```c
static atom_t
lisp_function_add(const atom_t closure)
{
  /* ... */
}

LISP_PLUGIN_REGISTER(add, add, X, Y, NIL)
```
### Argument processing

The argument symbols are defined as follows:

| C version                                 | Minima.l equivalent     |
|:------------------------------------------|:------------------------|
| `LISP_PLUGIN_REGISTER(fun, fun)`            | `(def fun () ...)`        |
| `LISP_PLUGIN_REGISTER(fun, fun, @)`         | `(def fun @ ...)`         |
| `LISP_PLUGIN_REGISTER(fun, fun, A)`         | `(def fun (A) ...)`       |
| `LISP_PLUGIN_REGISTER(fun, fun, A, NIL)`    | `(def fun (A) ...)`       |
| `LISP_PLUGIN_REGISTER(fun, fun, A, B)`      | `(def fun (A . B) ...)`   |
| `LISP_PLUGIN_REGISTER(fun, fun, A, B, NIL)` | `(def fun (A B) ...)`     |
| `LISP_PLUGIN_REGISTER(fun, fun, A, B, C)`   | `(def fun (A B . C) ...)` |

Values for the declared symbols are passed to the plugin by the interpreter
through the `arguments` parameter. They can be retrieved using the `LISP_LOOKUP`
macro:
```c
#define LISP_LOOKUP(_v, _c, _x)
```
The first argument is a name to use for the variable to be assigned the value.
The second argument is the name of the `arguments` parameter. The last argument
is the name of the symbol to look up.