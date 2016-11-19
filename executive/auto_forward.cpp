#include "auto_forward.h"
#include "auto_stop.h"
#include "teleop.h"

using namespace std;

Executive Auto_forward::next_mode(Next_mode_info info){
	if(!info.autonomous) return Executive{Teleop()};
	static const Time DRIVE_TIME = 3;//seconds, assumed
	if(info.since_switch > DRIVE_TIME) return Executive{Auto_stop()};
	return Executive{Auto_forward()};
}


Toplevel::Goal Auto_forward::run(Run_info){
	Toplevel::Goal goals;
	static const double SPEED = .45;
	goals.drive.y = SPEED;
	return goals;
}

bool Auto_forward::operator==(Auto_forward const&)const{ return true; };

#ifdef AUTO_FORWARD_TEST
#include "test.h"
int main(){
	Auto_forward a;
	test_executive(a);
}
#endif

