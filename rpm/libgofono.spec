Name: libgofono
Version: 2.0.12
Release: 0
Summary: Ofono client library
License: BSD
URL: https://git.sailfishos.org/mer-core/libgofono
Source: %{name}-%{version}.tar.bz2

%define libglibutil_version 1.0.28

BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libglibutil) >= %{libglibutil_version}
Requires:   libglibutil >= %{libglibutil_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Provides glib-based ofono client API

%package devel
Summary: Development library for %{name}
Requires: %{name} = %{version}
Requires: pkgconfig

%description devel
This package contains the development library for %{name}.

%prep
%setup -q

%build
make %{_smp_mflags} LIBDIR=%{_libdir} KEEP_SYMBOLS=1 release pkgconfig

%install
rm -rf %{buildroot}
make LIBDIR=%{_libdir} DESTDIR=%{buildroot} install-dev

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/%{name}.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/*.pc
%{_libdir}/%{name}.so
%{_includedir}/gofono/*.h
