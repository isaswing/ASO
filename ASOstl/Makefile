CXX     = g++
RE2C	= ./bin/re2c
OS     ?= WIN

CFLAGS	= -Wall -g -fpermissive -I./lib/
LDFLAGS	= 

ifeq ($(OS),WIN)
LIBS    = -lwinmm
else
LIBS	=
endif

TARGETS	= ASO

all: ${TARGETS}

clean:
	rm -f *.o
	rm -f ${TARGETS}

parsePref.cpp: parsePref.re .FORCE
	${RE2C} -o $@ $<

parsePrefQBF.cpp: parsePrefQBF.re .FORCE
	${RE2C} -o $@ $<

.FORCE:

%.o:%.cpp
	${CXX} ${CFLAGS} -o $@ -c $<

ASO: Formula.o ASPSolver.o QBFSolver.o parsePref.o parsePrefQBF.o main.o
	${CXX} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}
