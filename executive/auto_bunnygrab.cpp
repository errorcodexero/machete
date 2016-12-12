#include "auto_bunnygrab.h"
#include "auto_stop.h"
#include "teleop.h"

using namespace std;

Executive Auto_bunnygrab::next_mode(Next_mode_info info){
	if(!info.autonomous) return Executive{Teleop()};
	const Time DRIVE_TIME = 3;//seconds
	if(info.since_switch > DRIVE_TIME) return Executive{Auto_stop()};
	return Executive{Auto_bunnygrab()};
}


Toplevel::Goal Auto_bunnygrab::run(Run_info){
	Toplevel::Goal goals;
	
	return goals;
}

bool Auto_bunnygrab::operator==(Auto_bunnygrab const&)const{ return true; };

#ifdef AUTO_BUNNYGRAB_TEST
#include "test.h"
int main(){
	Auto_bunnygrab a;
	test_executive(a);
}
#endif

