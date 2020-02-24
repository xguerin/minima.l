# Interpreter

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

(load '(math +))
(+ 1 2)
```
By default, only the symbols `def`, `load`, and `quote` from the `std` module are
available. Scripts are strongly encouraged to load what they use.

## Environment variables

### MNML_DEBUG

If Minima.l has been compiled with debug support, this variable controls which
class of debug output to generate. It accepts a comma-separated list of category
names:

* `BIND`: argument binding operations
* `CHAN`: I/O channel operations
* `CONS`: list construction operations
* `MAKE`: atom creation operations
`* MODL`: module operations
* `REFC`: reference counting operations
* `SLOT`: slot allocator operations
* `SLAB`: slab allocator operations
```
$ MNML_DEBUG=MODL,SLOT mnml
```
The variable can be left empty to restrict debug output to `eval`.

### MNML_MODULE_PATH

This variable controls where to look for modules, the default being the
`lib/mnml` directory in the installation prefix. It accepts a colon-separated
list of paths:
```
$ MNML_MODULE_PATH=/some/path:/some/other/path mnml
```
### MNML_SCRIPT_PATH

This variable controls where to look for lisp scripts, the default being
`share/mnml` in the installation prefix. It accepts a colon-separated
list of paths:
```
$ MNML_SCRIPT_PATH=/some/path:/some/other/path mnml
```
When set, that path is used to match occurences of `@lib` in `(load)` statements.
