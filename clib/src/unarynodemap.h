#ifndef _UNARYNODEMAP_H
#define _UNARYNODEMAP_H

#include "nodemap.h"

namespace GF {

class UnaryNodeMap: public NodeMap {

public:
	virtual Node map(Node)=0;
	//virtual ~UnaryNodeMap()=0;
	// This class should have a destructor. jhrg 4/8/14
	virtual ~UnaryNodeMap()
	{
	}
private:
};

} // namespace GF

#endif /* _UNARYNODEMAP_H */
