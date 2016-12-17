#ifndef GUN_H
#define GUN_H

#include <set>
#include "../util/interface.h"
#include "../util/countdown_timer.h"

struct Gun {
	struct Goal {
		public:
		#define GUN_GOAL_MODES \
			X(OFF) \
			X(REV) \
			X(SHOOT) \
			X(NUMBERED_SHOOT)
		#define X(name) name,
		enum class Mode{GUN_GOAL_MODES};
		#undef X

		private:
		Mode mode_;
		int to_shoot_;

		public:
		Goal();

		Mode mode()const;
		int to_shoot()const;

		static Goal off();
		static Goal rev();
		static Goal shoot();
		static Goal numbered_shoot(int);
	};

	enum class Output{OFF,REV,SHOOT};

	struct Input{
		bool enabled;
	};

	struct Status_detail {
		#define REV_MODES \
			X(OFF) \
			X(REVVING) \
			X(REVVED)
		#define X(name) name,
		enum class Rev_mode{REV_MODES};
		#undef X
		Rev_mode mode;
		int shots_fired;
	};

	typedef Status_detail Status;

	struct Input_reader {
		Input operator()(Robot_inputs const&)const;
		Robot_inputs operator()(Robot_inputs, Input)const;
	};
	
	struct Output_applicator{
		Output operator()(Robot_outputs)const;
		Robot_outputs operator()(Robot_outputs,Output)const;
	};
	
	struct Estimator{
		Status_detail last;
		Output last_output;
		Countdown_timer rev_timer;
		Countdown_timer shot_timer;
		
		void update(Time,Input,Output);
		Status_detail get()const;
		Estimator();
	};
	
	Input_reader input_reader;
	Output_applicator output_applicator;
	Estimator estimator;
};

std::ostream& operator<<(std::ostream&,Gun::Goal);
std::ostream& operator<<(std::ostream&,Gun::Output);
std::ostream& operator<<(std::ostream&,Gun::Input);
std::ostream& operator<<(std::ostream&,Gun::Status_detail);
std::ostream& operator<<(std::ostream&,Gun::Estimator);
std::ostream& operator<<(std::ostream&,Gun);

bool operator<(Gun::Goal,Gun::Goal);

bool operator==(Gun::Input,Gun::Input);
bool operator!=(Gun::Input,Gun::Input);
bool operator<(Gun::Input,Gun::Input);

bool operator==(Gun::Status_detail,Gun::Status_detail);
bool operator!=(Gun::Status_detail,Gun::Status_detail);
bool operator<(Gun::Status_detail,Gun::Status_detail);

bool operator==(Gun::Input_reader,Gun::Input_reader);
bool operator<(Gun::Input_reader,Gun::Input_reader);

bool operator==(Gun::Estimator,Gun::Estimator);
bool operator!=(Gun::Estimator,Gun::Estimator);

bool operator==(Gun::Output_applicator,Gun::Output_applicator);

bool operator==(Gun,Gun);
bool operator!=(Gun,Gun);

std::set<Gun::Input> examples(Gun::Input*);
std::set<Gun::Goal> examples(Gun::Goal*);
std::set<Gun::Output> examples(Gun::Output*);
std::set<Gun::Status_detail> examples(Gun::Status_detail*);

Gun::Output control(Gun::Status_detail,Gun::Goal);
Gun::Status status(Gun::Status_detail);
bool ready(Gun::Status,Gun::Goal);

#endif
