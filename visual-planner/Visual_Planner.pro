#-------------------------------------------------
#
# Project created by QtCreator 2015-02-27T12:34:16
#
#-------------------------------------------------

QT       += core gui\
            sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Visual_Planner
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    kkplansql.cpp \
    kkagentplanclass.cpp \
    playoff.cpp \
    kkpoplansql.cpp

HEADERS  += mainwindow.h \
    kkplansql.h \
    kkagentplanclass.h \
    base.h \
    playoff.h \
    kkpoplansql.h

FORMS    += mainwindow.ui