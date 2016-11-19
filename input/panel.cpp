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
		p.auto_select=interpret_10_turn_pot(auto_mode);
	}
	{//two position switches
		p.arm_pos = d.button[0];//TODO: assumed value
	}
	{//buttons
		#define X(button) p.button = false;
		BUTTONS
		#undef X
		
		#define AXIS_RANGE(axis, last, curr, next, var, val) if (axis > curr-(curr-last)/2 && axis < curr+(next-curr)/2) var = val;
		float op = d.axis[2];
		static const float DEFAULT=-1, GRABBER_OPEN=-.5, GRABBER_CLOSE=0, PREP=.5, SHOOT=.1;//TODO: assumed values
		AXIS_RANGE(op, DEFAULT, GRABBER_OPEN, GRABBER_CLOSE, p.grabber_open, 1)
		else AXIS_RANGE(op, GRABBER_OPEN, GRABBER_CLOSE, PREP, p.grabber_close, 1)
		else AXIS_RANGE(op, GRABBER_CLOSE, PREP, SHOOT, p.prep, 1)
		else AXIS_RANGE(op, PREP, SHOOT, 1.38, p.shoot, 1)
		#undef AXIS_RANGE
		
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
