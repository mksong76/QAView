#
#	Makefile for QAView.
#
#

TRANSLATE_LANGS	= ko
TRANSLATE_TSS	= $(patsubst %,qaview_%.ts,$(TRANSLATE_LANGS))
TRANSLATE_MSGS	= $(patsubst %,qaview_%.qm,$(TRANSLATE_LANGS))

target : all $(TRANSLATE_MSGS)

qaview.mak : qaview.pro
	@ echo "Remaking $@ from $<"
	@ tmake $< > $@

#-----------------------------------------------------------------------------
#   making translated messages..
$(TRANSLATE_MSGS) : %.qm : %.ts
	@ echo "Making message $@"
	@ lrelease $<
$(TRANSLATE_TSS) :
	@ lupdate qaview.pro

#-----------------------------------------------------------------------------
#   making package.
TDIR=ipkg
package :
	@ echo "Copying binary..."
	@ mkdir -p $(TDIR)/opt/QtPalmtop/bin
	@ cp qaview $(TDIR)/opt/QtPalmtop/bin
	@ echo "Copying images.."
	@ mkdir -p $(TDIR)/opt/QtPalmtop/pics144/qaview
	@ cp icons/*.png $(TDIR)/opt/QtPalmtop/pics144/qaview
	@ mkdir -p $(TDIR)/opt/QtPalmtop/pics
	@ ( cd $(TDIR)/opt/QtPalmtop/pics ; ln -sf ../pics144/qaview . )
	@ echo "Copy other files.."
	@ mkdir -p $(TDIR)/opt/QtPalmtop/apps/Applications
	@ cp qaview.desktop $(TDIR)/opt/QtPalmtop/apps/Applications
	@ mkdir -p $(TDIR)/CONTROL
	@ cp qaview.control $(TDIR)/CONTROL/control
	@ \
	for k in $(TRANSLATE_LANGS) ; do \
	    if [ -f "qaview_$$k.qm" ] ; then \
		mkdir -p $(TDIR)/opt/QtPalmtop/i18n/ko; \
		cp qaview_$$k.qm $(TDIR)/opt/QtPalmtop/i18n/$$k/qaview.qm;\
	    fi; \
	done
	@ bash ./ipkg-build.sh $(TDIR)

-include qaview.mak

T_DZFILE_SRC	= dzfile.cpp t_dzfile.cpp
T_DZFILE_HDR	= dzfile.h
t_dzfile : $(T_DZFILE_SRC) $(T_DZFILE_HDR)
	$(CXX) $(CXXFLAGS) $(LFLAGS) $(LIBS) $(T_DZFILE_SRC) -o $@ \
	  -DTEST_DEBUG
	
