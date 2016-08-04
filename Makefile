SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,%.o,$(SRC))


all: $(OBJ)
	gcc $(OBJ) -o bin/traceroute
	rm -rf *.o

$(OBJ):	$(SRC)
	gcc $(SRC) -c

clean:
	rm -rf *.o bin/*

run:
	sudo ./bin/traceroute 8.8.8.8