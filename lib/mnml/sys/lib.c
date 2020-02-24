#include <mnml/module.h>

LISP_MODULE_DECL(slabinfo);
LISP_MODULE_DECL(time);

module_entry_t ENTRIES[] = { LISP_MODULE_REGISTER(slabinfo),
                             LISP_MODULE_REGISTER(time),
                             { NULL, NULL } };

const char*
lisp_module_name()
{
  return "sys";
}

const module_entry_t*
lisp_module_entries()
{
  return ENTRIES;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
