CC=g++
CFLAGS= -g -Wall -Werror

INC=-I./include
SRC=./src
OBJ=bin

# Available flags:
# DEBUG: DEBUG outputs from my code
# DEBUG_PROXY: DEBUG outputs from the request parsing
#              library
# VERBOSE: Prints some more info
DEBUG=

SRCFILES=$(addprefix $(OBJ)/, $(subst .c,.o, $(subst .cpp,.o, $(subst $(SRC)/,,$(wildcard $(SRC)/*)))))

print-%  : ; @echo $* = $($*)

all:
	make clean
	mkdir $(OBJ)
	make proxy

proxy: $(SRCFILES)
	mkdir -p $(OBJ)
	$(CC) $(LDFLAGS) -o hop $(SRCFILES) $(LDLIBS) $(INC)

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(DEBUG) -c $< -o $@ $(INC)

$(OBJ)/%.o: $(SRC)/%.cpp
	mkdir -p $(OBJ)
	$(CC) $(DEBUG) -c $< -o $@ $(INC)

clean:
	rm -rf $(OBJ)
	rm -f hop
