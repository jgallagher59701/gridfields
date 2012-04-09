
COMP = $(CC) $(CFLAGS) -c $< $(INCL)

VTKLIBPATH = /usr/lib64/ 
VTKINCPATH = /usr/include/vtk/

PYTHON_INCL = /usr/include/python2.5
PYTHON_CONF = /usr/lib/python2.5/config

NETCDFINCL = /usr/local/netcdf/include/
NETCDFLIBDIR = /usr/local/netcdf/lib/

SRC = ../src/

CC         = gcc
CXX        = g++
#/opt/intel_cc_80/bin/icc 
CFLAGS     = -g -Wno-deprecated -isystem$(SRC)/phony_sysinclude/ -fPIC
SWIGOPT    =
#SWIG       = /home/howew/local/bin/swig
SWIG       = /usr/bin/env swig


LDSHARED=	gcc -shared
CCSHARED=	-fPIC
CXXSHARED=      g++ -shared -O3 -isystem$(SRC)/phony_sysinclude -fPIC
#CXXSHARED=      g++ -shared -g  -isystem$(SRC)/phony_sysinclude -fPIC
# ========= Changes unlikely to be needed below this point =============

INTERFACES = gridfield.i gfvis.i
PYTHON_WRAPPERS = $(INTERFACES:.i=.py)

# for the gridfield module
CSRCS = elio.c 
CLIBS = 
CINCLUDES = 

OPERATOR_SOURCES = restrict.cc bind.cc bindconstant.cc cross.cc merge.cc \
                   project.cc apply.cc accumulate.cc \
                   aggregate.cc sift.cc aggregations.cc \
                   assignments.cc tag.cc iterate.cc state.cc  \
                   datadump.cc output.cc scaninternal.cc tonetcdf.cc stuebe.cc

IOSRCS = arrayreader.cc elcircfile.cc arraywriter.cc  
GFSRCS = tuple.cc gridfield.cc grid.cc array.cc cellarray.cc type.o \
         implicit0cells.cc cell.cc timing.cc util.cc dataset.cc rankeddataset.cc \
         gridfieldoperator.cc constarray.cc expr.cc fparser.cc

CXXSRCS = $(GFSRCS) $(IOSRCS) $(OPERATOR_SOURCES)

CXXOBJS = $(CXXSRCS:.cc=.o)
COBJS = $(CSRCS:.c=.o)

OBJS = $(CXXOBJS) $(COBJS)

CXXLIBS = -lm -L$(NETCDFLIBDIR) -lnetcdf_c++ -lnetcdf
CXXINCLUDES = -I$(SRC) -I$(SRC)/vis/ -I$(SRC)/phony_sysinclude #-I$(NETCDFINCL)

SWIG_MAKEFILE = Makefile.swig

# for the vtkgridfield integration module
VTKGFSRCS = vtkGridField.cxx 

VTKCXXOBJS = $(VTKGFSRCS:.cxx=.o) 
VTKOBJS = $(VTKCXXOBJS:.cc=.o)

VTKLIBS = -lvtkFiltering -lvtkCommon -lvtkRendering -lvtkIO -lvtkCommonPythonD
VTKLIB = -L$(VTKLIBPATH) $(VTKLIBS)
VTKINCL = -I$(VTKINCPATH)


# rules

all: gridfield gfvis

gridfield: objs
	$(MAKE) -f $(SWIG_MAKEFILE) OBJS='$(OBJS)' \
	CXXLIBS='$(CXXLIBS)' CLIBS='$(CLIBS)' \
        CINCLUDES='$(CINCLUDES)' CXXINCLUDES='$(CXXINCLUDES)' \
        PYTHON_INCL='$(PYTHON_INCL)' \
        PYTHON_CONF='$(PYTHON_CONF)' \
        CXX='$(CXX)' CC='$(CC)' CFLAGS='$(CFLAGS)'\
        SWIGOPT='$(SWIGOPT)' SWIG='$(SWIG)' \
        LDSHARED='$(LDSHARED)' CCSHARED='$(CCSHARED)' CXXSHARED='$(CXXSHARED)'\
        LANGUAGE='python' INTERFACE='gridfield.i' OBJPATH='$(SRC)'

objs: 
	$(MAKE) -f Makefile -C $(SRC) OBJS='$(OBJS)' \
        VTKLIBPATH='$(VTKLIBPATH)' \
        VTKINCPATH='$(VTKINCPATH)' \

gfvis: vtkobjs
	$(MAKE) -f $(SWIG_MAKEFILE) OBJS='$(VTKOBJS)' \
	CLIBS='$(CLIBS)' CXXLIBS='$(CXXLIBS) $(VTKLIB)' \
        CXXINCLUDES='$(CXXINCLUDES) $(VTKINCL)' \
        CINCLUDES='$(CINCLUDES)' \
        CXX='$(CXX)' CC='$(CC)' CFLAGS='$(CFLAGS)'\
        SWIGOPT='$(SWIGOPT)' SWIG='$(SWIG)' \
        LDSHARED='$(LDSHARED)' CCSHARED='$(CCSHARED)' CXXSHARED='$(CXXSHARED)'\
        PYTHON_INCL='$(PYTHON_INCL)' \
        PYTHON_CONF='$(PYTHON_CONF)' \
        LANGUAGE='python' INTERFACE='gfvis.i' OBJPATH='$(SRC)/vis/'
        
vtkobjs:
	$(MAKE) -f Makefile -C $(SRC) OBJS='$(VTKOBJS)' \
        VTKLIBPATH='$(VTKLIBPATH)' \
        VTKINCPATH='$(VTKINCPATH)' \
        
clean: 
	rm -f *.o *.so *_wrap* *~ .~* core  mypython *.pyc $(PYTHON_WRAPPERS) 
