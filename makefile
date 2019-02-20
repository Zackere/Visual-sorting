CC=clang
SFML=/usr/local
CFLAGS=-Wall -Werror -pedantic -std=c++11
LFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lstdc++ -lpthread
TARGET=sfml-app
FILES=main.o

${TARGET}: ${FILES}
	${CC} ${FILES} -o ${TARGET} -L${SFML}/lib ${LFLAGS}

main.o: main.cpp
	${CC} -c main.cpp -I${SFML}/include ${CFLAGS}

.PHONY: clean

clean:
	-rm -f ${TARGET} ${FILES}
