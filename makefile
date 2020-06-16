mensajeria: mensajeria.o md5.o
	g++ -o mensajeria mensajeria.o md5.o
md5.o: md5.cpp md5.h
	g++ -c md5.cpp
mensajeria.o: mensajeria.cpp md5.h
	g++ -c mensajeria.cpp
clean:
	rm -f *.o
	rm -f mensajeria md5