
NAME=	readnfccc

SRC=	readnfccc.c

LIBS=	-lnfc

OBJ=$(SRC:.c=.o)

CC=	gcc

CFLAGS+=	-std=gnu99 -W -Wall -pedantic

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LIBS)

all: $(NAME)

clean:
	rm -rf $(OBJ) $(NAME)

re:	clean all
