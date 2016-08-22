CC=g++
CFLAGS= -g -Wall -Werror

INC=-I./include
SRC=./src
OBJ=bin

SRCFILES=$(addprefix $(OBJ)/, $(subst .c,.o, $(subst .cpp,.o, $(subst $(SRC)/,,$(wildcard $(SRC)/*)))))

print-%  : ; @echo $* = $($*)

all:
	make clean
	mkdir $(OBJ)
	make proxy

proxy: $(SRCFILES)
	mkdir -p $(OBJ)
	$(CC) $(LDFLAGS) -o proxy $(SRCFILES) $(LDLIBS) $(INC)

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(DEBUG) -c $< -o $@ $(INC)

$(OBJ)/%.o: $(SRC)/%.cpp
	mkdir -p $(OBJ)
	$(CC) $(DEBUG) -c $< -o $@ $(INC)

clean:
	rm -rf $(OBJ)
	rm -f proxy

tar:
	tar -cvzf ass1.tgz src include README Makefile
	#tar -cvzf ass1.tgz proxy.c README Makefile proxy_parse.c proxy_parse.h
