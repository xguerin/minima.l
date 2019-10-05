# Continuations

We use the greek letter κ (0x03BA) to represent the current continuation.

## Principle

### Function with one arguments
```scheme
; Normal style
(not T)

; Continuation-passing style
(define (notcps v κ) (k (not v)))
```
### Function with two arguments
```scheme
; Normal style
(add 1 2)

; Continuation-passing style
(define (add a b κ) (κ (+ a b)))
```
### Singly-nested functions
```scheme
; Normal style
(define (fn) (add (mul 1 2) 3)

; Continuation-passing style
(define (add a b κ) (k (+ a b)))
(define (mul a b κ) (k (* a b)))

(define (fn κ) (mul 1 2 (lambda (x) (add x 3 κ)))
```
### Doubly-nested functions
```scheme
; Normal style
(define (fn) (add (mul 1 2) (div 3 4)))

; Continuation-passing style
(define (add a b κ) (k (+ a b)))
(define (mul a b κ) (k (* a b)))
(define (div a b κ) (k (/ a b)))

(define (fn κ) (mul 1 2 (lambda (x) (div 4 2 (lambda (y) (add x y κ))))))
```
### Control structures

#### If-Then-Else
```scheme
; Normal style
(define (fn v)
    (if (equ v 0)
        (add v 1)
        (sub v 1)))

; Continuation-passing style
(define (fn v κ)
    (equ v 0 (lambda (x)
        (if x
            (add v 1 κ)
            (sub v 1 κ)))
```
## Evaluation

The continuation-passing style represent a function call as a chain of
continuation-accepting functions `f(..., κ)` where the function of rank
`N-1` is used as the continuation of the function of rank `N`.

### Function with two arguments

| Step | Description |
|:-----|:------------|
| 1    | Apply current continuation to the result | `(define (fn κ) (add (mul 1 2) (div 3 4) κ))` |
| 2    | Lift the second argument                 | `(define (fn κ) (div 3 4 (lambda (y) (add (mul 1 2) y κ))))` |
| 2    | Lift the first argument                  | `(define (fn κ) (mul 1 2 (lambda (x) (div (lambda (y) (add x y κ)) 3 4))))` |

### Tree view
```
(define (fn κ)
    (mul
        1   ; immediate
        2   ; immediate
        (kappa (x) (div     ;.......................................................< continuation_2
                        4   ; immediate                                             |
                        2   ; immediate                                             |
                        (kappa (y) (add     ;....................< continuation_1   |
                                        x   ; closure variable   |                  |
                                        y   ; argument variable  |                  |               
                                        κ   ; closure variable   |                  |
                        )))                                      |                  |
                    ))                                                              |
    ))
```
### ABI view

#### Types
```c
struct _continuation_t;

typedef union _value_t {
  int64_t integer;
  char * string;
  void (*continuation)(union _value_t * environment, union _value_t result);
}
value_t;

typedef value_t environment_t[16];

typedef void (*continuation_t)(environment_t environment, value_t result);
```
#### Basic functions
```c
void fn_add(environment_t E, value_t A0, value_t A1, continuation_t K) {
  const register value_t result = { .integer = A0.integer + A1.integer };
  K(E, result);
}

void fn_mul(environment_t E, value_t A0, value_t A1, continuation_t K) {
  const register value_t result = { .integer = A0.integer * A1.integer };
  K(E, result);
}

void fn_div(environment_t E, value_t A0, value_t A1, continuation_t K) {
  const register value_t result = { .integer = A0.integer / A1.integer };
  K(E, result);
}
```
#### Continuations
```c
void cont_1(environment_t E, value_t Y) {
  fn_add(E, E[1], Y, E[0].continuation);
}

void cont_2(environment_t E, value_t X) {
  register const value_t A0 = { .integer = 4 };
  register const value_t A1 = { .integer = 2 };
  E[1] = X;
  fn_div(E, A0, A1, cont_1);
}

void print(environment_t E, value_t X) {
  printf("Hello %lld\n", X.integer);
}
```
#### Top and main
```c
void fn(environment_t E, continuation_t K) {
  register const value_t A0 = { .integer = 1 };
  register const value_t A1 = { .integer = 2 };
  E[0].continuation = K;
  fn_mul(E, A0, A1, cont_2);
}

int main(const int argc, char ** const argv) {
  /*
   * Create an empty environment.
   */
  environment_t env;
  memset(env, 0, sizeof(env));
  /*
   * Call the function.
   */
  fn(env, print);
  return 0;
}
```
