# Tasks

* [X] Slab allocator for cells
* [X] Association lists for symbols
* [X] Lambdas
* [X] Closures
* [ ] Continuations

# Notes

## Closures

The implementation of proper closures requires the use of a `lambda`
operator. We need to embed the call-site closure with the lambda.

Closure computation uses shallow-copy duplication for performance.

Should the define-site and the call-site closure be merged? Pros: it allows
lambdas defined in `let` to be recursive. Cons: it overloads the lambda's
closure. OCaml solves this by using the `rec` attribute. This is maybe
something that needs to be dealt by the `let` operator.

## Continuations

On continuations. Two strategies: 1/ rewrite all statement in CPS style so
continuations are readily available; 2/ use coroutines. The former seems more
"pure" but also a lot more expensive. The latter is probably the way to go.

The coroutine stategy for continuations requires garbage collection to clean-up
aborted continuations. Since I don't want to use a GC, I am now exploring option
1. This approach requires to convert on the fly immediate-style expressions to
continuation-passing style.
