CXX = arm-frc-linux-gnueabi-g++
CXXFLAGS = -std=c++14

SRC = \
	arm.cpp\
	drivebase.cpp\
	force.cpp\
	force_interface.cpp\
	grabber.cpp\
	gun.cpp\
	log.cpp\
	main.cpp\
	monitor.cpp\
	motor_check.cpp\
	nop.cpp\
	pump.cpp\
	sim.cpp\
	toplevel.cpp\
	tote_sensors.cpp\
	winch.cpp

OBJS = $(SRC:.cpp=.o)

control.a: $(OBJS)
	$(AR) r $@ $(OBJS)
