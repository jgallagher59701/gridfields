Name:		gridfields
Version:	1.0.3
Release:	1%{?dist}
Summary:	The Gridfield library implements an algreba for irregular meshes.

Group:		Development/Libraries
License:	LGPLv2+
URL:		http://www.opendap.org/
Source0:	http://www.opendap.org/pub/source/gridfields-%{version}.tar.gz

BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Written by Bill Howe. Packaged by James Gallagher.

%package devel
Summary: Development and header files for teh gridfields library
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
This package contains all the files needed to develop applications that
will use libgridfields.

%prep
%setup -q

%build
%configure --disable-static --disable-dependency-tracking
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="%{__install} -p"
rm $RPM_BUILD_ROOT%{_libdir}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libgridfields.so.*
%doc README NEWS COPYING

%files devel
%defattr(-,root,root,-)
%{_libdir}/libgridfields.so
%{_includedir}/gridfields/

%changelog
