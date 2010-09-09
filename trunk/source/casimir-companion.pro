# -------------------------------------------------
# Project created by QtCreator 2010-05-23T13:55:00
# -------------------------------------------------
TARGET = casimir
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    cio.cpp \
    lowlevel.cpp \
    sd.cpp \
    portwindowimpl.cpp \
    precisetimer.cpp \
    dtime.cpp \
    keypresseater.cpp \
    pkt.cpp
HEADERS += mainwindow.h \
    cio.h \
    lowlevel.h \
    sd.h \
    portwindowimpl.h \
    xcommand.h \
    precisetimer.h \
    dtime.h \
    dtime.h \
    keypresseater.h \
    pkt.h
FORMS += mainwindow.ui \
    portwindow.ui \
    mergefiles.ui
RESOURCES += casimir-companion.qrc

# WINDOWS:
# Windows: library is 'qextserialport1'
win32:DEFINES += WIN32
win32:LIBS = -lqextserialport1
# win32:LIBS = -lqextserialportd1
