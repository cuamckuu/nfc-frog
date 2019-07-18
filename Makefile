NAME= readcc

SRC= main.cpp \
	 applicationhelper.cpp \
	 ccinfo.cpp \
	 tools.cpp

LIBS= -lnfc

OBJ=$(SRC:.cpp=.o)

CC=	g++

CFLAGS+= -W -Wall -pedantic

CXXFLAGS+= -std=c++0x

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LIBS)

all: $(NAME)

clean:
	rm -rf $(OBJ) $(NAME)

re:	clean all
