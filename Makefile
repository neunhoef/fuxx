all: fuxx

fuxx:	fuxx.cpp Makefile
	g++ fuxx.cpp -o fuxx -Wall -O3 -std=c++11
