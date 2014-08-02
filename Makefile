
NAME=	cchack

SRC=	cchack.cc

LIBS=	-lnfc

OBJ=$(SRC:.cc=.o)

CC=	g++

CFLAGS+= -W -Wall -pedantic

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LIBS)

all: $(NAME)

clean:
	rm -rf $(OBJ) $(NAME)

re:	clean all
