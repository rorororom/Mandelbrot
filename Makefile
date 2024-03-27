# Флаги компилятора для g++
FLAGS   = -g -DCOUNT_FPS -O0 -lsfml-graphics -lsfml-system -lsfml-window
# имя исполняемого файла
NAME    = app
OBJS    = manda1.o
CC      = g++
AS      = nasm
AFLAGS  = -f macho64

${NAME}: ${OBJS}
	${CC} ${FLAGS} ${OBJS} -o ${NAME}

main1.o: manda1.cpp
	${CC} ${FLAGS} -c manda1.cpp

Time.o: Time.s
	${AS} ${AFLAGS} -o Time.o Time.s

clean:
	rm -f ${OBJS} ${NAME}
