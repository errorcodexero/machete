#ifndef AUTO_FORWARD_H
#define AUTO_FORWARD_H

#include "executive.h"

struct Auto_forward: public Executive_impl<Auto_forward>{
	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_forward const&)const;
};

#endif
