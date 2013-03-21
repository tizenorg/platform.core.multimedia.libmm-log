Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version:    0.1.5
Release:    7
Group:      Multimedia/Multimedia Framework
License:    Apache-2.0
Source0:    libmm-log-%{version}.tar.gz
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


%build
export CFLAGS+=" -DUSE_DLOG"
./autogen.sh
%configure --disable-static --enable-dlog
make %{?jobs:-j%jobs}

%install
%make_install

%files devel
%defattr(-,root,root,-)
%{_includedir}/mm_log/mm_log.h
%{_libdir}/pkgconfig/mm-log.pc

