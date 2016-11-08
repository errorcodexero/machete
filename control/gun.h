#ifdef GUN_H
#define GUN_H

#include <set>
#include "../util/interface.h"

struct Gun {
	enum class Goal{OFF,REV,SHOOT};

	typedef Goal Output;

	//#define GUN_INPUT_ITEMS(X)
	struct Input{

	};

	struct Status_detail{
		bool rev,shot;
		Status_detail();
		Status_detail(bool,bool);	

	}

	typedef Status_detail Status;

	struct Input_reader {
		Input operator()(Robot_inputs const&)const;
		Robot_inputs operator()(Robot_inputs, Inputs)const;
	};
	
	struct Output_applicator{
		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_output const&)const;
	};
	
	struct Estimator{
		Status_detail last;

		
		void update(Time,Input,Output);
		Status_detail get()const;
		Estimator();

	};
	
	Input_reader input_reader;
	Output_applicator output_applicatorl
	Estimator estimator;


};














}

#endif
