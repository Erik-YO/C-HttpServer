# Redes 2 - P1


SRC=./src
INC=./include
OBJ=./obj
SRCLIB=./srclib
LIB=./lib

CC=gcc
LIBCOMP=ar
FLAGS= -ansi -pedantic -Wall -g -I$(INC)
LIB_FLAGS=-rcs
THREADF=-pthread -lpthread -lrt

TEST_THREAD=test_hilos
MAIN_CLIENT=client
MAIN_SERVER=server

EXE=$(MAIN_SERVER)

OBJECT_FILES=$(LIB)/tcp.a $(LIB)/hilos.a

#OBJECT_FILES=$(OBJ)/$(MAIN_SERVER).o $(OBJ)/tcp.o $(SRC)/fuente.o $(LIB)/libreria.a


all: $(EXE)



# Ejemplos de compilacion:
#
# src de librerias
#$(OBJ)/xxxx.o: $(SRCLIB)/xxxx.c $(INC)/xxxx.h
#	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/xxxx.c
#
#$(LIB)/xxxx.a: $(OBJ)/xxxx.o
#	$(LIBCOMP) $(LIB_FLAGS) $(LIB)/xxxx.a $(OBJ)/xxxx.o
#
# src estandar
#$(OBJ)/xxxx.o: $(SRC)/xxxx.c $(INC)/xxxx.h
#	$(CC) $(FLAGS) -o $@ -c $(SRC)/xxxx.c
#






# Funciones de conexion TCP
$(OBJ)/tcp.o: $(SRCLIB)/tcp.c $(INC)/tcp.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/tcp.c

$(LIB)/tcp.a: $(OBJ)/tcp.o
	$(LIBCOMP) $(LIB_FLAGS) $(LIB)/tcp.a $(OBJ)/tcp.o



# Funciones de gestion de hilos
$(OBJ)/hilos.o: $(SRCLIB)/hilos.c $(INC)/hilos.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/hilos.c $(THREADF)

$(LIB)/hilos.a: $(OBJ)/hilos.o
	$(LIBCOMP) $(LIB_FLAGS) $(LIB)/hilos.a $(OBJ)/hilos.o





# obj de servidor
$(OBJ)/$(MAIN_SERVER).o: $(SRC)/$(MAIN_SERVER).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(MAIN_SERVER).c $(THREADF)
	@echo "$(MAIN_SERVER).o generated"

# obj de cliente de pruebas
$(OBJ)/$(MAIN_CLIENT).o: $(SRC)/$(MAIN_CLIENT).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(MAIN_CLIENT).c $(THREADF)
	@echo "$(MAIN_CLIENT).o generated"

# obj de hilos pruebas
$(OBJ)/$(TEST_THREAD).o: $(SRC)/$(TEST_THREAD).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(TEST_THREAD).c $(THREADF)
	@echo "$(TEST_THREAD).o generated"



# Main de servidor
$(MAIN_SERVER): $(OBJ)/$(MAIN_SERVER).o $(OBJECT_FILES)
	$(CC) -o ./$(MAIN_SERVER) $(OBJ)/$(MAIN_SERVER).o $(OBJECT_FILES) $(THREADF)
	@echo "$(MAIN_SERVER) generated"

# Main de cliente de pruebas
$(MAIN_CLIENT): $(OBJ)/$(MAIN_CLIENT).o $(OBJECT_FILES)
	$(CC) -o ./$(MAIN_CLIENT) $(OBJ)/$(MAIN_CLIENT).o $(OBJECT_FILES) $(THREADF)
	@echo "$(MAIN_CLIENT) generated"

# Main de hilos de pruebas
# DUDA: ¿quitamos /hilos.o para crear test_hilos? (ya que está hilos.a)
$(TEST_THREAD): $(OBJ)/$(TEST_THREAD).o $(OBJECT_FILES) $(OBJ)/hilos.o
	$(CC) -o ./$(TEST_THREAD) $(OBJ)/$(TEST_THREAD).o $(OBJECT_FILES) $(THREADF)
	@echo "$(TEST_THREAD) generated"




# obj main de pruebas
$(OBJ)/main.o: $(SRC)/main.c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/main.c $(THREADF)
	@echo "main.o generated"


# Main de pruebas
main: $(OBJ)/main.o $(OBJECT_FILES)
	$(CC) -o ./main $(OBJ)/main.o $(OBJECT_FILES) $(THREADF)
	@echo "main generated"



clean:
	rm $(OBJ)/*.o $(LIB)/*.a ./$(MAIN_CLIENT) ./$(MAIN_SERVER) ./main



valgrind: main
	valgrind --leak-check=full --show-leak-kinds=all ./main


#
#$(OBJ)/$(MAIN_CLIENT).o: $(SRC)/$(MAIN_CLIENT).c
#	$(CC) $(FLAGS) -I$(INC) -c $(SRC)/$(MAIN_CLIENT).c -o $(OBJ)/$(MAIN_CLIENT).o
#
#	valgrind --leak-check=full ./$(MAIN_SERVER)
#
#$(OBJ)/$(MAIN_SERVER).o: $(SRC)/$(MAIN_SERVER).c
#	$(CC) $(FLAGS) -I$(INC) -c $(SRC)/$(MAIN_SERVER).c -o $(OBJ)/$(MAIN_SERVER).o
#


##$(OBJ)/box.o: $(SRC)/box.c $(INC)/box.h $(INC)/types.h
##	gcc $(FLAGS) -I$(INC) -c $(SRC)/box.c -o $(OBJ)/box.o

