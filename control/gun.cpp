#include "gun.h"

using namespace std;


std::ostream& operator<<(std::ostream& o,Gun::Goal g){
	#define X(name) if(g==Gun::Goal::name) return  o<<"Gun::Goal("#name")";
	X(OFF) X(REV) X(SHOOT)
	#undef X
	assert(0);
}
std::ostream& operator<<(std::ostream& o, Gun::Status_detail a){
	return o<< "Gun::Status_detail(Rev: " << a.rev << " Shot: " << a.shot << ")";
}
std::ostream& operator<<(std::ostream& o, Gun::Estimator a){
	return o<< "Gun::Estimator( " << a.get() << ")";
}
std::ostream& operator<<(std::ostream& o, Gun g){
	return o<< "Gun( " << g.estimator << ")";
}
























#ifdef GUN_TEST
#include "formal.h"
int Main(){
	Gun g;
	tester(g);
}
#endif
