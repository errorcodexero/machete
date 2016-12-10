#include "auto_distance.h"
#include "teleop.h"

using namespace std;

Executive Auto_distance::next_mode(Next_mode_info info){
	if(!info.autonomous) return Executive{Teleop()};
	
	Drivebase::Encoder_ticks encoder_differences={
		#define X(ENCODER) info.status.drive.ticks.ENCODER - initial_encoders.ENCODER
		X(l),
		X(r),
		X(c)
	};
	
	const double TARGET_DISTANCE = 5.0*12.0;//inches
	const double TOLERANCE = 6.0;//inches
	
	motion_profile.set_goal(TARGET_DISTANCE);
	
	cout<<"\nencoder_differences:"<<encoder_differences<<"   left(inches):"<<ticks_to_inches(encoder_differences.l)<<"   target(inches):"<<TARGET_DISTANCE<<"\n";
	
	if(ticks_to_inches(encoder_differences.l) >= TARGET_DISTANCE-TOLERANCE && ticks_to_inches(encoder_differences.l) <= TARGET_DISTANCE+TOLERANCE){
		in_range.update(info.in.now,info.in.robot_mode.enabled);
	}
	else{
		const double IN_RANGE_TIME = 2.0;//seconds - the time that the robot needs to be within a certain distance from the target
		in_range.set(IN_RANGE_TIME);
	}
	if(in_range.done()){
		return Executive{Teleop()};
	}
	return Executive{Auto_distance(CONSTRUCT_STRUCT_PARAMS(AUTO_DISTANCE_ITEMS))};
}

Toplevel::Goal Auto_distance::run(Run_info info){
	Toplevel::Goal goals;
	double power = -motion_profile.target_speed(ticks_to_inches(info.toplevel_status.drive.ticks.l));//assuming that the left and right encoder values are similar enough
	goals.drive.y = power;
	return goals;
}

bool Auto_distance::operator==(Auto_distance const&)const{ return true; }//TODO: update with values in that struct


#ifdef AUTO_DISTANCE_TEST
#include "test.h"

int main(){
	Auto_distance a = {{0,0,0}};
	test_executive(a);
}
#endif
