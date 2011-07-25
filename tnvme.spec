%define _distro %(uname -r)

Name:           tnvme
Version:        %{_major}.%{_minor}
Release:        1%{?dist}
Summary:        NVM Express hardware compliance test suite application
Group:          Applications/Engineering
License:        Commercial
URL:            http://www.intel.com
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
NVM Express hardware compliance test suite application.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(755,root,root,755)
%{_bindir}/%{name}

%post

%preun

%postun

%changelog
