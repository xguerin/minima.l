# Closures

Minima.l implement lexical scope closures using stack-oriented association
lists. These lists are created by symbol-binding operations and are carried over
by functions and lambdas.

## Example

Let's consider the following `let` operation:
```lisp
(let ((a . 1) (b . 2)) PRG)
```
The closure made available to `PRG` is:
```lisp
(((a . 1) (b . 2)))
```
## Problem definition

Symbol binding operations are operations that associate a value to a symbol.
This association can either happen at the _global_ scope or at the _local_ scope.

Operations that impact the global scope are `def`, `setq` and `<-` to the extent
that it only _alters_ existing symbols.

Operations that impact the local scope are `let` and `<-`. The `let` operation
actually creates a local scope and can be used to define lambdas as local
functions: 
```lisp
(let ((fn . (\ (a b) (+ a b)))) (fn 1 2))
```
These local scopes can be nested, and so can be these local functions:
```lisp
(let ((fn0 . (\ (a b) (+ a b))))
  (let ((fn1 . (\ (a b) (- b a))))
    (fn1 (fn0 2 2) (fn0 1 1))))
```
Local functions need to be shielded from nesting operations that redefine
symbols. For instance:
```lisp
(let ((a . 1)                  # Level 0 scope
      (fn0 . (\ (x) (+ x a)))) #
  (let ((a . 2))               # Level 1 scope
    (fn0 3)))                  #
```
This expression is expected to return the value `4` as symbol `a` was assigned
the value `1` when the lambda was _defined_. To ensure that behavior, the lambda
assigned to `fn0` must save the closure available at its _definition site_.

## Closure stack

The solution implemented in Minima.l to achieve this is a closure stack. 
At the bottom level of the stack is the global scope. Then, each nested local
scope pushes its closure on the stack.

When symbols are resolved, Minima.l crawls up the stack of closure to find the
first match. The global scope is always checked at the last resort.

When a function is defined at the global scope using `def`, its closure capture
is empty. When a function is defined at the local scope using `let` and a
lambda, the `lambda` function captures the closure into the generated function
definition.
