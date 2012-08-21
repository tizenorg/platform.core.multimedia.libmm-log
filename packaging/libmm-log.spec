Name:       libmm-log
Summary:    Multimedia Framework LOG Lib
Version: 0.1.3
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
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
%configure --disable-static \
    --prefix=%{_prefix} --enable-dlog

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files devel
%defattr(-,root,root,-)
/usr/include/mm_log/mm_log.h
/usr/lib/pkgconfig/mm-log.pc

