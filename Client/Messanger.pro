#-------------------------------------------------
#
# Project created by QtCreator 2018-05-14T15:34:14
#
#-------------------------------------------------

QT       += core gui
QT       += widgets network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Messanger
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        authorization.cpp \
    client.cpp \
    useritem.cpp \
    interaction.cpp \
    memberlist.cpp \
    audio.cpp \
    voipclient.cpp

HEADERS += \
        authorization.h \
    client.h \
    useritem.h \
    interaction.h \
    memberlist.h \
    audio.h \
    voipclient.h \
    servercommand.h

FORMS += \
        authorization.ui \
    client.ui \
    interaction.ui \
    interaction.ui \
    memberlist.ui
