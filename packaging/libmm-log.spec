
Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version:    0.1.2
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    libmm-log-%{version}.tar.gz
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
#### variables for logmessage.conf :+:091130 ####
# reference : logmanager/logmessage.conf.in
# [setting]
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
##################################################

%configure --disable-static \
    --prefix=%{_prefix} --enable-dlog

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install




%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig





%files
%defattr(-,root,root,-)
/usr/bin/mmlogviewer
/opt/etc/logmessage.conf
/usr/lib/libmm_log.so.*


%files devel
%defattr(-,root,root,-)
/usr/include/mm_log/mm_log.h
/usr/lib/libmm_log.so
/usr/lib/pkgconfig/mm-log.pc

