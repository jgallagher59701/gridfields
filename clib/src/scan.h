#ifndef _SCANOP_H
#define _SCANOP_H

#include "gridfieldoperator.h"

namespace GF {

class ScanOp: public ZeroaryGridFieldOperator {
public:
	ScanOp(string filename, long offset) :
			filename(filename), offset(offset)
	{
	}
	;

	ScanOp() :
			filename(""), offset(0)
	{
	}
	;
	string filename;
	long offset;
	//virtual static GridField *Scan(string filename) = 0;
private:

};

} // namespace GF

#endif
