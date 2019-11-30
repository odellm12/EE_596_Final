#-------------------------------------------------
#
# Project created by QtCreator 2011-03-14T15:04:01
#
#-------------------------------------------------

QT       += core gui widgets

QT       += core gui
QT    += widgets

TARGET = FaceDetection
TEMPLATE = app


SOURCES += main.cpp\
        +=C:\Git Workspace\ComputerVision-master/Plugins/Databases/Cache/Detectors.cpp \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Common/Context.cpp \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/eye.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/faces.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/fast2.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/fft.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/follow.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/gif.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/im_face.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/img.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/ip.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/just.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/kdoi.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/kmain.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/list.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/mat.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/myhebp.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/nn.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/search.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/system.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/tclHash.c \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/tool.cc \
        +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/vote.cc \
        +=C:\Git Workspace\ComputerVision-master/Common/Frame/Frame.cpp \
        Project4.cpp \
        context.cpp \
        eye.cpp \
        fast2.cpp \
        follow.cpp \
        img.cpp \
        list.cpp \
        mainwindow.cpp \
        mat.cpp \
        search.cpp \
        system.cpp \
        tclhash.cpp

HEADERS  += mainwindow.h \
    +=C:\Git Workspace\ComputerVision-master/Plugins/Databases/Cache/Detectors.h \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Common/Context.h \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/eye.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/faces.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/fast2.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/fft.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/follow.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/gif.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/img.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/ip.h \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/just.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/kdoi.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/list.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/maincore.h \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/mat.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/myhebp.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/nn.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/search.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/system.hpp \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/tclHash.h \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/tool.h \
    +=C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/vote.hpp \
    +=C:\Git Workspace\ComputerVision-master/Common/Frame/Frame.h \
    context.h \
    detect.h \
    eye.h \
    fast2.h \
    follow.h \
    img.h \
    list.h \
    mat.h \
    search.h \
    structs.h \
    system.h \
    tclhash.h

FORMS    += mainwindow.ui
INCLUDEPATH += D:\opencv\build\include
