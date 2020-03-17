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

set iskeyword+=%,?,+,*,/,\\,=,>,<,_,:,&,!,\|,\$

syn case match

syn match  MinimalCharacter   /\^./
syn match  MinimalComment     /#.*$/
syn match  MinimalNumber      /\v<[-]?\d+(\.\d+)?>/
syn region MinimalString      start=/"/ skip=/\\\\\|\\"/ end=/"/
syn match  MinimalParentheses /[()]/

syn keyword MinimalSpecial  NIL T _ ' ` ~ ARGV CONFIG ENV

syn keyword MinimalFuncs
      \ %
      \ *
      \ +
      \ -
      \ /
      \ <
      \ <-
      \ <=
      \ <>
      \ <\|>
      \ =
      \ >
      \ >=
      \ >&
      \ \\
      \ \|>
      \ $+
      \ and
      \ append
      \ assoc
      \ atm?
      \ caar
      \ cddr
      \ cadar
      \ caddr
      \ cadr
      \ car
      \ cdar
      \ cdr
      \ chr
      \ chr?
      \ close
      \ conc
      \ cond
      \ cons
      \ dedup
      \ def
      \ erase
      \ eval
      \ filter
      \ flatten
      \ foldl
      \ foldr
      \ has
      \ if
      \ in
      \ insert
      \ iter
      \ iter2
      \ len
      \ let
      \ last
      \ list
      \ load
      \ lst?
      \ map
      \ map2
      \ match
      \ merge
      \ nil?
      \ not
      \ ntoa
      \ num?
      \ or
      \ out
      \ prin
      \ prinl
      \ print
      \ printl
      \ prog
      \ quit
      \ quote
      \ read
      \ replc
      \ readline
      \ rep
      \ rev
      \ setq
      \ split
      \ str
      \ str?
      \ sym
      \ sym?
      \ time
      \ truncl
      \ truncr
      \ unless
      \ unlink
      \ tru?
      \ when
      \ while
      \ zip

hi default link MinimalComment Comment
hi default link MinimalCommentRegion Comment

hi default link MinimalCharacter  Character
hi default link MinimalNumber     Number
hi default link MinimalString     String
hi default link MinimalSpecial    Constant
hi default link MinimalCond       Conditional
hi default link MinimalFuncs      Function
hi default link MinimalOperator   Operator

set lisp
set lispwords=%,*,+,-,/,<,<-,<=,<>,<\|>,=,>,>=,>&,\\,\|>,$+,and,append,assoc,atm?,caar,cddr,cadar,caddr,cadr,car,cdar,cdr,chr,chr?,close,conc,cond,cons,dedup,def,dup,eval,filter,flatten,foldl,foldr,has,if,in,insert,iter,iter2,last,len,let,list,load,lst?,map,map2,match,merge,nil?,not,ntoa,num?,or,out,prin,prinl,print,printl,prog,quit,quote,read,readline,rep,rev,setq,split,str,str?,sym,sym?,time,truncl,truncr,unless,unlink,tru?,when,while,zip

let b:current_syntax = "minimal"
