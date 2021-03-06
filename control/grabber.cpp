#include "grabber.h"

using namespace std;

#define HALL_EFFECT_LOC 7 //sensor not on robot
#define PISTON_LOC 6 

Grabber::Input::Input():closed(false),enabled(false){}
Grabber::Input::Input(bool a,bool b):closed(a),enabled(b){}

Grabber::Estimator::Estimator():last(Grabber::Status_detail::OPEN){
	const Time OPEN_TIME = .2, CLOSE_TIME = .2;
	open_timer.set(OPEN_TIME);
	close_timer.set(CLOSE_TIME);
}

bool operator==(const Grabber::Input a,const Grabber::Input b){
	if(a.closed != b.closed) return false;
	return a.enabled == b.enabled;
}

bool operator!=(const Grabber::Input a,const Grabber::Input b){
	return !(a==b);
}

bool operator<(const Grabber::Input a,const Grabber::Input b){
	if(a.closed && !b.closed) return false;
	return !a.enabled && b.enabled;
}

bool operator==(const Grabber::Estimator a,const Grabber::Estimator b){
	if(a.last != b.last) return false;
	return a.open_timer == b.open_timer;
}

bool operator!=(const Grabber::Estimator a,const Grabber::Estimator b){
	return !(a==b);
}

ostream& operator<<(ostream& o, const Grabber::Estimator a){
	return o<<"Estimator(last:"<<a.last<<" open_timer:"<<a.open_timer<<")";
}

ostream& operator<<(ostream& o,const Grabber::Input a){
	return o<<" Input( closed:"<<a.closed<<" enabled:"<<a.enabled<<")";
}

ostream& operator<<(ostream& o,const Grabber::Goal a){
	#define X(GOAL) if(a==Grabber::Goal::GOAL) return o<<""#GOAL;
	X(OPEN) X(CLOSE)
	#undef X
	assert(0);
}

ostream& operator<<(ostream& o,const Grabber::Status_detail a){
	#define X(STATUS) if(a==Grabber::Status_detail::STATUS) return o<<""#STATUS;
	X(OPEN)
	X(OPENING)
	X(CLOSING)
	X(CLOSED)
	#undef X
	assert(0);
}

set<Grabber::Input> examples(Grabber::Input*){
	return {
		{false,false},
		{true,false},
		{false,true},
		{true,true}
	};
}

set<Grabber::Goal> examples(Grabber::Goal*){
	return {Grabber::Goal::OPEN,Grabber::Goal::CLOSE};
}

set<Grabber::Status_detail> examples(Grabber::Status_detail*){
	return {Grabber::Status_detail::OPEN,Grabber::Status_detail::OPENING,Grabber::Status_detail::CLOSING,Grabber::Status_detail::CLOSED};
}

ostream& operator<<(ostream& o,Grabber){
	return o<<"Grabber()";
}

Grabber::Status_detail Grabber::Estimator::get()const{
	return last;
}

void Grabber::Estimator::update(Time time,Input input,Output output){
	switch(output){
		case Grabber::Output::OPEN:
			if(last == Grabber::Status_detail::OPENING){
				open_timer.update(time,input.enabled);
			} else if(last != Grabber::Status_detail::OPEN){
				const Time OPEN_TIME = .2;//seconds .tested 
				open_timer.set(OPEN_TIME);
				last = Status_detail::OPENING;
			}
			if(open_timer.done()){
				 last = Grabber::Status_detail::OPEN;
			}
			break;
		case Grabber::Output::CLOSE:
			if(last == Grabber::Status_detail::CLOSING){
				close_timer.update(time,input.enabled);
			} else if(last != Grabber::Status_detail::CLOSED){
				const Time CLOSE_TIME = .2;//seconds tested
				close_timer.set(CLOSE_TIME);
				last = Status_detail::CLOSING;
			}
			if(close_timer.done()){
				last = Grabber::Status_detail::CLOSED;
			}
			//if(input.closed) last = Grabber::Status_detail::CLOSED;
			//else last = Grabber::Status_detail::CLOSING;
			break;
		default:
			assert(0);
	}
	
}

Grabber::Output control(Grabber::Status /*status*/, Grabber::Goal goal){
	return goal;
}

Robot_outputs Grabber::Output_applicator::operator()(Robot_outputs r,Output out)const{
	r.solenoid[PISTON_LOC] = out == Grabber::Output::OPEN; 
	return r;
}

Grabber::Output Grabber::Output_applicator::operator()(Robot_outputs r)const{
	return r.solenoid[PISTON_LOC] ? Grabber::Output::OPEN : Grabber::Output::CLOSE;
}

Robot_inputs Grabber::Input_reader::operator()(Robot_inputs r,Input in)const{
	r.digital_io.in[HALL_EFFECT_LOC] = in.closed ? Digital_in::_1 : Digital_in::_0;
	r.robot_mode.enabled = in.enabled;
	return r;
}

Grabber::Input Grabber::Input_reader::operator()(Robot_inputs r)const{
	return {(r.digital_io.in[HALL_EFFECT_LOC] == Digital_in::_1),r.robot_mode.enabled};
}

Grabber::Status status(Grabber::Status_detail status_detail){
	return status_detail;
}

bool ready(Grabber::Status status,Grabber::Goal goal){
	switch(goal){
		case Grabber::Goal::OPEN:
			return status == Grabber::Status::OPEN;
		case Grabber::Goal::CLOSE:
			return status == Grabber::Status::CLOSED;
		default:
			assert(0);
	}
	
	return false;
}

#ifdef GRABBER_TEST
#include "formal.h"

int main(){
	{
		Grabber g;
		tester(g);
	}
	{
		Grabber g;
		Grabber::Goal goal = Grabber::Goal::CLOSE;
		const bool ENABLED = true;
		const Time CLOSE_TIME = 5;//seconds - the amount of time before the simulated hall effect reads that it's closed
		for(Time t: range(1000)){
			bool closed = t >= CLOSE_TIME;
			Grabber::Status_detail status = g.estimator.get();
			Grabber::Output out = control(status,goal);
			
			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";
			
			g.estimator.update(t,Grabber::Input{closed,ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing.\n";
				break;
			}
		}

		goal = Grabber::Goal::OPEN;

		for(Time t: range(1000)){
			bool closed = g.estimator.get() == Grabber::Status_detail::CLOSED;
			Grabber::Status_detail status = g.estimator.get();
			Grabber::Output out = control(status,goal);
			
			cout<<"t:"<<t<<"\tgoal:"<<goal<<"\tstatus:"<<status<<"\n";
			
			g.estimator.update(t,Grabber::Input{closed,ENABLED},out);
			if(ready(status,goal)){
				cout<<"Goal "<<goal<<" reached. Finishing.\n";
				break;
			}
		}
	}
}
#endif
