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
        Project4.cpp \
        list.cpp \
        mainwindow.cpp \
        mat.cpp \
        stasm4.1.0/stasm4/MOD_1/facedet.cpp \
        stasm4.1.0/stasm4/MOD_1/initasm.cpp \
        stasm4.1.0/stasm4/asm.cpp \
        stasm4.1.0/stasm4/classicdesc.cpp \
        stasm4.1.0/stasm4/convshape.cpp \
        stasm4.1.0/stasm4/err.cpp \
        stasm4.1.0/stasm4/eyedet.cpp \
        stasm4.1.0/stasm4/eyedist.cpp \
        stasm4.1.0/stasm4/faceroi.cpp \
        stasm4.1.0/stasm4/hat.cpp \
        stasm4.1.0/stasm4/hatdesc.cpp \
        stasm4.1.0/stasm4/landmarks.cpp \
        stasm4.1.0/stasm4/misc.cpp \
        stasm4.1.0/stasm4/pinstart.cpp \
        stasm4.1.0/stasm4/print.cpp \
        stasm4.1.0/stasm4/shape17.cpp \
        stasm4.1.0/stasm4/shapehacks.cpp \
        stasm4.1.0/stasm4/shapemod.cpp \
        stasm4.1.0/stasm4/startshape.cpp \
        stasm4.1.0/stasm4/stasm.cpp \
        stasm4.1.0/stasm4/stasm_lib.cpp \
        tclhash.cpp

HEADERS  += mainwindow.h \
    context.h \
    detect.h \
    frame.h \
    list.h \
    mat.h \
    search.h \
    stasm4.1.0/stasm4/MOD_1/facedet.h \
    stasm4.1.0/stasm4/MOD_1/initasm.h \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p00_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p01_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p02_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p03_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p04_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p05_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p06_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p07_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p08_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p09_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p10_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p11_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p12_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p13_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p14_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p15_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p16_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p17_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p18_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p19_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p20_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p21_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p22_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p23_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p24_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p25_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p26_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p27_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p28_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p29_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p30_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p31_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p32_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p33_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p34_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p35_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p36_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p37_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p38_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p39_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p40_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p41_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p42_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p43_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p44_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p45_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p46_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p47_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p48_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p49_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p50_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p51_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p52_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p53_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p54_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p55_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p56_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p57_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p58_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p59_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p60_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p61_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p62_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p63_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p64_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p65_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p66_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p67_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p68_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p69_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p70_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p71_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p72_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p73_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p74_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p75_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev0_p76_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p00_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p01_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p02_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p03_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p04_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p05_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p06_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p07_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p08_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p09_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p10_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p11_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p12_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p13_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p14_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p15_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p16_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p17_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p18_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p19_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p20_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p21_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p22_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p23_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p24_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p25_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p26_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p27_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p28_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p29_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p30_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p31_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p32_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p33_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p34_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p35_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p36_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p37_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p38_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p39_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p40_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p41_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p42_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p43_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p44_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p45_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p46_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p47_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p48_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p49_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p50_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p51_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p52_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p53_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p54_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p55_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p56_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p57_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p58_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p59_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p60_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p61_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p62_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p63_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p64_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p65_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p66_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p67_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p68_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p69_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p70_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p71_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p72_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p73_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p74_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p75_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev1_p76_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p00_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p01_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p02_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p03_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p04_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p05_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p06_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p07_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p08_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p09_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p10_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p11_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p12_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p13_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p14_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p15_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p16_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p17_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p18_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p19_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p20_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p21_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p22_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p23_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p24_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p25_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p26_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p27_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p28_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p29_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p30_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p31_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p32_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p33_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p34_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p35_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p36_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p37_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p38_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p39_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p40_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p41_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p42_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p43_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p44_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p45_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p46_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p47_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p48_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p49_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p50_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p51_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p52_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p53_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p54_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p55_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p56_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p57_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p58_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p59_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p60_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p61_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p62_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p63_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p64_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p65_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p66_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p67_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p68_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p69_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p70_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p71_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p72_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p73_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p74_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p75_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev2_p76_hat.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p00_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p01_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p02_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p03_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p04_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p05_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p06_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p07_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p08_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p09_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p10_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p11_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p12_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p13_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p14_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p15_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p16_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p17_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p18_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p19_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p20_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p21_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p22_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p23_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p24_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p25_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p26_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p27_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p28_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p29_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p30_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p31_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p32_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p33_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p34_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p35_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p36_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p37_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p38_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p39_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p40_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p41_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p42_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p43_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p44_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p45_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p46_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p47_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p48_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p49_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p50_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p51_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p52_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p53_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p54_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p55_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p56_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p57_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p58_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p59_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p60_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p61_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p62_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p63_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p64_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p65_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p66_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p67_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p68_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p69_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p70_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p71_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p72_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p73_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p74_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p75_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_lev3_p76_classic.mh \
    stasm4.1.0/stasm4/MOD_1/mh/yaw00_shapemodel.mh \
    stasm4.1.0/stasm4/asm.h \
    stasm4.1.0/stasm4/atface.h \
    stasm4.1.0/stasm4/basedesc.h \
    stasm4.1.0/stasm4/classicdesc.h \
    stasm4.1.0/stasm4/convshape.h \
    stasm4.1.0/stasm4/err.h \
    stasm4.1.0/stasm4/eyedet.h \
    stasm4.1.0/stasm4/eyedist.h \
    stasm4.1.0/stasm4/faceroi.h \
    stasm4.1.0/stasm4/hat.h \
    stasm4.1.0/stasm4/hatdesc.h \
    stasm4.1.0/stasm4/landmarks.h \
    stasm4.1.0/stasm4/landtab_muct77.h \
    stasm4.1.0/stasm4/misc.h \
    stasm4.1.0/stasm4/pinstart.h \
    stasm4.1.0/stasm4/print.h \
    stasm4.1.0/stasm4/shape17.h \
    stasm4.1.0/stasm4/shapehacks.h \
    stasm4.1.0/stasm4/shapemod.h \
    stasm4.1.0/stasm4/startshape.h \
    stasm4.1.0/stasm4/stasm.h \
    stasm4.1.0/stasm4/stasm_landmarks.h \
    stasm4.1.0/stasm4/stasm_lib.h \
    stasm4.1.0/stasm4/stasm_lib_ext.h \
    structs.h \
    system.h \
    tclhash.h

LIBS += D:\opencv\build\x64\vc15\lib\opencv_calib3d248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_contrib248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_core248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_features2d248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_flann248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_gpu248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_highgui248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_imgproc248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_legacy248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_ml248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_nonfree248.lib
LIBS += D:\opencv\build\x64\vc15\lib\opencv_objdetect248.lib
LIBS += D:\aminegar\Downloads\EE_596_Final\EE_596_Final\FinalProject\Code\stasm.lib
LIBS += D:\aminegar\Downloads\EE_596_Final\EE_596_Final\FinalProject\Code\stasm4.1.0\vc10\Debug\stasm_lib.obj
LIBS += D:\aminegar\Downloads\EE_596_Final\EE_596_Final\FinalProject\Code\stasm4.1.0\vc10\Debug\startshape.obj

FORMS    += mainwindow.ui
INCLUDEPATH += D:\opencv\build\include
INCLUDEPATH += D:\opencv\build\include\opencv2
INCLUDEPATH += D:\aminegar\Downloads\EE_596_Final\EE_596_Final\FinalProject\Code\stasm4.1.0\

DISTFILES += \
    stasm4.1.0/stasm4/MOD_1/README.txt \
    stasm4.1.0/stasm4/MOD_1/log/tasm.log
