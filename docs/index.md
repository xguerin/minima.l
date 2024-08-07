# Minima.l

Opinionated LISP dialect that takes some of its inspirations from [Picolisp](https://picolisp.com),
[CHICKEN Scheme](http://call-cc.org), and other great languages. The source code is located [here](https://github.com/xguerin/minima.l).

## Example
```lisp
(load '(math + - <=) '(io prinl) '(std if))

(def fib (N)
  (if (<= N 1)
    N
    (+ (fib (- N 1)) (fib (- N 2)))))

(prinl "Result: " (fib 30))
```
## License
```plain
Copyright (c) 2020 Xavier R. Guérin <copyright@applepine.org>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```
