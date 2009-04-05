CPPC = g++
CC = gcc

VTKLIBS = -lvtkFiltering -lvtkCommon -lvtkRendering -lvtkIO 
#VTKLIBPATH = /usr/local/vtk5/lib
#VTKLIBPATH = /usr/local/lib/vtk
#VTKINCPATH = /usr/local/vtk5/include/vtk-5.0/
#VTKINCPATH = /usr/local/include/vtk

#/opt/intel_cc_80/bin/icc 
CFLAGS = -g -m64 -fPIC
LIBS = #-lvmon
INCL = -I. -I/opt/boost-1.30.2/ 
COMP = $(CPPC) $(CFLAGS) -c $< $(INCL) -isystem phony_sysinclude
LINK = $(CPPC) -static $(CFLAGS) -o $@ $^ $(LIBS)
COMPLINK = $(CPPC) $(CFLAGS) -o $@ $^ $(LIBS) $@.cc

NETCDFINCL = /usr/local/netcdf/include/
NETCDFLIBDIR = /usr/local/netcdf/lib/

VTKLIBPATH = /usr/lib64/ 
VTKINCPATH = /usr/include/vtk/

CODEDIR = ../code

OPERATOR = gridfieldoperator.o
APPLY = apply.o fparser.o
RESTRICT = restrict.o refrestrict.o expr.o $(APPLY)
MERGE = merge.o 
REGRID = regrid.o 
CROSS = cross.o 
ELIO = elcircfile.o elio.o
READOBJS = $(ELIO) $(ARRAYIO)
BIND = bind.o bindconstant.o $(READOBJS)
#SCAN = scanhgrid.o scanvgrid.o scaninternal.o bind.o
SCAN = scaninternal.o bind.o
OUTPUT = output.o datadump.o tonetcdf.o stuebe.o
JOIN = join.o expr.o
SIFT = sift.o 
CONNECT = connect.o triangle.o Delaunay.o
SUBAPPLY = subapply.o
GFIO = scaninternal.o $(OUTPUT)
TAG = tag.o iterate.o state.o
AGGREGATE = aggregate.o apply.o accumulate.o project.o fparser.o aggregations.o assignments.o tag.o
SIMPLEOPS = $(OPERATOR) $(RESTRICT) $(MERGE) $(CROSS) $(AGGREGATE) $(BIND)
OPERATOROBJS = $(SIMPLEOPS) $(SCAN) $(SIFT) $(TAG) 
TUPLEOBJS = tuple.o fparser.o $(UTILOBJS)
DATAOBJS = constarray.o array.o dataset.o rankeddataset.o $(TUPLEOBJS)
GRIDFIELDOBJS = gridfield.o gridfieldoperator.o grid.o $(CELLOBJS) $(DATAOBJS) 
CELLOBJS = cellarray.o cell.o implicit0cells.o
UTILOBJS = timing.o expr.o util.o CmdLine.o type.o
FILEDISPLAYOBJS = elio.o read63.o write63.o arrayreader.o arraywriter.o
ARRAYIO = arrayreader.o arraywriter.o datadump.o 
DISPLAYOBJS = vtkGridField.o visualize.o


# for 'triangle'
CSWITCHES = -O -DLINUX -I/usr/X11R6/include -L/usr/X11R6/lib
TRILIBDEFS = -DTRILIBRARY

all: $(OBJS)

clean:
	rm *.o

# Individual operator programs

vtkXOpenGLOffscreenRenderWindow.o: vtkXOpenGLOffScreenRenderWindow.cxx vtkXOpenGLOffScreenRenderWindow.h 
	$(COMP) -I$(VTKINCPATH)

wrap_vtkXOpenGLOffScreenRenderWindow.o: wrap_vtkXOpenGLOffScreenRenderWindow.cc vtkXOpenGLOffScreenRenderWindow.h
	$(COMP) -I$(VTKINCPATH) -I/usr/local/include/python2.4

vtkoff.so: wrap_vtkXOpenGLOffScreenRenderWindow.o vtkXOpenGLOffScreenRenderWindow.o
	g++ -static -static-libgcc -shared -g -pg -L/usr/local/lib/python2.4/ -o $@ $^ $(LIBS)

gfrestrict: gfrestrict.o $(RESTRICT) $(GRIDFIELDOBJS) $(GFIO) $(OPERATOR) $(ARRAYIO)
	$(LINK) 

gfrestrict.o: gfrestrict.cc gfops.h 
	$(COMP) 

gfscan: gfscan.o elcircfile.o elio.o $(OPERATOR) $(GRIDFIELDOBJS) $(GFIO) $(ARRAYIO)
	$(LINK) 

gfscan.o: gfscan.cc gfops.h
	$(COMP) 

gfapply: gfapply.o $(APPLY) $(OPERATOR) $(GRIDFIELDOBJS) $(GFIO) $(ARRAYIO)
	$(LINK) 

gfapply.o: gfapply.cc gfops.h
	$(COMP) 

gfbindvar: gfbindvar.o $(ELIO) $(BIND) $(OPERATOR) $(GRIDFIELDOBJS) $(GFIO) $(ARRAYIO)
	$(LINK) 

gfbindvar.o: gfbindvar.cc gfops.h
	$(COMP) 

# recipe tests

extstation : extstation.o $(READOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) corierecipes.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

extstation.o : extstation.cc extstation.h
	$(COMP) -I$(VTKINCPATH)

gfview : gfview.o $(SCAN) $(GRIDFIELDOBJS) $(OPERATOROBJS) $(DISPLAYOBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

gfview.o : gfview.cc
	$(COMP) -I$(VTKINCPATH)

nestedgf : nestedgf.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) corierecipes.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

nestedgf.o : nestedgf.cc nestedgf.h visualize.o
	$(COMP) -I$(VTKINCPATH)

plugin : $(DISPLAYOBJS) plugin.o $(GRIDFIELDOBJS) $(OPERATOROBJS) corierecipes.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

plugin.o : plugin.cc corierecipes.h
	$(COMP) -I$(VTKINCPATH)

dataprods : dataprods.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) corierecipes.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

dataprods.o : dataprods.cc dataprods.h corierecipes.h
	$(COMP) -I$(VTKINCPATH)
corierecipes : corierecipes.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) 
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

corierecipes.o : corierecipes.cc corierecipes.h visualize.h
	$(COMP) -I$(VTKINCPATH)

transect : transect.o $(UTILOBJS) $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS) 

transect.o : transect.cc transect.h 
	$(COMP) -I$(VTKINCPATH)

isolines : isolines.o $(UTILOBJS) $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) corierecipes.o output.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS) 

isolines.o : isolines.cc visualize.h 
	$(COMP) -I$(VTKINCPATH)

scalar : scalar.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) 
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

scalar.o : scalar.cc visualize.h
	$(COMP) -I$(VTKINCPATH)

unopt-scalar : unopt-scalar.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS) 
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

unopt-scalar.o : unopt-scalar.cc visualize.h
	$(COMP) -I$(VTKINCPATH)

readtrans : readtrans.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS)  
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS) 

readtrans.o : readtrans.cc visualize.h assignments.h aggregations.h 
	$(COMP) -I$(VTKINCPATH)

transpose : transpose.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS)  
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS) 

transpose.o : transpose.cc visualize.h assignments.h aggregations.h 
	$(COMP) -I$(VTKINCPATH)

testvtk : testvtk.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

testvtk.o : testvtk.cc 
	$(COMP) -I$(VTKINCPATH)

wedgetest : wedgetest.o $(DISPLAYOBJS) $(GRIDFIELDOBJS) $(OPERATOROBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

wedgetest.o : wedgetest.cc
	$(COMP) -I$(VTKINCPATH)

bench.o : bench.cc
	$(COMP) -I$(VTKINCPATH)

# vtk comparison tests

vtkscalar : vtkscalar.o vtkCORIEReader.o $(UTILOBJS) visualize.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

vtkscalar.o : vtkscalar.cc
	$(COMP) -I$(VTKINCPATH)

vtkCORIEReader.o : vtkCORIEReader.cxx vtkCORIEReader.h
	$(COMP) -I$(VTKINCPATH)

visualize.o : visualize.cc visualize.h 
	$(COMP) -I$(VTKINCPATH)

vtkGridField.o : vis/vtkGridField.cxx vis/vtkGridField.h
	$(COMP) -I$(VTKINCPATH) -I.

elcircfile.o : elcircfile.cc elcircfile.h elio.o access.h
	$(COMP)

read63 : read63.o elio.o $(OPERATOROBJS) $(GRIDFIELDOBJS)
	$(LINK)

read63.o: read63.cc elio.h
	$(COMP)

write63 : write63.o elio.o $(OPERATOROBJS) $(GRIDFIELDOBJS)
	$(LINK)

write63.o: write63.cc elio.h type.h
	$(COMP)

elio.o: elio.c elio.h
	gcc -c $(CFLAGS) $<

datadump.o : datadump.cc datadump.h
	$(COMP)

arrayreader.o : arrayreader.cc arrayreader.h grid.h gridfield.h array.h
	$(COMP)

arraywriter.o : arraywriter.cc arraywriter.h grid.h gridfield.h array.h
	$(COMP)

# io unit tests

testarrayreader : testarrayreader.o arrayreader.o $(GRIDFIELDOBJS) elcircfile.o elio.o
	$(LINK) 

testarrayreader.o : testarrayreader.cc arrayreader.h grid.h gridfield.h array.h
	$(COMP) 

testwrite63 : testwrite63.o write63.o read63.o expr.o elio.o $(GRIDFIELDOBJS)
	$(LINK) 

testwrite63.o : testwrite63.cc write63.h 
	$(COMP) 

# operators
gridfieldoperator.o : gridfieldoperator.cc gridfieldoperator.h
	$(COMP)

sift.o : sift.cc sift.h
	$(COMP)

restrict.o : restrict.cc restrict.h apply.h gridfield.h
	$(COMP)

refrestrict.o : refrestrict.cc refrestrict.h apply.h gridfield.h
	$(COMP)

merge.o : merge.cc merge.h gridfield.h grid.h
	$(COMP)

aggregate.o : aggregate.cc aggregate.h assignments.h aggregations.h
	$(COMP)

aggregations.o : aggregations.cc aggregations.h
	$(COMP)

assignments.o : assignments.cc assignments.h
	$(COMP)

apply.o : apply.cc apply.h gridfield.h array.h
	$(COMP)

tag.o : tag.cc tag.h constarray.h array.h gridfield.h
	$(COMP)

accumulate.o : accumulate.cc accumulate.h gridfield.h
	$(COMP)

iterate.o : iterate.cc iterate.h state.h
	$(COMP)

state.o : state.cc state.h 
	$(COMP)

triangle.o : triangle.c triangle.h
	$(CC) -g $(CSWITCHES) $(TRILIBDEFS) -c -o triangle.o triangle.c

Delaunay.o : Delaunay.cpp Delaunay.h
	$(COMP)

connect.o : connect.cc connect.h Delaunay.h gridfield.h grid.h cellarray.h abstractcellarray.h
	$(COMP) -I$(VTKINCPATH)

project.o : project.cc project.h gridfield.h
	$(COMP)

cross.o : cross.cc cross.h crossordmap.h
	$(COMP)

join.o : join.cc join.h crossordmap.h
	$(COMP)

scanhgrid.o : scanhgrid.cc scanhgrid.h
	$(COMP)
scanvgrid.o : scanvgrid.cc scanvgrid.h
	$(COMP)

bindconstant.o : bindconstant.cc bindconstant.h
	$(COMP)

bind.o : bind.cc bind.h
	$(COMP)

scaninternal.o : scaninternal.cc scaninternal.h
	$(COMP)

subapply.o : subapply.cc subapply.h
	$(COMP)

regrid.o : regrid.cc regrid.h
	$(COMP)

output.o : output.cc output.h
	$(COMP) 

tonetcdf.o : tonetcdf.cc tonetcdf.h
	$(COMP) 

stuebe.o : stuebe.cc stuebe.h
	$(COMP) 

outputtuples.o : outputtuples.cc outputtuples.h
	$(COMP) 

# operator unit tests
testnetcdf : $(GRIDFIELDOBJS) $(OUTPUT) $(ARRAYIO) testnetcdf.o
	$(LINK) -L$(NETCDFLIBDIR) -lnetcdf_c++ -lnetcdf

testjoin : $(GRIDFIELDOBJS) testjoin.o $(JOIN) $(OPERATOR) arrayreader.o -lefence
	$(LINK)

testcross : $(GRIDFIELDOBJS) testcross.o $(CROSS) $(OPERATOR) arrayreader.o
	$(LINK)

selfe2netcdf : $(GRIDFIELDOBJS) selfe2netcdf.o $(SIMPLEOPS) $(BIND) $(GFIO) sift.o
	$(LINK) -L$(NETCDFLIBDIR) -lnetcdf_c++ -lnetcdf

testelcirc : $(GRIDFIELDOBJS) testelcirc.o $(SIMPLEOPS) $(BIND) $(GFIO) $(DISPLAYOBJS) sift.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS) -L$(NETCDFLIBDIR) -lnetcdf_c++ -lnetcdf

testtag : $(GRIDFIELDOBJS) testtag.o $(OPERATOROBJS) $(BIND) $(GFIO) $(DISPLAYOBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

testvis : $(GRIDFIELDOBJS) testvis.o $(OPERATOROBJS) $(DISPLAYOBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

testsmallrestrict : $(GRIDFIELDOBJS) testsmallrestrict.o $(OPERATOROBJS) $(BIND) $(GFIO) $(DISPLAYOBJS)
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

testaggregate : $(GRIDFIELDOBJS) testaggregate.o $(AGGREGATE) $(OPERATOR)
	$(LINK)

testrestrict : $(GRIDFIELDOBJS) testrestrict.o $(RESTRICT) apply.o $(OPERATOR) arrayreader.o
	$(LINK)

testmerge : $(GRIDFIELDOBJS) testmerge.o $(MERGE) $(RESTRICT) $(OPERATOR)
	$(LINK)

testscan : $(GRIDFIELDOBJS) testscan.o $(SCAN) $(OPERATOR) arrayreader.o elio.o
	$(LINK)

testbind : $(GRIDFIELDOBJS) testbind.o  $(OPERATOR) $(SIMPLEOPS) arrayreader.o elio.o elcircfile.o
	$(LINK)

testoutput : $(GRIDFIELDOBJS) $(OPERATOROBJS) testoutput.o $(OUTPUT) arrayreader.o arraywriter.o elio.o
	$(LINK) 

testsubapply : $(GRIDFIELDOBJS) testsubapply.o $(OPERATOROBJS) $(DISPLAYOBJS) expr.o
	$(LINK) -L$(VTKLIBPATH) $(VTKLIBS)

testnetcdf.o : testnetcdf.cc tonetcdf.h
	$(COMP)

testcross.o : testcross.cc onegrid.h 
	$(COMP)

testsmallrestrict.o : testsmallrestrict.cc 
	$(COMP) -I$(VTKINCPATH)

selfe2netcdf.o : selfe2netcdf.cc $(SIMPLEOPS)
	$(COMP) -I$(VTKINCPATH)

testelcirc.o : testelcirc.cc $(SIMPLEOPS)
	$(COMP) -I$(VTKINCPATH)

testtag.o : testtag.cc tag.cc tag.h aggregations.h assignments.h
	$(COMP) -I$(VTKINCPATH)

testvis.o : testvis.cc restrict.h vtkGridField.h gridfield.h 
	$(COMP) -I$(VTKINCPATH)

testjoin.o : testjoin.cc onegrid.h
	$(COMP)

testaggregate.o : testaggregate.cc aggregations.h assignments.h 
	$(COMP)

testregrid.o : testregrid.cc regrid.h 
	$(COMP)

testmerge.o : testmerge.cc merge.cc merge.h
	$(COMP)

testrestrict.o : testrestrict.cc restrict.cc restrict.h 
	$(COMP)

testscan.o : testscan.cc scanhgrid.h scanvgrid.h
	$(COMP)

testbind.o : testbind.cc bind.h bind.cc
	$(COMP)

testoutput.o : testoutput.cc rankeddataset.h gridfield.h grid.h output.h tonetcdf.h
	$(COMP) 

testsubapply.o : testsubapply.cc subapply.h
	$(COMP)



# gridfield components 
gridfield.o : gridfield.cc gridfield.h array.h rankeddataset.h
	$(COMP)

tuple.o : tuple.cc tuple.h type.h
	$(COMP) -I$(VTKINCPATH)

grid.o : grid.cc grid.h crossnodemap.h crossordmap.h
	$(COMP) 

unarynodemap.o : unarynodemap.cc 
	$(COMP) 

cell.o : cell.cc cell.h crossnodemap.h crossordmap.h
	$(COMP) -I$(VTKINCPATH)

cellarray.o : cellarray.cc cellarray.h timing.h
	$(COMP)

implicit0cells.o : implicit0cells.cc implicit0cells.h timing.h
	$(COMP)

rankeddataset.o : rankeddataset.cc rankeddataset.h dataset.h util.h
	$(COMP)

dataset.o : dataset.cc dataset.h util.h array.h
	$(COMP)

array.o : array.cc array.h expr.h access.h
	$(COMP)

constarray.o : constarray.cc constarray.h array.h tuple.h expr.h
	$(COMP)

# gridfield component unit tests

testgridfield : $(GRIDFIELDOBJS) testgridfield.o
	$(LINK)

testgridfield.o : testgridfield.cc 
	$(COMP)

testsift : $(GRIDFIELDOBJS) testsift.o sift.o
	$(LINK)

testsift.o : testsift.cc
	$(COMP)

testdataset : $(DATAOBJS) testdataset.o
	$(LINK)

testdataset.o : testdataset.cc testdataset.h testarray.h
	$(COMP)

testgrid : grid.o cellarray.o testgrid.o timing.o cell.o util.o implicit0cells.o
	$(LINK)

testgrid.o : testgrid.cc 
	$(COMP)

testcellarray : $(CELLOBJS) testcellarray.o timing.o util.o
	$(LINK)

testcellarray.o : testcellarray.cc cellarray.h abstractcellarray.h implicit0cells.h
	$(COMP)

testtuple : testtuple.o tuple.o expr.o type.o util.o fparser.o fparser.hh fptypes.hh fpconfig.hh util.h expr.h type.h
	$(LINK)

testtuple.o : testtuple.cc tuple.h
	$(COMP)

testarray.o : testarray.cc array.h util.h tuple.h expr.h type.h
	$(COMP)

testarray : array.o constarray.o testarray.o ${TUPLEOBJS}
	$(LINK) 

testexpr : testexpr.cc expr.o tuple.o fparser.o type.o
	g++ -g testexpr.cc expr.o tuple.o -lwayne fparser.o -o testexpr

netcdfadaptor.o : netcdfadaptor.cc netcdfadaptor.h gridfield.h cell.h
	$(COMP)

testnetcdfadaptor.o : tests/testnetcdfadaptor.cc gridfield.h netcdfadaptor.h
	$(COMP)

testnetcdfadaptor : testnetcdfadaptor.o netcdfadaptor.o $(GRIDFIELDOBJS) $(SIMPLEOPS) util.o
	$(LINK) -L$(NETCDFLIBDIR) -lnetcdf_c++ -lnetcdf

# utilities

CmdLine.o : CmdLine.cpp CmdLine.h
	$(COMP)

timing.o : timing.cc timing.h
	$(COMP)

fparser.o : fparser.cc fparser.hh fpconfig.hh fptypes.hh
	$(COMP)

type.o : type.cc type.h
	$(COMP)

expr.o : expr.cc expr.h 
	$(COMP)

util.o : util.cc util.h
	$(COMP)

# support for the build mechanism we're playing with
symlinks:
	rm -rf *.cc *.cxx *.h
	find $(CODEDIR) -type f -name \*.cc -exec ln -s {} . \;
	find $(CODEDIR) -type f -name \*.c -exec ln -s {} . \;
	find $(CODEDIR) -type f -name \*.cxx -exec ln -s {} . \;
	find $(CODEDIR) -type f -name \*.cpp -exec ln -s {} . \;
	find $(CODEDIR) -type f -name \*.h -exec ln -s {} . \;

todo : 
	find $(CODEDIR) -type f -name \todo  | cat > todo
