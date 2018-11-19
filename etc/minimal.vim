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

set iskeyword+=?,+,*,\\,=,>,<,_,:

syn case match

syn match  MinimalComment     /#.*$/
syn match  MinimalNumber      /\v<[-]?\d+(\.\d+)?>/
syn region MinimalString      start=/"/ skip=/\\\\\|\\"/ end=/"/
syn match  MinimalParentheses /[()\[\]]/

syn keyword MinimalSpecial  NIL T _ ' ` ENV ARGV

syn keyword MinimalFuncs
      \ *
      \ +
      \ -
      \ /
      \ <
      \ <=
      \ <>
      \ =
      \ >
      \ >=
      \ \\
      \ and
      \ append
      \ assoc
      \ atm?
      \ caar
      \ cadar
      \ caddr
      \ cadr
      \ car
      \ cdar
      \ cdr
      \ chr?
      \ conc
      \ cond
      \ cons
      \ close
      \ dedup
      \ def
      \ dup
      \ eval
      \ exec
      \ exit
      \ filter
      \ flatten
      \ foldl
      \ foldr
      \ fork
      \ if
      \ in
      \ insert
      \ len
      \ let
      \ list
      \ load
      \ lst?
      \ map
      \ map2
      \ match
      \ merge
      \ nil?
      \ not
      \ num?
      \ or
      \ out
      \ pipe
      \ prin
      \ prinl
      \ print
      \ printl
      \ prog
      \ quote
      \ read
      \ readlines
      \ rev
      \ run
      \ setq
      \ sym
      \ sym?
      \ tru?
      \ wait
      \ zip

hi default link MinimalComment Comment
hi default link MinimalCommentRegion Comment

hi default link MinimalNumber   Number
hi default link MinimalString   String
hi default link MinimalSpecial  Constant
hi default link MinimalCond     Conditional
hi default link MinimalFuncs    Function
hi default link MinimalOperator Operator

set lisp
set lispwords=*,+,-,/,<,<=,<>,=,>,>=,\\,and,append,assoc,atm?,caar,cadar,caddr,cadr,car,cdar,cdr,chr?,conc,cond,cons,close,dedup,def,dup,eval,exec,exit,filter,flatten,foldl,foldr,fork,if,in,insert,len,let,list,load,lst?,map,map2,match,merge,nil?,not,num?,or,out,pipe,prin,prinl,print,printl,prog,quote,read,readlines,rev,run,setq,sym,sym?,tru?,wait,zip

let b:current_syntax = "minimal"
