run: build
	./lock

build:
	g++ -O3 -Wall -std=c++11 main.cpp -o lock

clean:
	rm lock
