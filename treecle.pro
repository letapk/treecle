#-------------------------------------------------
#
# Project created by QtCreator 2014-10-21T13:15:04
#
#-------------------------------------------------

#//Last modified Sept 14, 2024

#This should be uncommented to produce a compile time error
#for string literals that are not enclosed within a tr()
#This is useful to catch strings that have not been translated
#if a translation file is available.
#DEFINES  += QT_NO_CAST_FROM_ASCII

QT += core gui widgets

TARGET = treecle
TEMPLATE = app

SOURCES += \
    treemisc.cpp \
    treemain.cpp \
    treefile.cpp \
    treetree.cpp \
    treeprefs.cpp

HEADERS  += \
    treecle.h

#Put libqtspell-qt6.so in /usr/lib or /usr/lib64
LIBS += -lqtspell-qt6

RESOURCES = treecle.qrc

OTHER_FILES += \
    COPYING \
    AUTHORS \
    README

TRANSLATIONS += treecle_de.ts


