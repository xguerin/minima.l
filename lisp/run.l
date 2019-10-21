(load 'close 'dup 'exec 'fork 'pipe 'wait "append.l" "map.l")

#
# Run process at PATH with arguments ARG and environment ENV:
#
# * PATH must be a full path to the binary
# * ARGS is a list of string
# * ENV is an assoc list of strings.
#
(def run (path args env)
  "Run process at PATH with arguments ARG and environment ENV."
  (let ((fenv         . (map (\ ((k . v)) (append k (cons ^= v))) env))
        ((pin . pout) . (pipe))
        (pid          . (fork)))
    (if (= pid 0)
      (prog
        (dup pout 1)
        (close pin pout)
        (exec path args fenv))
      (prog
        (close pout)
        (let ((lines . (in pin (readlines)))
              (code  . (wait pid)))
          (close pin)
          (cons code lines))
        ))))
