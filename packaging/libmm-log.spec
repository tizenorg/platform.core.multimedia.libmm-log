Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version:    0.1.3
Release:    1
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    libmm-log-%{version}.tar.gz
Source1001: packaging/libmm-log.manifest 
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(iniparser)


%description
Multimedia Framework LOG Library



%package devel
Summary:    Multimedia Framework LOG Lib (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Multimedia Framework LOG Library (devel)


%prep
%setup -q -n %{name}-%{version}


%build
cp %{SOURCE1001} .
export direction=3
export ownermask=0xFFFFFF
export classmask=0x0C
export filename=/var/log/logmessage.log
#[color]
export classinfomation=MAGENTA
export classwarning=BLUE
export classerror=RED
export clsascritical=RED
export classassert=RED

%configure --disable-static \
    --prefix=%{_prefix} --enable-dlog

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install




%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig





%files
%manifest libmm-log.manifest
/usr/bin/mmlogviewer
%attr(644,root,root) /opt/etc/logmessage.conf
/usr/lib/libmm_log.so.*


%files devel
%manifest libmm-log.manifest
/usr/include/mm_log/mm_log.h
/usr/lib/libmm_log.so
/usr/lib/pkgconfig/mm-log.pc

