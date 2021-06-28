Name:          rom-properties
Version:       1.7.3
Release:       0
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
BuildRequires: lz4-devel
BuildRequires: lzo-devel
BuildRequires: zstd-devel
BuildRequires: curl-devel
BuildRequires: png-devel
BuildRequires: jpeg-devel
BuildRequires: pkgconfig(tinyxml2)
BuildRequires: seccomp-devel
BuildRequires: nettle

%description
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.

##%package -n rom-properties-kde4
##Summary:       ROM Properties Page shell extension kde4
##Requires:      rom-properties-utils
##Requires:      rom-properties-xdg
##Requires:      rom-properties-data
##
##%description -n rom-properties-kde4
##This shell extension provides thumbnailing and property page functionality
##for ROM images, disc images, and save files for various game consoles,
##including Nintendo GameCube and Wii.
##.
##This package contains the KDE 4 version.

%package -n rom-properties-kf5
Summary:       ROM Properties Page shell extension KF5
Requires:      rom-properties-utils
Requires:      rom-properties-xdg
Requires:      rom-properties-data
Conflicts:     rom-properties-kde5
BuildRequires: kio-devel
BuildRequires: qtbase5-common-devel
BuildRequires: kfilemetadata-devel

%description -n rom-properties-kf5
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
.
This package contains the KDE Frameworks 5 version.

##%package -n rom-properties-xfce
##Summary:       ROM Properties Page shell extension XFCE
##Requires:      rom-properties-thumbnailer-dbus
##Requires:      rom-properties-utils
##Requires:      rom-properties-xdg
##Requires:      rom-properties-data
##BuildRequires: libthunarx-devel
##BuildRequires: gtk2-devel
##
##%description -n rom-properties-xfce
##This shell extension provides thumbnailing and property page functionality
##for ROM images, disc images, and save files for various game consoles,
##including Nintendo GameCube and Wii.
##.
##This package contains the XFCE (GTK+ 2.x) version.
##.
##The thumbnailer-dbus package is required for thumbnailing on
##some XFCE versions.

%package -n rom-properties-gtk3
Summary:       ROM Properties Page shell extension GTK3
Requires:      rom-properties-utils
Requires:      rom-properties-thumbnailer-dbus
Requires:      rom-properties-xdg
Requires:      rom-properties-data
Conflicts:     rom-properties-gnome, rom-properties-mate, rom-properties-cinnamon, rom-proprties-gtk3-common
BuildRequires: gtk+3.0-devel
BuildRequires: nautilus-devel
BuildRequires: nemo-extension-devel
BuildRequires: caja-devel
BuildRequires: thunarx-devel
BuildRequires: libcanberra-devel
BuildRequires: libcanberra-gtk3-devel

%description -n rom-properties-gtk3
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
.
This package contains the GTK+ 3.x version, which supports Nautilus
(GNOME), Caja (MATE), Nemo (Cinnamon), and Thunar (XFCE).
.
The utils package is required for thumbnailing.
.
The thumbnailer-dbus package is required for thumbnailing on
some XFCE versions.

%package -n rom-properties-cli
Summary:       ROM Properties Page shell extension CLI
Requires:      rom-properties-data

%description -n rom-properties-cli
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
This package contains the command line version.

%package -n rom-properties-utils
Summary:       ROM Properties Page shell extension
Conflicts:     rom-properties-stub

%description -n rom-properties-utils
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
.
This package contains helper executables for invoking the configuration
UI, thumbnailing on some desktop environments, and downloading images
from online databases.

%package -n rom-properties-thumbnailer-dbus
Summary:       ROM Properties Page shell extension

%description -n rom-properties-thumbnailer-dbus
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
.
This package contains the D-Bus Thumbnailer service, used by XFCE's
tumblerd thumbnailing subsystem.

%package -n rom-properties-xdg
Summary:       ROM Properties Page shell extension

%description -n rom-properties-xdg
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
.
This package contains the MIME package for files supported by rom-properties
that aren't currently listed in FreeDesktop.org's shared-mime-info database.

%package -n rom-properties-data
Summary:       ROM Properties Page shell extension

%description -n rom-properties-data
This shell extension provides thumbnailing and property page functionality
for ROM images, disc images, and save files for various game consoles,
including Nintendo GameCube and Wii.
.
This package contains data files used for various parsers, e.g. the
Nintendo amiibo parser.

%prep
%setup -q -n %{name}-%{version}

%build
mkdir build
cd build
cmake \
    -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=None -DCMAKE_INSTALL_SYSCONFDIR=/etc -DCMAKE_INSTALL_LOCALSTATEDIR=/var -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON "-GUnix Makefiles" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_INSTALL_LIBDIR=lib/x86_64-linux-gnu \
%if %{with test}
    -DBUILD_TESTING=ON \
%endif
	-DCMAKE_BUILD_TYPE=Release \
	-DENABLE_JPEG=ON \
	-DSPLIT_DEBUG=OFF \
	-DINSTALL_DEBUG=OFF \
	-DINSTALL_APPARMOR=ON \
	-DBUILD_KDE4=OFF \
	-DBUILD_KF5=ON \
	-DBUILD_XFCE=OFF \
	-DBUILD_GTK3=ON \
	-DBUILD_CLI=ON \
	-DENABLE_PVRTC=ON \
	-DENABLE_LTO=OFF \
	-DENABLE_PCH=ON \
	-DUSE_SECCOMP=ON \
	-DENABLE_SECCOMP_DEBUG=OFF\
    ..
%make_build
##make

%install
%{make_install} -C build
mkdir %{buildroot}/usr/libexec
#ln -vfs %{_bindir}/rp-stub %{buildroot}/%{_libexecdir}/rp-thumbnail
ln -vfs %{_bindir}/rp-stub %{buildroot}%{_bindir}/rp-config
mkdir -p %{buildroot}/usr/plugins/
ln -vfs %{_libdir}/qt5/plugins/rom-properties-kf5.so %{buildroot}/usr/plugins/rom-properties-kf5.so

%find_lang rom-properties

%if %{with test}
%check
LC_NUMERIC=en_US CTEST_OUTPUT_ON_FAILURE=1 %make_build -C build test
%endif
rm -fr %{buildroot}/usr/src/debug/

%clean
%{__rm} -rf %{buildroot}

%files
%doc README.md
%doc NEWS.md
%doc LICENSE
%doc doc/COMPILING.md
%doc doc/keys.conf.example
%doc doc/rom-properties.conf.example
%defattr(-,root,root,0755)
%{_datadir}/locale/*

##%files -n rom-properties-kde4
##%{_libdir}/kde4/rom-properties-kde4.so
##%{_datadir}/kde4/services/rom-properties-kde4.KPropertiesDialog.desktop
##%{_datadir}/kde4/services/rom-properties-kde4.ThumbCreator.desktop


%files -n rom-properties-kf5
%{_datadir}/kservices5/rom-properties-kf5.KPropertiesDialog.desktop
%{_datadir}/kservices5/rom-properties-kf5.ThumbCreator.desktop
%{_libdir}/qt5/plugins/rom-properties-kf5.so
%{_libdir}/qt5/plugins/kf5/kfilemetadata/kfilemetadata_rom-properties-kf5.so
%{_libdir}/qt5/plugins/kf5/overlayicon/overlayiconplugin_rom-properties-kf5.so
/usr/plugins/rom-properties-kf5.so

%files -n rom-properties-cli
%{_bindir}/rpcli
/etc/apparmor.d/usr.bin.rpcli


%files -n rom-properties-utils
%{_bindir}/rp-stub
%{_bindir}/rp-config
/etc/apparmor.d/usr.lib.x86_64-linux-gnu.libexec.rp-download
/usr/lib/x86_64-linux-gnu/libexec/rp-download
/usr/lib/x86_64-linux-gnu/libexec/rp-thumbnail

##%files -n rom-properties-xfce
##%{_libdir}/thunarx-2/rom-properties-xfce.so

%files -n rom-properties-gtk3
%{_datadir}/thumbnailers/rom-properties.thumbnailer
%{_libdir}/caja/extensions-2.0/rom-properties-gtk3.so
%{_libdir}/nautilus/extensions-3.0/rom-properties-gtk3.so
%{_libdir}/nemo/extensions-3.0/rom-properties-gtk3.so
%{_libdir}/thunarx-3/rom-properties-gtk3.so

%files -n rom-properties-thumbnailer-dbus
%{_bindir}/rp-thumbnailer-dbus
%{_datadir}/dbus-1/services/com.gerbilsoft.rom-properties.SpecializedThumbnailer1.service
%{_datadir}/thumbnailers/com.gerbilsoft.rom-properties.SpecializedThumbnailer1.service

%files -n rom-properties-xdg
/usr/share/mime/packages/rom-properties.xml

%files -n rom-properties-data
/usr/share/rom-properties/amiibo-data.bin

%changelog

* Sun Sep 27 2020 shad <shad> 1.7.3-0
- update to 1.7.3

* Fri Sep 25 2020 shad <shad> 1.7.2-0
- update to 1.7.2

* Tue Sep 22 2020 shad <shad> 1.7.1-0
- update to 1.7.1

* Tue Aug 04 2020 shad <shad> 1.6.1-0
- update to 1.6.1+

* Mon Mar 16 2020 shad <shad> 1.5-0
- update to 1.5

* Sat Dec 22 2018 shad <shad> 1.4-0
- update to 1.4

* Tue Jan 09 2018 shad <shad> 1.3-0
- update to 1.3

* Sun Nov 12 2017 shad <shad> 1.2-1
- update to 1.2

* Mon May 22 2017 shad <shad> 1.0-1
- add rom-properties.conf.example file
- Build test (can be disable passing --without test to rpmbuild)
- cleanup useless placeholder

* Thu May 4 2017 shad <shad> 1.0-0
- test git packages

* Wed May 3 2017 shad <shad> 0.9-beta2
- first package
