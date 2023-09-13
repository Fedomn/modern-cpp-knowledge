build:
	bison -d calculator.y
	flex calculator.l
	cc -o calculator calculator.tab.c lex.yy.c -ll

clean:
	rm -rf calculator.tab.*
	rm -rf ./calculator ./lex.yy.c
