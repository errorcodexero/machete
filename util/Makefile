CXX = arm-frc-linux-gnueabi-g++
CXXFLAGS = -std=c++14

SRC = \
	bound_tracker.cpp\
	countdown_timer.cpp\
	countup_timer.cpp\
	debounce.cpp\
	driver_station_interface.cpp\
	fixVictor.cpp\
	input.cpp\
	interface.cpp\
	jag_interface.cpp\
	meta.cpp\
	motion_profile.cpp\
	nav2.cpp\
	nav.cpp\
	negedge_trigger.cpp\
	network.cpp\
	output.cpp\
	perf_tracker.cpp\
	point.cpp\
	posedge_toggle.cpp\
	posedge_trigger.cpp\
	posedge_trigger_debounce.cpp\
	pwm.cpp\
	settable_toggle.cpp\
	string_utils.cpp\
	tagger.cpp\
	type.cpp\
	util.cpp\
	visitable.cpp

OBJS = $(SRC:.cpp=.o)

util.a: $(OBJS)
	$(AR) r $@ $(OBJS)
