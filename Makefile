#
# Makefile for compire
#
CR = cr
OBJS = main.o lexical.o scan.o id-list.o parse.o 

${CR} : ${OBJS}
	${CC} ${OBJS} -o $@
	
clean:
	${RM} *.o ${CR}