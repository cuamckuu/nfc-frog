
NAME=	readcc

SRC=	cchack.cc \
	application.cc \
	ccinfo.cc \
	misc.cc

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
