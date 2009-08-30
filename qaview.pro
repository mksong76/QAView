TEMPLATE        = app
HEADERS         = \
    zlib.h zconf.h zipfs.h zipfsi.h zfile.h qzfile.h \
    qzfs.h qzdir.h afile.h \
    timestamp.h aconfig.h gconfig.h \
    unicode.h qunicode.h para.h parser.h ansiparser.h \
    aview.h formatter.h hcode.h fileselect.h optiondlg.h \
    askdlg.h battery.h \
    zipfile.h af_file.h userevent.h filelister.h \
    monitor.h \
    view.h document.h content.h textdoc.h \
    statusbar.h
SOURCES         = \
    zipfs.cpp zipfsi.cpp zfile.cpp qzfile.cpp  \
    qzfs.cpp qzdir.cpp \
    timestamp.cpp aconfig.cpp gconfig.cpp \
    unicode.cpp qunicode.cpp para.cpp parser.cpp ansiparser.cpp \
    qaview.cpp aview.cpp formatter.cpp hcode.c fileselect.cpp \
    optiondlg.cpp askdlg.cpp battery.cpp \
    zipfile.c af_file.c filelister.cpp monitor.cpp \
    view.cpp document.cpp content.cpp textdoc.cpp \
    statusbar.cpp

INTERFACES      = optionform.ui fileopen.ui askform.ui
TRANSLATIONS    = qaview_ko.ts qaview_en.ts
#CONFIG          = qpe qt debug
CONFIG          = qpe qt release
INCLUDEPATH     = conv
MOC_DIR		= build
OBJECTS_DIR	= build
TARGET          = qaview
LIBS            += -lqpe -lqte -lstdc++ -lpthread
#LIBS            += -lstdc++ -lqpe -lz
