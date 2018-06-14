TEMPLATE = app

QT += sql widgets network

DEPENDPATH += .
INCLUDEPATH += .

SOURCES += \
    main.cpp \
    server.cpp \
    user.cpp \
    database.cpp

HEADERS += \
    server.h \
    user.h \
    servercommand.h \
    database.h
