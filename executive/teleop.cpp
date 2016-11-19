#include "teleop.h"
#include <math.h>
#include "delay.h"
#include "../input/util.h"

using namespace std;

ostream& operator<<(ostream& o,Teleop::Nudge const& ){
	o<<"Nudge(";
	o<<"...";
	return o<<")";
}

double set_drive_speed(double axis,double boost,double slow){
	static const float MAX_SPEED=1;//Change this value to change the max power the robot will achieve with full boost (cannot be larger than 1.0)
	static const float DEFAULT_SPEED=.4;//Change this value to change the default power
	static const float SLOW_BY=.5;//Change this value to change the percentage of the default power the slow button slows
	return (pow(axis,3)*((DEFAULT_SPEED+(MAX_SPEED-DEFAULT_SPEED)*boost)-((DEFAULT_SPEED*SLOW_BY)*slow)));
}

bool operator<(Teleop::Nudge const& a,Teleop::Nudge const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	NUDGE_ITEMS(X)
	#undef X
	return 0;
}

bool operator==(Teleop::Nudge const& a,Teleop::Nudge const& b){
	#define X(A,B) if(a.B!=b.B) return 0;
	NUDGE_ITEMS(X)
	#undef X
	return 1;
}

Executive Teleop::next_mode(Next_mode_info info) {
	if (info.autonomous_start) {
		if (info.panel.in_use) {
			return Executive{Delay()};
		}
	}
	Teleop t(CONSTRUCT_STRUCT_PARAMS(TELEOP_ITEMS));
	return Executive{t};
}

Teleop::Teleop(){
	arm_mode = Arm_mode::DOWN;
}

IMPL_STRUCT(Teleop::Teleop,TELEOP_ITEMS)

Toplevel::Goal Teleop::run(Run_info info) {
	Toplevel::Goal goals;
	
	bool enabled = info.in.robot_mode.enabled;

	{//Set drive goals
		//bool spin=fabs(info.main_joystick.axis[Gamepad_axis::RIGHTX])>.01;//drive turning button
		double boost=info.main_joystick.axis[Gamepad_axis::LTRIGGER],slow=info.main_joystick.axis[Gamepad_axis::RTRIGGER];//turbo and slow buttons	
	
		for(int i=0;i<NUDGES;i++){
			const array<unsigned int,NUDGES> nudge_buttons={Gamepad_button::Y,Gamepad_button::A,Gamepad_button::X,Gamepad_button::B,Gamepad_button::LB,Gamepad_button::RB};
			//Forward, backward, left, right, clockwise, counter-clockwise
			if(nudges[i].trigger(boost<.25 && info.main_joystick.button[nudge_buttons[i]])) nudges[i].timer.set(.1);
			nudges[i].timer.update(info.in.now,enabled);
		}
		const double X_NUDGE_POWER=.45,Y_NUDGE_POWER=.2,ROTATE_NUDGE_POWER=.5;
		goals.drive.y=([&]{
			if(!nudges[Nudges::FORWARD].timer.done()) return -Y_NUDGE_POWER;
			if(!nudges[Nudges::BACKWARD].timer.done()) return Y_NUDGE_POWER;
			return set_drive_speed(info.main_joystick.axis[Gamepad_axis::LEFTY],boost,slow);
		}());
		goals.drive.x=clip([&]{
			if(!nudges[Nudges::LEFT].timer.done()) return -X_NUDGE_POWER;
			else if(!nudges[Nudges::RIGHT].timer.done()) return X_NUDGE_POWER;
			return set_drive_speed(info.main_joystick.axis[Gamepad_axis::LEFTX],boost,slow);
		}());
		goals.drive.theta=clip([&]{
			if(!nudges[Nudges::CLOCKWISE].timer.done()) return ROTATE_NUDGE_POWER;
			else if (!nudges[Nudges::COUNTERCLOCKWISE].timer.done()) return -ROTATE_NUDGE_POWER;
			return -set_drive_speed(info.main_joystick.axis[Gamepad_axis::RIGHTX],boost,slow);
		}());
		//cout<<"\nDrive goals: "<<goals.drive<<"\n";
	}

	goals.gun=[&]{
		if(info.gunner_joystick.axis[Gamepad_axis::LTRIGGER]>.9){
			if(info.gunner_joystick.axis[Gamepad_axis::RTRIGGER]>.9) return Gun::Goal::SHOOT;
			else return Gun::Goal::REV;
		}
		else return Gun::Goal::OFF;
	}();

	goals.arm=[&]{
		arm_mode = [&]{
			if(info.gunner_joystick.button[Gamepad_button::Y]) return Arm_mode::UP;
			if(info.gunner_joystick.button[Gamepad_button::A]) return Arm_mode::DOWN;
			return arm_mode;
		}();
		switch(arm_mode){
			case Arm_mode::DOWN:
				return Arm::Goal::DOWN;
			case Arm_mode::UP:
				return Arm::Goal::UP;
			default:
				assert(0);		
		}
	}();
	return goals;
}

#define TELEOP_ITEMS_NO_TYPE(X)\
	X(nudges)\

bool Teleop::operator<(Teleop const& a)const{
	#define X(NAME) if(NAME<a.NAME) return 1; if(a.NAME<NAME) return 0;
	TELEOP_ITEMS_NO_TYPE(X)
	#undef X
	return 0;
}

bool Teleop::operator==(Teleop const& a)const{
	#define X(NAME) if(NAME!=a.NAME) return 0;
	TELEOP_ITEMS_NO_TYPE(X)
	#undef X
	return 1;
}

void Teleop::display(ostream& o)const{
	o<<"Teleop( ";
	#define X(NAME) o<<""#NAME<<":"<<(NAME)<<" ";
	//TELEOP_ITEMS_NO_TYPE(X)
	#undef X
	o<<")";
}

#ifdef TELEOP_TEST
#include "test.h"

int main() {
	Teleop a;
	test_executive(a);
}
#endif
