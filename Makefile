.PHONY: all cls compile run clean test

EXECUTABLE= readcc

CC=	g++
CFLAGS+= -W -Wall -pedantic
CXXFLAGS+= -std=c++0x
LIBS= -lnfc

SRC= main.cpp \
	 applicationhelper.cpp \
	 ccinfo.cpp \
	 tools.cpp

OBJ=$(SRC:.cpp=.o)

all: cls compile clean

cls:
	clear

compile: $(OBJ)
	$(CC) -o $(EXECUTABLE) $(OBJ) $(LIBS)

clean:
	rm -rf $(OBJ) $(NAME)

run:
	./$(EXECUTABLE)
