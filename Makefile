.phony all:

all: ACS

debug: CXXFLAGS += -DDEBUG -g
debug: CCFLAGS += -DDEBUG -g
debug: ACS

ACS: ACS.c
	gcc ACS.c -lhistory -pthread -o ACS -ggdb

.PHONY clean:
clean:
	-rm -rf *.o *.exe
