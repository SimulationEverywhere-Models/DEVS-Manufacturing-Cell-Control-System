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
#include "../atomics/control.hpp"
#include "../atomics/storage.hpp"
#include "../atomics/handling.hpp"
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
//ports for the TOP model
struct top_out_end: public out_port<int>{};
struct top_out_mat_prepared: public out_port<int>{};
//ports for the Inventory handler
struct ih_in_load: public in_port<Message_t>{};
struct ih_in_prep: public in_port<Message_t>{};
struct ih_out_loaded: public out_port<Message_t>{};
struct ih_out_unloaded: public out_port<Message_t>{};
//ports for the MCCS
struct mccs_in_start: public in_port<int>{};
struct mccs_out_mat_prepared: public out_port<int>{};
struct mccs_out_end: public out_port<int>{};


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
template<typename T>
class InputReader_Int : public iestream_input<int,T> {
    public:
        InputReader_Int () = default;
        InputReader_Int (const char* file_path) : iestream_input<int,T>(file_path) {}
};

/***** (3) *****/
/***** Create the main function *****/
int main (int argc, char **argv){
	if (argc < 2) {
        cout << "Wrong parameters. The program must be invoked as: ";
        cout << argv[0] << " path to the input file " << endl;
        return 1; 
    }
	
	/****** Input Reader atomic model instantiation ******/
	string input = argv[1];
	const char *i_input_data_main_start = input.c_str();
	//create a shared pointer to hold the instantiation
	shared_ptr<dynamic::modeling::model> input_reader_main_start = dynamic::translate::make_dynamic_atomic_model
					<InputReader_Int, TIME, const char*>("input_reader_main_start", move(i_input_data_main_start));
					
					
	/***** (4) *****/
	/***** Handling atomic model instantiation *****/
	shared_ptr<dynamic::modeling::model> handling1;
	handling1 = dynamic::translate::make_dynamic_atomic_model<Handling, TIME>("handling1");
	/***** Storage atomic model instantiation *****/
	shared_ptr<dynamic::modeling::model> storage1;
	storage1 = dynamic::translate::make_dynamic_atomic_model<Storage, TIME>("storage1");
	/***** Control atomic model instantiation *****/
	shared_ptr<dynamic::modeling::model> control1;
	control1 = dynamic::translate::make_dynamic_atomic_model<Control, TIME>("control1");
	
	
	/***** (5) *****/
	/***** INVENTORY HANDLER COUPLED MODEL *****/
	//input ports
	dynamic::modeling::Ports iports_IH = {typeid(ih_in_load), typeid(ih_in_prep)};		
	//output ports
	dynamic::modeling::Ports oports_IH = {typeid(ih_out_loaded), typeid(ih_out_unloaded)};
	//Submodels
	dynamic::modeling::Models submodels_IH = {storage1, handling1};
	//EICs
	dynamic::modeling::EICs eics_IH = {dynamic::translate::make_EIC<ih_in_load, Storage_defs::loadIn>("storage1"),
				dynamic::translate::make_EIC<ih_in_prep, Handling_defs::prepIn>("handling1")};			
	//EOCs
	dynamic::modeling::EOCs eocs_IH = {dynamic::translate::make_EOC<Storage_defs::loadedOut, ih_out_loaded>("storage1"),
				dynamic::translate::make_EOC<Storage_defs::unloadedOut, ih_out_unloaded>("storage1")};
	//ICs
	dynamic::modeling::ICs ics_IH = {dynamic::translate::make_IC<Handling_defs::unloadOut, Storage_defs::unloadIn>("handling1","storage1")};
	//Create an instance of the INVENTORY HANDLER (IH) coupled model
	shared_ptr<dynamic::modeling::coupled<TIME>> 
	IH = make_shared<dynamic::modeling::coupled<TIME>>
		("IH", submodels_IH, iports_IH, oports_IH, eics_IH, eocs_IH, ics_IH);
		
		
	/*******MCCS COUPLED MODEL********/
	//input ports
	dynamic::modeling::Ports iports_MCCS = {typeid(mccs_in_start)};	
	//output ports
	dynamic::modeling::Ports oports_MCCS = {typeid(mccs_out_mat_prepared), typeid(mccs_out_end)};
	//Submodels
	dynamic::modeling::Models submodels_MCCS = {control1, IH};
	//EICs
	dynamic::modeling::EICs eics_MCCS = {dynamic::translate::make_EIC<mccs_in_start, Control_defs::startIn>("control1")};			//no external input
	//EOCs
	dynamic::modeling::EOCs eocs_MCCS = {dynamic::translate::make_EOC<Control_defs::matPreparedOut, mccs_out_mat_prepared>("control1"),
			dynamic::translate::make_EOC<Control_defs::endOut, mccs_out_end>("control1")};
	//ICs
	dynamic::modeling::ICs ics_MCCS = {dynamic::translate::make_IC<Control_defs::loadOut, ih_in_load>("control1", "IH"),
			dynamic::translate::make_IC<Control_defs::prepOut, ih_in_prep>("control1", "IH"),
			dynamic::translate::make_IC<ih_out_loaded, Control_defs::loadedIn>("IH", "control1"),
			dynamic::translate::make_IC<ih_out_unloaded, Control_defs::unloadedIn>("IH", "control1")};
			
	/***** Create an instance of the TOP model *****/
	/* The parameters of the method are the name of the coupled model (i.e. ???TOP???), and all the components 
	 * we have defined in the following order: submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
	 */
	shared_ptr<dynamic::modeling::coupled<TIME>> 
	MCCS = make_shared<dynamic::modeling::coupled<TIME>>
		("MCCS", submodels_MCCS, iports_MCCS, oports_MCCS, eics_MCCS, eocs_MCCS, ics_MCCS);
		
		
	/*******TOP MODEL********/
	//create a variable iports_TOP to store input ports of the top model
	dynamic::modeling::Ports iports_TOP = {};		//no input in this case --> empty vector
	//output ports
	dynamic::modeling::Ports oports_TOP = {typeid(top_out_mat_prepared), typeid(top_out_end)};
	//Submodels
	dynamic::modeling::Models submodels_TOP = {MCCS, input_reader_main_start};
	//EICs
	dynamic::modeling::EICs eics_TOP = {};			//no external input
	//EOCs
	dynamic::modeling::EOCs eocs_TOP = {dynamic::translate::make_EOC<mccs_out_mat_prepared, top_out_mat_prepared>("MCCS"),
			dynamic::translate::make_EOC<mccs_out_end, top_out_end>("MCCS")};
	//ICs
	dynamic::modeling::ICs ics_TOP = {dynamic::translate::make_IC<iestream_input_defs<int>::out, mccs_in_start>("input_reader_main_start", "MCCS")};
	/***** Create an instance of the TOP model *****/
	/* The parameters of the method are the name of the coupled model (i.e. ???TOP???), and all the components 
	 * we have defined in the following order: submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
	 */
	shared_ptr<dynamic::modeling::coupled<TIME>> 
	TOP = make_shared<dynamic::modeling::coupled<TIME>>
		("TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP);
	
	
	/***** (6) *****/
	/*************** Loggers *******************/
	static ofstream out_messages("../simulation_results/MCCS_main_test_output_messages.txt");//the output file to log messages
	struct oss_sink_messages{
		static ostream& sink(){
			return out_messages;
		}
	};
	static ofstream out_state("../simulation_results/MCCS_main_test_output_state.txt");//the output file to log states
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