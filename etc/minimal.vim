" Minima.l syntax file
" Language: Minima.l 
" Maintainer: Xavier Guerin
" Last Change: 2018 Jan 18

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
    syntax clear
elseif exists("b:current_syntax")
    finish
endif

" Minima.l is case sensitive.
syn case match

setl iskeyword+=?,+,*,/,\\,=,>,<,_

syn match  MinimalComment     /#.*$/
syn match  MinimalNumber      /\v<[-+]?\d+(\.\d+)?>/
syn region MinimalString      start=/"/ skip=/\\\\\|\\"/ end=/"/
syn match  MinimalParentheses /[()\[\]]/

syn keyword MinimalSpecial  NIL T _ @ @@ @@@

syn keyword MinimalFuncs match ? ?: ?!
syn keyword MinimalFuncs + - * / = <> < <= > >= and or not
syn keyword MinimalFuncs prog atm? nil? lst? num? chr? sym?
syn keyword MinimalFuncs car cdr conc cons def eval let list load
syn keyword MinimalFuncs in out prin prinl print printl read
syn keyword MinimalFuncs quote setq sym

syn keyword MinimalDebug trace closure

hi default link MinimalComment Comment
hi default link MinimalCommentRegion Comment

hi default link MinimalNumber   Number
hi default link MinimalString   String
hi default link MinimalSpecial  Constant
hi default link MinimalCond     Conditional
hi default link MinimalFuncs    Function
hi default link MinimalOperator Operator
hi default link MinimalDebug    Type

set lisp

set lispwords=
set lispwords+=prog,match,?,?:,?!,+,-,*,/,\,=,<>,<,<=,>,>=
set lispwords+=atm?,lst?,num?,chr?,nil?,sym?
set lispwords+=car,cdr,conc,cons,def,eval,let,list,load
set lispwords+=in,line,out,prin,prinl,print,printl,read
set lispwords+=quote,setq

let b:current_syntax = "minimal"
