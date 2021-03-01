#include <mnml/module.h>

LISP_MODULE_DECL(and);
LISP_MODULE_DECL(equ);
LISP_MODULE_DECL(neq);
LISP_MODULE_DECL(not );
LISP_MODULE_DECL(or);

module_entry_t ENTRIES[] = {
  LISP_MODULE_REGISTER(and), LISP_MODULE_REGISTER(equ),
  LISP_MODULE_REGISTER(neq), LISP_MODULE_REGISTER(not ),
  LISP_MODULE_REGISTER(or),  { NULL, NULL }
};

const char* USED
lisp_module_name()
{
  return "logic";
}

const module_entry_t* USED
lisp_module_entries()
{
  return ENTRIES;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
