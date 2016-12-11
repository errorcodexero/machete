#ifndef GRABBER_H
#define GRABBER_H

#include "../util/interface.h"
#include <set>
#include "../util/countdown_timer.h"

struct Grabber{
	enum class Goal{OPEN,CLOSE};
	
	typedef Goal Output;

	struct Input{
		bool closed;//hall-effect
		bool enabled;
		Input();
		Input(bool,bool);
	};
	
	struct Input_reader{
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};

	enum class Status_detail{OPEN,OPENING,CLOSING,CLOSED};

	typedef Status_detail Status;	
	
	struct Output_applicator{
		Output operator()(Robot_outputs)const;
		Robot_outputs operator()(Robot_outputs,Output)const;
	};

	struct Estimator{
		Status_detail last;	
		Countdown_timer open_timer;
		Countdown_timer close_timer;
	
		void update(Time,Input,Output);
		Status_detail get()const;
	
		Estimator();
	};
	
	Input_reader input_reader;
	Output_applicator output_applicator;
	Estimator estimator;
};


std::set<Grabber::Goal> examples(Grabber::Goal*);
std::set<Grabber::Input> examples(Grabber::Input*);
std::set<Grabber::Status_detail> examples(Grabber::Status_detail*);

std::ostream& operator<<(std::ostream&,Grabber);
std::ostream& operator<<(std::ostream&,const Grabber::Input);
std::ostream& operator<<(std::ostream&,const Grabber::Goal);
std::ostream& operator<<(std::ostream&,const Grabber::Status_detail);

bool operator==(const Grabber::Input,const Grabber::Input);
bool operator!=(const Grabber::Input,const Grabber::Input);
bool operator<(const Grabber::Input,const Grabber::Input);

bool operator==(const Grabber::Estimator,const Grabber::Estimator);
bool operator!=(const Grabber::Estimator,const Grabber::Estimator);

Grabber::Output control(Grabber::Status,Grabber::Goal);
Grabber::Status status(Grabber::Status_detail);
bool ready(Grabber::Status,Grabber::Goal);

#endif
