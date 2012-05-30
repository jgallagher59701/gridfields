
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

#include <cstdlib> // for exit function



#include "../src/grid.h"
#include "../src/gridfield.h"
#include "../src/array.h"
#include "../src/remesh.h"
#include "../src/arrayreader.h"
#include "../src/apply.h"



Grid *makeGrid(int scale,const char *name) {
  CellArray *twocells;
  CellArray *onecells;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;

  bool wf;
  int i;
  twocells = new CellArray();
  for (i=0; i<scale/2; i++) {
    triangle[0] = i;
    triangle[1] = i+1;
    triangle[2] = i+2;
    twocells->addCellNodes(triangle, 3);
  }
  
  //twocells->print();
  //getchar(); 
  //onecells->print();
  
  //getchar(); 
  grid = new Grid(name, 2);
  grid->setImplicit0Cells(scale);
  grid->setKCells(twocells, 2);
  //grid->print(0);
  //getchar();
  return grid; 
}


Array *makeFloatArray(int size,const char *name) {
  Array *arr;
  arr = new Array(name, FLOAT, size);
  float *data;
  arr->getData(data);
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  return arr;  
}
GridField *makeGridField(int size,const char *gridname,const char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(12, "A");
  k = 0;
  data = makeFloatArray(12, "x");

  GF = new GridField(G);
  GF->AddAttribute(k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {


cout<<"TEST 1:"<<endl;
int ntot1=15;
int ntot2=15;
int ntot3=14;
int ntot4=15;
int size1=392;
int size2=364;


  CellArray *twocells;CellArray *twocells2;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;




//  ifstream input("data/trigrid1.txt");


    int n=size1;
    int m=3;
    
    std::vector<std::vector<double> >   tri1(n,std::vector<double>(m));

    ifstream myfile1("data/test1/trigrid1.txt");
    int a1 = 0;
    int b1 = 0;

    while(!myfile1.eof())
    {
      myfile1 >> tri1[b1][a1];
      if(a1 ==2)
      {
           a1=0;
           ++b1;
           myfile1 >> tri1[b1][a1];
      }
      a1++;
    }
  myfile1.close();
  bool wf;
  int i;
  twocells = new CellArray();
  for (i=0; i<size1; i++) {
    triangle[0] = tri1[i][0];
    triangle[1] = tri1[i][1];
    triangle[2] = tri1[i][2];
    twocells->addCellNodes(triangle, 3);
  }

  
    n=ntot1*ntot2;
    m=1;

    Array* x1=new Array("x",FLOAT,ntot1*ntot2);
    std::ifstream filex1("data/test1/xgrid1.txt",ifstream::in);
    float placehold;

    for(int loop=0;loop<ntot1*ntot2;loop++)
    { 
      filex1 >> placehold;
      x1->set(loop,placehold);

    }  
    filex1.close();
    n=ntot1*ntot2;
    m=1;
    std::ifstream filey1("data/test1/ygrid1.txt",ifstream::in);

    Array* y1=new Array("y",FLOAT,ntot1*ntot2);


    for(int loop=0;loop<ntot1*ntot2;loop++)
    {
      filey1 >> placehold;
      y1->set(loop,placehold);

    }  
    filey1.close();
  //onecells = new CellArray();
  //for (i=0; i<3200; i++) {
  //  segment[0] = ;
  //  segment[1] = i+1;
  //  onecells->addCellNodes(segment, 2);
  //}

    n=ntot1*ntot2;
    m=1;
    std::ifstream fileu1("data/test1/ugrid1.txt",ifstream::in);

    Array* u1=new Array("u",FLOAT,ntot1*ntot2);

    for(int loop=0;loop<ntot1*ntot2;loop++)
    {
      fileu1 >> placehold;
      u1->set(loop,placehold);

    }  
    fileu1.close();
    Grid* grid1 = new Grid("grid1", 2);
    grid1->setImplicit0Cells(ntot1*ntot2);
    grid1->setKCells(twocells, 2);
    GridField* GForig=new GridField(grid1);
     
    GForig->Bind(0,x1);
    GForig->Bind(0,y1);
    GForig->Bind(0,u1);




    std::ifstream filetri2("data/test1/trigrid2.txt",ifstream::in);

    n=size2;
    m=3;
    

    std::vector<std::vector<double> >   tri2(n,std::vector<double>(m));

    ifstream myfile("data/test1/trigrid2.txt");

    a1 = 0;
    b1 = 0;

    while(!myfile.eof())
    {
      myfile >> tri2[b1][a1];
      if(a1 ==2)
      {
           a1=0;
           ++b1;
           myfile >> tri2[b1][a1];
      }
      a1++;
    }
    filetri2.close();
  int a,b,c;
  twocells2 = new CellArray();
  for (i=0; i<size2; i++) {
    triangle[0] = tri2[i][0];
    triangle[1] = tri2[i][1];
    triangle[2] = tri2[i][2];
    twocells2->addCellNodes(triangle, 3);
    a=triangle[0];
    b=triangle[1];
    c=triangle[2];

  }



    n=ntot3*ntot4;
    m=1;
    std::ifstream filex2("data/test1/xgrid2.txt",ifstream::in);

    Array *x2=new Array("x",FLOAT,ntot3*ntot4);


    for(int loop=0;loop<ntot3*ntot4;loop++)
    {
      filex2 >> placehold;
      x2->set(loop,placehold);

    } 
    filex2.close();
    n=ntot3*ntot4;
    m=1;
    std::ifstream filey2("data/test1/ygrid2.txt",ifstream::in);

    Array *y2=new Array("y",FLOAT,ntot3*ntot4);


    for(int loop=0;loop<ntot3*ntot4;loop++)
    {
      filey2 >> placehold;
      y2->set(loop,placehold);

    }  
    filey2.close();
    Grid* grid2 = new Grid("grid2", 2);
    grid2->setImplicit0Cells(ntot3*ntot4);
    grid2->setKCells(twocells2, 2);
    GridField* GFnew=new GridField(grid2);
    GFnew->Bind(0,x2);
    GFnew->Bind(0,y2);
    
    Array* ua;
    vector<double> uout1;
    ofstream arrayData;

    RemeshOp *op1 = new RemeshOp::RemeshOp("x","y",GFnew, GForig);
    GFnew = op1->Remesh("x","y",GForig,GFnew);
  ua=GFnew->GetAttribute(0, "u");
  uout1=ua->makeArrayf();

    arrayData.open("data/test1/ugrid2.txt"); // File Creation(on C drive)

    for(int k=0;k<uout1.size();k++)
    {
        arrayData<<uout1.at(k)<<endl; //Outputs array to txtFile
    }
    arrayData.close();


delete x1;
delete y1;
delete u1;
delete twocells;
delete x2;
delete y2;
delete twocells2;
delete GForig;
delete GFnew;
delete op1;
////////////////
cout<<"TEST 2:"<<endl;

ntot1=40;
ntot2=80;
ntot3=40;
ntot4=60;
size1=6162;
size2=4602;




//  ifstream input("data/trigrid1.txt");


    n=size1;
    m=3;
    
    tri1.clear();
    tri1.resize(n,std::vector<double>(m));
    std::ifstream myfile3("data/test2/trigrid1.txt");
    a1 = 0;
    b1 = 0;

    while(!myfile3.eof())
    {
      myfile3 >> tri1[b1][a1];
      if(a1 ==2)
      {
           a1=0;
           ++b1;
           myfile3 >> tri1[b1][a1];
      }
      a1++;
    }



  twocells = new CellArray();
  for (i=0; i<size1; i++) {
    triangle[0] = tri1[i][0];
    triangle[1] = tri1[i][1];
    triangle[2] = tri1[i][2];
    twocells->addCellNodes(triangle, 3);
  }
  myfile3.close();

    n=ntot1*ntot2;
    m=1;
    x1=new Array("x",FLOAT,ntot1*ntot2);
    filex1.open("data/test2/xgrid1.txt");

    for(int loop=0;loop<ntot1*ntot2;loop++)
    { 
      filex1 >> placehold;
      x1->set(loop,placehold);

    }  
    filex1.close();

    n=ntot1*ntot2;
    m=1;
    filey1.open("data/test2/ygrid1.txt",ifstream::in);

    y1=new Array("y",FLOAT,ntot1*ntot2);


    for(int loop=0;loop<ntot1*ntot2;loop++)
    {
      filey1 >> placehold;
      y1->set(loop,placehold);

    }  
    filey1.close();

  //onecells = new CellArray();
  //for (i=0; i<3200; i++) {
  //  segment[0] = ;
  //  segment[1] = i+1;
  //  onecells->addCellNodes(segment, 2);
  //}

    n=ntot1*ntot2;
    m=1;
    fileu1.open("data/test2/ugrid1.txt",ifstream::in);

    u1=new Array("u",FLOAT,ntot1*ntot2);

    for(int loop=0;loop<ntot1*ntot2;loop++)
    {
      fileu1 >> placehold;
      u1->set(loop,placehold);

    }  
    fileu1.close();
    grid1 = new Grid("grid1", 2);
    grid1->setImplicit0Cells(ntot1*ntot2);
    grid1->setKCells(twocells, 2);
    GForig=new GridField(grid1);
     
    GForig->Bind(0,x1);
    GForig->Bind(0,y1);
    GForig->Bind(0,u1);
    



    n=size2;
    m=3;
    

    tri2.clear();
    tri2.resize(n,std::vector<double>(m));

    std::ifstream myfile2("data/test2/trigrid2.txt");

    a1 = 0;
    b1 = 0;

    while(!myfile2.eof())
    {
      myfile2 >> tri2[b1][a1];
      if(a1 ==2)
      {
           a1=0;
           ++b1;
           myfile2 >> tri2[b1][a1];
      }
      a1++;
    }
  myfile2.close();


  twocells2 = new CellArray();
  for (i=0; i<size2; i++) {
    triangle[0] = tri2[i][0];
    triangle[1] = tri2[i][1];
    triangle[2] = tri2[i][2];
    twocells2->addCellNodes(triangle, 3);
    a=triangle[0];
    b=triangle[1];
    c=triangle[2];

  }



    n=ntot3*ntot4;
    m=1;

    filex2.open("data/test2/xgrid2.txt");

    x2=new Array("x",FLOAT,ntot3*ntot4);


    for(int loop=0;loop<ntot3*ntot4;loop++)
    {
      filex2 >> placehold;
      x2->set(loop,placehold);

    } 
    filex2.close();
    n=ntot3*ntot4;
    m=1;
    filey2.open("data/test2/ygrid2.txt",ifstream::in);

    y2=new Array("y",FLOAT,ntot3*ntot4);


    for(int loop=0;loop<ntot3*ntot4;loop++)
    {
      filey2 >> placehold;
      y2->set(loop,placehold);

    }  
    filey2.close();

    grid2 = new Grid("grid2", 2);
    grid2->setImplicit0Cells(ntot3*ntot4);
    grid2->setKCells(twocells2, 2);
    GFnew=new GridField(grid2);
    GFnew->Bind(0,x2);
    GFnew->Bind(0,y2);
    


    RemeshOp * op2 = new RemeshOp::RemeshOp("x","y",GFnew, GForig);
    op2->Remesh("x","y",GForig,GFnew);
    //ua=GFnew->GetAttribute(0, "u");
    //uout1=ua->makeArrayf();

    //arrayData.open("data/test2/ugrid2.txt"); // File Creation(on C drive)

    //for(int k=0;k<uout1.size();k++)
    {
    //    arrayData<<uout1.at(k)<<endl; //Outputs array to txtFile
    }
    //arrayData.close();
}


