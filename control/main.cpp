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

/*ostream& operator<<(ostream& o,Main::Mode a){
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
*/
//TODO: at some point, might want to make this whatever is right to start autonomous mode.
Main::Main():
	//mode(Mode::TELEOP),
	mode_(Executive{Teleop()}),
	/*motion_profile(0.0,.01),
	in_br_range(),*/
	autonomous_start(0)
{
	/*in_br_range.set(2.0);
	set_initial_encoders = true;
	initial_encoders = make_pair(0,0);
	br_step=0;
	myfile2.open(NAVLOG2);
	myfile2 << "test start" << endl;*/
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
		
	if(SLOW_PRINT) cout<<"panel:"<<panel<<"\n";
	//cout << "Goals: " << motion_profile.goal << " Current: " << ticks_to_inches(toplevel_status.drive.ticks.first/*in.digital_io.encoder[0]*/) << endl;
	
	//if(SLOW_PRINT) cout<<"br_step:"<<br_step<<"\n";
	
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
	
	auto next=mode_.next_mode(Next_mode_info{in.robot_mode.autonomous,autonomous_start_now,toplevel_status,since_switch.elapsed(),panel,in});
	if(SLOW_PRINT) cout<<"mode_: "<<mode_<<"\n";
	//next_mode(mode,in.robot_mode.autonomous,autonomous_start_now,toplevel_status,since_switch.elapsed(),panel,topready,in,initial_encoders,br_step,set_initial_encoders,motion_profile,in_br_range);
	
	since_switch.update(in.now,mode_/*mode*/!=next);
	mode_=next;
		
	Toplevel::Output r_out=control(toplevel_status,goals); 
	auto r=toplevel.output_applicator(Robot_outputs{},r_out);
	
	r.panel_output[Panel_outputs::SHOOT_READY] = Panel_output(
		static_cast<int>(Panel_output_ports::SHOOT_READY),
		goals.gun==Gun::Goal::OFF && ready(toplevel_status.gun,goals.gun)
	);
	
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
	//o<<" "<<m.mode;
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

/*void test_autonomous(Main::Mode mode){
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

*/
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
	//test_modes();
}

#endif
