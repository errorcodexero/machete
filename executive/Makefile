CXX = arm-frc-linux-gnueabi-g++
CXXFLAGS = -std=c++14

SRC = \
	auto_null.cpp\
	auto_stop.cpp\
	auto_test.cpp\
	delay.cpp\
	executive.cpp\
	step.cpp\
	teleop.cpp\
	test.cpp\
	timed.cpp

OBJS = $(SRC:.cpp=.o)

executive.a: $(OBJS)
	$(AR) r $@ $(OBJS)
