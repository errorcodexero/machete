#include "winch.h"

static const int WINCH_PWM = 5;
static const float WINCH_POWER = .8;

#define LIMIT_LOC 4 //not real number

Winch::Input::Input():in(true){}
Winch::Input::Input(bool a):in(a){}

Winch::Estimator::Estimator(){
	const double EXTEND_TIME=2;//seconds, time assumed it takes the arm to extend fully
	out_timer.set(EXTEND_TIME);
}

std::set<Winch::Goal> examples(Winch::Goal*){
	return {Winch::Goal::IN, Winch::Goal::OUT, Winch::Goal::STOP};
}

std::set<Winch::Input> examples(Winch::Input*){
	return {{true},{false}};
}

std::set<Winch::Status_detail> examples(Winch::Status_detail*){
	return {Winch::Status_detail::IN,Winch::Status_detail::GOING_IN,Winch::Status_detail::STOPPED,Winch::Status_detail::GOING_OUT,Winch::Status_detail::OUT};
}

std::ostream& operator<<(std::ostream& o,Winch::Goal g){
	#define X(name) if(g==Winch::Goal::name) return o<<"Winch::Goal("#name")";
	X(IN) X(OUT) X(STOP)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Winch::Input a){
	return o<<"Input( in:"<<a.in<<")";
}

std::ostream& operator<<(std::ostream& o,Winch::Status_detail a){
	#define X(STATUS) if(a==Winch::Status_detail::STATUS) return o<<""#STATUS;
	X(IN)
	X(GOING_IN)
	X(STOPPED)
	X(GOING_OUT)
	X(OUT)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Winch const&){
	return o<<"Winch()";
}

bool operator<(Winch::Input a,Winch::Input b){ return a.in && !b.in; }
bool operator==(Winch::Input a,Winch::Input b){ return a.in == b.in; }
bool operator!=(Winch::Input a, Winch::Input b){ return !(a==b); }

bool operator==(Winch::Estimator,Winch::Estimator){ return 1; }
bool operator!=(Winch::Estimator a, Winch::Estimator b){ return !(a==b); }

bool operator==(Winch,Winch){ return 1; }
bool operator!=(Winch a, Winch b){ return !(a==b); }

Winch::Input Winch::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.digital_io.in[LIMIT_LOC] == Digital_in::_1};
}

Robot_inputs Winch::Input_reader::operator()(Robot_inputs r, Winch::Input in) const{
	r.digital_io.in[LIMIT_LOC] = in.in ? Digital_in::_1 : Digital_in::_0;
	return r;
}

Robot_outputs Winch::Output_applicator::operator()(Robot_outputs r, Winch::Output o)const{
	if(o==Winch::Goal::OUT) r.pwm[WINCH_PWM]=WINCH_POWER;
	else if(o==Winch::Goal::IN) r.pwm[WINCH_PWM]=-WINCH_POWER;
	else r.pwm[WINCH_PWM]=0;
	return r;
}

Winch::Goal Winch::Output_applicator::operator()(Robot_outputs const& r)const{
	if(r.pwm[WINCH_PWM]>0)	return Winch::Goal::OUT;
	if(r.pwm[WINCH_PWM]<0)	return Winch::Goal::IN;
	return Winch::Goal::STOP;
}

void Winch::Estimator::update(Time time,Winch::Input input,Winch::Goal goal){
	switch(goal){
		case Winch::Goal::OUT:
			out_timer.update(time,true);
			last = Status::GOING_OUT;
			break;
		case Winch::Goal::IN:
			last = Status::GOING_IN;
			break;
		case Winch::Goal::STOP:
			break;
		default:
			assert(0);
	}
	if(input.in) last = Status::IN;
	else if(out_timer.done()) last = Status::OUT;
}

Winch::Status Winch::Estimator::get()const{
	return last;
}

Winch::Output control(Winch::Status,Winch::Goal goal){
	return goal;
}

Winch::Status status(Winch::Status s){
	return s;
}

bool ready(Winch::Status status,Winch::Goal goal){
	switch(goal){
		case Winch::Goal::IN:
			return status == Winch::Status::IN;
		case Winch::Goal::STOP:
			return status == Winch::Status::STOPPED;
		case Winch::Goal::OUT:
			return status == Winch::Status::OUT;
		default:
			assert(0);
	}
	return false;
}

#ifdef WINCH_TEST
#include "formal.h"
int main(){
	Winch w;
	tester(w);
}
#endif
