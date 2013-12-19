Name: hscp
Source: %name-%version.tar.bz2
Version: 0.9.21
Url: http://hscp.sf.net/
License: BSD like
Release: 1
Summary: Hybrid scp
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
 HSCP (Hybrid scp) is developing to transmit the large size file at 
 high speed on the long distance and wideband infrastructure. It has 
 achieved the fast transfer by changing the file transfer part of scp 
 into the UDP(using UDT).

%prep
%setup

%build
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%files
/etc/%name.conf
%_bindir/%name
