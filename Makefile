#
# Makefile for compire
#
OBJS = main.o lexical.o scan.o parse.o
PP = pp

# ${TC} : ${OBJS}
# 	${CC} ${OBJS} -o $@

${PP} : ${OBJS}
	${CC} ${OBJS} -o $@
	
clean:
	${RM} *.o ${TC} ${PP} 