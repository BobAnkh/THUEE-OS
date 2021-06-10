build:
	g++ quicksort.cpp -o quicksort.exe -std=c++11 -lpthread

generate:
	g++ generate.cpp -o generate.exe -std=c++11; ./generate.exe

check:
	g++ check.cpp -o check.exe -std=c++11; ./check.exe

.PHONY: clean
clean distclean:
	rm -f *.exe