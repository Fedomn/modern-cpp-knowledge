build:
	# translates the scanner into a C program called lex.yy.c
	flex wc.l
	# ^D is a Unix/Linux end-of-file character
	cc -ll lex.yy.c -o wc

clean:
	rm -rf lex.yy.c wc
