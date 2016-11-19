#include "main.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <stdlib.h>
#include "toplevel.h"
#include "../util/util.h"
#include "../input/util.h"
#include "../util/point.h"
#include <vector>
#include <assert.h>
#include <fstream>
#include "../executive/teleop.h"

using namespace std;

ofstream myfile2;

static int print_count=0;
#define SLOW_PRINT (print_count%10==0)

ostream& operator<<(ostream& o,Main::Mode a){
	#define X(NAME) if(a==Main::Mode::NAME) return o<<""#NAME;
	MODES
	#undef X
	assert(0);
}

#ifdef MAIN_TEST
static const string NAVLOG2="navlog2.txt";
#else
static const string NAVLOG2="/home/lvuser/navlogs/navlog2.txt";
#endif

//TODO: at some point, might want to make this whatever is right to start autonomous mode.
Main::Main():
	mode(Mode::TELEOP),
	mode_(Executive{Teleop()}),
	motion_profile(0.0,.01),
	in_br_range(),
	autonomous_start(0)
{
	in_br_range.set(2.0);
	set_initial_encoders = true;
	initial_encoders = make_pair(0,0);
	br_step=0;
	myfile2.open(NAVLOG2);
	myfile2 << "test start" << endl;
}


template<size_t LEN>
array<double,LEN> floats_to_doubles(array<float,LEN> a){
	array<double,LEN> r;
	for(size_t i=0;i<LEN;i++) r[i]=a[i];
	return r;
}

pair<float,float> driveatwall(const Robot_inputs in){
	const float targetinches=3; //Desired distance from wall
	float currentinches=range(in);
	pair<float,float> motorvoltmods;
	//motorvoltmods.first = left; motorvoltmods.second = right
	float adjustment=0;
	if(targetinches<currentinches){
		motorvoltmods.first=adjustment;
		motorvoltmods.second=adjustment*-1;
	}
	return motorvoltmods;
}

/*double ticks_to_inches(int ticks){//Moved to drivebase.cpp
	const unsigned int TICKS_PER_REVOLUTION=100;
	const double WHEEL_DIAMETER=8.0;//inches
	const double WHEEL_CIRCUMFERENCE=WHEEL_DIAMETER*PI;//inches
	const double INCHES_PER_TICK=WHEEL_CIRCUMFERENCE/(double)TICKS_PER_REVOLUTION;//0.25 vs 0.251327
	return ticks*INCHES_PER_TICK;
}*/

double ticks_to_degrees(int ticks){
	const double DEGREES_PER_TICK=0.716197244;//Assumed for now
	return ticks * DEGREES_PER_TICK;//Degrees clockwise
}

/*Main::Mode next_mode(Main::Mode m,bool autonomous,bool autonomous_start,Toplevel::Status_detail const& status,Time since_switch, Panel panel,bool const& topready,Robot_inputs const& in,pair<int,int> initial_encoders, unsigned int& br_step,bool& set_initial_encoders, Motion_profile& motion_profile,Countdown_timer& in_br_range){
	pair<int,int> current_encoders=status.drive.ticks;//{encoderconv(in.digital_io.encoder[0]),encoderconv(in.digital_io.encoder[1])};//first is left, second is right
	pair<int,int> encoder_differences=make_pair(current_encoders.first-initial_encoders.first,current_encoders.second-initial_encoders.second);
	if(SLOW_PRINT){
		cout<<"initial_encoders:"<<initial_encoders<<"\n";
		cout<<"current_encoders:"<<current_encoders<<"\n";
		cout<<"encoder_differences:"<<encoder_differences<<"\n";
	}
	switch(m){
		case Main::Mode::TELEOP:	
			if(autonomous_start){
				myfile2 << "NEXT_MODE:AUTO_REACH***" << endl;
				//return Main::Mode::AUTO_STATIC;//just for testing purposes
				if(panel.in_use){
					return Main::Mode::DELAY;
				}
				return Main::Mode::TELEOP; //during testing put the mode you want to test without the driverstation.
			}
			return m;
		case Main::Mode::DELAY:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > (panel.speed_dial+1)*5 || since_switch>8) return get_auto(panel);
			return Main::Mode::DELAY;
		case Main::Mode::AUTO_NULL:
			return Main::Mode::TELEOP;
		
		case Main::Mode::AUTO_REACH:
			myfile2 << "NEXT_MODE:SS" << since_switch << endl;
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > .8) return Main::Mode::AUTO_STOP;
			return Main::Mode::AUTO_REACH;

		case Main::Mode::AUTO_STATIC:
			if (!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 2) return Main::Mode::AUTO_STOP;
//Time used to be 1.5, prior to match 65 PNW, where it was ~3in short on the rampart crossings
			return Main::Mode::AUTO_STATIC;

		case Main::Mode::AUTO_STATICTWO:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 2.5) return Main::Mode::AUTO_STOP;
			return Main::Mode::AUTO_STATICTWO;

		case Main::Mode::AUTO_STOP:
			myfile2 << "NEXT_MODE:DONE=>TELEOP" << endl;
			return Main::Mode::TELEOP;

		case Main::Mode::AUTO_TEST:
			if(since_switch > 1 || !autonomous) return Main::Mode::TELEOP;
			return Main::Mode::AUTO_TEST;

		case Main::Mode::AUTO_PORTCULLIS:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 4) return Main::Mode::AUTO_PORTCULLIS_DONE;
			return Main::Mode::AUTO_PORTCULLIS;
	
		case Main::Mode::AUTO_PORTCULLIS_DONE:
			if(since_switch > 2.5 || !autonomous) return Main::Mode::TELEOP;
			return Main::Mode::AUTO_PORTCULLIS_DONE;

		case Main::Mode::AUTO_CHEVALPOS:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 1.8) return Main::Mode::AUTO_CHEVALWAIT;
			return Main::Mode::AUTO_CHEVALPOS;

		case Main::Mode::AUTO_CHEVALWAIT:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 3) return Main::Mode::AUTO_CHEVALDROP;
			return Main::Mode::AUTO_CHEVALWAIT;

		case Main::Mode::AUTO_CHEVALDROP:
			if(!autonomous) return Main::Mode::TELEOP;
			if(topready) return Main::Mode::AUTO_CHEVALDRIVE;
			return Main::Mode::AUTO_CHEVALDROP;
				
		case Main::Mode::AUTO_CHEVALDRIVE:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > .45) return Main::Mode::AUTO_CHEVALSTOW;
			return Main::Mode::AUTO_CHEVALDRIVE;
		
		case Main::Mode::AUTO_CHEVALSTOW:
			if(since_switch > 2.5 || !autonomous) return Main::Mode::TELEOP;
			return Main::Mode::AUTO_CHEVALSTOW;

		case Main::Mode::AUTO_LBLS_CROSS_LB:
		{
			if(!autonomous) return Main::Mode::TELEOP;
			if(encoder_differences.first >= 670) return Main::Mode::AUTO_LBLS_CROSS_MU;
// 100 ticks per 1 revalition| 8in wheal| 167 in for first run| cir:25.12| 100 ticks / 25 in| 4 ticks / 1 in| 668 ticks / 167 in.
			return Main::Mode::AUTO_LBLS_CROSS_LB;
			
		}
		case Main::Mode::AUTO_LBLS_CROSS_MU:
		
			if(!autonomous) return Main::Mode::TELEOP;
			if(topready) return Main::Mode::AUTO_LBLS_SCORE_SEEK;
			return Main::Mode::AUTO_LBLS_CROSS_MU;
		
		case Main::Mode::AUTO_LBLS_SCORE_SEEK:
		
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > .76) return Main::Mode::AUTO_LBLS_SCORE_LOCATE;
			return Main::Mode::AUTO_LBLS_SCORE_SEEK;
		
		case Main::Mode::AUTO_LBLS_SCORE_LOCATE:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 1) return Main::Mode::AUTO_LBLS_SCORE_CD;
			return Main::Mode::AUTO_LBLS_SCORE_LOCATE;

		case Main::Mode::AUTO_LBLS_SCORE_CD:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 2.8) return Main::Mode::AUTO_LBLS_SCORE_EJECT;//when vision is in remove.
			return Main::Mode::AUTO_LBLS_SCORE_CD;
		
		case Main::Mode::AUTO_LBLS_SCORE_EJECT:
			if(since_switch > 1 || !autonomous) return Main::Mode::TELEOP;
			return Main::Mode::AUTO_LBLS_SCORE_EJECT;

		case Main::Mode::AUTO_LBWLS_WALL:
		{
			if(!autonomous) return Main::Mode::TELEOP;
			if(encoder_differences.first >= 670|| since_switch > 4.5) return Main::Mode::AUTO_LBWLS_MUP;// The value that is 670 is the value for encoders and 4.5 is a back up time for if there are not encoders
// 100 ticks per 1 revalition| 8in wheal| 167 in for first run| cir:25.12| 100 ticks / 25 in| 4 ticks / 1 in| 668 ticks / 167 in. 
			return Main::Mode::AUTO_LBWLS_WALL;
			
		}

		case Main::Mode::AUTO_LBWLS_MUP:
			if(!autonomous) return Main::Mode::TELEOP;
			//cout << "stall: " << status.drive.stall << endl;
			if(status.drive.stall) return Main::Mode::AUTO_LBWLS_BACK;// stall is in drive base and is base on motor currnt 
			return Main::Mode::AUTO_LBWLS_MUP;

		case Main::Mode::AUTO_LBWLS_ROTATE:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 1) return Main::Mode::AUTO_LBWLS_C; // this mode turns after the first wall
			return Main::Mode::AUTO_LBWLS_ROTATE;

		case Main::Mode::AUTO_LBWLS_C:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 2) return Main::Mode::AUTO_LBWLS_TOWER; // this mode drives to the corner
			return Main::Mode::AUTO_LBWLS_C;

		case Main::Mode::AUTO_LBWLS_EJECT:
			if(since_switch > 1 || !autonomous) return Main::Mode::TELEOP; //shoots low
			return Main::Mode::AUTO_LBLS_SCORE_EJECT;

		case Main::Mode::AUTO_LBWLS_BACK:
			if(!autonomous) return Main::Mode::TELEOP;// drives back from the first wall
			if(since_switch >.5) return Main::Mode::AUTO_LBWLS_ROTATE;
			return Main::Mode::AUTO_LBWLS_BACK;

		case Main::Mode::AUTO_LBWLS_TOWER:
			if(!autonomous) return Main::Mode::TELEOP; 
			if(since_switch > 2) return Main::Mode::AUTO_LBWLS_BR;//drives to the tower
			return Main::Mode::AUTO_LBWLS_TOWER;

		case Main::Mode::AUTO_LBWLS_BR:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > .39) return Main::Mode::AUTO_LBWLS_EJECT; //rotates on the batter to shoot
			return Main::Mode::AUTO_LBWLS_BR;
//-----------------------------------------------------------------------------------------------------------------------------------------------
			case Main::Mode::AUTO_LBWHS_WALL:
		{
			if(!autonomous) return Main::Mode::TELEOP;
			if(encoder_differences.first >= 606|| since_switch > 4.5) return Main::Mode::AUTO_LBWHS_MUP;
// 100 ticks per 1 revalition| 8in wheal| 167 in for first run| cir:25.12| 100 ticks / 25 in| 4 ticks / 1 in| 668 ticks / 167 in.
			return Main::Mode::AUTO_LBWHS_WALL;
			
		}

		case Main::Mode::AUTO_LBWHS_MUP:
			if(!autonomous) return Main::Mode::TELEOP;
			//cout << "stall: " << status.drive.stall << endl;
			if(status.drive.stall) return Main::Mode::AUTO_LBWHS_BACK;
			return Main::Mode::AUTO_LBWHS_MUP;

		case Main::Mode::AUTO_LBWHS_ROTATE:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 1) return Main::Mode::AUTO_LBWHS_C;
			return Main::Mode::AUTO_LBWHS_ROTATE;

		case Main::Mode::AUTO_LBWHS_C:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 1) return Main::Mode::AUTO_LBWHS_TOWER;
			return Main::Mode::AUTO_LBWHS_C;

		case Main::Mode::AUTO_LBWHS_EJECT:
			if(since_switch > 1 || !autonomous) return Main::Mode::TELEOP;
			return Main::Mode::AUTO_LBWHS_EJECT;

		case Main::Mode::AUTO_LBWHS_BACK:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch >.5) return Main::Mode::AUTO_LBWHS_ROTATE;
			return Main::Mode::AUTO_LBWHS_BACK;

		case Main::Mode::AUTO_LBWHS_TOWER:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 2.7) return Main::Mode::AUTO_LBWHS_BR;
			return Main::Mode::AUTO_LBWHS_TOWER;

		case Main::Mode::AUTO_LBWHS_BR:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > .7) return Main::Mode::AUTO_LBWHS_PREP;
			return Main::Mode::AUTO_LBWHS_BR;

		case Main::Mode::AUTO_LBWHS_PREP:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 3) return Main::Mode::AUTO_LBWHS_EJECT;
			return Main::Mode::AUTO_LBWHS_PREP;

		case Main::Mode::AUTO_LBWHS_BP:
			if(!autonomous) return Main::Mode::TELEOP;
			if(since_switch > 2) return Main::Mode::AUTO_LBWHS_PREP;
			return Main::Mode::AUTO_LBWHS_BP;
		
		case Main::Mode::AUTO_BR_STRAIGHTAWAY:
			{
				if(!autonomous) return Main::Mode::TELEOP;
				const double TARGET_DISTANCE=5.0*12.0;//in inches
				const double TOLERANCE = 6; //inches
				motion_profile.set_goal(TARGET_DISTANCE);
				cout<<"\n"<<encoder_differences.first<<"   "<<ticks_to_inches(encoder_differences.first)<<"   "<<TARGET_DISTANCE<<"\n";
				if(ticks_to_inches(encoder_differences.first) >= TARGET_DISTANCE-TOLERANCE && ticks_to_inches(encoder_differences.first) <= TARGET_DISTANCE+TOLERANCE){
					in_br_range.update(in.now,in.robot_mode.enabled);
				}
				else{
					in_br_range.set(2.0);
				}
				if(in_br_range.done()){
					set_initial_encoders=false;
					br_step++;
					return Main::Mode::TELEOP;
					//return Main::Mode::AUTO_BR_INITIALTURN;
				}
				return Main::Mode::AUTO_BR_STRAIGHTAWAY;
			}
		case Main::Mode::AUTO_BR_INITIALTURN:
			{
				if(!autonomous) return Main::Mode::TELEOP;
				const double TARGET_TURN=30;//in degrees, clockwise
				if(ticks_to_degrees(encoder_differences.first) >= TARGET_TURN){
					set_initial_encoders=true;
					return Main::Mode::AUTO_BR_SIDE;
				}
				return Main::Mode::AUTO_BR_INITIALTURN;
			
			}
		case Main::Mode::AUTO_BR_SIDE:
			{
				if(!autonomous) return Main::Mode::TELEOP;
				const double TARGET_DISTANCE=7.5*12;//in inches
				if(ticks_to_inches(encoder_differences.first) >= TARGET_DISTANCE){
					set_initial_encoders=true;
					br_step++;
					return Main::Mode::AUTO_BR_SIDETURN;
				}
				return Main::Mode::AUTO_BR_SIDE;
			}	
		case Main::Mode::AUTO_BR_SIDETURN:
			{
				if(!autonomous) return Main::Mode::TELEOP;
				const double TARGET_TURN=-300;//in degrees, clockwise
				if(ticks_to_degrees(encoder_differences.first) <= TARGET_TURN){
					set_initial_encoders=true;
					return Main::Mode::AUTO_BR_SIDE;
				}
				return Main::Mode::AUTO_BR_SIDETURN;
			}
		case Main::Mode::AUTO_BR_ENDTURN:
			{
				if(!autonomous) return Main::Mode::TELEOP;
				const double TARGET_TURN=150;//in degrees, clockwise
				if(ticks_to_degrees(encoder_differences.first) >= TARGET_TURN){
					set_initial_encoders=true;
					return Main::Mode::AUTO_BR_STRAIGHTAWAY;
				}
				return Main::Mode::AUTO_BR_ENDTURN;
			}
		
		default: assert(0);
	}
	return m;	
}*/

Robot_outputs Main::operator()(Robot_inputs in,ostream&){
	print_count++;

	perf.update(in.now);
	Joystick_data main_joystick=in.joystick[0];
	Joystick_data gunner_joystick=in.joystick[1];
	Panel panel=interpret(in.joystick[2]);

	if(!in.robot_mode.enabled){
	}

	force.update(
		main_joystick.button[Gamepad_button::A],
		main_joystick.button[Gamepad_button::LB],
		main_joystick.button[Gamepad_button::RB],
		main_joystick.button[Gamepad_button::BACK],
		main_joystick.button[Gamepad_button::B],
		main_joystick.button[Gamepad_button::X]
	);
	
	Toplevel::Status_detail toplevel_status=toplevel.estimator.get();
		
	//if(SLOW_PRINT) cout<<"panel:"<<panel<<"\n";
	//cout << "Goals: " << motion_profile.goal << " Current: " << ticks_to_inches(toplevel_status.drive.ticks.first/*in.digital_io.encoder[0]*/) << endl;
	
	if(SLOW_PRINT) cout<<"br_step:"<<br_step<<"\n";
	
	bool autonomous_start_now=autonomous_start(in.robot_mode.autonomous && in.robot_mode.enabled);
	since_auto_start.update(in.now,autonomous_start_now);
		
	Toplevel::Goal goals;
	//decltype(in.current) robotcurrent;
	//for(auto &a:robotcurrent) a = 0;
	/*if((toplevel_status.drive.ticks.first && initial_encoders.first==10000) || (toplevel_status.drive.ticks.second && initial_encoders.second==10000)) set_initial_encoders=true;
	if(set_initial_encoders){
		set_initial_encoders=false;
		cout<<"\nSET INITIAL ENCODER VALUES\n";
		initial_encoders = toplevel_status.drive.ticks;	
	}*/
	goals = mode_.run(Run_info{in,main_joystick,gunner_joystick,panel,toplevel_status});
	/*switch(mode){
		case Mode::TELEOP:
			goals=teleop(in,main_joystick,gunner_joystick,panel,toplevel_status,level,low,top,cheval,drawbridge);
			break;
		case Mode::DELAY:
			break;
		case Mode::AUTO_NULL:
			break;
		case Mode::AUTO_REACH:
			myfile2 << "RUN:ON" << endl;
			goals.drive.left=-.45;
			goals.drive.right=-.45;
			break;
		case Mode::AUTO_STATIC:
			goals.drive.left=-1;
			goals.drive.right=-1;
			break;
		case Mode::AUTO_STATICTWO:
			goals.drive.left=-.5;
			goals.drive.right=-.5;
			break;
		case Mode::AUTO_STOP:
			myfile2 << "RUN:OFF" << endl;
			goals.drive.left=0;
			goals.drive.right=0;
			break;
		case Mode::AUTO_TEST:
			goals.drive.left=.10;
			goals.drive.right=.10;
			break;
		case Mode::AUTO_PORTCULLIS:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;
			goals.collector.tilt=low;

			if(ready(toplevel_status.collector.tilt.angle,goals.collector.tilt)){
				goals.drive.left=-.50;
				goals.drive.right=-.50;
			}
			break;
		case Mode::AUTO_PORTCULLIS_DONE:
			//goals.drive.left=0;
			//goals.drive.right=0;
			goals.collector.tilt=top;
			break;

		case Mode::AUTO_CHEVALPOS:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;
			goals.drive.left=-.25;
			goals.drive.right=-.25;
			break;
		case Mode::AUTO_CHEVALWAIT:
			goals.drive.left=0;
			goals.drive.right=0;
			break;
		
		case Mode::AUTO_CHEVALDROP:
			goals.drive.left=0;
			goals.drive.right=0;
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;
			goals.collector.tilt=cheval;
			Main::topready=ready(toplevel_status.collector.tilt.angle,goals.collector.tilt);
			
			break;

		case Mode::AUTO_CHEVALDRIVE:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;
			goals.drive.right=-.5;
			goals.drive.left=-.5;
			break;
			
		case Mode::AUTO_CHEVALSTOW:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;
			goals.drive.right=-.5;
			goals.drive.left=-.5;
			goals.collector.tilt=top;
			break;
		case Main::Mode::AUTO_LBLS_CROSS_LB:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;

			goals.collector.tilt=low;
					
			if(ready(toplevel_status.collector.tilt.angle,goals.collector.tilt)){
				goals.drive.left=-.50;
				goals.drive.right=-.50;
			}
			break;

		case Main::Mode::AUTO_LBLS_CROSS_MU:
			//encoderflag = false;
			//cout << "FLAG FALSE";
			goals.drive.left=0;
			goals.drive.right=0;
			goals.collector.front=Front::Goal::OFF;	
			goals.collector.sides=Sides::Goal::OFF;

			goals.collector.tilt=top;
			Main::topready=ready(toplevel_status.collector.tilt.angle,goals.collector.tilt);
			break;
		
		case Main::Mode::AUTO_LBLS_SCORE_SEEK:
			goals.drive.right=.50;
			goals.drive.left= -.50;
			break;
		case Main::Mode::AUTO_LBLS_SCORE_LOCATE:
			break;
		case Main::Mode::AUTO_LBLS_SCORE_CD:
			goals.drive.right=-.50;
			goals.drive.left=-.50;
			break;
		case Main::Mode::AUTO_LBLS_SCORE_EJECT:
			goals.drive.left=0;
			goals.drive.right=0;
			goals.collector={Front::Goal::OUT,Sides::Goal::OFF,top};
			break;
		case Main::Mode::AUTO_LBWLS_WALL:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;

			goals.collector.tilt=low;

			if(ready(toplevel_status.collector.tilt.angle,goals.collector.tilt)){
				goals.drive.left=-.54;
				goals.drive.right=-.50;
			}
			break;
		case Main::Mode::AUTO_LBWLS_MUP:
			//encoderflag = false;
			//cout << "FLAG FALSE";
			goals.drive.left=-.67;
			goals.drive.right=-.50;
			goals.collector.front=Front::Goal::OFF;	
			goals.collector.sides=Sides::Goal::OFF;

			goals.collector.tilt=top;
			Main::topready=ready(toplevel_status.collector.tilt.angle,goals.collector.tilt);
			break;
		case Main::Mode::AUTO_LBWLS_ROTATE:
			goals.drive.right=.50;
			goals.drive.left=-.50;
			break;
		case Main::Mode::AUTO_LBWLS_C:
			goals.drive.right=.50;
			goals.drive.left=.50;
			break;
		case Main::Mode::AUTO_LBWLS_EJECT:
			goals.drive.left=0;
			goals.drive.right=0;
			goals.collector={Front::Goal::OUT,Sides::Goal::OFF,top};
			break;
		case Main::Mode::AUTO_LBWLS_BACK:
			goals.drive.left=.2;
			goals.drive.right=.2;
			break;
		case Main::Mode::AUTO_LBWLS_TOWER:
			goals.drive.left=-.5;
			goals.drive.right=-.5;
			break;
		case Main::Mode::AUTO_LBWLS_BR:
			goals.drive.left=.5;
			goals.drive.right=-.5;
			break;
//-----------------------------------------------------------------------------------------------------------------------------------------------
		case Main::Mode::AUTO_LBWHS_WALL:
			goals.collector.front=Front::Goal::OFF;
			goals.collector.sides=Sides::Goal::OFF;

			goals.collector.tilt=low;

			goals.drive.left=-.6;
			goals.drive.right=-.6;

			if(ready(toplevel_status.collector.tilt.angle,goals.collector.tilt)){
				goals.drive.left=-.8;
				goals.drive.right=-.8;
			}
			break;
		case Main::Mode::AUTO_LBWHS_MUP:
			//encoderflag = false;
			//cout << "FLAG FALSE";
			goals.drive.left=-.5;
			goals.drive.right=-.5;
			goals.collector.front=Front::Goal::OFF;	
			goals.collector.sides=Sides::Goal::OFF;

			goals.collector.tilt=top;
			Main::topready=ready(toplevel_status.collector.tilt.angle,goals.collector.tilt);
			break;
		case Main::Mode::AUTO_LBWHS_ROTATE:
			goals.drive.right=-.50;
			goals.drive.left=.50;
			break;
		case Main::Mode::AUTO_LBWHS_C:
			goals.drive.right=-.50;
			goals.drive.left=-.50;
			break;
		case Main::Mode::AUTO_LBWHS_EJECT:
			goals.collector.front = Front::Goal::IN;
			goals.shooter=shoot_action(Panel::Shooter_mode::CLOSED_AUTO,0,false);
			break;
		case Main::Mode::AUTO_LBWHS_BACK:
			goals.drive.left=.2;
			goals.drive.right=.2;
			break;
		case Main::Mode::AUTO_LBWHS_TOWER:
			goals.drive.left=.5;
			goals.drive.right=.5;
			break;
		case Main::Mode::AUTO_LBWHS_BR:
			goals.drive.left=.5;
			goals.drive.right=-.5;
			break;	
		case Main::Mode::AUTO_LBWHS_PREP:
			goals.drive.left=.12;
			goals.drive.right=.12;
			goals.collector.front = Front::Goal::OFF;
			goals.shooter=shoot_action(Panel::Shooter_mode::CLOSED_AUTO,0,false);
			break;
		case Main::Mode::AUTO_LBWHS_BP:
			goals.drive.left=.35;
			goals.drive.right=.35;
			break;
		case Main::Mode::AUTO_BR_STRAIGHTAWAY:
			goals.drive.left=-motion_profile.target_speed(ticks_to_inches(toplevel_status.drive.ticks.first));
			goals.drive.right=-motion_profile.target_speed(ticks_to_inches(toplevel_status.drive.ticks.first));
			break;
		case Main::Mode::AUTO_BR_INITIALTURN:
			goals.drive.left=-.5;
			goals.drive.right=0;
			break;
		case Main::Mode::AUTO_BR_SIDE:
			goals.drive.left=-.5;
			goals.drive.right=-.5;
			break;
		case Main::Mode::AUTO_BR_SIDETURN:
			goals.drive.left=.5;
			goals.drive.right=0;
			break;
		case Main::Mode::AUTO_BR_ENDTURN:
			goals.drive.left=-.5;
			goals.drive.right=0;
			break;
		//shooter_protical call in here takes in robot inputs,toplevel goal,toplevel status detail
		default: assert(0);
	}*/
	auto next=mode_.next_mode(Next_mode_info{in.robot_mode.autonomous,autonomous_start_now,toplevel_status,since_switch.elapsed(),panel,in});
	if(SLOW_PRINT) cout<<"mode_: "<<mode_<<"\n";
	//next_mode(mode,in.robot_mode.autonomous,autonomous_start_now,toplevel_status,since_switch.elapsed(),panel,topready,in,initial_encoders,br_step,set_initial_encoders,motion_profile,in_br_range);
	
	since_switch.update(in.now,mode_/*mode*/!=next);
	mode_=next;
		
	Toplevel::Output r_out=control(toplevel_status,goals); 
	auto r=toplevel.output_applicator(Robot_outputs{},r_out);
	
	r=force(r);
	auto input=toplevel.input_reader(in);

	/*auto talonPower = Talon_srx_output();
	talonPower.power_level = .5;
	r.talon_srx[0]= talonPower;*/
	
	toplevel.estimator.update(
		in.now,
		input,
		toplevel.output_applicator(r)
	);
	log(in,toplevel_status,r);
	return r;
}

bool operator==(Main const& a,Main const& b){
	return a.force==b.force && 
		a.perf==b.perf && 
		a.toplevel==b.toplevel && 
		a.since_switch==b.since_switch && 
		a.since_auto_start==b.since_auto_start &&
		a.autonomous_start==b.autonomous_start;
}

bool operator!=(Main const& a,Main const& b){
	return !(a==b);
}

ostream& operator<<(ostream& o,Main const& m){
	o<<"Main(";
	o<<" "<<m.mode;
	o<<" "<<m.force;
	o<<" "<<m.perf;
	o<<" "<<m.toplevel;
	o<<" "<<m.since_switch;
	return o<<")";
}

bool approx_equal(Main a,Main b){
	if(a.force!=b.force) return 0;
	if(a.toplevel!=b.toplevel) return 0;
	return 1;
}

#ifdef MAIN_TEST
#include<fstream>
#include "monitor.h"

template<typename T>
vector<T> uniq(vector<T> v){
	vector<T> r;
	for(auto a:v){
		if(!(r.size()&&r[r.size()-1]==a)){
			r|=a;
		}
	}
	return r;
}

void test_teleop(){
	/*Main m;
	m.mode=Main::Mode::TELEOP;
	
	Robot_inputs in;
	in.now=0;
	in.robot_mode.autonomous=0;
	in.robot_mode.enabled=1;//enable robot
	in.joystick[2].button[1] = 1;//set tilt to auto
	in.joystick[2].button[2] = 1;//set sides to auto
	in.joystick[2].button[3] = 1;//set front to auto
	in.joystick[2].axis[2] = -1;//set all buttons to off

	const Time RUN_TIME=5;//in seconds
	int print_count=0;
	
	const Time INCREMENT=.01;
	
	while(in.now<=RUN_TIME){	
		static const Time PUSH_CHEVAL=1,RELEASE_CHEVAL=1.5,ARRIVE_AT_CHEVAL_GOAL=3;
		
		if(in.now >= PUSH_CHEVAL) in.joystick[2].axis[2] = .62;
		if(in.now >= RELEASE_CHEVAL) in.joystick[2].axis[2] = -1;
		
		stringstream ss;
		Robot_outputs out = m(in,ss);
		//Panel panel=interpret(in.joystick[2]);
		
		if(SLOW_PRINT){
			cout<<"Now: "<<in.now<<"    Panel buttons: "<<in.joystick[2].axis[2]<<"   PWM: "<<out.pwm[4];
			cout<<"    Left Wheels: "<<out.pwm[0]<<"    Right Wheels: "<<out.pwm[1]<<"\n";
		}
		
		in.now+=INCREMENT;
		print_count++;
	}*/
}

void update_pos(Pt &current_pos, Robot_outputs out, const Time INCREMENT){
	const double FT_PER_SEC = 10;//ft/sec assumed for full power for now and that different percent powers correspond to the same percent of that assumption
	const double RAD_PER_SEC = 1.96;//rad/sec assumed for now at full speed
	double x_diff = 0, y_diff = 0, theta_diff = 0;
	if (-out.pwm[0] == out.pwm[1]) {
		double dist = FT_PER_SEC * INCREMENT;
		x_diff = cos(current_pos.theta) * (dist * out.pwm[0]);
		y_diff = sin(current_pos.theta) * (dist * out.pwm[0]);
		if (fabs(x_diff) < .000001) x_diff = 0;
		if (fabs(y_diff) < .000001) y_diff = 0;
		//cout<<"\nx:"<<cos(current_pos.theta)<<"   y:"<<sin(current_pos.theta)<<"    x:"<<x_diff<<"    y:"<<y_diff<<"    t:"<<theta_diff<<"\n";
	} else {
		theta_diff = RAD_PER_SEC * INCREMENT * out.pwm[0];//assuming robot is either driving straight or turning in place
	}
	current_pos+={x_diff,y_diff,theta_diff};
}

void test_autonomous(Main::Mode mode){
	Main m;

	m.mode=mode;
	
	Robot_inputs in;
	in.now=0;
	in.robot_mode.autonomous=1;
	in.robot_mode.enabled=1;
	
	const Time RUN_TIME=4;//in seconds
	Pt pos;//0rad is right
	pos.theta=PI/2;//start facing forward
	
	const Time INCREMENT=.01;	
	int print_count=0;
	
	while(in.now<=RUN_TIME){
		stringstream ss;
		Robot_outputs out=m(in,ss);
		
		update_pos(pos,out,INCREMENT);
		
		if(SLOW_PRINT) cout<<"Now: "<<in.now<<"    Left wheels: "<<out.pwm[0]<<"     Right wheels: "<<out.pwm[1]<<"   Position: "<<pos<<"\n";
		
		in.now+=INCREMENT;
		print_count++;
	}
}

void test_modes(){
	const vector<Main::Mode> MODE_LIST{
		#define X(NAME) Main::Mode::NAME,
		MODES
		#undef X
	};
	for(Main::Mode mode:MODE_LIST){
		cout<<"\nTest mode: "<<mode<<"\n\n"; 
		if(mode==Main::Mode::TELEOP) test_teleop();
		else test_autonomous(mode);
	}
}


void test_next_mode(){
	//Main::Mode next_mode(Main::Mode m,bool autonomous,bool autonomous_start,Toplevel::Status_detail /*status*/,Time since_switch, Panel panel,unsigned int navindex,vector<Nav2::NavS> NavV,int & stepcounter,Nav2::aturn Aturn){
	/*vector<Main::Mode> MODE_LIST{
		#define X(NAME) Main::Mode::NAME,
		MODES
		#undef X
	};
	for(auto mode:MODE_LIST){
		Toplevel::Status_detail st=example((Toplevel::Status_detail*)nullptr);
		bool topready = true;
		Robot_inputs in;
		unsigned int br_step=0;
		bool set_initial_encoders=true;	
		Motion_profile motion_profile;
		Countdown_timer in_br_range;
		auto next=next_mode(mode,0,0,st,0,Panel{},topready,in,make_pair(0,0),br_step,set_initial_encoders,motion_profile,in_br_range);
		cout<<"Testing mode "<<mode<<" goes to "<<next<<"\n";
		assert(next==Main::Mode::TELEOP);
	}*/
}

int main(){
	//test_next_mode();
	test_modes();
}

#endif
