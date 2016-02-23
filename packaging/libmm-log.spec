Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version:    0.1.10
Release:    0
VCS:        framework/multimedia/libmm-log#libmm-log_0.1.5-6-18-g93d660f3979dfbde901e8d2c6cdc6d9d78ce3442
Group:      Multimedia/LOG
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
./autogen.sh


%build
export CFLAGS+=" -DUSE_DLOG"
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS+=" -DTIZEN_ENGINEER_MODE"
%endif


%configure --disable-static \
%if 0%{?tizen_build_binary_release_type_eng}
	--enable-engineer_mode \
%endif
    --prefix=%{_prefix} --enable-dlog

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}%{_datadir}/license
cp LICENSE.APLv2 %{buildroot}%{_datadir}/license/%{name}

%files devel
%defattr(-,root,root,-)
%{_datadir}/license/%{name}
%{_includedir}/mm_log/mm_log.h
%{_libdir}/pkgconfig/mm-log.pc

