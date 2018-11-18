(def run (path args env)
  "Run process at PATH with arguments ARG and environment ENV."
  (let (((pin . pout) . (pipe))
        (pid . (fork)))
    (if (= pid 0)
      (prog
        (dup pout 1)
        (close pin pout)
        (prinl (exec path args env)))
      (prog
        (close pout)
        (let ((lines . (readlines pin))
              (code  . (wait pid)))
          (close pin)
          (cons code lines)))
      )))
