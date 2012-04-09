#include "grid.h"
#include "onegrid.h"
#include "gridfield.h"
#include "array.h"
#include "arrayreader.h"
#include "tag.h"
#include "iterate.h"
#include "state.h"
#include "apply.h"
#include "project.h"
#include "tonetcdf.h"
#include "merge.h"
#include "bind.h"
#include "cross.h"
#include "sift.h"
#include "restrict.h"
#include "elcircfile.h"
#include "accumulate.h"
#include "aggregations.h"
#include "assignments.h"
#include <sstream>
#include "timing.h"
#include "connect.h"
#include "catalog.h"
#include "bindconstant.h"
#include "datadump.h"
#include "output.h"
#include "scaninternal.h"


void selfe2netcdf( int argc, char *argv[] ) {
  float start = gettime();

  if (argc != 4) {
    cout << "<cmd> <run directory> <run day> <timestep as 1-based record number>" << endl;
    exit(1);
  }
  string rundir = string(argv[1]);
  string runday = string(argv[2]);
  int timestep = atoi(argv[3]) - 1; //convert to zero-based
  string filename("gridfield.nc");

  ElcircFile ef(rundir + runday + string("_salt.63"));
  ElcircFile z63(rundir + runday + string("_zcor.63"));
  ElcircFile temp63(rundir + runday + string("_temp.63"));
  ElcircFile vert63(rundir + runday + string("_vert.63"));
  ElcircFile hvel64(rundir + runday + string("_hvel.64"));

  float timestamp = (atoi(runday.c_str()) - 1) * ef.h.nsteps * ef.h.timestep + (timestep + 1) * ef.h.timestep;

  string starttime = string(ef.h.start_time);
  cout << starttime << ", " << timestep << ", " << timestamp  << endl;
  GridField *H = ef.readHGrid();
  AccumulateOp *ordH = new AccumulateOp(H, 0, "node", "node+1", "0");
  SiftOp *sift = new SiftOp(0,H);
  
  GridField *V = ef.readVGrid();
  AccumulateOp *ordV = new AccumulateOp(V, 0, "level", "level+1", "0");
  SiftOp *siftV = new SiftOp(0,ordV);
  //ordV->getResult()->print();

  ProjectOp *projV = new ProjectOp(siftV, 0, "level");

  Assign::Nodes nodes;
  Aggregate::_min<float> mn("z");
  Aggregate::_max<float> mx("z");
  Aggregate::dotwo minmax(mn, mx);
  AggregateOp V1(V, 1, &nodes, &minmax, V, 0);
  //GridField *gV1 = V1.getResult();
  //gV1->GetScheme(1).print();
  //getchar();
  
  //D->print();
  //exit(1);
  
  stringstream ss;
  ss << "column-b+" << V->Size(0) << endl;
  AccumulateOp *column = new AccumulateOp(ordH, 0, "column", ss.str(), "0");
  column->SetOffset(-1);

  // RestrictOp *zoom = new RestrictOp("(350000<x)&(x<370000)&(280000<y)&(y<315000)", 0, column);
  // just a tautology as a palceholder
  RestrictOp *zoom = new RestrictOp("node > -1", 0, column);


  RestrictOp *rV = new RestrictOp("(1560<z)&(z<4890)", 0, ordV);

  CrossOp *cross = new CrossOp(zoom, projV);

  ApplyOp *addrV = new ApplyOp("addr=column+level-b", 0, cross);

//  RestrictOp *restr = new RestrictOp("level > (b-1)", 0, addrV);
  GridFieldOperator *restr = addrV;

  ArrayReader *arz = z63.getVariableReader("z", timestep, "addr");
  BindOp *bindz = new BindOp("z", FLOAT, arz, 0, restr);

  ArrayReader *arsalt = ef.getVariableReader("salt", timestep, "addr");
  BindOp *bindsalt = new BindOp("salt", FLOAT, arsalt, 0, bindz);

  ArrayReader *artemp = temp63.getVariableReader("temp", timestep, "addr");
  BindOp *bindtemp = new BindOp("temp", FLOAT, artemp, 0, bindsalt);

  ArrayReader *arw = vert63.getVariableReader("w", timestep, "addr");
  BindOp *bindw = new BindOp("w", FLOAT, arw, 0, bindtemp);

  ArrayReader *aru = hvel64.getVariableReader("u", timestep, "addr");
  BindOp *bindu = new BindOp("u", FLOAT, aru, 0, bindw);

  ArrayReader *arv = hvel64.getVariableReader("v", timestep, "addr");
  BindOp *bindv = new BindOp("v", FLOAT, arv, 0, bindu);

  string nulls = string("salt=(level>(b-1))*salt + (level<=(b-1))*-99; ") +
                string("temp=(level>(b-1))*temp + (level<=(b-1))*-99; ") +
                string("z=(level>(b-1))*z + (level<=(b-1))*-99; ") +
                string("u=(level>(b-1))*u; ") +
                string("v=(level>(b-1))*v; ") + 
                string("w=(level>(b-1))*w");
              
  ApplyOp *mask = new ApplyOp(nulls, 0, bindv);

  NcFile *ncdf = new NcFile(filename.c_str(), NcFile::Replace);

  ArrayReader *arelev = ef.getSurfReader(timestep, "node");
  BindOp *bindelev = new BindOp("elev", FLOAT, arelev, 0, zoom);

  ProjectOp *projH = new ProjectOp(bindelev, 0, "node,x,y,h,b,elev");
  ProjectOp *proj3D = new ProjectOp(mask, 0, "salt,temp,u,v,w");

  OutputNetCDFDim *Horiz = new OutputNetCDFDim(ncdf, 2, "node", projH);
  GridFieldOperator *Vert = new OutputNetCDFDim(ncdf, 0, "level", siftV);
  OutputNetCDFVars *Vars = new OutputNetCDFVars(ncdf, Scheme("node,level"), 0, proj3D, timestep, timestamp);
  Vars->SetDate(starttime);

  GridField *Result;
  Result = Horiz->getResult();
  Result = Vert->getResult();
  Result = Vars->getResult();
  cout << filename << " created" << endl;

/*
  vtkGridField *vtkgrid = toVTK(mask->getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  cout << "Time: " << gettime() - start << endl;
  DirectVis(vtkgrid->GetOutput(), renWin);
*/
}

int main( int argc, char *argv[] ) {
  selfe2netcdf( argc, argv );
}
