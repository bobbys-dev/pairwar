pairwar: main.cpp
	g++ -Wall -o pairwar.o main.cpp -lpthread

clean:
	rm -rf pairwar.o
