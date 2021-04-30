#ifndef BOOST_SIMULATION_MESSAGE_HPP
#define BOOST_SIMULATION_MESSAGE_HPP

#include <assert.h>
#include <iostream>
#include <string>

using namespace std;

//=======================MESSAGE=======================
struct Message_t{
	//default constructor: message filled with garbage
	Message_t(){};		
	
	//overloaded constructor
	Message_t(int i_material, bool i_ready)
		: material(i_material), ready(i_ready){};
		
	//message contents
	int material;	
	bool ready;			//material ready for processing
};

istream& operator>> (istream& is, Message_t& msg);
ostream& operator<< (ostream& os, const Message_t& msg);
//=====================================================


#endif // BOOST_SIMULATION_MESSAGE_HPP