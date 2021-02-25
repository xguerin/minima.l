#include <mnml/module.h>

LISP_MODULE_DECL(car);
LISP_MODULE_DECL(cdr);
LISP_MODULE_DECL(chr);
LISP_MODULE_DECL(conc);
LISP_MODULE_DECL(cond);
LISP_MODULE_DECL(cons);
LISP_MODULE_DECL(def);
LISP_MODULE_DECL(eval);
LISP_MODULE_DECL(if);
LISP_MODULE_DECL(isatm);
LISP_MODULE_DECL(ischr);
LISP_MODULE_DECL(islst);
LISP_MODULE_DECL(isnil);
LISP_MODULE_DECL(isnum);
LISP_MODULE_DECL(isstr);
LISP_MODULE_DECL(issym);
LISP_MODULE_DECL(istru);
LISP_MODULE_DECL(lambda);
LISP_MODULE_DECL(len);
LISP_MODULE_DECL(let);
LISP_MODULE_DECL(list);
LISP_MODULE_DECL(load);
LISP_MODULE_DECL(match);
LISP_MODULE_DECL(ns);
LISP_MODULE_DECL(prog);
LISP_MODULE_DECL(quit);
LISP_MODULE_DECL(quote);
LISP_MODULE_DECL(set);
LISP_MODULE_DECL(setq);
LISP_MODULE_DECL(str);
LISP_MODULE_DECL(stream);
LISP_MODULE_DECL(sym);
LISP_MODULE_DECL(unless);
LISP_MODULE_DECL(use);
LISP_MODULE_DECL(when);
LISP_MODULE_DECL(while);

module_entry_t ENTRIES[] = { LISP_MODULE_REGISTER(car),
                             LISP_MODULE_REGISTER(cdr),
                             LISP_MODULE_REGISTER(chr),
                             LISP_MODULE_REGISTER(conc),
                             LISP_MODULE_REGISTER(cond),
                             LISP_MODULE_REGISTER(cons),
                             LISP_MODULE_REGISTER(def),
                             LISP_MODULE_REGISTER(eval),
                             LISP_MODULE_REGISTER(if),
                             LISP_MODULE_REGISTER(isatm),
                             LISP_MODULE_REGISTER(ischr),
                             LISP_MODULE_REGISTER(islst),
                             LISP_MODULE_REGISTER(isnil),
                             LISP_MODULE_REGISTER(isnum),
                             LISP_MODULE_REGISTER(isstr),
                             LISP_MODULE_REGISTER(issym),
                             LISP_MODULE_REGISTER(istru),
                             LISP_MODULE_REGISTER(lambda),
                             LISP_MODULE_REGISTER(len),
                             LISP_MODULE_REGISTER(let),
                             LISP_MODULE_REGISTER(list),
                             LISP_MODULE_REGISTER(load),
                             LISP_MODULE_REGISTER(match),
                             LISP_MODULE_REGISTER(ns),
                             LISP_MODULE_REGISTER(prog),
                             LISP_MODULE_REGISTER(quit),
                             LISP_MODULE_REGISTER(quote),
                             LISP_MODULE_REGISTER(set),
                             LISP_MODULE_REGISTER(setq),
                             LISP_MODULE_REGISTER(str),
                             LISP_MODULE_REGISTER(stream),
                             LISP_MODULE_REGISTER(sym),
                             LISP_MODULE_REGISTER(unless),
                             LISP_MODULE_REGISTER(use),
                             LISP_MODULE_REGISTER(when),
                             LISP_MODULE_REGISTER(while),
                             { NULL, NULL } };

const char* USED
lisp_module_name()
{
  return "std";
}

const module_entry_t* USED
lisp_module_entries()
{
  return ENTRIES;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
