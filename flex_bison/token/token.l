%{
  enum tokenType {
    NUMBER = 1,
    ADD = 2,
    SUB = 3,
    MUL = 4,
    DIV = 5,
    EOL = 6
  };
  int val = -1;
%}

%%
"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
[0-9]+ { val = atoi(yytext); return NUMBER; }
\n { return EOL; }
[ \t] { /* ignore whitespace */ }
. { printf("Mystery character %c\n", *yytext); }
%%

int main(int argc, char **argv)
{
  int token;
  while((token = yylex())) {
    printf("token=%d, ", token);
    /* if(token == NUMBER) { */
      printf("val=%d\n", val);
    /* } else { */
      /* printf("\n"); */
    /* } */
    val = -1;
  }
  return 0;
}
