This folder contains the Manufacturing Cell Control System (MCCS) DEVS model implemented in Cadmium. This project was done in a windows machine and all compilations were done using Cygwin.

/**************************/
/****FILES ORGANIZATION****/
/**************************/
README.txt	
MCCS.doc
MCCS.pdf
makefile
MCCS_DEVSmodelsForm.docx
MCCS_ModelDescription.xml


/*******************/
/***** Folders *****/
/*******************/
atomics [This folder contains three atomic models implemented in Cadmium]
	control.hpp
	storage.hpp
	handling.hpp
bin 	[This folder will be created automatically the first time you compile the poject.
     	It will contain all the executables]
build 	[This folder will be created automatically the first time you compile the poject.
       	It will contain all the build files (.o) generated during compilation]
data_structures [This folder contains message data structure used in the model]
	message.hpp
	message.cpp
input_data [This folder contains all the input data to run the model and the tests]
	MCCS_input_test_startIn.txt
	InventoryHandler_input_test_loadIn.txt
	InventoryHandler_input_test_prepIn.txt
	sender_input_test_ack_In.txt
	control_input_test_startIn.txt
	control_input_test_loadedIn.txt
	control_input_test_unloadedIn.txt
	storage_input_test_loadIn.txt
	storage_input_test_unloadIn.txt
	handling_input_test.txt
simulation_results [This folder will be created automatically the first time you compile the poject.
                    It will store the outputs from your simulations and tests]
test [This folder contains the unit test of all the atomic models and the Inventory handler coupled model]
	main_control_test.cpp
	main_storage_test.cpp
	main_handling_test.cpp
	main_inventory_handler_test.cpp
top_model [This folder contains the MCCS top model]	
	main.cpp
	

/*************/
/****STEPS****/
/*************/
0 - MCCS.doc contains the description, behavior, and test cases for each component of the Model

1 - Update include path in the makefile in this folder and subfolders. You need to update the following lines:
	INCLUDECADMIUM=-I ../../cadmium/include
	INCLUDEDESTIMES=-I ../../DESTimes/include
    Update the relative path to cadmium/include from the folder where the makefile is. You need to take into account where you copied the folder during the installation process
	Example: INCLUDECADMIUM=-I ../../cadmium/include
	Do the same for the DESTimes library
    NOTE: if you follow the step by step installation guide you will not need to update these paths.

2 - Compile the project and the tests
	1 - Open the terminal (Ubuntu terminal for Linux and Cygwin for Windows) in the Project folder
	2 - To compile only individual tests, type in the terminal
			make clean; make simulator  --> to complile only the MCCS.exe file
			make clean; make ih  --> to complile only the IH_TEST.exe file
			make clean; make control  --> to complile only the CONTROL_TEST.exe file
			make clean; make storage  --> to complile only the STORAGE_TEST.exe file
			make clean; make handling  --> to complile only the HANDLING_TEST.exe file
	3 - To compile the entire project and all the tests, type in the terminal:
			make clean; make all

3 - Run individual tests
	1 - Open the terminal in the bin folder. 
	2 - To run the test, type in the terminal "./NAME_OF_THE_COMPILED_FILE" (For windows, "./NAME_OF_THE_COMPILED_FILE.exe"). 
		For testing the control agent you need to type:
			./CONTROL_TEST (or ./CONTROL_TEST.exe for Windows)
		For testing the storage agent you need to type:
			./STORAGE_TEST (or ./STORAGE_TEST.exe for Windows)
		For testing the handling agent you need to type:
			./HANDLING_TEST (or ./HANDLING_TEST.exe for Windows)
		For testing the inventory handler you need to type:
			./IH_TEST (or ./IH_TEST.exe for Windows)
	3 - To check the output of the control test, go to the folder simulation_results and open  
			"Control_test_output_messages.txt" and "Control_test_output_state.txt"
		For others, check for corrsponding names in the text files.
			
5 - Run the top MCCS model
	1 - Open the terminal (Ubuntu terminal for Linux and Cygwin for Windows) in the bin folder.
	3 - To run the model, type in the terminal "./NAME_OF_THE_COMPILED_FILE NAME_OF_THE_INPUT_FILE". For this test you need to type:
		./MCCS ../input_data/MCCS_input_test_startIn.txt (for Windows: ./MCCS.exe ../input_data/MCCS_input_test_startIn.txt)
	4 - To check the output of the model, go to the folder simulation_results and open 
		"MCCS_main_test_output_messages.txt" and "MCCS_main_test_output_state.txt"
	5 - To run the model with different inputs
		5.1. Create new .txt files with the same structure as MCCS_input_test_startIn.txt in the folder input_data
		5.2. Run the model using the instructions in step 3
		5.3. If you want to keep the output, rename "MCCS_main_test_output_messages.txt" and "MCCS_main_test_output_state.txt". Otherwise it will be overwritten when you run the next simulation.

/*** Results for the simulations done and explained in the report were moved to a folder named "old_results" inside the "simulation_results" directory ***/

