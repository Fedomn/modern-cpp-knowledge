/* The declarations here include C code to be copied to the beginning of the generated C parser */
%{
#include <stdio.h>
void yyerror(const char* msg) {
    fprintf(stderr, "%s yyerror\n", msg);
}
int yylex();
%}

/* declare tokens 
telling bison the names of the symbols in the parser that are tokens.
Any symbols not declared as tokens have to appear on the left side of at least one rule in the program.
*/
%token NUMBER
%token ADD SUB MUL DIV
%token EOL

%%
/* define grammar
In BNF, ::= can be read “is a” or “becomes,” and | is “or,”
Each symbol in a bison rule has a value; the value of the target
symbol (the one to the left of the colon) is called $$ in the action code,
and the values on the right are numbered $1, $2, and so forth, 
up to the number of symbols in the rule
*/
calclist: /* do nothing, matches at beginning of input */
    | calclist exp EOL { printf("= %d\n", $2); }
    ;

exp: factor  {$$ = $1;}
    | exp ADD factor { $$ = $1 + $3; }
    | exp SUB factor { $$ = $1 - $3; }
    ;

factor: term {$$ = $1;}
    | factor MUL term { $$ = $1 * $3; }
    | factor DIV term { $$ = $1 / $3; }
    ;

term: NUMBER {$$ = $1;}
    ;
%%

int main(int argc, char **argv)
{
    printf("A simple calculator.\n");
    yyparse();
    return 0;
}
