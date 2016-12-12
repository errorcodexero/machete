#include "arm.h"

using namespace std;

#define PISTON_LOC 5 

Arm::Input::Input():enabled(false){}
Arm::Input::Input(bool a):enabled(a){}

Arm::Estimator::Estimator(){
	last = Status_detail::DOWN;
}

std::set<Arm::Goal> examples(Arm::Goal*){
	return {Arm::Goal::DOWN, Arm::Goal::UP};
}

std::set<Arm::Input> examples(Arm::Input*){
	return {
		{true},
		{false}
	};
}

std::set<Arm::Status_detail> examples(Arm::Status_detail*){
	return {Arm::Status_detail::DOWN,Arm::Status_detail::GOING_DOWN,Arm::Status_detail::GOING_UP,Arm::Status_detail::UP};
}

std::ostream& operator<<(std::ostream& o,Arm::Goal g){
	#define X(name) if(g==Arm::Goal::name) return o<<""#name;
	X(DOWN) X(UP)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Arm::Input a){
	return o<<"Input(enabled:"<<a.enabled<<")";
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

bool operator<(Arm::Input a,Arm::Input b){ 
	return !a.enabled && b.enabled;
}
bool operator==(Arm::Input a,Arm::Input b){
	return a.enabled == b.enabled;
}
bool operator!=(Arm::Input a, Arm::Input b){ return !(a==b); }

bool operator==(Arm::Estimator,Arm::Estimator){ return 1; }
bool operator!=(Arm::Estimator a, Arm::Estimator b){ return !(a==b); }
ostream& operator<<(ostream& o, Arm::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" state_timer:"<<a.state_timer<<")";
}

bool operator==(Arm,Arm){ return 1; }
bool operator!=(Arm a, Arm b){ return !(a==b); }

Arm::Input Arm::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.robot_mode.enabled};
}

Robot_inputs Arm::Input_reader::operator()(Robot_inputs r, Arm::Input in) const{
	r.robot_mode.enabled = in.enabled;
	return r;
}

Robot_outputs Arm::Output_applicator::operator()(Robot_outputs r, Arm::Output o)const{
	r.solenoid[PISTON_LOC] = o == Arm::Output::UP;
	return r;
}

Arm::Output Arm::Output_applicator::operator()(Robot_outputs const& r)const{
	return r.solenoid[PISTON_LOC] ? Output::UP : Output::DOWN;
}

void Arm::Estimator::update(Time time,Arm::Input input,Arm::Output output){
	switch(output){
		case Arm::Output::UP:
			if(last == Status::GOING_UP){
				state_timer.update(time,input.enabled);
			} else if(last != Status::UP){ 
				const Time UP_TIME = 1.0;//seconds. tested
				last = Status::GOING_UP;
				state_timer.set(UP_TIME);
			}
			if(state_timer.done() || last == Status::UP) {
				last = Status::UP;
			}
			break;
		case Arm::Output::DOWN:
			if(last == Status::GOING_DOWN){
				state_timer.update(time,input.enabled);
			} else if(last != Status::DOWN){ 
				const Time DOWN_TIME = 5.0;//seconds. tested
				last = Status::GOING_DOWN;
				state_timer.set(DOWN_TIME);
			}
			if(state_timer.done() || last == Status::DOWN) { 
				last = Status::DOWN;
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
	{
		Arm a;
		tester(a);
	}
	{
		Arm a;
		Arm::Goal goal = Arm::Goal::UP;

		const bool ENABLED = true;	
		for(Time t: range(100)){
			Arm::Status_detail status = a.estimator.get();
			Arm::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Arm::Input{ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}
		}

		goal = Arm::Goal::DOWN;
		
		for(Time t: range(100)){
			Arm::Status_detail status = a.estimator.get();
			Arm::Output out = control(status,goal);

			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";

			a.estimator.update(t,Arm::Input{ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing\n";
				break;
			}

		}
	}
}
#endif
