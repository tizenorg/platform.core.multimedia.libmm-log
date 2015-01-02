Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version:    0.1.9
Release:    17
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
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}

%files devel
/usr/share/license/%{name}
%defattr(-,root,root,-)
<<<<<<< HEAD
%{_includedir}/mm_log/mm_log.h
%{_libdir}/pkgconfig/mm-log.pc
=======
/usr/include/mm_log/mm_log.h
/usr/lib/pkgconfig/mm-log.pc
>>>>>>> 396f150dee4bc5a14eaf416a49180d6f8a8cf954

