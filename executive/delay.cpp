#include "delay.h"

#include "teleop.h" 
#include "auto_null.h"

using namespace std;

using Mode=Executive;

Executive auto_mode_convert(Next_mode_info info){
	if (info.panel.in_use) {
		switch(info.panel.auto_switch){ 
			case 1: 
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 0:
			default:
				return Executive{Auto_null()};
		}
	}
	return Mode{Delay()};
}

Mode Delay::next_mode(Next_mode_info info){
	if(!info.autonomous) return Mode{Teleop()};
	if(info.since_switch > 8) return auto_mode_convert(info);
	return Mode{Delay()};
}

Toplevel::Goal Delay::run(Run_info){
	
	return {};
}

bool Delay::operator==(Delay const&)const{ return 1; }

#ifdef DELAY_TEST
#include "test.h"
int main(){
	Delay a;
	test_executive(a);
}
#endif 
