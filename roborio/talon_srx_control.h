#ifndef TALON_SRX_CONTROL_H
#define TALON_SRX_CONTROL_H

#include "../util/interface.h"
#include "../util/checked_array.h"

class CANTalon;

class Talon_srx_control{
	private:
	CANTalon *talon;
	
	Talon_srx_output out;
	//Talon_srx_output last_out;
	Talon_srx_input in;
	
	int since_query;
	
	public:
	#define TALON_SRX_MODES X(INIT) X(VOLTAGE) X(SPEED) X(DISABLE)
	enum class Mode{
		#define X(NAME) NAME,
		TALON_SRX_MODES
		#undef X
	};

	private:
	Mode mode;
	
	public:
	Talon_srx_control();
	~Talon_srx_control();
	explicit Talon_srx_control(int CANBusAddress);
	
	void init(int CANBusAddress);
	void set(Talon_srx_output, bool);
	void set(CAN_out, bool);
	Talon_srx_input get();
	friend std::ostream& operator<<(std::ostream&,Talon_srx_control);
};

class Talon_srx_controls{
	private:
	std::array<Talon_srx_control,Robot_outputs::TALON_SRX_OUTPUTS> talons;
	bool init_;
	std::array<unsigned int,Robot_outputs::TALON_SRX_OUTPUTS> addresses;//needs to be the same size as talons

	public:
	void init(Checked_array<CAN_out,Robot_outputs::CAN_OUTPUTS>);//TODO: look at length of array 
	void set(Checked_array<CAN_out,Robot_outputs::CAN_OUTPUTS> const&,Checked_array<bool,Robot_outputs::TALON_SRX_OUTPUTS> const&);
	//void set(Checked_array<Talon_srx_output,Robot_outputs::TALON_SRX_OUTPUTS> const&,Checked_array<bool,Robot_outputs::TALON_SRX_OUTPUTS> const&);
	std::array<Talon_srx_input,Robot_inputs::TALON_SRX_INPUTS> get();
	
	Talon_srx_controls();
	friend std::ostream& operator<<(std::ostream&,Talon_srx_controls const&);
};

std::ostream& operator<<(std::ostream&,Talon_srx_control::Mode);

#endif
