Name:           hackforge-paint
Version:        1.0.0
Release:        1%{?dist}
Summary:        A lightweight paint application built with SDL3

License:        MIT
URL:            https://github.com/wccug/hackforge-paint
Source0:        %{url}/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  SDL3-devel

%description
Hackforge Paint is a collaborative, lightweight paint tool utilizing SDL3, designed for community coding sessions and multi-platform graphics work.

%prep
%autosetup

%build
%cmake
%cmake_build

%install
%cmake_install

%files
%{_bindir}/hackforge-paint

%changelog
* Thu Aug 20 2026 Hackforge <info@hackforge.ca> - 0.1.0-1
- Initial RPM release with SDL3 support for Fedora
