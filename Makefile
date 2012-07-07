# Should export SYNC_TOP_SRC=`pwd`

default: clean
	make -C src
clean:
	rm -rf lib/*
	rm -rf installing/bin/*
	make -C src clean

install:
	@echo 'Ready to pack libsync.a, libsync.so, fsync.h'
	make -C src install
