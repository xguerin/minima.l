# Type inference

## On control

A missing `else` branch is not an error. It should be forwarded to the next
continuation. How should that be forwarded? As an atom pointing to `NIL`? Do all
continuation need to check for `NIL` then? Or should we simply disallow
`if`-only control?

### Why not use atoms?

We don't use `atom_t` for performance reason as we don't want to hit the slab
allocator for every new value. That choice deprives us from a type annotation.

### Optionals

The default return type is really `T | NIL`, not simply `T`. In that case, what
we are really handling are `T optional`. How do I efficiently implement
`optional` in C? Should I use another register in the `value_t` definition to
embed whether the  value is present or not? This solution would not optimize
well.
```c
typedef struct _value {
  bool present;
  union {
    int64_t number;
    atom_t atom;
    atom_t (*callback)(struct _closure * C, struct _value V);
  };
}
value_t;
```
### Forbid undefined scenarios

Another solution would be to simply forbid that case since `C` forbids it. That
allows us keep using simple types.

### Decision

We will forbid undefined scenarios.
