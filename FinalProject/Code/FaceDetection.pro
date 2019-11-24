#-------------------------------------------------
#
# Project created by QtCreator 2011-03-14T15:04:01
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = FaceDetection
TEMPLATE = app


SOURCES += main.cpp\
        ../../../ComputerVision-master/Algorithms/Detection/Common/Context.cpp \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/eye.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/faces.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/fast2.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/fft.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/follow.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/gif.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/im_face.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/img.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/ip.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/just.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/kdoi.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/kmain.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/list.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/mat.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/myhebp.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/nn.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/search.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/system.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/tclHash.c \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/tool.cc \
        ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/vote.cc \
        ../../../ComputerVision-master/Common/Frame/Frame.cpp \
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
    ../../../ComputerVision-master/Algorithms/Detection/Common/Context.h \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/eye.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/faces.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/fast2.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/fft.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/follow.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/gif.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/img.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/ip.h \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/just.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/kdoi.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/list.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/maincore.h \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/mat.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/myhebp.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/nn.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/search.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/system.hpp \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/tclHash.h \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/tool.h \
    ../../../ComputerVision-master/Algorithms/Detection/Rowley/detect/vote.hpp \
    ../../../ComputerVision-master/Common/Frame/Frame.h \
    context.h \
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
