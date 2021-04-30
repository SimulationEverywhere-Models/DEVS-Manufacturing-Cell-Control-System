#ifndef _Control_HPP__
#define _Control_HPP__

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
struct Control_defs{											//Convention: DevsAtomicModel_defs
	//Output ports
	struct loadOut : public out_port<Message_t>{};			//out_port and in_port are template structures
	struct prepOut : public out_port<Message_t>{};
	struct matPreparedOut : public out_port<int>{};
	struct endOut : public out_port<int>{};
	//Input ports
	struct startIn  : public in_port<int>{};					//new batch request
	struct loadedIn  : public in_port<Message_t>{};			//other output ports can handle messages of type "Message_t"
	struct unloadedIn  : public in_port<Message_t>{};	
};


/***** (2)Model Definition *****/
//Define the model as a C++ class
template<typename TIME> class Control{								//typename indicates a data type that follows


//port assignment	
public:
	using input_ports = tuple<typename Control_defs::startIn, Control_defs::loadedIn, Control_defs::unloadedIn>;		//typename overwrites the template class
	using output_ports= tuple<typename Control_defs::loadOut, Control_defs::prepOut,
										Control_defs::matPreparedOut, Control_defs::endOut>;
	
	
	/***** (3)State Definition *****/
	//define a structure named "state_type" and a variable named "state"
	struct state_type{
		Message_t message;
		bool sending;
		int phase;						//0 = Idle, 1 = Init, 2 = Prep
		int total_mats;					//total number of materials to be prepared for processing
		int num_prepared;				//number of materials prepared up until now
		bool fin;						//whether a material has been prepared
//		TIME next_internal;
	};	
	state_type state;
	
	
	/***** (4)Default Constructor *****/
	//must define a default one "without parameters"
	Control(){
		state.sending = false;
		state.phase = 0;				//initially in Idle phase
		state.total_mats = 0;
		state.num_prepared = 0;
		state.fin = false;
	}
	
	
	/***** (5)Internal Transition (dint) *****/
	void internal_transition(){
		state.sending = false;
		state.fin = false;
	}
	
	
	/***** (6)External Transition (dext) *****/
	//declare a bag of messages as inputs
	void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
		if ((get_messages<typename Control_defs::startIn>(mbs).size() + 
			get_messages<typename Control_defs::loadedIn>(mbs).size() +
			get_messages<typename Control_defs::unloadedIn>(mbs).size())>1) 
			assert(false && "C - Only one message is allowed per time unit");
		
		for(const auto &x : get_messages<typename Control_defs::startIn>(mbs)){
			state.message = {state.num_prepared+1, false};		//generate a request message
			
			if (state.phase == 0){		//check if system state is idle when "start" request arrives
				state.phase = 1;		//switch to init mode
				state.total_mats += x;	//new batch request
				state.sending = true;
			} else {
				state.total_mats += x;	//add new batch request amount to the previous ones
			}
		}
		for(const auto &x : get_messages<typename Control_defs::loadedIn>(mbs)){
			if (state.phase == 1){		//check if system state is init when "loaded" request arrives
				state.message = x;
				if (!state.message.ready){
					state.phase = 2;		//switch to prep mode
					state.sending = true;
				} else {
					assert(false && "C - invalid input from S, material storage cannot be full while material has already been moved");
				}
			} else {
				assert(false && "C - loaded messages only allowed in init mode");		//input should not be here, ignore input
			}
		}
		for(const auto &x : get_messages<typename Control_defs::unloadedIn>(mbs)){
			if (state.phase == 2){		//check if system state is prep when "unloaded" request arrives
			state.message = x;
				if (state.message.ready == 1){		//check if the current material has been prepared
					state.fin = true;		
					state.num_prepared++;
				} else {
					assert(false && "C - invalid input from S, material storage cannot be empty while material has not moved yet");
				}
				state.sending = true;
				if (state.num_prepared == state.total_mats){
					state.phase = 0;		//all materials prepared; switch to idle mode
				} else {
					state.phase = 1;	//switch to init mode and keep preparing new materials
					state.message = {state.num_prepared+1, false};	//also generate a new/same(not prepared) material request
				}
			} else {
				assert(false && "C - unloaded messages only allowed in prep mode");		//input should not be here, ignore input
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
		
		if (state.fin){		//one material was just prepared, send additional message
				get_messages<typename Control_defs::matPreparedOut>(bags).push_back(state.num_prepared);
			} 
			
		if (state.phase == 0 && state.sending){
			get_messages<typename Control_defs::endOut>(bags).push_back(1);
		} else if (state.phase == 1 && state.sending){
			get_messages<typename Control_defs::loadOut>(bags) = bag_port_out; //send contents of "bag_port_out"
		} else if (state.phase == 2 && state.sending){
			get_messages<typename Control_defs::prepOut>(bags) = bag_port_out; //send contents of "bag_port_out"
		} else {
			;		//do nothing
		}
		return bags;
	}
	
	
	/***** (8)Time Advance ta(s) *****/
	TIME time_advance() const{
		TIME next_internal;
		
		if (state.sending){
			next_internal = TIME("00:00:00");			//immediately trigger lambda and dint
		} else {
			next_internal = numeric_limits<TIME>::infinity();		//PASSIVATE the model
		}
		return next_internal;
	}
	
	
	/***** (8)Output State Log *****/
	friend ostringstream& operator<< (ostringstream& os, const typename Control<TIME>::state_type& i){
		string current_phase;
		if (i.phase == 0){
			current_phase = "idle";
		} else if (i.phase == 1) {
			current_phase = "init";
		} else if (i.phase == 2){
			current_phase = "prep";
		} else {
			current_phase = "unknown phase";
		}
		os << ":\n\tphase: " << current_phase << "   sending: " << i.sending << "   fin: " << i.fin <<
		"\n\ttotal requests: " << i.total_mats << "   current prepared materials: " << i.num_prepared;
		return os;
	}
};

#endif //_Control_HPP__