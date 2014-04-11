#include <string>
#include "gridfield.h"
#include "gridfieldoperator.h"

namespace GF {

class MergeOp: public BinaryGridFieldOperator {
public:
	MergeOp(GridFieldOperator *left, GridFieldOperator *right);
	void Execute();
	static GridField *Merge(GridField *A, GridField *B);
private:
	static string newName(string Aname, string Bname);

};

} // namespace GF
