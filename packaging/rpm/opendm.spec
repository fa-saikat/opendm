# RPM spec for OpenDM (Fedora, RHEL/Alma/Rocky with EPEL, openSUSE).
#
# OpenDM was created by Nader Mahbub Khan (https://github.com/nadermkhan/opendm).
# This package builds the modified GTK3 fork; see NOTICE and AUTHORS.
#
# Build with scripts/package_rpm.sh, which creates the source tarball and
# fills in the version from CMakeLists.txt.

%{!?opendm_version: %global opendm_version 0.0.0}

Name:           opendm
Version:        %{opendm_version}
Release:        1%{?dist}
Summary:        High-speed segmented download manager (GTK3)
License:        GPL-3.0-or-later
URL:            https://github.com/fa-saikat/opendm
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  pkgconfig(gtk+-3.0) >= 3.22
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  cmake(nlohmann_json)
BuildRequires:  desktop-file-utils

Recommends:     ffmpeg

%description
OpenDM is a native Linux download accelerator with dynamic multipart
segmentation, browser integration via native messaging, remote ZIP
inspection, and YouTube stream extraction with automatic media muxing.

This is a modified fork of OpenDM, created by Nader Mahbub Khan
(https://github.com/nadermkhan/opendm), ported to GTK3.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/opendm.desktop
# YouTubeEngineTest needs live access to youtube.com, which build systems don't have.
%ctest --exclude-regex YouTubeEngineTest

%files
%license LICENSE NOTICE
%doc README.md AUTHORS
%{_bindir}/opendm
%{_bindir}/opendm-native-host
%{_datadir}/applications/opendm.desktop
%{_datadir}/icons/hicolor/*/apps/opendm.*
%{_datadir}/metainfo/io.github.opendm.OpenDM.metainfo.xml
%{_datadir}/opendm/
%dir %{_prefix}/lib/mozilla
%dir %{_prefix}/lib/mozilla/native-messaging-hosts
%{_prefix}/lib/mozilla/native-messaging-hosts/io.github.opendm.json
%dir %{_sysconfdir}/opt/chrome
%dir %{_sysconfdir}/opt/chrome/native-messaging-hosts
%{_sysconfdir}/opt/chrome/native-messaging-hosts/io.github.opendm.json
%dir %{_sysconfdir}/chromium
%dir %{_sysconfdir}/chromium/native-messaging-hosts
%{_sysconfdir}/chromium/native-messaging-hosts/io.github.opendm.json

%changelog
* Tue Nov 03 2026 FA Saikat <fahimabrar.saikat@gmail.com> - 1.0.0-1
- GTK3 port and RPM packaging of the OpenDM fork
