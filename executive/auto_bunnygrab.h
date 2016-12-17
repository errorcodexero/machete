#ifndef AUTO_BUNNYGRAB_H
#define AUTO_BUNNYGRAB_H

#include "executive.h"

struct Auto_bunnygrab: public Executive_impl<Auto_bunnygrab>{
	Executive next_mode(Next_mode_info);
	Toplevel::Goal run(Run_info);
	bool operator==(Auto_bunnygrab const&)const;
};

#endif
