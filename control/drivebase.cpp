#include "drivebase.h"
#include "../util/fixVictor.h"
#include <iostream>
#include <math.h>
#include "../util/util.h"

using namespace std;


#define R_MOTOR_LOC_1 4
#define R_MOTOR_LOC_2 5
#define L_MOTOR_LOC_1 2
#define L_MOTOR_LOC_2 3

unsigned pdb_location(Drivebase::Motor m){
	#define X(NAME,INDEX) if(m==Drivebase::NAME) return INDEX;
	//WILL NEED CORRECT VALUES
	X(LEFT1,12)
	X(LEFT2,13)
	X(RIGHT1,14)
	X(RIGHT2,15)
	X(CENTER1,2)
	X(CENTER2,3)
	#undef X
	assert(0);
	//assert(m>=0 && m<Drivebase::MOTORS);
}

int encoderconv(Maybe_inline<Encoder_output> encoder){
	if(encoder) return *encoder;
	return 10000;
}

double ticks_to_inches(const int ticks){
	const unsigned int TICKS_PER_REVOLUTION=100;
	const double WHEEL_DIAMETER=7.4;//inches
	const double WHEEL_CIRCUMFERENCE=WHEEL_DIAMETER*PI;//inches
	const double INCHES_PER_TICK=WHEEL_CIRCUMFERENCE/(double)TICKS_PER_REVOLUTION;//0.25 vs 0.251327
	return ticks*INCHES_PER_TICK;
}

#define L_ENCODER_PORTS 0,1
#define R_ENCODER_PORTS 2,3
#define C_ENCODER_PORTS 4,5//doesn't exist?
#define L_ENCODER_LOC 0
#define R_ENCODER_LOC 1

Robot_inputs Drivebase::Input_reader::operator()(Robot_inputs all,Input in)const{
	for(unsigned i=0;i<MOTORS;i++){
		all.current[pdb_location((Motor)i)]=in.current[i];
	}
	/*auto set=[&](unsigned index,Digital_in value){
		all.digital_io.in[index]=value;
	};
	auto encoder=[&](unsigned a,unsigned b,Encoder_info e){
		set(a,e.first);
		set(b,e.second);
	};
	encoder(L_ENCODER_PORTS,in.left);
	encoder(R_ENCODER_PORTS,in.right);
	encoder(C_ENCODER_PORTS,in.center);
	all.digital_io.encoder[L_ENCODER_LOC] = in.ticks.first;
	all.digital_io.encoder[R_ENCODER_LOC] = in.ticks.second;*/
	return all;
}

Drivebase::Input Drivebase::Input_reader::operator()(Robot_inputs const& in)const{
	/*auto encoder_info=[&](unsigned a, unsigned b){
		return make_pair(in.digital_io.in[a],in.digital_io.in[b]);
	};*/
	return Drivebase::Input{
		[&](){
			array<double,Drivebase::MOTORS> r;
			for(unsigned i=0;i<Drivebase::MOTORS;i++){
				Drivebase::Motor m=(Drivebase::Motor)i;
				r[i]=in.current[pdb_location(m)];
			}
			return r;
		}(),
		/*encoder_info(L_ENCODER_PORTS),
		encoder_info(R_ENCODER_PORTS),
		encoder_info(C_ENCODER_PORTS),
		{encoderconv(in.digital_io.encoder[L_ENCODER_LOC]),encoderconv(in.digital_io.encoder[R_ENCODER_LOC])}*/
	};
}

float range(const Robot_inputs in){
	float volts=in.analog[2];
	const float voltsperinch=1; 
	float inches=volts*voltsperinch;
	return inches;
}

IMPL_STRUCT(Drivebase::Status::Status,DRIVEBASE_STATUS)
IMPL_STRUCT(Drivebase::Input::Input,DRIVEBASE_INPUT)
IMPL_STRUCT(Drivebase::Output::Output,DRIVEBASE_OUTPUT)

CMP_OPS(Drivebase::Input,DRIVEBASE_INPUT)

CMP_OPS(Drivebase::Status,DRIVEBASE_STATUS)

set<Drivebase::Status> examples(Drivebase::Status*){
	return {Drivebase::Status{
		array<Motor_check::Status,Drivebase::MOTORS>{
			Motor_check::Status::OK_,
			Motor_check::Status::OK_
		}
		,
		false,
		Drivebase::Piston::FULL//,
		//{0.0,0.0},
		//{0.0,0.0}
	}};
}

set<Drivebase::Goal> examples(Drivebase::Goal*){
	return {
		Drivebase::Goal{0,0,0},
		Drivebase::Goal{0,1,0}
	};
}

ostream& operator<<(ostream& o,Drivebase::Goal const& a){
	return o<<"Drivebase::Goal("<<a.x<<" "<<a.y<<" "<<a.theta<<")";
}

#define CMP(name) if(a.name<b.name) return 1; if(b.name<a.name) return 0;

bool operator<(Drivebase::Goal const& a,Drivebase::Goal const& b){
	CMP(x)
	CMP(y)
	CMP(theta)
	return 0;
}

CMP_OPS(Drivebase::Output,DRIVEBASE_OUTPUT)

set<Drivebase::Output> examples(Drivebase::Output*){
	return {
		Drivebase::Output{0,0,0,1},
		Drivebase::Output{1,1,0,0}
	};
}

set<Drivebase::Input> examples(Drivebase::Input*){
	/*auto d=Digital_in::_0;
	auto p=make_pair(d,d);*/
	return {Drivebase::Input{
		{0,0,0}//,p,p,p,{0,0}
	}};
}
Drivebase::Estimator::Estimator(){
	stall = false;
	piston_last = false;
	piston = Drivebase::Piston::EMPTYING;
	timer.set(.05);
	piston_timer.set(0);
	//last_ticks = {0,0};
	//speeds = {0.0,0.0};
}

Drivebase::Status_detail Drivebase::Estimator::get()const{
	array<Motor_check::Status,MOTORS> a;
	for(unsigned i=0;i<a.size();i++){
		a[i]=motor_check[i].get();
	}
	
	return Status{a,stall,piston/*,speeds,last_ticks*/};
}

ostream& operator<<(ostream& o,Drivebase::Piston a){
	#define X(NAME) if(a==Drivebase::Piston::NAME) return o<<""#NAME;
	X(FULL) X(EMPTY) X(FILLING) X(EMPTYING)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Drivebase::Output_applicator){
	return o<<"output_applicator";
}

ostream& operator<<(ostream& o,Drivebase const& a){
	return o<<"Drivebase("<<a.estimator.get()<<")";
}

double get_output(Drivebase::Output out,Drivebase::Motor m){
	#define X(NAME,POSITION) if(m==Drivebase::NAME) return out.POSITION;
	X(LEFT1,l)
	X(LEFT2,l)
	X(RIGHT1,r)
	X(RIGHT2,r)
	X(CENTER1,c)
	X(CENTER2,c)
	#undef X
	assert(0);
}
double sum(std::array<double, 6ul> a){
	double sum = 0;
	for(unsigned int i=0;i<a.size();i++)
		sum+=a[i];

	return sum;
}
double mean(std::array<double, 6ul> a){
	return sum(a)/a.size();
}
void Drivebase::Estimator::update(Time now,Drivebase::Input in,Drivebase::Output out){\
	timer.update(now,true);
	/*static const double POLL_TIME = .05;//seconds
	if(timer.done()){
		speeds.first = ticks_to_inches((last_ticks.first-in.ticks.first)/POLL_TIME);
		speeds.second = ticks_to_inches((last_ticks.second-in.ticks.second)/POLL_TIME);
		last_ticks = in.ticks;
		timer.set(POLL_TIME);
	}
	
	//cout << "Encoder in: " << in << endl;
	for(unsigned i=0;i<MOTORS;i++){
		Drivebase::Motor m=(Drivebase::Motor)i;
		auto current=in.current[i];
		auto set_power_level=get_output(out,m);
		motor_check[i].update(now,current,set_power_level);
		
	}*/
	if(out.piston==piston_last){
		piston_timer.update(now,true);//use enabled?
		if(piston_timer.done()){
			if(piston_last){
				piston=Piston::FULL;
			}else{
				piston=Piston::EMPTY;
			}
		}
	}else{
		if(out.piston){
			piston=Piston::FILLING;
		}else{
			piston=Piston::EMPTYING;
		}
		piston_last=out.piston;
		piston_timer.set(.2);//total guess
	}
	stall = mean(in.current) > 5;
}

Robot_outputs Drivebase::Output_applicator::operator()(Robot_outputs robot,Drivebase::Output b)const{
	robot.talon_srx[R_MOTOR_LOC_1].power_level = b.r;
	robot.talon_srx[R_MOTOR_LOC_2].power_level = b.r;
	robot.talon_srx[L_MOTOR_LOC_1].power_level = b.l;
	robot.talon_srx[L_MOTOR_LOC_2].power_level = b.l;
	robot.pwm[6]=pwm_convert(b.c);

	robot.solenoid[0] = b.piston;

	/*robot.digital_io[0]=Digital_out::encoder(0,1);
	robot.digital_io[1]=Digital_out::encoder(0,0);
	robot.digital_io[2]=Digital_out::encoder(1,1);
	robot.digital_io[3]=Digital_out::encoder(1,0);*/
	return robot;
}

Drivebase::Output Drivebase::Output_applicator::operator()(Robot_outputs robot)const{
	//assuming both motors on the same side are set to the same value//FIXME ?
	return Drivebase::Output{	
		robot.talon_srx[L_MOTOR_LOC_1].power_level,
		robot.talon_srx[R_MOTOR_LOC_1].power_level,
		from_pwm(robot.pwm[6]),
		robot.solenoid[0]
	};
}

bool operator==(Drivebase::Output_applicator const&,Drivebase::Output_applicator const&){
	return true;
}

bool operator==(Drivebase::Estimator const& a,Drivebase::Estimator const& b){
	for(unsigned i=0; i<Drivebase::MOTORS; i++){
		if(a.motor_check[i]!=b.motor_check[i])return false;
	}
	return true;
}

bool operator!=(Drivebase::Estimator const& a,Drivebase::Estimator const& b){
	return !(a==b);
}

bool operator==(Drivebase const& a,Drivebase const& b){
	return a.estimator==b.estimator && a.output_applicator==b.output_applicator;
}

bool operator!=(Drivebase const& a,Drivebase const& b){
	return !(a==b);
}

Drivebase::Output control(Drivebase::Status status,Drivebase::Goal goal){

	double l=goal.y+goal.theta;
	double r=goal.y-goal.theta;
	auto m=max(1.0,max(fabs(l),fabs(r)));

	auto main_wheel_portion=max(fabs(l),fabs(r));
	auto strafe_portion=fabs(goal.x);
	auto mostly_stationary=max(main_wheel_portion,strafe_portion)<.1;
	bool piston=[=]()->bool{
		if(mostly_stationary){
			switch(status.piston){
				case Drivebase::Piston::FULL:
				case Drivebase::Piston::FILLING:
					return 1;
				case Drivebase::Piston::EMPTY:
				case Drivebase::Piston::EMPTYING:
					return 0;
				default: assert(0);
			}
		}
		return strafe_portion>=main_wheel_portion/2;
	}();

	return Drivebase::Output{l/m,r/m,goal.x,piston};
}

Drivebase::Status status(Drivebase::Status a){ return a; }

bool ready(Drivebase::Status,Drivebase::Goal){ return 1; }

#ifdef DRIVEBASE_TEST
#include "formal.h"
int main(){
	Drivebase d;
	tester(d);
}
#endif
