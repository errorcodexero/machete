#include "auto_forward.h"
#include "auto_stop.h"
#include "teleop.h"

using namespace std;

Executive Auto_forward::next_mode(Next_mode_info info){
	if(!info.autonomous) return Executive{Teleop()};
	if(info.since_switch > 3) return Executive{Auto_stop()};
	return Executive{Auto_forward()};
}


Toplevel::Goal Auto_forward::run(Run_info){
	return {};
}

bool Auto_forward::operator==(Auto_forward const&)const{ return true; };
