#include "auto_forward.h"
#include "auto_stop.h"
#include "teleop.h"

using namespace std;

Executive Auto_forward::nextmode(Next_mode_info info){
	if(!info.autonomous) return Exectutive{Teleop()};
	if(info.since_switch > 3) return Exectutive{Auto_stop()};
	return Exectutive{Auto_forward()};
}

Toplevel::Goal
