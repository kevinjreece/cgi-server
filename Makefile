all: download

download: main.cpp
	g++ -std=c++11 main.cpp -o server

clean:
	rm -r !(*.cpp|*.h|Makefile)
