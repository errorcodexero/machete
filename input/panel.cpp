#include "panel.h"
#include <iostream>
#include <stdlib.h> 
#include "util.h"
#include "../util/util.h"
#include <cmath>

using namespace std;

#define BUTTONS \
	X(grabber_open) X(grabber_close) X(prep) X(shoot)

#define TWO_POS_SWITCHES \
	X(arm_pos)

#define THREE_POS_SWITCHES \

#define TEN_POS_SWITCHES \
	X(auto_select)

#define DIALS 

#define PANEL_ITEMS \
	BUTTONS \
	TWO_POS_SWITCHES \
	THREE_POS_SWITCHES \
	TEN_POS_SWITCHES \
	DIALS


Panel::Panel():
	in_use(0),
	#define X(BUTTON) BUTTON(false),
	BUTTONS
	#undef X
	#define X(TWO_POS_SWITCH) TWO_POS_SWITCH(false),
	TWO_POS_SWITCHES
	#undef X
	auto_select(0)
{}

ostream& operator<<(ostream& o,Panel p){
	o<<"Panel(";
	o<<"in_use:"<<p.in_use;
	#define X(NAME) o<<", "#NAME":"<<p.NAME;
	PANEL_ITEMS
	#undef X
	return o<<")";
}

bool operator==(Panel const& a,Panel const& b){
	return true
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

bool set_button(const Volt AXIS_VALUE, const Volt LOWER_VALUE, const Volt TESTING_VALUE, const Volt UPPER_VALUE){
	assert(LOWER_VALUE < TESTING_VALUE && TESTING_VALUE < UPPER_VALUE);
	float lower_tolerance = (TESTING_VALUE - LOWER_VALUE)/2;
	float upper_tolerance = (UPPER_VALUE - TESTING_VALUE)/2;
	float min = TESTING_VALUE - lower_tolerance;
	float max = TESTING_VALUE + upper_tolerance; 
	return (AXIS_VALUE > min && AXIS_VALUE < max);
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
		if(!p.in_use) return p;
	}
	{//set the auto mode number from the dial value
		Volt auto_dial_value = d.axis[0];
		p.auto_select = interpret_10_turn_pot(auto_dial_value);
	}
	{//two position switches
		p.arm_pos = d.button[0];//TODO: assumed value
	}
	{//buttons
		//sets all buttons to off beacuse we assume that only one should be pressed on this axis at a time
		#define X(button) p.button = false;
		BUTTONS
		#undef X
		
		const Volt AXIS_VALUE = d.axis[2];
		static const Volt DEFAULT=-1, GRABBER_OPEN=-.5, GRABBER_CLOSE=0, PREP=.5, SHOOT=1, ARTIFICIAL_MAX = 1.38;//TODO: assumed value, also recalculate the artifical maxs
		p.grabber_open = set_button(AXIS_VALUE, DEFAULT, GRABBER_OPEN, GRABBER_CLOSE);
		p.grabber_close = set_button(AXIS_VALUE, GRABBER_OPEN, GRABBER_CLOSE, PREP);
		p.prep = set_button(AXIS_VALUE, GRABBER_CLOSE, PREP, SHOOT);
		p.shoot = set_button(AXIS_VALUE, PREP, SHOOT, ARTIFICIAL_MAX);
		/*#define AXIS_RANGE(axis, last, curr, next, var, val) if (axis > curr-(curr-last)/2 && axis < curr+(next-curr)/2) var = val;
		AXIS_RANGE(op, DEFAULT, GRABBER_OPEN, GRABBER_CLOSE, p.grabber_open, 1)
		else AXIS_RANGE(op, GRABBER_OPEN, GRABBER_CLOSE, PREP, p.grabber_close, 1)
		else AXIS_RANGE(op, GRABBER_CLOSE, PREP, SHOOT, p.prep, 1)
		else AXIS_RANGE(op, PREP, SHOOT, 1.38, p.shoot, 1)
		#undef AXIS_RANGE*/
		
	}
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
