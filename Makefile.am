ACLOCAL_AMFLAGS = -I m4
SHELL = /bin/bash
SOURCES= \
main.cpp \
src/base64.cpp  \
src/CgiService.cpp  \
src/Log.cpp  \
src/Request.cpp  \
src/Response.cpp \
src/Config.cpp \
src/Server.cpp \
src/cam.cpp \
src/HttpClient.cpp \
src/DeviceList.cpp  \
src/Thread.cpp  \
src/UPnP_MediaRenderer.cpp \
src/Container.cpp  \
src/Item.cpp \
src/UPnP_CP.cpp  \
src/UPnP_MediaServer.cpp \
src/Device.cpp  \
src/Icon.cpp  \
src/UPnP.cpp   \
src/xmltools.cpp \
src/Service.cpp \
src/Serial.cpp



INCLUDE=-Iinclude -I/usr/include
DEBDEP=bash (>= 2.05a-11), awk, procps (>= 1:2.0.7-8), sed (>= 3.02-8), grep (>= 2.4.2-3), coreutils (>= 5.0-5), libupnp6, curl, libfcgi0ldbl, libtinyxml2.6.2, libmagick++5

if HAVE_IMAGEMAGICK
MAGICK_INCLUDE = `Magick++-config --cppflags --cxxflags`
MAGICK_LIB = `Magick++-config --ldflags --libs`
SOURCES += src/ImgMgk.cpp
endif

if DEBUG
AM_CXXFLAGS = -std=gnu++0x -g -O0 -Wall -DDEBUG $(INCLUDE) $(MAGICK_INCLUDE)
bin_PROGRAMS = mjpeg-cgi-dev
mjpeg_cgi_dev_SOURCES = $(SOURCES)
mjpeg_cgi_dev_LDFLAGS = $(LD_ADD) $(MAGICK_LIB)
else
AM_CXXFLAGS = -std=gnu++0x -Wall  $(CFLAGS) $(INCLUDE) $(MAGICK_INCLUDE)
bin_PROGRAMS = mjpeg-cgi
mjpeg_cgi_SOURCES = $(SOURCES)
mjpeg_cgi_LDFLAGS = $(LD_ADD) $(MAGICK_LIB)
endif

AUTOMAKE_OPTIONS = subdir-objects

Debug: all

cleanDebug: clean

debian_BUILD = $(bin_PROGRAMS)-$(VERSION)

MKDIR_P = mkdir -p

dist_doc_DATA=README

sysconf_DATA= \
index.html \
js/jquery.min.js \
js/jquery.timer.js

install_f =\
echo -n "install to $1"; \
$(MKDIR_P) $(1)/etc/$(bin_PROGRAMS)/js; \
$(MKDIR_P) $(1)/etc/init.d; \
$(MKDIR_P) $(1)/usr/sbin; \
$(MKDIR_P) $(1)/var/www/js; \
$(INSTALL) ./index.html $(1)/var/www; \
$(INSTALL) ./js/jquery.min.js $(1)/var/www/js; \
$(INSTALL) ./js/jquery.timer.js $(1)/var/www/js; \
$(INSTALL) ./config.xml $(1)/etc/$(bin_PROGRAMS)/config.xml; \
$(INSTALL) $(bin_PROGRAMS) $(1)/usr/sbin; \
$(INSTALL) ./conf/mjpeg-cgi $(1)/etc/init.d/$(bin_PROGRAMS); \
chmod +x  $(1)/etc/init.d/$(bin_PROGRAMS); \
$(INSTALL) ./conf/mjpeg-cgi.service /lib/systemd/system; \
systemctl enable /lib/systemd/system/mjpeg-cgi.service; \
useradd -G dialout www-data; \
for p in $(sysconf_DATA); do \
$(INSTALL_DATA) $$p "$(1)/etc/$(bin_PROGRAMS)/$$p" || exit $$?; \
done; \
echo "..ok"

install: install-binPROGRAMS
	@DOLLAR_SIGN@(call install_f,$(prefix))

deb: $(bin_PROGRAMS)
	rm -rf $(debian_BUILD)
	$(MKDIR_P) $(debian_BUILD)/DEBIAN
	echo "Package: $(bin_PROGRAMS)">  $(debian_BUILD)/DEBIAN/control
	echo "Version: $(VERSION)" >> $(debian_BUILD)/DEBIAN/control
	echo "Section: base" >> $(debian_BUILD)/DEBIAN/control
	echo "Priority: optional" >> $(debian_BUILD)/DEBIAN/control
	echo "Architecture: all" >> $(debian_BUILD)/DEBIAN/control
	echo "Depends: $(DEBDEP)" >> $(debian_BUILD)/DEBIAN/control
	echo "Maintainer: Alex D <wvdial@gmail.com>" >> $(debian_BUILD)/DEBIAN/control
	echo "Description:  multithread demon, to return html with video from nginx." >> $(debian_BUILD)/DEBIAN/control
	@DOLLAR_SIGN@(call install_f,$(debian_BUILD))
	cd $(debian_BUILD) &&  for p in `find . -type f -print | grep -v DEBIAN`; do md5sum $$p | sed 's|^.||' >>./DEBIAN/md5sum; done
	dpkg-deb --build $(debian_BUILD)
	rm -rf $(debian_BUILD)
