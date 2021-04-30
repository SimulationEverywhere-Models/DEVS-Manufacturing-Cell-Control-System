#ifndef _Storage_HPP__
#define _Storage_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <assert.h>
#include <string>			
#include <limits>			//for Passivating: set ta(s) to infinity when needed
#include <random>			//for random number generation

#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;


/***** (1)Port Definition *****/
//Define ports as structures
struct Storage_defs{										//Convention: DevsAtomicModel_defs
	struct loadedOut : public out_port<Message_t>{};			//out_port and in_port are template structures
	struct unloadedOut : public out_port<Message_t>{};	
	struct loadIn  : public in_port<Message_t>{};				//both ports can handle messages of type "Message_t"
	struct unloadIn  : public in_port<Message_t>{};	
};


/***** (2)Model Definition *****/
//Define the model as a C++ class
template<typename TIME> class Storage{								//typename indicates a data type that follows


//port assignment	
public:
	using input_ports = tuple<typename Storage_defs::loadIn, Storage_defs::unloadIn>;		//typename overwrites the template class
	using output_ports= tuple<typename Storage_defs::loadedOut, Storage_defs::unloadedOut>;
	
	
	/***** (3)State Definition *****/
	//define a structure named "state_type" and a variable named "state"
	struct state_type{
		Message_t message;
		bool sending;
		bool full;				//phase full or not (empty)
		int load_request_index;			//keep track of the number of load and unload requests received
		int unload_request_index;
	};	
	state_type state;
	TIME loading_time;
	
	
	/***** (4)Default Constructor *****/
	//must define a default one "without parameters"
	Storage(){
		loading_time = TIME("00:00:02:00");
		state.sending = false;
		state.full = false;				//initially in empty phase
		state.load_request_index = 0;				//no messages received yet
		state.unload_request_index = 0;
	}
	
	
	/***** (5)Internal Transition (dint) *****/
	void internal_transition(){
		state.sending = false;
		if (!state.full){
			state.full = true;
		} else {
			state.full = false;
		}
	}
	
	
	/***** (6)External Transition (dext) *****/
	//declare a bag of messages as inputs
	void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
		if ((get_messages<typename Storage_defs::loadIn>(mbs).size() + 
			get_messages<typename Storage_defs::unloadIn>(mbs).size())>1) 
			assert(false && "S - Only one message is allowed per time unit");
		
		for(const auto &x : get_messages<typename Storage_defs::loadIn>(mbs)){
			
			if (!state.full){		//check if storage is empty when "load" request arrives
				state.load_request_index++;
				state.message = x;
				if (!state.message.ready){		//check whether material has already been moved
					state.sending = true;
				} else {
					assert(false && "S - Cannot load an already moved material");
				}
			} else {
				assert(false && "S - Invalid load request while material storage already full");		//input should not be here, ignore input
			}
		}
		for(const auto &x : get_messages<typename Storage_defs::unloadIn>(mbs)){
			
			if (state.full){			//check if storage is full when "unload" request arrives
				state.unload_request_index++;
				state.message = x;
				if (state.message.ready){		//check whether material has already been moved
					state.sending = true;
				} else {
					assert(false && "S - unload request without moving the material");
				}
			} else {
				assert(false && "S - Invalid unload request while material storage already empty");		//input should not be here, ignore input
			}
		}
	}
	
	
	/***** (7)Confluent Transition *****/
	//Use default implementation: call internal first and then external with zero elapsed time
	void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
		internal_transition();
		external_transition(TIME(), move(mbs));
	}
	
	
	/***** (8)Output Function (lambda) *****/
	typename make_message_bags<output_ports>::type output() const{
		typename make_message_bags<output_ports>::type bags;
		vector<Message_t> bag_port_out;
		bag_port_out.push_back(state.message);
		
		if (!state.full && state.sending){
			get_messages<typename Storage_defs::loadedOut>(bags) = bag_port_out; //send contents of "bag_port_out"
		} else if (state.full && state.sending){
			get_messages<typename Storage_defs::unloadedOut>(bags) = bag_port_out; //send contents of "bag_port_out"
		} else {
			;		//do nothing
		}
		return bags;
	}
	
	
	/***** (8)Time Advance ta(s) *****/
	TIME time_advance() const{
		TIME next_internal;
		
		if (!state.full && state.sending){
			next_internal = loading_time; 			//time required (2s) to load one unit of material
		} else if (state.full && state.sending){
			next_internal = TIME("00:00:00");			//immediately trigger lambda and dint
		} else {
			next_internal = numeric_limits<TIME>::infinity();		//PASSIVATE the model
		}
		return next_internal;
	}
	
	
	/***** (8)Output State Log *****/
	friend ostringstream& operator<< (ostringstream& os, const typename Storage<TIME>::state_type& i){
		os << ":\n\tphase: " << ((i.full) ? "full" : "empty") << "   sending: " << i.sending << 
		"   load requests received: " << i.load_request_index << "   unload requests received: " << i.unload_request_index;
		return os;
	}
};

#endif //_Storage_HPP__