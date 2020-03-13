#include <mnml/module.h>

LISP_MODULE_DECL(close);
LISP_MODULE_DECL(dup);
LISP_MODULE_DECL(exec);
LISP_MODULE_DECL(fork);
LISP_MODULE_DECL(pipe);
LISP_MODULE_DECL(time);
LISP_MODULE_DECL(unlink);
LISP_MODULE_DECL(wait);

module_entry_t ENTRIES[] = {
  LISP_MODULE_REGISTER(close), LISP_MODULE_REGISTER(dup),
  LISP_MODULE_REGISTER(exec),  LISP_MODULE_REGISTER(fork),
  LISP_MODULE_REGISTER(pipe),  LISP_MODULE_REGISTER(unlink),
  LISP_MODULE_REGISTER(wait),  { NULL, NULL }
};

const char* USED
lisp_module_name()
{
  return "unix";
}

const module_entry_t* USED
lisp_module_entries()
{
  return ENTRIES;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
