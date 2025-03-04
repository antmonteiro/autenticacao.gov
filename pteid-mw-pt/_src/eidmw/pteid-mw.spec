#
# spec file for package pteid-mw
#
# Copyright (c) 2011-2021 Caixa Magica Software
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.


#Disable suse-specific checks: there is no way to disable just the lib64 check
%if 0%{?suse_version}
%ifarch x86_64
%define __arch_install_post %{nil}
%endif
%endif

%define git_revision git20220601
%define app_version 3.8.0

Name:           pteid-mw
BuildRequires:  pcsc-lite-devel make
BuildRequires:  swig >= 4.0.0
BuildRequires:  libzip-devel
BuildRequires:  openjpeg2-devel
Requires:       pcsc-lite curl lato-fonts polkit


%if 0%{?suse_version}
BuildRequires:  libcurl-devel libxerces-c-devel libopenssl-1_1-devel

Requires: pcsc-ccid xerces-c libqt5-qtquickcontrols libqt5-qtgraphicaleffects
%endif

%if 0%{?suse_version}
BuildRequires:  java-11-openjdk-devel
BuildRequires:  libpoppler-qt5-devel
BuildRequires:  libqt5-qtbase-devel
BuildRequires:  libqt5-qttools-devel
BuildRequires:  libqt5-qtdeclarative-devel
BuildRequires:  libqt5-qtquickcontrols2
BuildRequires:  libQt5QuickControls2-devel
BuildRequires:  libQt5Gui-private-headers-devel
# Make sure that we don't run the OpenSUSE brp scripts - we don't comply with a lot of the checks...
BuildRequires:	-brp-check-suse
BuildRequires:	-post-build-checks
BuildRequires:	-rpmlint

BuildRequires:  libxml-security-c-devel
%endif

%if 0%{?fedora} || 0%{?centos_ver}
BuildRequires:  java-11-openjdk-devel
Requires:       poppler-qt5
Requires:       pcsc-lite-ccid
Requires:       qt5-qtquickcontrols
Requires:       qt5-qtquickcontrols2

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtbase-private-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtquickcontrols2-devel
BuildRequires:  qt5-qttools-devel
BuildRequires:  qt5-qtquickcontrols
BuildRequires:  qt5-qtquickcontrols2

BuildRequires:  libpng-devel

BuildRequires:  xml-security-c-devel
BuildRequires:  poppler-qt5-devel
BuildRequires:  gcc gcc-c++ xerces-c-devel
BuildRequires:  qt-devel curl-devel

BuildRequires:  openssl-devel

%endif

Conflicts:  cartao_de_cidadao

License:        GPLv2+
Group:          System/Libraries
Version:        %{app_version}.%{git_revision}
Release:        1
Summary:        Portuguese eID middleware
Url:            https://github.com/amagovpt/autenticacao.gov
Vendor:         Portuguese Government
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source0:        pteid-mw_%{app_version}+%{git_revision}.tar.xz
Source1:        pteid-mw-gui.desktop
Source2:        pteid-scalable.svg
Source3:        pteid-signature.png

%if 0%{?suse_version}
BuildRequires:  update-desktop-files
%endif

Requires(post): /usr/bin/gtk-update-icon-cache
Requires(postun): /usr/bin/gtk-update-icon-cache

%description
 The Autenticação.Gov package provides a utility application (eidguiV2), a set of
 libraries and a PKCS#11 module to use the Portuguese Identity Card
 (Cartão de Cidadão) and Chave Móvel Digital in order to authenticate securely
 in certain websites and sign documents.
%prep
%setup -q -n pteid-mw_%{app_version}-%{git_revision}


%build
%if 0%{?suse_version}
%ifarch x86_64
#./configure --lib+=-L/usr/lib64
qmake-qt5 "PREFIX_DIR += /usr/local" "INCLUDEPATH += /usr/lib64/jvm/java-11-openjdk-11/include/ /usr/lib64/jvm/java-11-openjdk-11/include/linux/" pteid-mw.pro
%else
qmake-qt5 "PREFIX_DIR += /usr/local" "INCLUDEPATH += /usr/lib/jvm/java-11-openjdk-11/include/ /usr/lib/jvm/java-11-openjdk-11/include/linux/" pteid-mw.pro
%endif
%endif

%if 0%{?fedora} || 0%{?centos_ver}
qmake-qt5 "PREFIX_DIR += /usr/local" "INCLUDEPATH += /usr/lib/jvm/java-11-openjdk/include/ /usr/lib/jvm/java-11-openjdk/include/linux/" pteid-mw.pro
%endif

make %{?jobs:-j%jobs}

%install

#install libs
mkdir -p $RPM_BUILD_ROOT/usr/local/lib/
install -m 755 -p lib/libpteidcommon.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteidcommon.so.2.0.0
install -m 755 -p lib/libpteiddialogsQT.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteiddialogsQT.so.2.0.0
install -m 755 -p lib/libpteidcardlayer.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteidcardlayer.so.2.0.0
install -m 755 -p lib/libpteidpkcs11.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteidpkcs11.so.2.0.0
install -m 755 -p lib/libpteidapplayer.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteidapplayer.so.2.0.0
install -m 755 -p lib/libpteidlib.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteidlib.so.2.0.0
install -m 755 -p lib/libpteidlibj.so.2.0.0 $RPM_BUILD_ROOT/usr/local/lib/libpteidlibj.so.2.0.0
install -m 755 -p lib/libCMDServices.so.1.0.0 $RPM_BUILD_ROOT/usr/local/lib/libCMDServices.so.1.0.0

#install header files
mkdir -p $RPM_BUILD_ROOT/usr/local/include
install -m 644 eidlib/eidlib.h $RPM_BUILD_ROOT/usr/local/include/
install -m 644 eidlib/eidlibcompat.h $RPM_BUILD_ROOT/usr/local/include/
install -m 644 eidlib/eidlibdefines.h $RPM_BUILD_ROOT/usr/local/include/
install -m 644 eidlib/eidlibException.h $RPM_BUILD_ROOT/usr/local/include/
install -m 644 common/eidErrors.h $RPM_BUILD_ROOT/usr/local/include/
mkdir -p $RPM_BUILD_ROOT/usr/local/share/certs/
install -m 755 -p misc/certs/*.der $RPM_BUILD_ROOT/usr/local/share/certs/
install -m 755 -p misc/certs/*.pem $RPM_BUILD_ROOT/usr/local/share/certs/

mkdir -p $RPM_BUILD_ROOT/usr/local/share/pteid-mw/www/
install -m 755 -p misc/web/*.html $RPM_BUILD_ROOT/usr/local/share/pteid-mw/www/

mkdir -p $RPM_BUILD_ROOT/usr/local/lib/pteid_jni/
install -m 755 -p jar/pteidlibj.jar $RPM_BUILD_ROOT/usr/local/lib/pteid_jni/

mkdir -p $RPM_BUILD_ROOT/usr/local/bin/
install -m 755 eidguiV2/eidguiV2 $RPM_BUILD_ROOT/usr/local/bin/eidguiV2

install -m 755 -p bin/pteiddialogsQTsrv $RPM_BUILD_ROOT/usr/local/bin/pteiddialogsQTsrv
install -m 644 -p eidguiV2/eidmw_en.qm $RPM_BUILD_ROOT/usr/local/bin/
install -m 644 -p eidguiV2/eidmw_nl.qm $RPM_BUILD_ROOT/usr/local/bin/

mkdir -p $RPM_BUILD_ROOT/usr/local/share/pteid-mw/fonts/
install -m 644 -p eidguiV2/fonts/myriad/MyriadPro-Regular.otf $RPM_BUILD_ROOT/usr/local/share/pteid-mw/fonts/
install -m 644 -p eidguiV2/fonts/myriad/MyriadPro-Bold.otf $RPM_BUILD_ROOT/usr/local/share/pteid-mw/fonts/

mkdir -p $RPM_BUILD_ROOT/usr/share/applications
install -m 644 %{SOURCE1} $RPM_BUILD_ROOT/usr/share/applications

mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/scalable/apps/
install -m 644 -p %{SOURCE2} $RPM_BUILD_ROOT/usr/share/icons/hicolor/scalable/apps/

mkdir -p $RPM_BUILD_ROOT/usr/share/pixmaps
install -m 644 -p %{SOURCE3} $RPM_BUILD_ROOT/usr/share/pixmaps
mkdir -p $RPM_BUILD_ROOT/usr/share/mime/packages

%if 0%{?suse_version}
 %suse_update_desktop_file -i pteid-mw-gui Office Presentation
  export NO_BRP_CHECK_RPATH=true
%endif
%clean
rm -rf $RPM_BUILD_ROOT

%post
ln -s -f /usr/local/lib/libpteidcommon.so.2.0.0 /usr/local/lib/libpteidcommon.so
ln -s -f /usr/local/lib/libpteidcommon.so.2.0.0 /usr/local/lib/libpteidcommon.so.2
ln -s -f /usr/local/lib/libpteidcommon.so.2.0.0 /usr/local/lib/libpteidcommon.so.2.0
ln -s -f /usr/local/lib/libpteiddialogsQT.so.2.0.0 /usr/local/lib/libpteiddialogsQT.so
ln -s -f /usr/local/lib/libpteiddialogsQT.so.2.0.0 /usr/local/lib/libpteiddialogsQT.so.2
ln -s -f /usr/local/lib/libpteiddialogsQT.so.2.0.0 /usr/local/lib/libpteiddialogsQT.so.2.0
ln -s -f /usr/local/lib/libpteidcardlayer.so.2.0.0 /usr/local/lib/libpteidcardlayer.so
ln -s -f /usr/local/lib/libpteidcardlayer.so.2.0.0 /usr/local/lib/libpteidcardlayer.so.2
ln -s -f /usr/local/lib/libpteidcardlayer.so.2.0.0 /usr/local/lib/libpteidcardlayer.so.2.0
ln -s -f /usr/local/lib/libpteidpkcs11.so.2.0.0 /usr/local/lib/libpteidpkcs11.so
ln -s -f /usr/local/lib/libpteidpkcs11.so.2.0.0 /usr/local/lib/libpteidpkcs11.so.2
ln -s -f /usr/local/lib/libpteidpkcs11.so.2.0.0 /usr/local/lib/libpteidpkcs11.so.2.0
ln -s -f /usr/local/lib/libpteidapplayer.so.2.0.0 /usr/local/lib/libpteidapplayer.so
ln -s -f /usr/local/lib/libpteidapplayer.so.2.0.0 /usr/local/lib/libpteidapplayer.so.2
ln -s -f /usr/local/lib/libpteidapplayer.so.2.0.0 /usr/local/lib/libpteidapplayer.so.2.0
ln -s -f /usr/local/lib/libpteidlib.so.2.0.0 /usr/local/lib/libpteidlib.so
ln -s -f /usr/local/lib/libpteidlib.so.2.0.0 /usr/local/lib/libpteidlib.so.2
ln -s -f /usr/local/lib/libpteidlib.so.2.0.0 /usr/local/lib/libpteidlib.so.2.0
ln -s -f /usr/local/lib/libCMDServices.so.1.0.0 /usr/local/lib/libCMDServices.so
ln -s -f /usr/local/lib/libCMDServices.so.1.0.0 /usr/local/lib/libCMDServices.so.1
ln -s -f /usr/local/lib/libCMDServices.so.1.0.0 /usr/local/lib/libCMDServices.so.1.0

ln -s /usr/share/pixmaps/pteid-signature.png /usr/share/icons/hicolor/64x64/mimetypes/application-x-signedcc.png
ln -s /usr/share/pixmaps/pteid-signature.png /usr/share/icons/hicolor/64x64/mimetypes/gnome-mime-application-x-signedcc.png

%if 0%{?fedora} || 0%{?centos_version}
# BLURP: Add usr local to ldconf

echo "/usr/local/lib" > /etc/ld.so.conf.d/pteid.conf
# MDV still uses old pcscd services
if [ -x /etc/init.d/pcscd ]
then
  /etc/init.d/pcscd restart
fi

%if 0%{?fedora} >= 16
systemctl restart pcscd.service
%endif
%endif

# suse 11.4 pcscd service seems broken
%if 0%{?suse_version}  > 1140
if [ -x /etc/init.d/pcscd ]
then
  /etc/init.d/pcscd restart
fi
%endif
gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/sbin/ldconfig > /dev/null 2>&1

%postun
if [ "$1" = "0" ]; then
rm -rf /usr/local/lib/libpteidcommon.so
rm -rf /usr/local/lib/libpteidcommon.so.2
rm -rf /usr/local/lib/libpteidcommon.so.2.0
rm -rf /usr/local/lib/libpteiddialogsQT.so              
rm -rf /usr/local/lib/libpteiddialogsQT.so.2
rm -rf /usr/local/lib/libpteiddialogsQT.so.2.2
rm -rf /usr/local/lib/libpteidcardlayer.so
rm -rf /usr/local/lib/libpteidcardlayer.so.2
rm -rf /usr/local/lib/libpteidcardlayer.so.2.0
rm -rf /usr/local/lib/libpteidpkcs11.so
rm -rf /usr/local/lib/libpteidpkcs11.so.2
rm -rf /usr/local/lib/libpteidpkcs11.so.2.0
rm -rf /usr/local/lib/libpteidapplayer.so
rm -rf /usr/local/lib/libpteidapplayer.so.2
rm -rf /usr/local/lib/libpteidapplayer.so.2.0
rm -rf /usr/local/lib/libpteidlib.so
rm -rf /usr/local/lib/libpteidlib.so.2
rm -rf /usr/local/lib/libpteidlib.so.2.0

rm -rf /usr/share/icons/hicolor/64x64/mimetypes/application-x-signedcc.png
rm -rf /usr/share/icons/hicolor/64x64/mimetypes/gnome-mime-application-x-signedcc.png

# Delete all .pteid-ng folder and its contents from all users
users=$(getent passwd | awk -F: '$3 >= 1000 && $3 <= 6000' | cut -d: -f6);
for d in $users; do
  if [ -d "$d/.pteid-ng" ]; then
      if [ "$(ls -A $d/.pteid-ng)" ]; then
          # Delete only files pertaining to the cache system
          rm -f "$d"/.pteid-ng/*.ebin "$d"/.pteid-ng/*.bin "$d"/.pteid-ng/updateCertsLog.txt "$d"/.pteid-ng/updateNewsLog.txt
      fi
      rmdir --ignore-fail-on-non-empty "$d"/.pteid-ng
  fi

  # Delete scap attributes and eidmwcache folder and its contents
  if [ -d "$d/.eidmwcache" ]; then
    if [ -d "$d/.eidmwcache/scap_attributes" ]; then
      if [ "$(ls -A $d/.eidmwcache/scap_attributes)" ]; then
        # Delete only files pertaining to the cache system
        rm -f "$d"/.eidmwcache/scap_attributes/*.xml
      fi
      rmdir --ignore-fail-on-non-empty "$d"/.eidmwcache/scap_attributes
    fi
    rmdir --ignore-fail-on-non-empty "$d"/.eidmwcache
  fi
done

%if 0%{?fedora} || 0%{?centos_version}
rm -rf /etc/ld.so.conf.d/pteid.conf
%endif

gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/sbin/ldconfig > /dev/null 2>&1
fi

%files
%defattr(-,root,root)
/usr/local/lib/*
/usr/local/bin/eidguiV2
/usr/local/bin/pteiddialogsQTsrv
/usr/local/bin/eidmw_en.qm
/usr/local/bin/eidmw_nl.qm
/usr/local/include/*
/usr/share/applications/*
/usr/share/icons/*
/usr/share/pixmaps/*
/usr/local/share/certs
/usr/local/share/pteid-mw

%changelog
* Wed Jun 1 2022 André Guerreiro <andre.guerreiro@caixamagica.pt>
  - Chave Movel Digital signature support in the pteidlib SDK
  - New notifications menu and cache preference mandatory notification
  - New certificate status menu
  - Improvements in PDF signature

* Wed Dec 15 2021 André Guerreiro <andre.guerreiro@caixamagica.pt>
  - Improvements in visible PDF signature: ability to resize signature seal and select visible fields
  - Support all available signature algorithms in pteid-pkcs11 module
  - Bugfix in SCAP attributes loading
  - Bugfix in address reading. Address PIN is always requested when needed
  - Fixes in SDK exception handling
  - Improvements in Address Confirmation feature and error messages

* Wed May 26 2021 André Guerreiro <andre.guerreiro@caixamagica.pt>
  - SCAP signature improvements
  - Improved signature with custom image
  - Batch PDF signature improvements

* Mon Mar 29 2021 André Guerreiro <andre.guerreiro@caixamagica.pt>
  - New unified signature page in GUI app
  - Bugfixes in SCAP signatures
  - Stop bundling lato font and use the distro-provided TTF file
  - Extended support for expired cards until December 31st 2021
  - New feature - diagnostic report
  - Improved detection of unsupported PDF documents with XFA forms

* Wed Oct 28 2020 André Guerreiro <andre.guerreiro@caixamagica.pt>
  - Support for legislative change regarding expired cards until March 31st 2021
  - Improvement in trusted certificates update feature

* Fri Oct 16 2020 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  - SCAP signature with timestamp is a new option
  - Small format signature in simple signature menu
  - Commandline interface in the GUI app as a shortcut for signature features

* Wed Feb 26 2020 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  - CC PKI certificates self-update
  - GUI scaling options to better support high-DPI screens
  - New feature: Export certificate to file
  - Accessibility improvements

* Fri Sep 13 2019 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  Accessibility improvements in the GUI application
  Support for loading SCAP attributes using Chave Movel
  New feature: export photo in PNG or JPEG format

* Wed May 29 2019 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  SCAP Signature improvements
  Improved support for new 3072-bit RSA smartcards

* Tue Apr 16 2019 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  PDF Signature fixes
  Proxy support in SCAP signature

* Mon Feb 11 2019 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New release - version 3.0.16
  SCAP Signature improvements
  Support for new 3072-bit RSA smartcards

* Wed Sep 05 2018 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  PDF signature bugfix and Linux GTK plugin bugfix

* Wed Jul 04 2018 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  Address Change critical bugfix and various minor fixes in SCAP and CMD signature

* Tue Apr 10 2018 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New major version of pteid-mw: first build of 3.0 branch

* Wed Feb 21 2018 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 5339 - Disable checking of the address file hash in SOD only if the card was issued in a specific range of dates

* Thu Feb 15 2018 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 5334 - Disable checking of the address file hash contained and other minor bugfixes

* Fri Oct 27 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 5239 - PKCS#11 corrections and new ECRaizEstado certificate

* Thu Sep 28 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 5176 - Java SDK compatibility work and new CC 004 cert

* Tue Jul 25 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4973 - Address Change bugfixes and add missing eidlib headers

* Tue Jul 18 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4962 - Address Change bugfixes and new error messages

* Tue Jul 04 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4943 - New Production CA Certificates - ECRaizEstado and Multicert Root CA

* Fri Jun 09 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4823

* Thu Apr 27 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4672

* Fri Apr 21 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4662

* Thu Apr 20 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4654

* Mon Apr 17 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4633

* Thu Feb 02 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4397

* Wed Feb 01 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4391

* Wed Jan 04 2017 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4309
  Remove libgsoap dependency

* Thu Dec 22 2016 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4301
  Add new dependency on QT5, drop SCAP hacks


* Wed Oct 19 2016 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 4044
  Dropped bundled xml-security

* Mon Nov 02 2015 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3867
  Various fixes, new export PDF design and dss-standalone final version

* Tue Dec 09 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3788
  Add new mimetype for ccsigned files

* Thu Nov 27 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3781

* Tue Nov 18 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3771

* Tue Oct 14 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3715 (Without changes related to the PDF Signature redesign and new features)

* Wed Oct 08 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  Change major version to 2.3.0 and a couple of smallfixes

* Tue Oct 07 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3714 (Without changes related to the PDF Signature redesign and new features)

* Wed Oct 01 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3704 (Without changes related to the PDF Signature redesign and new features)

* Mon Sep 29 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3696 (Without changes related to the PDF Signature redesign and new features)

* Wed Sep 17 2014 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3577 (plus some later handpicked changes) - New feature: Address Change, Improved XAdES signature and other bugfixes

* Fri Dec 13 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot: revision 3484 - Fixed locking issue on SignPDF

* Mon Dec 09 2013 Andre Guerreiro 
  New SVN snapshot: revision 3468 - Fix CAP Pin change functionality

* Fri Nov 22 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New major version 2.2 revision 3453 - Fixed certificates included in the PDF signature

* Wed Nov 20 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New major version 2.2 revision 3446 - fixed PKCS11 issue for RSA_SHA1 signatures and a couple of small fixes

* Thu Nov 14 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot : revision 3431 - new CA certificates

* Tue Nov 12 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot : revision 3422 - Various bugfixes 

* Wed Sep 11 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot : revision 3400 - GUI Changes as requested by AMA, various bugfixes and new CA Certificates

* Thu Jan 31 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  New SVN snapshot : revision 3271 - Fix in PKCS11 module to support acroread SHA-256 signatures

* Wed Jan 30 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
  Fix %post and %postun, it was breaking the library symlinks on upgrade

* Mon Jan 21 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- New SVN snapshot : revision 3256 - Fix the titlebar in pteidcertinstall.xpi

* Tue Jan 08 2013 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- Remove references to pteidpp-gempc.so This file is not part of the package anymore

* Fri Dec 07 2012 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- New SVN snapshot : revision 3153 - Fixes for Adobe Reader interop

* Mon Nov 05 2012 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- Improved package description

* Mon Oct 31 2012 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- New SVN snapshot revision 3078 - More fine-grained positioning for signatures and 
  some GUI improvements

* Mon Oct 29 2012 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- New SVN snapshot revision 3058 - fix multiline issue in signature and parallel build 
  for poppler 

* Fri Oct 26 2012 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- New major release 2.1 - PDF Signatures is the new feature
 
* Fri Jul 27 2012 Andre Guerreiro <andre.guerreiro@caixamagica.pt>
- Bump to 2957
- Fix Fedora 17 build

