#include "auto_distance.h"
#include "teleop.h"

using namespace std;

Executive Auto_distance::next_mode(Next_mode_info info){
	Drivebase::Encoder_ticks encoder_differences={
		#define X(ENCODER) info.status.drive.ticks.ENCODER-initial_encoders.ENCODER
		X(left),
		X(right),
		X(center)
	};
	if(!info.autonomous) return Executive{Teleop()};
	const double TARGET_DISTANCE = 5.0*12.0;//inches
	const double TOLERANCE = 6.0;//inches
	motion_profile.set_goal(TARGET_DISTANCE);
	cout<<"\nencoder_differences:"<<encoder_differences<<"   "<<ticks_to_inches(encoder_differences.left)<<"   "<<TARGET_DISTANCE<<"\n";
	if(ticks_to_inches(encoder_differences.left) >= TARGET_DISTANCE-TOLERANCE && ticks_to_inches(encoder_differences.left) <= TARGET_DISTANCE+TOLERANCE){
		in_range.update(info.in.now,info.in.robot_mode.enabled);
	}
	else{
		in_range.set(2.0);
	}
	if(in_range.done()){
		return Executive{Teleop()};
	}
	return Executive{Auto_distance(CONSTRUCT_STRUCT_PARAMS(AUTO_DISTANCE_ITEMS))};
}

Toplevel::Goal Auto_distance::run(Run_info info){
	Toplevel::Goal goals;
	double power=-motion_profile.target_speed(ticks_to_inches(info.toplevel_status.drive.ticks.left));
	goals.drive.y=power;
	return goals;
}

bool Auto_distance::operator==(Auto_distance const&)const{ return true; }


#ifdef AUTO_DISTANCE_TEST
#include "test.h"
int main(){
	Auto_distance a = {{0,0,0}};
	test_executive(a);
}
#endif
