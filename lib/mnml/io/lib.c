#include <mnml/module.h>

LISP_MODULE_DECL(in);
LISP_MODULE_DECL(out);
LISP_MODULE_DECL(prin);
LISP_MODULE_DECL(prinl);
LISP_MODULE_DECL(print);
LISP_MODULE_DECL(printl);
LISP_MODULE_DECL(read);
LISP_MODULE_DECL(readline);

module_entry_t ENTRIES[] = { LISP_MODULE_REGISTER(in),
                             LISP_MODULE_REGISTER(out),
                             LISP_MODULE_REGISTER(prin),
                             LISP_MODULE_REGISTER(prinl),
                             LISP_MODULE_REGISTER(print),
                             LISP_MODULE_REGISTER(printl),
                             LISP_MODULE_REGISTER(read),
                             LISP_MODULE_REGISTER(readline),
                             { NULL, NULL } };

const char* USED
lisp_module_name()
{
  return "io";
}

const module_entry_t* USED
lisp_module_entries()
{
  return ENTRIES;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
