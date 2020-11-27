#-------------------------------------------------
#
# Dependencies
#
#-------------------------------------------------

# qredisclient
include($$PWD/qredisclient/qredisclient.pri)
include($$PWD/qsshclient/qsshclient.pri)

# Easylogging
INCLUDEPATH += $$PWD/easyloggingpp/src
HEADERS += $$PWD/easyloggingpp/src/easylogging++.h
SOURCES += $$PWD/easyloggingpp/src/easylogging++.cc

win32-msvc* {
    message("Configure 3rdparty for MSVC")
    QMAKE_CXXFLAGS += /MP
    QMAKE_LFLAGS_RELEASE += /MAP
    QMAKE_CFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /debug /opt:ref
} else {
    QMAKE_CXXFLAGS+=-g
    QMAKE_CFLAGS_RELEASE+=-g
}

win32-g++ {
    # Workaround for mingw
    # QMAKE_LFLAGS_RELEASE=
}

unix:macx { # OSX
    LIBS += /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
    LIBS += /System/Library/Frameworks/CoreServices.framework/Versions/A/CoreServices
}

unix:!macx { # ubuntu & debian
    defined(CLEAN_RPATH, var) { # clean default flags
        message("DEB package build")
        QMAKE_LFLAGS_RPATH=
        QMAKE_LFLAGS = -Wl,-rpath=\\\$$ORIGIN/../lib
        QMAKE_LFLAGS += -static-libgcc -static-libstdc++
    } else {
        # Note: uncomment if qtcreator fails to find QtCore dependencies
        #QMAKE_LFLAGS = -Wl,-rpath=/home/user/Qt5.9.3/5.9.3/gcc_64/lib
    }
}
