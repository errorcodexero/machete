#include "arm.h"

static const int ARM_PWM = 5;
static const float ARM_POWER = .8;

#define PISTON_1_LOC 2 //not real number
#define PISTON_2_LOC 3 //not real number

Arm::Estimator::Estimator(){
	last = Status_detail::DOWN;
}

std::set<Arm::Goal> examples(Arm::Goal*){
	return {Arm::Goal::DOWN, Arm::Goal::UP};
}

std::set<Arm::Input> examples(Arm::Input*){
	return {{}};
}

std::set<Arm::Status_detail> examples(Arm::Status_detail*){
	return {Arm::Status_detail::DOWN,Arm::Status_detail::GOING_DOWN,Arm::Status_detail::GOING_UP,Arm::Status_detail::UP};
}

std::ostream& operator<<(std::ostream& o,Arm::Goal g){
	#define X(name) if(g==Arm::Goal::name) return o<<"Arm::Goal("#name")";
	X(DOWN) X(UP)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Arm::Input){
	return o<<"Input()";
}

std::ostream& operator<<(std::ostream& o,Arm::Status_detail a){
	#define X(STATUS) if(a==Arm::Status_detail::STATUS) return o<<""#STATUS;
	X(DOWN)
	X(GOING_DOWN)
	X(GOING_UP)
	X(UP)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Arm const&){
	return o<<"Arm()";
}

bool operator<(Arm::Input,Arm::Input){ 
	return false;
}
bool operator==(Arm::Input,Arm::Input){
	return true;
}
bool operator!=(Arm::Input a, Arm::Input b){ return !(a==b); }

bool operator==(Arm::Estimator,Arm::Estimator){ return 1; }
bool operator!=(Arm::Estimator a, Arm::Estimator b){ return !(a==b); }

bool operator==(Arm,Arm){ return 1; }
bool operator!=(Arm a, Arm b){ return !(a==b); }

Arm::Input Arm::Input_reader::operator()(Robot_inputs const&) const{
	return {};
}

Robot_inputs Arm::Input_reader::operator()(Robot_inputs r, Arm::Input) const{
	return r;
}

Robot_outputs Arm::Output_applicator::operator()(Robot_outputs r, Arm::Output o)const{
	r.solenoid[PISTON_1_LOC] = o == Arm::Output::UP;
	r.solenoid[PISTON_2_LOC] = o == Arm::Output::UP;
	return r;
}

Arm::Output Arm::Output_applicator::operator()(Robot_outputs const& r)const{
	assert(r.solenoid[PISTON_1_LOC] == r.solenoid[PISTON_2_LOC]);
	return r.solenoid[PISTON_1_LOC] ? Output::UP : Output::DOWN;
}

void Arm::Estimator::update(Time time,Arm::Input /*input*/,Arm::Output output){
	switch(output){
		case Arm::Output::UP:
			if(last == Status::GOING_UP){
				state_timer.update(time,true);//change to enabled
			} else if(state_timer.done() || last == Status::UP) {
				last = Status::UP;
			} else { 
				const Time UP_TIME = 1.0;//seconds. assumed
				last = Status::GOING_UP;
				state_timer.set(UP_TIME);
			}
			break;
		case Arm::Output::DOWN:
			if(last == Status::GOING_DOWN){

			} else if(state_timer.done() || last == Status::DOWN) { 
				last = Status::DOWN;
			} else { 
				const Time DOWN_TIME = 1.0;//seconds, assumed
				last = Status::GOING_DOWN;
				state_timer.set(DOWN_TIME);
			}
			break;
		default:
			assert(0);
	}
}

Arm::Status Arm::Estimator::get()const{
	return last;
}

Arm::Output control(Arm::Status,Arm::Goal goal){
	return goal;
}

Arm::Status status(Arm::Status s){
	return s;
}

bool ready(Arm::Status status,Arm::Goal goal){
	switch(goal){
		case Arm::Goal::DOWN:
			return status == Arm::Status::DOWN;
		case Arm::Goal::UP:
			return status == Arm::Status::UP;
		default:
			assert(0);
	}
	return false;
}

#ifdef ARM_TEST
#include "formal.h"
int main(){
	Arm a;
	tester(a);
}
#endif
