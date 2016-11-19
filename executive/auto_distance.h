#ifndef BARRELRACER_H
#define BARRELRACER_H

#include "executive.h"
#include "../util/motion_profile.h"
#include "../util/quick.h"

#define AUTO_DISTANCE_ITEMS(X)\
	X(Motion_profile,motion_profile)\
	X(Drivebase::Encoder_ticks,initial_encoders)

struct Auto_distance: public Executive_impl<Auto_distance>{
	#define AUTO_DISTANCE_STRAIGHTAWAY_ITEMS(X)\
		X(Countdown_timer,in_range)\
		AUTO_DISTANCE_ITEMS(X)
	STRUCT_MEMBERS(AUTO_DISTANCE_STRAIGHTAWAY_ITEMS)
	
	Auto_distance(Drivebase::Encoder_ticks initial_encoders1):initial_encoders(initial_encoders1){}
	IMPL_STRUCT(Auto_distance,AUTO_DISTANCE_ITEMS)

	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_distance const&)const;
};

#endif
