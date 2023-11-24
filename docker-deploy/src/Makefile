CC=g++

EXTRAFLAGS=-lpqxx -lpq -lpthread


main: main.cpp response.hpp db_func.cpp db_func.hpp tinyxml2.h tinyxml2.cpp request.hpp 
	$(CC) -std=c++11 -g -o $@ db_func.cpp  tinyxml2.cpp main.cpp $(EXTRAFLAGS) 


.PHONY:
	clean
clean:
	rm -rf *.o *.cpp~ *.hpp~ *# main