PROG_OBJ = 	master.o \
		instance.o \
		config.o \
		work_handler.o \
		mana_handler.o \
		socket.o \

PROG = slave
LIB = -lpthread
LIB += $(SYNC_TOP_SRC)/lib/libbase.a
C_FLAGS = -g

include $(SYNC_TOP_SRC)/mk/Makefile.prog
