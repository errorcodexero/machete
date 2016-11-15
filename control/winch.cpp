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
	return {Winch::Goal::AUTO_DOWN, Winch::Goal::DOWN, Winch::Goal::UP, Winch::Goal::STOP, Winch::Goal::AUTO_UP};
}

std::set<Winch::Output> examples(Winch::Output*){
	return {Winch::Output::DOWN, Winch::Output::STOP, Winch::Output::UP};
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
	return {Winch::Status_detail::ERROR_,Winch::Status_detail::DOWN,Winch::Status_detail::GOING_DOWN,Winch::Status_detail::STOPPED,Winch::Status_detail::GOING_UP,Winch::Status_detail::UP};
}

std::ostream& operator<<(std::ostream& o,Winch::Goal g){
	#define X(name) if(g==Winch::Goal::name) return o<<"Winch::Goal("#name")";
	X(AUTO_DOWN) X(DOWN) X(UP) X(STOP) X(AUTO_UP)
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
	X(ERROR_)
	#undef X
	assert(0);
}

std::ostream& operator<<(std::ostream& o, Winch::Output a){
	#define X(OUT) if(a==Winch::Output::OUT) return o<<""#OUT;
	X(DOWN)
	X(STOP)
	X(UP)
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
	if(o==Winch::Output::UP) r.pwm[WINCH_PWM] = WINCH_POWER;
	else if(o==Winch::Output::DOWN) r.pwm[WINCH_PWM] = -WINCH_POWER;
	else r.pwm[WINCH_PWM] = 0;
	return r;
}

Winch::Output Winch::Output_applicator::operator()(Robot_outputs const& r)const{
	if(r.pwm[WINCH_PWM] > 0) return Winch::Output::UP;
	if(r.pwm[WINCH_PWM] < 0) return Winch::Output::DOWN;
	return Winch::Output::STOP;
}

void Winch::Estimator::update(Time /*time*/,Winch::Input input,Winch::Output output){
	switch(output){
		case Winch::Output::UP:
			last = Status::GOING_UP;
			break;
		case Winch::Output::DOWN:
			last = Status::GOING_DOWN;
			break;
		case Winch::Output::STOP:
			last = Status::STOPPED;
			break;
		default:
			assert(0);
	}
	if(input.down) last = Status::DOWN;
	if(input.up) last = Status::UP;
	if(input.down && input.up) last = Status::ERROR_;
}

Winch::Status Winch::Estimator::get()const{
	return last;
}

Winch::Output control(Winch::Status status,Winch::Goal goal){
	switch(goal){
		case Winch::Goal::AUTO_DOWN:
			if(ready(status,goal)) return Winch::Output::STOP;
			return Winch::Output::DOWN;
		case Winch::Goal::DOWN:
			return Winch::Output::DOWN;
		case Winch::Goal::STOP:
			return Winch::Output::STOP;
		case Winch::Goal::UP:
			return Winch::Output::UP;
		case Winch::Goal::AUTO_UP:
			if(ready(status, goal)) return Winch::Output::STOP;
			return Winch::Output::UP;
		default:
			assert(0);
	}
}

Winch::Status status(Winch::Status s){
	return s;
}

bool ready(Winch::Status status,Winch::Goal goal){
	switch(goal){
		case Winch::Goal::AUTO_DOWN:
			return true;
		case Winch::Goal::DOWN:
			return status == Winch::Status::DOWN;
		case Winch::Goal::STOP:
			return status == Winch::Status::STOPPED;
		case Winch::Goal::UP:
			return true;
		case Winch::Goal::AUTO_UP:
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
