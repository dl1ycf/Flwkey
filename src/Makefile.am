# Copyright (c) 2008 Dave Freese, W1HKJ (w1hkj AT w1hkj DOT com)

bin_PROGRAMS = flwkey

flwkey_CPPFLAGS = @FLWKEY_BUILD_CPPFLAGS@
flwkey_CXXFLAGS = @FLWKEY_BUILD_CXXFLAGS@
flwkey_CFLAGS = $(flwkey_CXXFLAGS)
flwkey_LDFLAGS = @FLWKEY_BUILD_LDFLAGS@
flwkey_LDADD = @FLWKEY_BUILD_LDADD@

MINGW32_SRC = \
	flwkey-rc.rc \
	include/flwkeyrc.h \
	mingw.c \
	compat/regex.c \
	include/compat.h


# We distribute these but do not always compile them
EXTRA_flwkey_SOURCES = $(FLWKEY_WIN32_RES_SRC) $(MINGW32_SRC)

flwkey_SOURCES =

if MINGW32
if HAVE_WINDRES
  flwkey_SOURCES += $(MINGW32_SRC)
endif
endif

########################################################################


FLWKEY_VERSION_MAJOR = @FLWKEY_VERSION_MAJOR@
FLWKEY_VERSION_MINOR = @FLWKEY_VERSION_MINOR@
FLWKEY_VERSION_PATCH = @FLWKEY_VERSION_PATCH@
FLWKEY_VERSION = @FLWKEY_VERSION@

.EXPORT_ALL_VARIABLES: nsisinst appbundle

# Sources that are generated,
BUILT_SOURCES =
# not distributed,
nodist_flwkey_SOURCES = $(BUILT_SOURCES)
# and deleted by the clean targets
CLEANFILES = $(BUILT_SOURCES)
CLEAN_LOCAL =

if MINGW32
if HAVE_WINDRES
.rc.o:
	$(WINDRES) -I$(srcdir)/include -I$(srcdir)/../data/win32 $< -O coff $@
endif
endif

install-data-local:
	if test -f $(srcdir)/../data/flwkey.xpm; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/pixmaps; \
	    $(INSTALL_DATA) $(srcdir)/../data/flwkey.xpm $(DESTDIR)/$(datadir)/pixmaps; \
	fi
	if test -f $(srcdir)/../data/flwkey.desktop; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/applications; \
	    $(INSTALL_DATA) $(srcdir)/../data/flwkey.desktop $(DESTDIR)/$(datadir)/applications; \
	fi

uninstall-local:
	rm -f $(DESTDIR)/$(datadir)/pixmaps/flwkey.xpm
	rm -f $(DESTDIR)/$(datadir)/applications/flwkey.desktop

FLWKEY_FL_SRC = wkey_dialogs.fl

if HAVE_FLUID
flgen: $(FLWKEY_FL_SRC)
	(cd $(srcdir)/include; \
	for f in $(FLWKEY_FL_SRC); do \
	    c=$${f%.fl}.cxx; \
		h=$${f%.fl}.h; \
		h=$${h##*/}; \
	    $(FLUID) -c -o ../$$c -h $$h ../$$f; \
	done)
endif


INSTALLER_FILE = flwkey-$(FLWKEY_VERSION)_setup.exe
APPBUNDLE=flwkey-$(FLWKEY_VERSION)
APPBUNDLE_NOLIBS=$(APPBUNDLE)-nolibs

if HAVE_NSIS
nsisinst: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mknsisinst.sh "$(srcdir)/../data" .
    CLEANFILES += $(INSTALLER_FILE)
endif

if DARWIN
appbundle: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mkappbundle.sh "$(srcdir)/../data" .
    CLEAN_LOCAL += $(APPBUNDLE_NOLIBS) $(APPBUNDLE) $(APPBUNDLE)*.dmg
endif

#if HAVE_ASCIIDOC
#$(builddir)/../doc/guide.html: $(builddir)/../doc/guide.txt
#	$(MAKE) -C $(builddir)/../doc $(AM_MAKEFLAGS) guide.html

#$(builddir)/dialogs/guide.cxx: $(builddir)/../doc/guide.html
#	mkdir -p $(builddir)/dialogs
#	sed 's/"/\\"/g; s/$$/\\n\\/g; 1 s/.*/const char* szBeginner = "&/; $$ s/.*/&n";/' $< > $@
#dialogs/htmlstrings.cxx: $(builddir)/dialogs/guide.cxx

#distclean-local:
#	@if test "$(builddir)" != "$(srcdir)"; then \
#	    rm -f $(builddir)/dialogs/guide.cxx; \
#        fi
#endif

clean-local:
	-rm -rf $(CLEAN_LOCAL)

# Sources that we build. It is OK to have headers here.
flwkey_SOURCES += \
	combo.cxx \
	debug.cxx \
	dialogs.cxx \
	flslider2.cxx \
	flwkey.cxx \
	wkey_io.cxx \
	xmlrpc_io.cxx \
	serial.cxx \
	status.cxx \
	support.cxx \
	util.cxx \
	ValueSlider.cxx \
	FTextView.cxx \
	Fl_Text_Display_mod.cxx \
	Fl_Text_Editor_mod.cxx \
	Fl_Native_File_Chooser.cxx \
	flinput2.cxx \
	pixmaps.cxx \
	icons.cxx \
	fileselect.cxx \
	FTextRXTX.cxx \
	logbook/adif_io.cxx \
	logbook/date.cxx \
	logbook/logsupport.cxx \
	logbook/table.cxx \
	logbook/calendar.cxx \
	logbook/lgbook.cxx \
	logbook/logbook.cxx \
	logbook/qso_db.cxx \
	logbook/textio.cxx \
	re.cxx \
	wkey_dialogs.cxx \
	xmlrpccpp/XmlRpcClient.cpp \
	xmlrpccpp/XmlRpcMutex.cpp \
	xmlrpccpp/XmlRpcServerMethod.cpp \
	xmlrpccpp/XmlRpcThread.cpp \
	xmlrpccpp/XmlRpcValue.cpp \
	xmlrpccpp/XmlRpcDispatch.cpp \
	xmlrpccpp/XmlRpcServerConnection.cpp \
	xmlrpccpp/XmlRpcSocket.cpp \
	xmlrpccpp/XmlRpcServer.cpp \
	xmlrpccpp/XmlRpcSource.cpp \
	xmlrpccpp/XmlRpcUtil.cpp

# Additional source files that are distributed
EXTRA_DIST = \
	config.h \
	flnfc_common.cxx \
	compat/regex.c \
	compat/regex.h \
	Fl_Native_File_Chooser_FLTK.cxx \
	Fl_Native_File_Chooser_MAC.cxx \
	Fl_Native_File_Chooser_WIN32.cxx \
	flwkey_icon.cxx \
	wkey_dialogs.fl \
	logbook/lgbook.fl \
	include/adif_def.h \
	include/adif_io.h \
	include/calendar.h \
	include/date.h \
	include/field_def.h \
	include/lgbook.h \
	include/log.h \
	include/logbook.h \
	include/logger.h \
	include/logsupport.h \
	include/qso_db.h \
	include/table.h \
	include/textio.h \
	include/combo.h \
	include/debug.h \
	include/dialogs.h \
	include/fileselect.h \
	include/flwkeyrc.h \
	include/flslider2.h \
	include/font_browser.h \
	include/gettext.h \
	include/mingw.h \
	include/flwkey.h \
	include/wkey_io.h \
	include/serial.h \
	include/status.h \
	include/support.h \
	include/threads.h \
	include/re.h \
	include/util.h \
	include/ValueSlider.h \
	include/FTextView.h \
	include/FTextRXTX.h \
 	include/Fl_Text_Display_mod.H \
	include/Fl_Text_Editor_mod.H \
	include/flinput2.h \
	include/pixmaps.h \
	include/icons.h \
	include/fileselect.h \
	include/wkey_dialogs.h \
	include/FL/Fl_Native_File_Chooser.H \
	include/FL/Fl_Native_File_Chooser_FLTK.H \
	include/FL/Fl_Native_File_Chooser_MAC.H \
	include/FL/Fl_Native_File_Chooser_WIN32.H \
	include/base64.h \
	include/XmlRpc.h \
	include/XmlRpcServer.h \
	include/XmlRpcSource.h \
	include/XmlRpcUtil.h
	include/XmlRpcClient.h \
	include/XmlRpcMutex.h \
	include/XmlRpcServerMethod.h \
	include/XmlRpcValue.h \
	include/XmlRpcThreadedServer.h \
	include/XmlRpcDispatch.h \
	include/XmlRpcServerConnection.h \
	include/XmlRpcSocket.h \
	include/XmlRpcThread.h \
	include/XmlRpcException.h


# Additional non-source files that are distributed
# Additional source files that support non-Linux cross compilation
EXTRA_DIST += \
	$(srcdir)/../data/flwkey.desktop \
	$(srcdir)/../data/flwkey.xpm \
	$(srcdir)/../data/win32/fl_app.nsi \
	$(srcdir)/../data/win32/flwkey.ico \
	$(srcdir)/../scripts/mknsisinst.sh \
	$(srcdir)/../scripts/build4mingw.sh \
	$(srcdir)/../data/mac/Info.plist.in \
	$(srcdir)/../data/mac/flwkey.icns \
	$(srcdir)/../scripts/mkappbundle.sh \
	$(FLWKEY_WIN32_SRC) \
	$(FLWKEY_FL_SRC)
