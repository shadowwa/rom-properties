Name:          rom-properties
Version:       1.0
Release:       1
Summary:       ROM Properties Page shell extension
Group:         games
License:       GPL
URL:           https://github.com/GerbilSoft/rom-properties/
%bcond_without test

Source:        %{name}-%{version}.tar.gz
BuildRoot:     %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: cmake
BuildRequires: extra-cmake-modules
BuildRequires: nettle-devel
BuildRequires: zlib-devel
BuildRequires: curl-devel
BuildRequires: png-devel
BuildRequires: jpeg-devel
BuildRequires: pkgconfig(tinyxml2)
#Requires:      

%description
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.

%package -n rom-properties-kde5
Summary:       ROM Properties Page shell extension KDE5
BuildRequires: kio-devel
BuildRequires: qtbase5-common-devel

%description -n rom-properties-kde5
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
This package contains the KDE 5 version.

%package -n rom-properties-cli
Summary:       ROM Properties Page shell extension CLI

%description -n rom-properties-cli
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
This package contains the command line version.

%package -n rom-properties-xfce
Summary:       ROM Properties Page shell extension XFCE
BuildRequires: libthunarx-devel

%description -n rom-properties-xfce
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
This package contains the XFCE version.

%package -n rom-properties-gnome
Summary:       ROM Properties Page shell extension XFCE
BuildRequires: gtk2-devel
BuildRequires: gtk+3.0-devel
BuildRequires: nautilus-devel

%description -n rom-properties-gnome
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
This package contains the Nautilus (GNOME 3, Unity) version.

%prep
%setup -q -n %{name}-%{version}

%build
%cmake \
%if %{with test}
    -DBUILD_TESTING=ON \
%endif
	-DCMAKE_BUILD_TYPE=Release \
    -DENABLE_JPEG=ON \
	-DSPLIT_DEBUG=OFF \
	-DINSTALL_DEBUG=OFF \
	-DBUILD_KDE4=ON \
	-DBUILD_KDE5=ON \
	-DBUILD_XFCE=ON \
	-DBUILD_CLI=ON
%make_build

%install
%{makeinstall_std} -C build

%if %{with test}
%check
LC_NUMERIC=en_US CTEST_OUTPUT_ON_FAILURE=1 %make_build -C build test
%endif

%clean
%{__rm} -rf %{buildroot}

%files
%doc README.md
%doc LICENSE
%doc doc/COMPILING.md
%doc doc/keys.conf.example
%doc doc/rom-properties.conf.example
%defattr(-,root,root,0755)

%files -n rom-properties-kde5
%{_datadir}/kservices5/rom-properties-kde5.desktop
%{_libdir}/qt5/plugins/rom-properties-kde5.so

%files -n rom-properties-cli
%{_bindir}/rpcli

%files -n rom-properties-xfce
%{_libdir}/thunarx-2/rom-properties-xfce.so

%files -n rom-properties-gnome
%{_libdir}/nautilus/extensions-3.0/rom-properties-gnome.so
%{_libexecdir}/rp-thumbnail
%{_datadir}/thumbnailers/rom-properties.thumbnailer


%changelog

* Mon May 22 2017 shad <shad> 1.0-1
- add rom-properties.conf.example file
- Build test (can be disable passing --without test to rpmbuild)
- cleanup useless placeholder

* Thu May 4 2017 shad <shad> 1.0-0
- test git packages

* Wed May 3 2017 shad <shad> 0.9-beta2
- first package
