# Build libgridfields

AUTOMAKE_OPTIONS = foreign check-news
ACLOCAL_AMFLAGS = -I conf

# aclocaldir=$(datadir)/aclocal
# pkgconfigdir=$(libdir)/pkgconfig

AM_CPPFLAGS = 
AM_CXXFLAGS = 
if COMPILER_IS_GCC
# jhrg 2/13/14
# I got tired of the sign compare warnings, but they should be
# addressed. jhrg 4/3/14
AM_CXXFLAGS += -Wall -Wno-cast-align -Wno-deprecated -Wno-sign-compare
endif

# These are not used by automake but are often useful for certain types of
# debugging. The best way to use these is to run configure as:
#     export CXXFLAGS='...'; ./configure --disable-shared
# the --disable-shared is not required, but it seems to help with debuggers.
CXXFLAGS_DEBUG = -g3 -O0 -fno-defer-pop -Wall -Wcast-align
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

SUBDIRS = . tests 
# vis
# DIST_SUBDIRS = tests
# vis

# noinst_LTLIBRARIES = libparsers.la
lib_LTLIBRARIES = libgridfields.la

# bin_PROGRAMS =

# BUILT_SOURCES = 

libgridfields_la_SOURCES = $(SRCS)
libgridfields_la_LDFLAGS = -version-info $(LIBGF_VERSION)
libgridfields_la_CPPFLAGS = $(AM_CPPFLAGS)
libgridfields_la_LIBADD = 

# really?
pkginclude_HEADERS = $(HDRS)

# noinst_HEADERS = config.h

# EXTRA_DIST = 

# At some point write a m4 macro that can be used to test for the
# gridfields library; same for pkg config

# dist_aclocal_DATA = conf/libgridfields.m4
# pkgconfig_DATA = libgridfields.pc

CLEANFILES = 

# Not nearly as clean as it could be, but this removes .svn directories
# in subdirs.
dist-hook:
	rm -rf `find $(distdir) -name .svn`

# MAINTAINERCLEANFILE

SRCS = CmdLine.cpp src/accumulate.cc src/aggregate.cc			\
src/aggregations.cc src/apply.cc src/array.cc src/arrayreader.cc	\
src/arraywriter.cc src/assignments.cc src/bind.cc src/bindconstant.cc	\
src/cell.cc src/cellarray.cc src/constarray.cc src/cross.cc		\
src/datadump.cc src/dataset.cc src/elcircfile.cc src/elio.c		\
src/expr.cc src/fparser.cc src/grid.cc src/gridfield.cc			\
src/gridfieldoperator.cc src/implicit0cells.cc src/merge.cc		\
src/output.cc src/outputtuples.cc src/project.cc src/rankeddataset.cc	\
src/refrestrict.cc src/restrict.cc src/scaninternal.cc src/sift.cc	\
src/timing.cc src/tuple.cc src/type.cc src/unarynodemap.cc src/util.cc

if NETCDF_FOUND
SRCS += src/tonetcdf.cc src/netcdfadaptor.cc 
endif

HDRS = config_gridfields.h src/CmdLine.h src/RTree.h			\
src/abstractcellarray.h src/access.h src/accumulate.h src/aggregate.h	\
src/aggregations.h src/apply.h src/array.h src/arrayreader.h		\
src/arraywriter.h src/assignments.h src/binarynodemap.h src/bind.h	\
src/bindconstant.h src/cell.h src/cellarray.h src/constarray.h		\
src/counted_ptr.h src/cross.h src/crossnodemap.h src/crossordmap.h	\
src/datadump.h src/dataset.h src/elcircfile.h src/elio.h src/expr.h	\
src/fparser.hh src/fptypes.hh src/fpconfig.hh src/grid.h		\
src/gridfield.h src/gridfieldoperator.h src/idnodemap.h src/idordmap.h	\
src/implicit0cells.h src/implicitcrossnodemap.h src/leftnodemap.h	\
src/merge.h src/nodemap.h src/normnodemap.h src/object.h src/onegrid.h	\
src/ordmap.h src/output.h src/outputtuples.h src/partition.h		\
src/project.h src/rankeddataset.h src/refrestrict.h src/restrict.h	\
src/scan.h src/scaninternal.h src/sift.h src/subgridordmap.h		\
src/timing.h src/tuple.h src/type.h src/unarynodemap.h src/util.h

if NETCDF_FOUND
HDRS += src/tonetcdf.h src/netcdfadaptor.h
endif

OLD_REMOVED_SRCS = stuebe.cc
OLD_REMOVED_HDRS = stuebe.h

###########################################################################
#
# targets that are often useful but which also require some
# non-trivial configuration work.

.PHONY: docs
docs:
	doxygen $(srcdir)/doxy.conf
	(cd docs && tar -czf html.tar.gz html)

# cccc computes metrics like Lines of code and McCabe. It'a available
# on the web...
.PHONY: cccc
cccc:
	-mkdir cccc
	cccc --outdir=cccc $(SRCS) $(HDRS)

# Build linux RPMs. Use the environment variable 'RPM_OPTIONS' to pass in
# extra options like --nodeps and --macros. This requires .spec files

srpm: dist
	rpmbuild -ts --clean $(RPM_OPTIONS) @PACKAGE@-@PACKAGE_VERSION@.tar.gz

rpm: dist
	rpmbuild -tb --clean $(RPM_OPTIONS) @PACKAGE@-@PACKAGE_VERSION@.tar.gz

# Build OS/X Packages. The strange operations with configure and make
# are there so that the values built into dap-config will match the mac
# osx install dirs and not the temp directory used to build the packages

PACKAGEMAKER=/Developer/usr/bin/packagemaker
PKG_CONF_FLAGS=

clean-pkg:
	-rm -rf mac_osx @PACKAGE@-@PACKAGE_VERSION@.pkg

pkg-build: clean-pkg
	./configure --prefix=$(prefix) --disable-dependency-tracking $(PKG_CONF_FLAGS)
	make clean all
	DESTDIR=`pwd`/mac_osx make install

pkg-main: pkg-build
	./OSX_Resources/update_mac_package_contents.pl README
	cat ./OSX_Resources/Info.plist.proto | \
	   sed -e "s^_PREFIX_^$(prefix)^g" \
	       -e "s^_FULL_VERSION_^@PACKAGE_VERSION@^g" \
	       -e "s^_MAJOR_VERSION_^@PACKAGE_MAJOR_VERSION@^g" \
	       -e "s^_MINOR_VERSION_^@PACKAGE_MINOR_VERSION@^g" \
	       > foo
	mv foo ./OSX_Resources/Info.plist
	${PACKAGEMAKER} --root mac_osx --id org.opendap.@PACKAGE@ \
	    --title "@PACKAGE@ @PACKAGE_VERSION@" --version @PACKAGE_VERSION@ \
	    --out @PACKAGE@-@PACKAGE_VERSION@.pkg --resources OSX_Resources

pkg-dmg: pkg-main
	-rm -rf @PACKAGE@-@PACKAGE_VERSION@
	-rm -rf @PACKAGE@-@PACKAGE_VERSION@.dmg
	mkdir @PACKAGE@-@PACKAGE_VERSION@
	cp -r @PACKAGE@-@PACKAGE_VERSION@.pkg @PACKAGE@-@PACKAGE_VERSION@
	cp README README.* NEWS @PACKAGE@-@PACKAGE_VERSION@
	dropdmg -i --sanitize-for-servers --format zlib @PACKAGE@-@PACKAGE_VERSION@
	-rm -rf @PACKAGE@-@PACKAGE_VERSION@

pkg: pkg-main 

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

