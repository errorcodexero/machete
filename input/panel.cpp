#include "panel.h"
#include <iostream>
#include <stdlib.h> 
#include "util.h"
#include "../util/util.h"
#include <cmath>

using namespace std;

Panel::Panel():
	in_use(0),
	manual_up(false),
	manual_down(false),
	grabber_open(false),
	grabber_close(false),
	rev_gun(false),
	shoot(false),
	arm_mode(false),
	arm_position(Arm_position::STAY),	
	auto_switch(0)
{}

ostream& operator<<(ostream& o,Panel::Arm_position a){
	o<<"Panel::Arm_position(";
	#define X(name) if(a==Panel::Arm_position::name) return o<<""#name")";
	X(AUTO_DOWN) X(STAY) X(AUTO_UP)
	#undef X
	assert(0);
}

#define BUTTONS \
	X(manual_up) X(manual_down) X(grabber_open) X(grabber_close) X(rev_gun) X(shoot)

#define TWO_POS_SWITCHES \
	X(arm_mode)

#define THREE_POS_SWITCHES \
	X(arm_position)

#define TEN_POS_SWITCHES \
	X(auto_switch)

#define DIALS 

#define PANEL_ITEMS \
	BUTTONS \
	TWO_POS_SWITCHES \
	THREE_POS_SWITCHES \
	TEN_POS_SWITCHES \
	DIALS

ostream& operator<<(ostream& o,Panel p){
	o<<"Panel(";
	o<<"in_use:"<<p.in_use;
	#define X(name) o<<", "#name":"<<p.name;
	PANEL_ITEMS
	#undef X
	return o<<")";
}

bool operator==(Panel const& a,Panel const& b){
	return 1
	#define X(NAME) && a.NAME==b.NAME
	PANEL_ITEMS
	#undef X
	;
}

bool operator!=(Panel const& a,Panel const& b){
	return !(a==b);
}

float axis_to_percent(double a){
	return .5-(a/2);
}

Panel interpret(Joystick_data d){
	Panel p;
	{
		p.in_use=[&](){
			for(int i=0;i<JOY_AXES;i++) {
				if(d.axis[i]!=0)return true;
			}
			for(int i=0;i<JOY_BUTTONS;i++) {
				if(d.button[i]!=0)return true;
			}
			return false;
		}();
		if (!p.in_use) return p;
	}
	{
		Volt auto_mode=d.axis[0];
		p.auto_switch=interpret_10_turn_pot(auto_mode);
	}
	/*p.lock_climber = d.button[0];
	p.tilt_auto = d.button[1];
	p.sides_auto = d.button[2];
	p.front_auto = d.button[3];
	#define AXIS_RANGE(axis, last, curr, next, var, val) if (axis > curr-(curr-last)/2 && axis < curr+(next-curr)/2) var = val;
	{
		float op = d.axis[2];
		static const float DEFAULT=-1, COLLECTOR_UP=-.8, COLLECTOR_DOWN=-.62, SHOOT_HIGH=-.45, COLLECT=-.29, SHOOT_LOW=-.11, SHOOT_PREP=.09, DRAWBRIDGE=.33, CHEVAL=.62, LEARN=1;
		#define X(button) p.button = 0;
		X(collector_up) X(collector_down) X(shoot_high) X(collect) X(shoot_low) X(shoot_prep) X(drawbridge) X(cheval) X(learn)
		#undef X
		AXIS_RANGE(op, DEFAULT, COLLECTOR_UP, COLLECTOR_DOWN, p.collector_up, 1)
		else AXIS_RANGE(op, COLLECTOR_UP, COLLECTOR_DOWN, SHOOT_HIGH, p.collector_down, 1)
		else AXIS_RANGE(op, COLLECTOR_DOWN, SHOOT_HIGH, COLLECT, p.shoot_high, 1)
		else AXIS_RANGE(op, SHOOT_HIGH, COLLECT, SHOOT_LOW, p.collect, 1)
		else AXIS_RANGE(op, COLLECT, SHOOT_LOW, SHOOT_PREP, p.shoot_low, 1)
		else AXIS_RANGE(op, SHOOT_LOW, SHOOT_PREP, DRAWBRIDGE, p.shoot_prep, 1)
		else AXIS_RANGE(op, SHOOT_PREP, DRAWBRIDGE, CHEVAL, p.drawbridge, 1)
		else AXIS_RANGE(op, DRAWBRIDGE, CHEVAL, LEARN, p.cheval, 1)
		else AXIS_RANGE(op, CHEVAL, LEARN, 1.38, p.learn, 1)
	}
	{
		float collector_pos = d.axis[5];
		static const float LOW=-1, DEFAULT=0, STOW=1;
		p.collector_pos = Panel::Collector_pos::LOW;
		AXIS_RANGE(collector_pos, LOW, DEFAULT, STOW, p.collector_pos, Panel::Collector_pos::DEFAULT)
		else AXIS_RANGE(collector_pos, DEFAULT, STOW, 1.5, p.collector_pos, Panel::Collector_pos::STOW)
	}
	{
		float front = d.axis[4];
		static const float OUT=-1, OFF=.48, IN=1;
		p.front = Panel::Collector::OUT;
		AXIS_RANGE(front, OUT, OFF, IN, p.front, Panel::Collector::OFF)
		else AXIS_RANGE(front, OFF, IN, 1.5, p.front, Panel::Collector::IN)
	}
	{
		float sides = d.axis[6];
		static const float OUT=-1, OFF=0, IN=1;
		p.sides = Panel::Collector::OUT;
		AXIS_RANGE(sides, OUT, OFF, IN, p.sides, Panel::Collector::OFF)
		else AXIS_RANGE(sides, OFF, IN, 1.5, p.sides, Panel::Collector::IN)
	}
	{
		float winch = d.axis[3];
		static const float UP=-1, STOP=0, DOWN=1;
		p.winch = Panel::Winch::UP;
		AXIS_RANGE(winch, UP, STOP, DOWN, p.winch, Panel::Winch::STOP)
		else AXIS_RANGE(winch, STOP, DOWN, 1.5, p.winch, Panel::Winch::DOWN)
	}
	{
		//A three position switch connected to two digital inputs
		p.shooter_mode = Panel::Shooter_mode::CLOSED_MANUAL;
		if (d.button[5]) p.shooter_mode = Panel::Shooter_mode::CLOSED_AUTO;
		if (d.button[6]) p.shooter_mode = Panel::Shooter_mode::OPEN;
	}
	p.speed_dial = -d.axis[1];//axis_to_percent(d.axis[1]);*/
	#undef AXIS_RANGE
	return p;
}

Joystick_data driver_station_input_rand(){
	Joystick_data r;
	for(unsigned i=0;i<JOY_AXES;i++){
		r.axis[i]=(0.0+rand()%101)/100;
	}
	for(unsigned i=0;i<JOY_BUTTONS;i++){
		r.button[i]=rand()%2;
	}
	return r;
}

Panel rand(Panel*){
	return interpret(driver_station_input_rand());
}

#ifdef PANEL_TEST
int main(){
	Panel p;
	for(unsigned i=0;i<50;i++){
		interpret(driver_station_input_rand());
	}
	cout<<p<<"\n";
	return 0;
}
#endif
