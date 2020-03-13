#include <mnml/module.h>

LISP_MODULE_DECL(add);
LISP_MODULE_DECL(div);
LISP_MODULE_DECL(ge);
LISP_MODULE_DECL(gt);
LISP_MODULE_DECL(le);
LISP_MODULE_DECL(lt);
LISP_MODULE_DECL(mod);
LISP_MODULE_DECL(mul);
LISP_MODULE_DECL(sub);

module_entry_t ENTRIES[] = {
  LISP_MODULE_REGISTER(add), LISP_MODULE_REGISTER(div),
  LISP_MODULE_REGISTER(ge),  LISP_MODULE_REGISTER(gt),
  LISP_MODULE_REGISTER(le),  LISP_MODULE_REGISTER(lt),
  LISP_MODULE_REGISTER(mod), LISP_MODULE_REGISTER(mul),
  LISP_MODULE_REGISTER(sub), { NULL, NULL }
};

const char* USED
lisp_module_name()
{
  return "math";
}

const module_entry_t* USED
lisp_module_entries()
{
  return ENTRIES;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
