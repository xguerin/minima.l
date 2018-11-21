# Tasks

* [X] Slab allocator for cells
* [X] Association lists for symbols
* [X] Lambdas
* [X] Closures
* [ ] Continuations

# Notes

The implementation of proper closures requires the use of a `lambda`
operator. We need to embed the call-site closure with the lambda.

Closure computation uses shallow-copy duplication for performance.

On continuations. Two strategies: 1/ rewrite all statement in CPS style so
continuations are readily available; 2/ use coroutines. The former seems more
"pure" but also a lot more expensive. The latter is probably the way to go.
