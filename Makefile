.PHONY: all cls compile run clean test

EXECUTABLE= nfc-frog

CXX= g++
CXXFLAGS+= -std=c++11 -W -Wall -pedantic
LIBS= -lnfc

SRC= main.cpp \
	 device_nfc.cpp

OBJ=$(SRC:.cpp=.o)

all: cls compile clean

cls:
	clear

compile: $(OBJ)
	$(CXX) -o $(EXECUTABLE) $(OBJ) $(LIBS)

clean:
	rm -rf $(OBJ) $(NAME)

run:
	./$(EXECUTABLE)
