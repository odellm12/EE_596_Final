#ifndef FOLLOW_H
#define FOLLOW_H
#include <QtGui>
#include <QByteArray>
#include <QSize>
#include <QList>
#include <QApplication>
#include <QDir>

#include "search.h"
#include "mat.h"

extern Mat LightingCorrectionMatrix;
extern int LightingCorrectionMatrixInitialized;
extern void InitializeLightingCorrectionMatrix();

int FindNewLocation(int numScales, QImage* imagePyramid[], QImage* mask,
    int x, int y, int s, int dx, int dy, int ds, int step,
    int basenet,
    int* newx, int* newy, int* news);

#ifdef CompiledNets
int FindNewLocationCompiled(
    int numScales, QImage* imagePyramid[], QImage* mask,
    int x, int y, int s, int dx, int dy, int ds, int step,
    int basenet,
    int* newx, int* newy, int* news);

int FindNewLocationCompiled2(
    int numScales, QImage* imagePyramid[], QImage* mask,
    int x, int y, int s, int dx, int dy, int ds, int step,
    int basenet,
    int* newx, int* newy, int* news);
#endif

int FindNewLocationRecordMax(int numScales, QImage* imagePyramid[],
    QImage* mask,
    int x, int y, int s,
    int dx, int dy, int ds, int step,
    int basenet,
    int* x1, int* y1, int* s1, double* o1,
    int* x2, int* y2, int* s2, double* o2);
void StartFrameTimer();
void StopFrameTimer();

#endif
