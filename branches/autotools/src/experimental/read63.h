#ifndef _READ63_H
#define _READ63_H

#include "elio.h"
class GridField;

void GetHeader(ElcircHeader *h, char *fn); 
GridField *gfH(ElcircHeader &h);

GridField *gfV(ElcircHeader &h);

ElcircHeader *makeHeader(GridField *GF, ElcircHeader *h);

GridField *readHGrid(char *filename);
GridField *readVGrid(char *filename);
GridField *readHGrid(string filename);
GridField *readVGrid(string filename);

int newid(int node, int *map, int size); 

#endif /* _READ63_H */
