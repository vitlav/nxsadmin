Name: nxsadmin
Version: 0.2.1
Release: alt2

Summary: Administering graphic tool for FreeNX server

License: GPL
Url: http://nxsadmin.berlios.de/
Group: System/Configuration/Other

Requires: freenx

Source: http://download.berlios.de/nxsadmin/%name-%version.tar.bz2
Patch0: nxsadmin-desktop-path.patch
BuildPreReq: menu-devel

# Automatically added by buildreq on Fri Apr 11 2008
BuildRequires: gcc-c++ gcc-fortran glibc-devel libgtkmm2-devel perl-XML-Parser intltool

%description
FreeNX Sessions Administrator provides a graphical tool for managment of active NX sessions on FreeNX server

%prep
%setup -q
%patch0 -p0

%build
%configure
%make_build

%install
%makeinstall_std

mkdir -p %buildroot%_desktopdir %buildroot%_pixmapsdir/%name
install -m 644 %name.desktop %buildroot%_desktopdir
install -m 644 %name-icon.png %buildroot%_pixmapsdir/%name
%find_lang %name

%post
%update_menus

%postun
%clean_menus

%files -f %name.lang
%doc AUTHORS COPYING ChangeLog README TODO
%_sbindir/nxsadmin
%_desktopdir/%name.desktop

%changelog
* Mon Jun 16 2008 Boris Savelev <boris@altlinux.org> 0.2.1-alt2
- fix path in desktop file

* Fri Apr 11 2008 Boris Savelev <boris@altlinux.org> 0.2.1-alt1
- new version (0.2.1)

* Fri Mar 07 2008 Boris Savelev <boris@altlinux.org> 0.2-alt1
- initial build
