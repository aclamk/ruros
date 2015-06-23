.PHONY: clean
all: gen

gen.tab.c gen.tab.h: gen.y
	bison --defines $<

lex.yy.c: gen.l
	flex gen.l

gen.tab.o: gen.tab.c
	g++ -c $< -o $@

lex.yy.o: lex.yy.c
	g++ -c $< -o $@

gen.o: gen.cpp
	g++ -g -c $< -o $@

gen: lex.yy.o gen.o
	g++ $^ -o $@
	
clean:
	rm -f gen.tab.c gen.tab.h lex.yy.o lex.yy.c gen.o gen
