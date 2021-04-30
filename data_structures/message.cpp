#include <math.h> 
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include "message.hpp"

/***** OUTPUT STREAM *****/
// Send output to a filestream
ostream& operator<< (ostream& os, const Message_t& msg){
	os << "Material unit " << msg.material << " (" << ((msg.ready) ? "ready" : "not ready") << ") ";
	return os;
}


/***** INPUT STREAM *****/
// Take input from the stream and fill the structure
istream& operator>> (istream& is, Message_t& msg){
	is >> msg.material;
	is >> msg.ready;
	return is;
}