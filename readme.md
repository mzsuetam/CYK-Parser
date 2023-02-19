# CYK algorithm

Cocke–Younger–Kasami algorithm is a parsing algorithm
for context-free grammars.

This (standard) version of CYK operates on
context-free grammars given in Chomsky normal form (CNF).

### Context free grammar

G = (T,N,P,S) \
T - set of terminals \
N - set of non-terminals \
P - set of productions \
S - start symbol

### Grammar file

By default, grammar file is read from ./grammar.txt.
The path can be specified by flag  _-path_ with a path to file as parameter

File consist of four lines:
 
1. \<size T> \<size of N> \<size of P> S
2. \<list of terminals separated by space>
3. \<list of non-terminals separated by space>
4. \<list of productions separated by space>

Terminal _t_ is represented by single [a-z] char, \
a non-terminal _N_ is represented by a single [A-Z] char \
and production are represented by _N_>(_NN_|_t_|) string \
(where _N>_ is interpreted as epsilon production)