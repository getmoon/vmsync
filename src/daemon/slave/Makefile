PROG_OBJ = 	thread.o \
		socket.o \
		accept.o \
		child.o \
		config.o \
		file.o \
		slave.o
PROG = slave
LIB = -lpthread
LIB += $(SYNC_TOP_SRC)/lib/libbase.a
C_FLAGS = -g

include $(SYNC_TOP_SRC)/mk/Makefile.prog
