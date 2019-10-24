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

(+ 1 2)
```
By default, the interpreter preload most common [plugins](#native-functions) out of the box. What
is loaded can be altered by the `MNML_PRELOAD` variable.

## Environment variables

### MNML_DEBUG

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

### MNML_PRELOAD

This variable controls which plugin is preloaded by the interpreter. It accepts
a comman-separated list of symbols:
```
$ MNML_PRELOAD=load,quit mnml
```
Although it can be used empty, you may want to at least preload `load`. 

### MNML_PLUGIN_PATH

This variable controls where to look for plugins, the default being the
installation prefix. It accepts a colon-separated list of paths:
```
$ MNML_PLUGIN_PATH=/some/path:/some/other/path mnml
```
