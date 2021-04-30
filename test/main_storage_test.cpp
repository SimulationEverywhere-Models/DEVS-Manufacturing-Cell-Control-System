//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

//Time class header
#include <NDTime.hpp>

//Messages structures
#include "../data_structures/message.hpp"

//Atomic model headers
#include "../atomics/storage.hpp"
#include <cadmium/basic_model/pdevs/iestream.hpp> 	//Atomic model for inputs

//C++ libraries
#include <iostream>
#include <string>

//Namespaces
using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;
using TIME = NDTime;

/***** (1) *****/
/***** Define input port for coupled models *****/

/***** Define output ports for coupled model *****/
struct top_out_loaded: public out_port<Message_t>{};
struct top_out_unloaded: public out_port<Message_t>{};


/***** (2) *****/
/****** Input Reader atomic model declaration ******/
template<typename T>
class InputReader_Message_t : public iestream_input<Message_t, T>{		//Declare an atomic  class InputReader_Message_t
																		//inherits all methods of iestreaminput
public:
	InputReader_Message_t() = default;
	InputReader_Message_t(const char* file_path) 			//path to the text file where model inputs are defined
						: iestream_input<Message_t, T> (file_path){}
};

/***** (3) *****/
/***** Create the main function *****/
int main (){
	/****** Input Reader atomic model instantiation ******/
	// hardcode the path to the input file into a pointer
	const char *i_input_data_load = "../input_data/Storage_input_test_loadIn.txt";
	//create a shared pointer to hold the instantiation
	shared_ptr<dynamic::modeling::model> input_reader_load;
	//create a new class instantiation
	input_reader_load = dynamic::translate::make_dynamic_atomic_model
					<InputReader_Message_t, TIME, const char*>("input_reader_load", move(i_input_data_load));
					
	const char *i_input_data_unload = "../input_data/Storage_input_test_unloadIn.txt";
	shared_ptr<dynamic::modeling::model> input_reader_unload;
	input_reader_unload = dynamic::translate::make_dynamic_atomic_model
					<InputReader_Message_t, TIME, const char*>("input_reader_unload", move(i_input_data_unload));
					
					
	/***** (4) *****/
	/***** Subnet atomic model instantiation *****/
	//create an instance of the atomic submodel
	shared_ptr<dynamic::modeling::model> Storage1;
	Storage1 = dynamic::translate::make_dynamic_atomic_model<Storage, TIME>("Storage1");
	
	
	/***** (5) *****/
	/*******TOP MODEL********/
	//create a variable iports_TOP to store input ports of the top model
	dynamic::modeling::Ports iports_TOP;
	iports_TOP = {};		//no input in this case --> empty vector
	//output ports
	dynamic::modeling::Ports oports_TOP;
	oports_TOP = {typeid(top_out_loaded), typeid(top_out_unloaded)};
	//Submodels
	dynamic::modeling::Models submodels_TOP;
	submodels_TOP = {input_reader_load, input_reader_unload, Storage1};
	//EICs
	dynamic::modeling::EICs eics_TOP;
	eics_TOP = {};			//no external input
	//EOCs
	dynamic::modeling::EOCs eocs_TOP;
	eocs_TOP = {dynamic::translate::make_EOC<Storage_defs::loadedOut,top_out_loaded>("Storage1"),
				dynamic::translate::make_EOC<Storage_defs::unloadedOut,top_out_unloaded>("Storage1")};
	//ICs
	dynamic::modeling::ICs ics_TOP;
	ics_TOP = {dynamic::translate::make_IC<iestream_input_defs<Message_t>::out,Storage_defs::loadIn>("input_reader_load","Storage1"),
			dynamic::translate::make_IC<iestream_input_defs<Message_t>::out,Storage_defs::unloadIn>("input_reader_unload","Storage1")};
		
	/***** Create an instance of the coupled model *****/
	/* The parameters of the method are the name of the coupled model (i.e. “TOP”), and all the components 
	 * we have defined in the following order: submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
	 */
	shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
	TOP = make_shared<dynamic::modeling::coupled<TIME>>
		("TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP);
	
	
	/***** (6) *****/
	/*************** Loggers *******************/
	static ofstream out_messages("../simulation_results/Storage_test_output_messages.txt");//the output file to log messages
	struct oss_sink_messages{
		static ostream& sink(){
			return out_messages;
		}
	};
	static ofstream out_state("../simulation_results/Storage_test_output_state.txt");//the output file to log states
		struct oss_sink_state{
			static ostream& sink(){
				return out_state;
		}
	};
	
	using state = logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>,
	oss_sink_state>;
	using log_messages = logger::logger<logger::logger_messages,
	dynamic::logger::formatter<TIME>, oss_sink_messages>;
	using global_time_mes = logger::logger<logger::logger_global_time,
	dynamic::logger::formatter<TIME>, oss_sink_messages>;
	using global_time_sta = logger::logger<logger::logger_global_time,
	dynamic::logger::formatter<TIME>, oss_sink_state>;
	using logger_top = logger::multilogger<state, log_messages, global_time_mes,
	global_time_sta>;
	
	
	/***** (7) *****/
	/************** Runner call ************************/
	dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});	// Name of the TOP model, initial time ("TOP", 0)
	r.run_until(NDTime("05:00:00:000"));			//alternatively, run_until_passivate();
	return 0;
}