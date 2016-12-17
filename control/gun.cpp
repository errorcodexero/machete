#include "gun.h"

using namespace std;

#define GUN_ADDRESS 0
#define POWER_ADDRESS 4
#define GUN_POWER -.3
#define REV_TIME 5
#define SHOT_TIME .25

ostream& operator<<(ostream& o,Gun::Goal::Mode a){
	#define X(name) if(a==Gun::Goal::Mode::name) return  o<<"Gun::Goal::Mode("#name")";
	GUN_GOAL_MODES
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,Gun::Goal a){
	o<<"Gun::Goal(";
	o<<" mode:"<<a.mode();
	if (a.mode()==Gun::Goal::Mode::NUMBERED_SHOOT) o<<" to_shoot:"<<a.to_shoot();
	return o<<")";
}

ostream& operator<<(ostream& o,Gun::Output a){
	#define X(name) if(a==Gun::Output::name) return  o<<"Gun::Output("#name")";
	X(OFF) X(REV) X(SHOOT)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o, Gun::Input a){
	return o<<"Gun::Input( enabled:"<<a.enabled<<")";
}

ostream& operator<<(ostream& o, Gun::Status_detail::Rev_mode a){
	#define X(name) if (a==Gun::Status_detail::Rev_mode::name) return o<<"Gun::Status_detail::Rev_mode("#name")";
	REV_MODES
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o, Gun::Status_detail a){
	o<<"Gun::Status_detail(";
	o<<" mode:"<<a.mode;
	return o<<")";
}

ostream& operator<<(ostream& o, Gun::Estimator a){
	return o<<"Gun::Estimator( last:"<<a.last<<" rev_timer:"<<a.rev_timer<<" shot_timer"<<a.shot_timer<<")";
}

ostream& operator<<(ostream& o, Gun g){
	return o<< "Gun( " << g.estimator << ")";
}

bool operator<(Gun::Goal a,Gun::Goal b){
	if (a.mode()<b.mode()) return 1;
	if (b.mode()<a.mode()) return 0;
	switch(a.mode()) {
		case Gun::Goal::Mode::OFF:
		case Gun::Goal::Mode::REV:
		case Gun::Goal::Mode::SHOOT:
			return 0;
		case Gun::Goal::Mode::NUMBERED_SHOOT:
			return a.to_shoot() < b.to_shoot();
		default: assert(0);
	}
}

bool operator==(Gun::Input a,Gun::Input b){ return a.enabled==b.enabled; }
bool operator!=(Gun::Input a,Gun::Input b){ return !(a==b); }
bool operator<(Gun::Input a,Gun::Input b){ return a.enabled<b.enabled; }

bool operator==(Gun::Status_detail a,Gun::Status_detail b){ return a.mode==b.mode && a.shots_fired==b.shots_fired; }
bool operator!=(Gun::Status_detail a,Gun::Status_detail b){ return !(a==b); }
bool operator<(Gun::Status_detail a,Gun::Status_detail b){
	if (a.mode<b.mode) return 1;
	if (b.mode<a.mode) return 0;
	if (a.shots_fired<b.shots_fired) return 1;
	return 0;
}

bool operator==(Gun::Input_reader,Gun::Input_reader){ return true; }
bool operator<(Gun::Input_reader,Gun::Input_reader){ return false; }

bool operator==(Gun::Estimator a,Gun::Estimator b){ return a.last==b.last && a.rev_timer==b.rev_timer && a.shot_timer==b.shot_timer; }
bool operator!=(Gun::Estimator a,Gun::Estimator b){ return !(a==b); }

bool operator==(Gun::Output_applicator,Gun::Output_applicator){ return true; }

bool operator==(Gun a,Gun b){ return (a.input_reader==b.input_reader && a.estimator==b.estimator && a.output_applicator==b.output_applicator); }
bool operator!=(Gun a,Gun b){ return !(a==b); }

Gun::Input Gun::Input_reader::operator()(Robot_inputs const& a)const{
	return Input{a.robot_mode.enabled};
}

Robot_inputs Gun::Input_reader::operator()(Robot_inputs r,Gun::Input a)const{
	r.robot_mode.enabled=a.enabled;
	return r;
}

Gun::Output Gun::Output_applicator::operator()(Robot_outputs r)const{
	auto v=r.relay[GUN_ADDRESS];
	return (v==Relay_output::_11? Gun::Output::SHOOT : (v==Relay_output::_01? Gun::Output::REV : Gun::Output::OFF));
}

Robot_outputs Gun::Output_applicator::operator()(Robot_outputs r,Gun::Output out)const{
	r.relay[GUN_ADDRESS]=[&]{
		switch(out){
			case Gun::Output::OFF: return Relay_output::_00;
			case Gun::Output::REV: return Relay_output::_01;
			case Gun::Output::SHOOT: return Relay_output::_11;
			default: assert(0);
		}
	}();
	r.pwm[POWER_ADDRESS]=GUN_POWER;
	return r;
}

Gun::Goal::Goal():mode_(Mode::OFF),to_shoot_(0){}

Gun::Goal::Mode Gun::Goal::mode()const{
	return mode_;
}

int Gun::Goal::to_shoot()const{
	return to_shoot_;
}

Gun::Goal Gun::Goal::off(){
	Goal a;
	a.mode_=Goal::Mode::OFF;
	return a;
}

Gun::Goal Gun::Goal::rev(){
	Goal a;
	a.mode_=Goal::Mode::REV;
	return a;
}

Gun::Goal Gun::Goal::shoot(){
	Goal a;
	a.mode_=Goal::Mode::SHOOT;
	return a;
}

Gun::Goal Gun::Goal::numbered_shoot(int darts){
	Goal a;
	a.mode_=Goal::Mode::NUMBERED_SHOOT;
	a.to_shoot_=darts;
	return a;
}

Gun::Estimator::Estimator():last({Gun::Status_detail::Rev_mode::OFF, 0}),rev_timer(),shot_timer(){}

Gun::Status_detail Gun::Estimator::get()const{ return last; }

void Gun::Estimator::update(Time time,Gun::Input in,Gun::Output output){
	rev_timer.update(time,in.enabled);
	shot_timer.update(time,in.enabled);
	switch(last.mode){
		case Status_detail::Rev_mode::REVVED:
			if(output==Output::OFF){
				last.mode=Status_detail::Rev_mode::OFF;
				last.shots_fired=0;
			}else if(output==Output::SHOOT){
				if(last_output==Output::REV) {
					last.shots_fired=0;
					shot_timer.set(SHOT_TIME);
				}
				if(shot_timer.done()){
					last.shots_fired++;
					shot_timer.set(SHOT_TIME);
				}
			}else if(output==Output::REV && last_output==Output::SHOOT) last.shots_fired=0;
			break;
		case Status_detail::Rev_mode::REVVING:
			if(output==Output::OFF){
				last.mode=Status_detail::Rev_mode::OFF;
			}
			if(rev_timer.done()){
				last.mode=Status_detail::Rev_mode::REVVED;
			}
			break;
		case Status_detail::Rev_mode::OFF:
			if(output==Output::REV){
				last.mode=Status_detail::Rev_mode::REVVING;
				rev_timer.set(REV_TIME);
			}
			break;
		default: assert(0);
	}
	last_output=output;
}

set<Gun::Input> examples(Gun::Input*){ return {{0}, {1}}; }

set<Gun::Goal> examples(Gun::Goal*){ return {Gun::Goal::off(), Gun::Goal::rev(), Gun::Goal::shoot(), Gun::Goal::numbered_shoot(1)}; }

set<Gun::Output> examples(Gun::Output*){ return {Gun::Output::OFF, Gun::Output::REV, Gun::Output::SHOOT}; }

set<Gun::Status_detail> examples(Gun::Status_detail*){
	return {
		{Gun::Status_detail::Rev_mode::OFF, 0},
		{Gun::Status_detail::Rev_mode::REVVING, 0},
		{Gun::Status_detail::Rev_mode::REVVED, 0}
	};
}

Gun::Output control(Gun::Status_detail status,Gun::Goal goal){
	switch(goal.mode()){
		case Gun::Goal::Mode::NUMBERED_SHOOT:
			if(status.shots_fired>=goal.to_shoot()) return Gun::Output::REV;
			if(status.mode==Gun::Status_detail::Rev_mode::REVVED) return Gun::Output::SHOOT;
			return Gun::Output::REV;
		case Gun::Goal::Mode::SHOOT:
			if(status.mode==Gun::Status_detail::Rev_mode::REVVED) return Gun::Output::SHOOT;
			return Gun::Output::REV;
		case Gun::Goal::Mode::REV:
			return Gun::Output::REV;
		case Gun::Goal::Mode::OFF:
			return Gun::Output::OFF;
		default: assert(0);
	}
}

Gun::Status status(Gun::Status_detail a){ return a; }

bool ready(Gun::Status status,Gun::Goal goal){
	switch(goal.mode()){
		case Gun::Goal::Mode::NUMBERED_SHOOT:
		case Gun::Goal::Mode::SHOOT:
		case Gun::Goal::Mode::REV:
			return status.mode==Gun::Status::Rev_mode::REVVED;
		case Gun::Goal::Mode::OFF:
			return status.mode==Gun::Status::Rev_mode::OFF;
		default: assert(0);
	}
}

#ifdef GUN_TEST
#include "formal.h"
int main(){
	Gun g;
	tester(g);
}
#endif
