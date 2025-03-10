CXX=g++

lexer: lexer.h lexer.cpp
	$(CXX) -o lexical.out compiler.cpp lexer.cpp token.cpp -std=c++11

runLexer: lexer
	./lexical.out