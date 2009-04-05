
#ifndef DATAPROD_H
#define DATAPROD_H
#include <string>

class GridField;


int scalar3D( GridField *H, GridField *V, const char *filename, 
              int addr, string region, string dataprod);

void HorizontalSlice(GridField *H, GridField *V, const char *filename, 
                    int addr, string region, int depth);

void VerticalSlice( GridField *H, GridField *V, const char *filename, 
                    int addr, string region);

void InterpolateVertSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region); 
void BadInterpolateVertSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region); 
#endif
