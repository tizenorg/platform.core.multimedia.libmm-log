Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version:    0.1.5
Release:    0
Group:      Multimedia/Multimedia Framework
License:    Apache-2.0
Source0:    libmm-log-%{version}.tar.gz
Source1001: libmm-log.manifest
BuildRequires:  pkgconfig(dlog)

%description
Multimedia Framework LOG Library

%package devel
Summary:    Multimedia Framework LOG Lib (devel)
Group:      Development/Libraries

%description devel
Multimedia Framework LOG Library (devel)

%prep
%setup -q -n %{name}-%{version}
cp %{SOURCE1001} .

%build
CFLAGS="$CFLAGS -DUSE_DLOG"; export CFLAGS
./autogen.sh
%configure --disable-static --enable-dlog
%__make %{?_smp_mflags}

%install
%make_install

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/mm_log/mm_log.h
%{_libdir}/pkgconfig/mm-log.pc
