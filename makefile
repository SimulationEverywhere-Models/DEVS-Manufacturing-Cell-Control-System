CC=g++
CFLAGS=-std=c++17

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../DESTimes/include

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER (NOT SIMULATOR)
message.o: data_structures/message.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) data_structures/message.cpp -o build/message.o

#MCCS MAIN
main_top.o: top_model/main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main.cpp -o build/main_top.o

#HANDLING
main_handling_test.o: test/main_handling_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_handling_test.cpp -o build/main_handling_test.o

#STORAGE
main_storage_test.o: test/main_storage_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_storage_test.cpp -o build/main_storage_test.o

#CONTROL
main_control_test.o: test/main_control_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_control_test.cpp -o build/main_control_test.o


#COUPLED:INVENTORY_HANDLER(IH)
main_inventory_handler_test.o: test/main_inventory_handler_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_inventory_handler_test.cpp -o build/main_inventory_handler_test.o


#TESTS
tests: main_handling_test.o main_storage_test.o main_control_test.o main_inventory_handler_test.o message.o
		$(CC) -g -o bin/HANDLING_TEST build/main_handling_test.o build/message.o 
		$(CC) -g -o bin/STORAGE_TEST build/main_storage_test.o build/message.o 
		$(CC) -g -o bin/CONTROL_TEST build/main_control_test.o build/message.o 
		$(CC) -g -o bin/IH_TEST build/main_inventory_handler_test.o build/message.o

#SINGLE TESTS
handling_test: main_handling_test.o message.o
		$(CC) -g -o bin/HANDLING_TEST build/main_handling_test.o build/message.o 

storage_test: main_storage_test.o message.o
		$(CC) -g -o bin/STORAGE_TEST build/main_storage_test.o build/message.o 

control_test: main_control_test.o message.o
		$(CC) -g -o bin/CONTROL_TEST build/main_control_test.o build/message.o 
inventory_handler_test: main_inventory_handler_test.o message.o
		$(CC) -g -o bin/IH_TEST build/main_inventory_handler_test.o build/message.o


#TARGET TO COMPILE ONLY MCCS SIMULATOR
simulator: main_top.o message.o 
	$(CC) -g -o bin/MCCS build/main_top.o build/message.o 

#TARGET TO COMPILE EVERYTHING (ABP SIMULATOR + TESTS TOGETHER)
all: tests simulator

#TARGET ONLY SELECTED TESTS
control: control_test
storage: storage_test
handling: handling_test
ih: inventory_handler_test


#CLEAN COMMANDS
clean: 
	rm -f bin/* build/*