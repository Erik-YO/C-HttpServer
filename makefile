# Redes 2 - P1


SRC=./src
INC=./include
OBJ=./obj
SRCLIB=./srclib
LIB=./lib

CC=gcc
LIBCOMP=ar
FLAGS= -pedantic -Wall -g -I$(INC)
LIB_FLAGS=-rcs
THREADF=-pthread -lpthread -lrt

TEST_THREAD=test_hilos
MAIN_CLIENT=client
MAIN_SERVER=server
TEST_PROC=process_test

EXE=$(MAIN_SERVER)

OBJECT_FILES=$(OBJ)/process.o $(OBJ)/tcp.o $(OBJ)/hilos.o $(OBJ)/picohttpparser.o $(OBJ)/config.o

SERVER_LIB=libserver_lib

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



## FUNCIONALIDAD


# Funciones de conexion TCP
$(OBJ)/tcp.o: $(SRCLIB)/tcp.c $(INC)/tcp.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/tcp.c
	@echo " > $@ generated\n"


# Funciones de gestion de hilos
$(OBJ)/hilos.o: $(SRCLIB)/hilos.c $(INC)/hilos.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/hilos.c $(THREADF)
	@echo " > $@ generated\n"


# Funciones de parseo
$(OBJ)/picohttpparser.o: $(SRCLIB)/picohttpparser.c $(INC)/picohttpparser.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/picohttpparser.c
	@echo " > $@ generated\n"


# obj de process
$(OBJ)/process.o: $(SRCLIB)/process.c $(INC)/process.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/process.c
	@echo " > $@ generated\n"


# obj de config
$(OBJ)/config.o: $(SRCLIB)/config.c $(INC)/config.h
	$(CC) $(FLAGS) -o $@ -c $(SRCLIB)/config.c
	@echo " > $@ generated\n"









## EJECUTABLES

# obj de servidor
$(OBJ)/$(MAIN_SERVER).o: $(SRC)/$(MAIN_SERVER).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(MAIN_SERVER).c
	@echo " > $@ generated\n"

# obj de cliente de pruebas
$(OBJ)/$(MAIN_CLIENT).o: $(SRC)/$(MAIN_CLIENT).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(MAIN_CLIENT).c
	@echo " > $@ generated\n"

# obj de hilos pruebas
$(OBJ)/$(TEST_THREAD).o: $(SRC)/$(TEST_THREAD).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(TEST_THREAD).c
	@echo " > $@ generated\n"

# obj de test de process
$(OBJ)/$(TEST_PROC).o: $(SRC)/$(TEST_PROC).c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/$(TEST_PROC).c
	@echo " > $@ generated\n"






# Bibliotecas
$(LIB)/$(SERVER_LIB): $(OBJECT_FILES)
	$(LIBCOMP) $(LIB_FLAGS) $@.a $(OBJECT_FILES)
	ranlib $@.a








# Main de servidor
$(MAIN_SERVER): $(OBJ)/$(MAIN_SERVER).o $(LIB)/$(SERVER_LIB)
	$(CC) $(FLAGS) -o ./$@ $(OBJ)/$@.o -L$(LIB)/ -lserver_lib $(THREADF)
	@echo " > $@ generated\n"


# Main de cliente de pruebas
$(MAIN_CLIENT): $(OBJ)/$(MAIN_CLIENT).o $(LIB)/$(SERVER_LIB)
	$(CC) $(FLAGS) -o ./$@ $(OBJ)/$@.o -L$(LIB)/ -l$(SERVER_LIB) $(THREADF)
	@echo " > $@ generated\n"

# Main de hilos de pruebas
$(TEST_THREAD): $(OBJ)/$(TEST_THREAD).o $(LIB)/$(SERVER_LIB)
	$(CC) $(FLAGS) -o ./$@ $(OBJ)/$@.o -L$(LIB)/ -l$(SERVER_LIB) $(THREADF)
	@echo " > $@ generated\n"

# Main test process
$(TEST_PROC): $(OBJ)/$(TEST_PROC).o $(LIB)/$(SERVER_LIB)
	$(CC) $(FLAGS) -o ./$@ $(OBJ)/$@.o -L$(LIB)/ -l$(SERVER_LIB) $(THREADF)
	@echo " > $@ generated\n"





# obj main de pruebas
$(OBJ)/main.o: $(SRC)/main.c
	$(CC) $(FLAGS) -o $@ -c $(SRC)/main.c $(THREADF)
	@echo " > $@ generated\n"


# Main de pruebas
main: $(OBJ)/main.o $(LIB_FILES)
	$(CC) -o ./main $(OBJ)/main.o $(LIB_FILES) $(THREADF)
	@echo " > $@ generated\n"



clean:
	rm $(OBJ)/*.o $(LIB)/*.a ./$(MAIN_CLIENT) ./$(MAIN_SERVER) ./main ./$(TEST_THREAD) ./$(TEST_PROC)



valgrind_main: main
	valgrind --leak-check=full --show-leak-kinds=all ./main


valgrind_hilos: test_hilos
	valgrind --leak-check=full --show-leak-kinds=all ./test_hilos

valgrind_server: server
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

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

