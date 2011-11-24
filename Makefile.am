# Build libgridfields

AUTOMAKE_OPTIONS = foreign check-news
ACLOCAL_AMFLAGS = -I conf

# aclocaldir=$(datadir)/aclocal
# pkgconfigdir=$(libdir)/pkgconfig

AM_CPPFLAGS = 
AM_CXXFLAGS = 
if COMPILER_IS_GCC
AM_CXXFLAGS += -Wall -W -Wcast-align
endif

# These are not used by automake but are often useful for certain types of
# debugging. The best way to use these is to run configure as:
#     export CXXFLAGS='...'; ./configure --disable-shared
# the --disable-shared is not required, but it seems to help with debuggers.
CXXFLAGS_DEBUG = -g3 -O0 -fno-defer-pop -Wall -W -Wcast-align -Werror
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

SUBDIRS = tests 
# vis
DIST_SUBDIRS = tests
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

noinst_HEADERS = config.h

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

SRCS = CmdLine.cpp accumulate.cc aggregate.cc aggregations.cc apply.cc	\
array.cc arrayreader.cc arraywriter.cc assignments.cc bind.cc		\
bindconstant.cc cell.cc cellarray.cc constarray.cc cross.cc		\
datadump.cc dataset.cc elcircfile.cc elio.c expr.cc fparser.cc grid.cc	\
gridfield.cc gridfieldoperator.cc implicit0cells.cc 		\
merge.cc output.cc outputtuples.cc project.cc		\
rankeddataset.cc refrestrict.cc restrict.cc scaninternal.cc sift.cc	\
timing.cc tuple.cc type.cc unarynodemap.cc	\
util.cc 

if NETCDF_FOUND
SRCS += tonetcdf.cc netcdfadaptor.cc 
endif

HDRS = CmdLine.h RTree.h abstractcellarray.h access.h accumulate.h	\
aggregate.h aggregations.h apply.h array.h arrayreader.h arraywriter.h	\
assignments.h binarynodemap.h bind.h bindconstant.h cell.h cellarray.h	\
config.h constarray.h counted_ptr.h cross.h crossnodemap.h		\
crossordmap.h datadump.h dataset.h elcircfile.h elio.h expr.h grid.h	\
gridfield.h gridfieldoperator.h idnodemap.h idordmap.h			\
implicit0cells.h implicitcrossnodemap.h leftnodemap.h merge.h		\
nodemap.h normnodemap.h object.h onegrid.h ordmap.h	\
output.h outputtuples.h partition.h project.h rankeddataset.h		\
refrestrict.h restrict.h scan.h scaninternal.h sift.h		\
subgridordmap.h timing.h tuple.h type.h unarynodemap.h	\
util.h

if NETCDF_FOUND
HDRS += tonetcdf.h netcdfadaptor.h
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

