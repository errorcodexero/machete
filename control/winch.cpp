#include "winch.h"

static const int WINCH_PWM = 5;
static const float WINCH_POWER = .8;

#define HALL_EFFECT_DOWN_LOC 4 //not real number
#define HALL_EFFECT_UP_LOC 5 //not real number

Winch::Input::Input():down(true),up(false){}
Winch::Input::Input(bool a,bool b):down(a),up(b){}

Winch::Estimator::Estimator(){
	last = Status_detail::DOWN;
}

std::set<Winch::Goal> examples(Winch::Goal*){
	return {Winch::Goal::DOWN, Winch::Goal::UP, Winch::Goal::STOP};
}

std::set<Winch::Input> examples(Winch::Input*){
	return {
		{true,false},
		{false,false},
		{false,true},
		{true,true}//error case
	};
}

std::set<Winch::Status_detail> examples(Winch::Status_detail*){
	return {Winch::Status_detail::ERROR,Winch::Status_detail::DOWN,Winch::Status_detail::GOING_DOWN,Winch::Status_detail::STOPPED,Winch::Status_detail::GOING_UP,Winch::Status_detail::UP};
}

std::ostream& operator<<(std::ostream& o,Winch::Goal g){
	#define X(name) if(g==Winch::Goal::name) return o<<"Winch::Goal("#name")";
	X(DOWN) X(UP) X(STOP)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Winch::Input a){
	return o<<"Input( down:"<<a.down<<" up:"<<a.up<<")";
}

std::ostream& operator<<(std::ostream& o,Winch::Status_detail a){
	#define X(STATUS) if(a==Winch::Status_detail::STATUS) return o<<""#STATUS;
	X(DOWN)
	X(GOING_DOWN)
	X(STOPPED)
	X(GOING_UP)
	X(UP)
	X(ERROR)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Winch const&){
	return o<<"Winch()";
}

bool operator<(Winch::Input a,Winch::Input b){ 
	if(a.down && !b.down) return true;
	if(!a.up && b.up) return true;
	return false;
}
bool operator==(Winch::Input a,Winch::Input b){
	if(a.down != b.down) return false;
	return a.up == b.up;
}
bool operator!=(Winch::Input a, Winch::Input b){ return !(a==b); }

bool operator==(Winch::Estimator,Winch::Estimator){ return 1; }
bool operator!=(Winch::Estimator a, Winch::Estimator b){ return !(a==b); }

bool operator==(Winch,Winch){ return 1; }
bool operator!=(Winch a, Winch b){ return !(a==b); }

Winch::Input Winch::Input_reader::operator()(Robot_inputs const& r) const{
	return {r.digital_io.in[HALL_EFFECT_DOWN_LOC] == Digital_in::_1, r.digital_io.in[HALL_EFFECT_UP_LOC] == Digital_in::_1};
}

Robot_inputs Winch::Input_reader::operator()(Robot_inputs r, Winch::Input in) const{
	r.digital_io.in[HALL_EFFECT_DOWN_LOC] = in.down ? Digital_in::_1 : Digital_in::_0;
	r.digital_io.in[HALL_EFFECT_UP_LOC] = in.up ? Digital_in::_1 : Digital_in::_0;
	return r;
}

Robot_outputs Winch::Output_applicator::operator()(Robot_outputs r, Winch::Output o)const{
	if(o==Winch::Goal::UP) r.pwm[WINCH_PWM] = WINCH_POWER;
	else if(o==Winch::Goal::DOWN) r.pwm[WINCH_PWM] = -WINCH_POWER;
	else r.pwm[WINCH_PWM] = 0;
	return r;
}

Winch::Goal Winch::Output_applicator::operator()(Robot_outputs const& r)const{
	if(r.pwm[WINCH_PWM] > 0) return Winch::Goal::UP;
	if(r.pwm[WINCH_PWM] < 0) return Winch::Goal::DOWN;
	return Winch::Goal::STOP;
}

void Winch::Estimator::update(Time time,Winch::Input input,Winch::Goal goal){
	switch(goal){
		case Winch::Goal::UP:
			last = Status::GOING_UP;
			break;
		case Winch::Goal::DOWN:
			last = Status::GOING_DOWN;
			break;
		case Winch::Goal::STOP:
			last = Status::STOPPED;
			break;
		default:
			assert(0);
	}
	if(input.down) last = Status::DOWN;
	if(input.up) last = Status::UP;
	if(input.down && input.up) last = Status::ERROR;
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
		case Winch::Goal::DOWN:
			return status == Winch::Status::DOWN;
		case Winch::Goal::STOP:
			return status == Winch::Status::STOPPED;
		case Winch::Goal::UP:
			return status == Winch::Status::UP;
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
