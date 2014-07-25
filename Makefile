
NAME=	readnfccc

SRC=	readnfccc.c

LIBS=	-L./libnfc -lnfc

OBJ=$(SRC:.c=.o)

$(NAME): $(OBJ)
	gcc -o $(NAME) $(OBJ) $(LIBS)

all: $(NAME)

clean:
	rm -rf $(OBJ) $(NAME)

re:	clean all
