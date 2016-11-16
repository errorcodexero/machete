#ifndef PANEL_H
#define PANEL_H 

#include "../util/maybe.h"
#include "../util/interface.h"

struct Panel{
	bool in_use;
	//Buttons:
	bool manual_up;
	bool manual_down;
	bool grabber_open;
	bool grabber_close;
	bool rev_gun;
	bool shoot;
	//2 position swicthes:
	bool arm_mode;	
	//3 position switches: 
	enum class Arm_position{AUTO_DOWN,STAY,AUTO_UP};//momentary
	Arm_position arm_position;
	//10 position switches:
	int auto_switch;//0-9
	
	Panel();
};

bool operator!=(Panel const&,Panel const&);
std::ostream& operator<<(std::ostream&,Panel::Arm_position);
std::ostream& operator<<(std::ostream&,Panel);

Panel interpret(Joystick_data);
Joystick_data driver_station_input_rand();
Panel rand(Panel*);

#endif
