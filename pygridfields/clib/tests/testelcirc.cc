#include "visualize.h"
#include "vtkGridField.h"
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

#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"

void testTime() {
  ElcircFile ef("/home/bill/testbed/data/1_salt.63");
  GridField *T = ef.readTGrid();
  ApplyOp a("tstep=tstep+1", 0, T);
  a.getResult()->GetAttribute(0,"tstep")->print();
  a.getResult()->GetAttribute(0,"tstamp")->print();
}


void testInterpolate() {

  string conditionT = "332000<x & x<350000 & 285000<y & y<300000";
  string conditionS = "330000<x & x<355000 & 280500<y & y<305000";
  //string conditionT = "336000<x & x<344000 & 290000<y & y<295000";
  //string conditionS = "336000<x & x<344000 & 290000<y & y<295000";

//==================================
// Prepare Elcirc Slice

  ElcircFile elcircf("/home/workspace/ccalmr/forecasts/forecasts_ref/2005/2005-150/run/1_salt.63");
  GridField *elcirc = elcircf.readHGrid();

  AccumulateOp ordelcirc(elcirc, 0, "hpos", "hpos+1", "0");

  GridField *eV = elcircf.readVGrid();

  stringstream ss;
  ss << "column-b+" << eV->Card(0) << endl;
  AccumulateOp column(&ordelcirc, 0, "column", ss.str(), "0");
  column.SetOffset(-1);
  
  ApplyOp addrV("addr=column+61-b", 0, &column);
  
  stringstream econd;
  econd << conditionS << " & 61 > (b-1)";
  RestrictOp bathymetry(econd.str(), 0, &addrV);

  ArrayReader *ear = elcircf.getVariableReader("salt", 0, "addr");
  BindOp elcircbind("salt", FLOAT, ear, 0, &bathymetry); 
  
  RestrictOp elcircclamp("salt > -99", 0, &elcircbind);
  
//===============================
// Prepare Selfe Slice
  
  ElcircFile selfef("/home/workspace/ccalmr/forecasts/forecasts_exp/2005/2005-150/run/1_salt.63");
  GridField *selfe = selfef.readHGrid();

  AccumulateOp ordselfe(selfe, 0, "hpos", "hpos+1", "0");

  RestrictOp szoom(conditionT, 0, &ordselfe);

  GridField *sV = selfef.readVGrid();
  stringstream sss;
  sss << "addr=hpos*" << sV->Card(0) << "+ 25";
  ApplyOp G(sss.str(), 0, &szoom);

  ArrayReader *sar = selfef.getVariableReader("salt", 0, "addr");

  BindOp selfebind("salt", FLOAT, sar, 0, &G);
  ApplyOp selfeclamp("salt = (salt<33)*(salt>0)*salt", 0, &selfebind);

 //--------------------------------------------
 /*
  vtkGridField *vtkT = toVTK(elcircclamp.getResult(), "salt");
  vtkGridField *vtkS = toVTK(selfeclamp.getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  //DirectVis(vtkS->GetOutput(), renWin);
  ViewTwo(vtkS->GetOutput(), vtkT->GetOutput(), renWin);
  */

  //RestrictOp zoomT(conditionT, elcirc);
  //RestrictOp zoomS(conditionS, selfe);
  string attr = "salt";
  
  GridField *Source = selfeclamp.getResult();
  //GridField *Source = zoomS.getResult();
  Source->GetGrid()->normalize();
  GridField *Target = elcircclamp.getResult();
  //GridField *Target = zoomT.getResult();
  Target->GetGrid()->normalize();
  
  GridFieldOperator *T0 = Target;
  GridFieldOperator *S0 = Source;

  Assign::Nodes nodes;
  Aggregate::area area("area");
  Aggregate::_average<float> avg_salt(attr);
  Aggregate::mkvector poly("poly");
  Aggregate::dotwo poly_area(poly, area);
  Aggregate::dotwo avg_poly_area(avg_salt, poly_area);
  AggregateOp S2(Source, 2, &nodes, &avg_poly_area, Source, 0);

  S2.getResult();
  S2.getResult()->GetScheme(0).print();
  
  Assign::fastcontainedby cont;
  Aggregate::_average<float> avg("avg"+attr, 10000);
  Aggregate::Count cnt;
  Aggregate::dotwo both(avg, cnt);

  AggregateOp map(Target, 0, &cont, &avg, &S2, 2);

  map.getResult()->GetScheme(0).print();

  ApplyOp newval("newval=avgavg"+attr+"", 0, &map);

  RestrictOp translate_prep("newval<35", 0, &newval);
  ApplyOp translate("diff=(newval-salt); x=x+26000", 0,  &translate_prep);

  newval.getResult()->GetAttribute(0, "newval")->print();
  translate.getResult()->GetAttribute(0, "diff")->print();
  vtkGridField *vtkS = toVTK(translate.getResult(), "newval");
  //vtkGridField *vtkT = toVTK(newval.getResult(), "salt");
  //vtkGridField *vtkT = toVTK(newval.getResult(), "salt");
  //vtkGridField *vtkT = toVTK(Source->getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkS->GetOutput(), renWin);
  //ViewTwo(vtkS->GetOutput(), vtkT->GetOutput(), renWin);
}

void testElev() {
  float start = gettime();
  ElcircFile ef("/home/workspace/ccalmr/forecasts/2003-184/run/1_elev.61");
  GridField *H = ef.readHGrid();
  AccumulateOp *ordH = new AccumulateOp(H, 0, "hpos", "hpos+1", "0");
  RestrictOp *zoom = new RestrictOp("(350000<x)&(x<355000)&(290000<y)&(y<310000)", 0, ordH);
  
  ArrayReader *ar = ef.getVariableReader("elev", 20, "hpos");
  BindOp *bind = new BindOp("elev", FLOAT, ar, 0, zoom);
  
  vtkGridField *vtkgrid = toVTK(bind->getResult(), "elev");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  cout << "Time: " << gettime() - start << endl;
  DirectVis(vtkgrid->GetOutput(), renWin);
}  

void testScalar3d( int argc, char *argv[] ) {
  float start = gettime();

  if (argc != 4) {
    cout << "<cmd> <run directory> <run day> <timestep as 1-based record number>" << endl;
    exit(1);
  }
  string rundir = string(argv[1]);
  string runday = string(argv[2]);
  int timestep = atoi(argv[3]);

  ElcircFile ef(rundir + runday + string("_salt.63"));
  ElcircFile z63(rundir + runday + string("_zcor.63"));
  ElcircFile temp63(rundir + runday + string("_temp.63"));
  ElcircFile vert63(rundir + runday + string("_vert.63"));
  ElcircFile hvel64(rundir + runday + string("_hvel.64"));
  
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
  
  GridField *D = ef.readDGrid();
  D->print();
  //exit(1);
  
  stringstream ss;
  ss << "column-b+" << V->Size(0) << endl;
  AccumulateOp *column = new AccumulateOp(ordH, 0, "column", ss.str(), "0");
  column->SetOffset(-1);

  RestrictOp *zoom = new RestrictOp("(350000<x)&(x<370000)&(280000<y)&(y<315000)", 0, column);

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

  NcFile *ncdf = new NcFile("gridfield.nc", NcFile::Replace);

  ProjectOp *projH = new ProjectOp(zoom, 0, "node,x,y,h,b");
  ProjectOp *proj3D = new ProjectOp(mask, 0, "salt,temp,u,v,w");

  GridFieldOperator *Horiz = new OutputNetCDFDim(ncdf, 2, "node", projH);
  GridFieldOperator *Vert = new OutputNetCDFDim(ncdf, 0, "level", siftV);
  GridFieldOperator *Vars = new OutputNetCDFVars(ncdf, Scheme("node,level"), 0, proj3D, timestep*900);

  GridField *Result;
  Result = Horiz->getResult();
  Result = Vert->getResult();
  Result = Vars->getResult();
  

/*
  vtkGridField *vtkgrid = toVTK(mask->getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  cout << "Time: " << gettime() - start << endl;
  DirectVis(vtkgrid->GetOutput(), renWin);
*/
}

int main( int argc, char *argv[] ) {
  testScalar3d( argc, argv );
  //testInterpolate();
  //testElev();
  //testTime();
}
