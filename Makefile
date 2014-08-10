
NAME=	readcc

SRC=	main.cc \
	application.cc \
	ccinfo.cc \
	tools.cc

LIBS=	-lnfc

OBJ=$(SRC:.cc=.o)

CC=	g++

CFLAGS+= -W -Wall -pedantic

CXXFLAGS+=	-std=c++0x

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LIBS)

all: $(NAME)

clean:
	rm -rf $(OBJ) $(NAME)

re:	clean all
