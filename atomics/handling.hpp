#ifndef _HANDLING_HPP__
#define _HANDLING_HPP__

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
struct Handling_defs{										//Convention: DevsAtomicModel_defs
	struct unloadOut : public out_port<Message_t>{};			//out_port and in_port are template structures defined in the simulator
	struct prepIn  : public in_port<Message_t>{};				//both ports can handle messages of type "Message_t"
};


/***** (2)Model Definition *****/
//Define the model as a C++ class
template<typename TIME> class Handling{								//typename indicates a data type that follows

//port assignment	
public:
	using input_ports = tuple<typename Handling_defs::prepIn>;			//typename overwrites the template class
	using output_ports= tuple<typename Handling_defs::unloadOut>;
	
	/***** (3)State Definition *****/
	//define a structure named "state_type" and a variable named "state"
	struct state_type{
		Message_t message;
		bool sending;
		bool active;				//phase active or not
		int index;
	};	
	state_type state;
	TIME moving_time;
	
	/***** (4)Default Constructor *****/
	//must define a default one "without parameters"
	Handling(){
		moving_time = TIME("00:00:05:00");
		state.sending = false;
		state.active = false;				//initially in passive phase
		state.index = 0;					//no messages received yet
	}
	
	
	/***** (5)Internal Transition (dint) *****/
	void internal_transition(){
		if (!state.active){
			;				//should never be triggered
		} else {
			state.active = false;
			state.sending = false;
		}
	}
	
	
	/***** (6)External Transition (dext) *****/
	//declare a bag of messages as inputs
	void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
		vector<Message_t> bag_port_in;										//a vector of elements of type Message_t
		bag_port_in = get_messages<typename Handling_defs::prepIn>(mbs);	//retrieve input bag, put into "bag_port_in"
		if(bag_port_in.size()>1) 
			assert(false && "H - Only one message at a time"); 
		
		if (!state.active){
			state.index++;
			state.message = bag_port_in[0];
			if (!state.message.ready){
				state.message.ready = true;		//After moving_time, material should be "ready" for processing
			} else {
				assert(false && "H - Cannot move an already moved material");
			}
			state.active = true;				//switch to active/move behavior
			state.sending = true;
		} else {
				assert(false && "H - invalid input while material preparation still in progress");		//input should not be here, ignore input and stay active
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
				
		get_messages<typename Handling_defs::unloadOut>(bags) = bag_port_out; //Access contents of "bag_port_out" for sending
		return bags;
	}
	
	
	/***** (8)Time Advance ta(s) *****/
	TIME time_advance() const{
		TIME next_internal;
		
		if (state.sending){
			next_internal = moving_time;			//time required (5s) to move one unit of material
		}
		else {
			next_internal = numeric_limits<TIME>::infinity();		//PASSIVATE the model
		}
		return next_internal;
	}
	
	
	/***** (8)Output State Log *****/
	friend ostringstream& operator<< (ostringstream& os, const typename Handling<TIME>::state_type& i){
		os << ":\n\tphase: " << ((i.active) ? "active" : "passive") << "   sending: " << i.sending << "   index: " << i.index;
		return os;
	}
};

#endif //_HANDLING_HPP__