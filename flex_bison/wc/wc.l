/* declarations, 在声明部分，%{ 和 %} 内的代码被逐字复制到生成的 C 源文件开头附近。*/
%{
  int chars = 0;
  int words = 0;
  int lines = 0;
%}
/* 
a list of patterns and actions，pattern 必须在行开头。
yytext is set to point to the input text that the pattern just matched.
*/
%%
[a-zA-Z]+ { words++; chars += strlen(yytext); }
\n { chars++; lines++; }
•  { chars++; }
%%
/* C code that is copied to the generated scanne */
int main(int argc, char **argv)
{
  yylex();
  printf("lines=%d words=%d chars=%d\n", lines, words, chars);
  return 0;
}
