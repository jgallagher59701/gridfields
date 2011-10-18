# Tests

AUTOMAKE_OPTIONS = foreign

# Arrange to build with the backward compatibility mode enabled.
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/experimental \
	-I$(top_srcdir)/scratch
AM_CXXFLAGS =  

# These are not used by automake but are often useful for certain types of
# debugging. 
CXXFLAGS_DEBUG = -g3 -O0 -fno-defer-pop -Wall -W -Wcast-align -Werror
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

# This determines what gets built by make check
check_PROGRAMS = $(TESTS)

# This determines what gets run by 'make check.'
TESTS = testarrayreader testcross testaggregate	\
testrestrict testmerge testbind testgridfield	\
testsift testdataset testcellarray testtuple testarray

BROKEN_TESTS = testwrite63 testjoin testscan testgrid 

VTK_TESTS= testvtk wedgetest testtag testvis testsmallrestrict	\
testelcirc testsubapply

NETCDF_TESTS = testnetcdf selfe2netcdf testnetcdfadaptor testoutput 

if NETCDF_FOUND
TESTS += $(NETCDF_TESTS) 
endif

noinst_HEADERS = test.h	testarray.h testdataset.h transect.h

# EXTRA_DIST = 

# CLEANFILES = 

# DISTCLEANFILES = 

# Note that the gcov options -f and -b are useful but sometimes make looking
# at the results of coverage analysis a little taxing. -b reports on all
# branched and -f reports on all functions. The -l -o options summarize on a
# per-file basis. 3/27/98 jhrg
collect-coverage-data:
	(cd test-coverage; \
         cov_dat="coverage-data-`date +%m.%d.%y`"; \
         touch $$cov_dat; \
         for f in $(ALLSRCS); do \
            echo "\n*** Coverage data for $$f ***\n" >> $$cov_dat; \
            gcov -l -o ../ $$f >> $$cov_dat; \
         done)

#############################

if NETCDF_FOUND
testsmallrestrict_SOURCES = testsmallrestrict.o
testsmallrestrict_LDADD = testsmallrestrict.cc -L$(VTKLIBPATH) \
	$(VTKLIBS) ../libgridfields.la 

testsubapply_SOURCES = testsubapply.o
testsubapply_LDADD = testsubapply.cc -L$(VTKLIBPATH) $(VTKLIBS) ../libgridfields.la 

testnetcdfadaptor_SOURCES = testnetcdfadaptor.o
testnetcdfadaptor_LDADD = testnetcdfadaptor.cc \
	-lnetcdf_c++ -lnetcdf ../libgridfields.la 

testnetcdf_SOURCES = testnetcdf.o
testnetcdf_LDADD = testnetcdf.cc  \
	-lnetcdf ../libgridfields.la 

selfe2netcdf_SOURCES = selfe2netcdf.o 
elfe2netcdf_LDADD = selfe2netcdf.cc  \
	-lnetcdf ../libgridfields.la 

testoutput_SOURCES = testoutput.cc
testoutput_LDADD = ../libgridfields.la 
endif

if VTK_FOUND
# testvtk_SOURCES = testvtk.cc
# testvtk_LDADD = testvtk.o -L$(VTKLIBPATH) $(VTKLIBS) ../libgridfields.la 

# wedgetest_SOURCES = wedgetest.cc
# wedgetest_LDADD = wedgetest.o -L$(VTKLIBPATH) $(VTKLIBS) ../libgridfields.la 

# testelcirc_SOURCES = testelcirc.o
# testelcirc_LDADD = testelcirc.cc -L$(VTKLIBPATH) $(VTKLIBS) \
# 	-L$(NETCDFLIBDIR) -lnetcdf_c++ -lnetcdf ../libgridfields.la 

# testtag_SOURCES = testtag.o
# testtag_LDADD = testtag.cc -L$(VTKLIBPATH) $(VTKLIBS) ../libgridfields.la 

# testvis_SOURCES = testvis.o
# testvis_LDADD = testvis.cc -L$(VTKLIBPATH) $(VTKLIBS) ../libgridfields.la 
endif

# io unit tests

testarrayreader_SOURCES = testarrayreader.cc
testarrayreader_LDADD = ../libgridfields.la 

# testwrite63_SOURCES = testwrite63.cc
# testwrite63_LDADD = ../libgridfields.la 

# operator unit tests

# testjoin_SOURCES = testjoin.cc
# testjoin_LDADD = ../libgridfields.la 
# LDADD originally included -lefence

testcross_SOURCES = testcross.cc
testcross_LDADD = ../libgridfields.la 

testaggregate_SOURCES = testaggregate.cc
testaggregate_LDADD = ../libgridfields.la 

testrestrict_SOURCES = testrestrict.cc
testrestrict_LDADD = ../libgridfields.la 

testmerge_SOURCES = testmerge.cc
testmerge_LDADD = ../libgridfields.la 

# testscan_SOURCES = testscan.cc
# testscan_LDADD = ../libgridfields.la 

testbind_SOURCES = testbind.cc
testbind_LDADD = ../libgridfields.la 

# gridfield component unit tests

testgridfield_SOURCES = testgridfield.cc
testgridfield_LDADD = ../libgridfields.la 

testsift_SOURCES = testsift.cc
testsift_LDADD = ../libgridfields.la 

testdataset_SOURCES = testdataset.cc
testdataset_LDADD = ../libgridfields.la 

# testgrid_SOURCES = testgrid.cc
# testgrid_LDADD = ../libgridfields.la 

testcellarray_SOURCES = testcellarray.cc
testcellarray_LDADD = ../libgridfields.la 

testtuple_SOURCES = testtuple.cc
testtuple_LDADD = ../libgridfields.la 

testarray_SOURCES = testarray.cc
testarray_LDADD = ../libgridfields.la 

# testexpr : testexpr.cc expr.o tuple.o fparser.o type.o
#	g++ -g testexpr.cc expr.o tuple.o -lwayne fparser.o -o testexpr


