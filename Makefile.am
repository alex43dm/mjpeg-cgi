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
src/cam.cpp

INCLUDE=-Iinclude -I/usr/include
DEBDEP=bash (>= 2.05a-11), awk, procps (>= 1:2.0.7-8), sed (>= 3.02-8), grep (>= 2.4.2-3), coreutils (>= 5.0-5)
BUILDDIR=$(PWD)/rpmbuild

if DEBUG
AM_CXXFLAGS = -g -O0 -Wall -DDEBUG $(INCLUDE)
bin_PROGRAMS = mjpeg-cgi-dev
mjpeg_cgi_dev_SOURCES = $(SOURCES)
mjpeg_cgi_dev_LDFLAGS = $(LD_ADD)
else
AM_CXXFLAGS = -Wall  $(CFLAGS) $(INCLUDE)
bin_PROGRAMS = mjpeg-cgi
mjpeg_cgi_SOURCES = $(SOURCES)
mjpeg_cgi_LDFLAGS = $(LD_ADD)
endif

AUTOMAKE_OPTIONS = subdir-objects

dist_doc_DATA=README

install_f =\
echo -n "install to $1"; \
$(MKDIR_P) $(1)/usr/bin; \
$(INSTALL) $(bin_PROGRAMS) $(1)/usr/bin; \
echo "..ok"

install: install-binPROGRAMS
	@DOLLAR_SIGN@(call install_f,$(prefix))

rpmbuild: $(bin_PROGRAMS) $(bin_PROGRAMS).spec
	@echo VERSION $(VERSION)
	@echo :Building RPM for $(bin_PROGRAMS)
	@DOLLAR_SIGN@(call install_f,$(BUILDDIR)/BUILDROOT/$(bin_PROGRAMS)-$(VERSION)-$(RELEASE).$(ARCH))
	@echo - Building the RPM
	@echo Version : $(VERSION)
	@rpmbuild --define "_topdir $(BUILDDIR)" \
  --define "_builddir %{_topdir}" \
  --define "_rpmdir %{_topdir}" \
  --define "_srcrpmdir %{_topdir}" \
  --define '_rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm' \
  --define "_specdir %{_topdir}" \
  --define "_sourcedir  %{_topdir}" \
  --define "release $(RELEASE)" \
  --define "version $(VERSION)" \
  -bb $(bin_PROGRAMS).spec

Debug: all

cleanDebug: clean
