
#ifndef hpux
#include <math.h>
#endif
#ifdef hpux
#include <math.h>
#endif

#include "follow.h"
#include "mat.h"
#include "img.h"
#include "system.h"
#include "myhebp.h"
#include "fast2.h"
#include "search.h"

//#define DEBUG

#ifdef sparc
#ifndef solaris
extern "C" {
    int gettimeofday(struct timeval* tp, struct timezone* tzp);
}
#endif
#endif

// USED
// Store the lighting correction matrix, and whether or not it is computed
Mat LightingCorrectionMatrix;
int LightingCorrectionMatrixInitialized = 0;

// USED
// The lighting correction matrix is used to fit an affine function to
// the intensity values in a window
void InitializeLightingCorrectionMatrix()
{
    QImage* mask = FindQImageWithName("facemask");
    int ptr = 0;
    Mat mat = Zero(3, 3);
    int i;
    for (int j = -mask->height / 2; j < mask->height - mask->height / 2; j++)
        for (i = -mask->width / 2; i < mask->width - mask->width / 2; i++)
            // Only use pixels inside the mask
            if (mask->data(ptr++)) {
                mat(0, 0) += i * i;
                mat(0, 1) += i * j;
                mat(0, 2) += i;
                mat(1, 0) += i * j;
                mat(1, 1) += j * j;
                mat(1, 2) += j;
                mat(2, 0) += i;
                mat(2, 1) += j;
                mat(2, 2) += 1;
            }
    LightingCorrectionMatrix = LUInverse(mat);
    LightingCorrectionMatrixInitialized = 1;
}

extern "C" {
    void face_net(int* inputs, float* outputs);
    void face12_net(int* inputs, float* outputs);
    void face13_net(int* inputs, float* outputs);
    void face_net_debug(int* inputs, float* outputs, FastForwardStruct* net);
}

// USED
// Given a candidate detection location and the image pyramid, search around
// in that candidate to try and find out whether there is really a face there.
// Uses two networks (merged together), both of which have to agree on a
// detection.  If both networks agree on multiple locations, the centroid of
// those locations is computed.
int FindNewLocation(int numScales, QImage* imagePyramid[], QImage* mask,
    int x, int y, int s, int dx, int dy, int ds, int step,
    int basenet,
    int* newX, int* newY, int* news)
{
    if (!LightingCorrectionMatrixInitialized)
        InitializeLightingCorrectionMatrix();

    double totX = 0;      // Used to record centroid of face detections
    double totY = 0;
    double totS = 0;
    double tot = 0;

    int ptr;
    int hist[512];                  // Histogram
    int map[512];                   // Cumulative histogram
    Vec vec(3);                     // Part of affine fitting
    int* tmp = new int[mask->size];   // Window

    int halfX = mask->width / 2;
    int halfY = mask->height / 2;

    x += halfX;     // Input location is upper left corner of
    y += halfY;     // the "centered" face.  This makes location be face center.

    FastForwardStruct* net = real_fast_list[basenet];
    FastForwardStruct* net2 = real_fast_list[basenet + 1];

    // Loop over scales
    for (int ss = s - ds; ss <= s + ds; ss++) {
        if (ss < 0 || ss >= numScales) continue;

        int cx = (int)floor(x * pow(1.2, s - ss) + 0.5);  // Map center position to scale
        int cy = (int)floor(y * pow(1.2, s - ss) + 0.5);

        QImage* image = imagePyramid[ss];
        for (int yy = cy - dy; yy <= cy + dy; yy += step)  // Scan over position
            for (int xx = cx - dx; xx <= cx + dx; xx += step) {
                //        if (yy!=cy && xx!=cx) continue;

                ptr = 0;
                double v0 = 0, v1 = 0, v2 = 0;

                // The next two loops copy the window into the tmp variable, and
                // begin computing the affine fit (using only pixels inside the mask)
                // The first version is for windows inside the image, the second
                // replicates the edge pixels of the image.
                if (xx - halfX >= 0 && yy - halfY >= 0 &&
                    xx + halfX <= image->width && yy + halfY <= image->height) {
                    for (int iy = yy - halfY; iy < yy + halfY; iy++)
                        for (int ix = xx - halfX; ix < xx + halfX; ix++) {
                            int val = image->data(ix, iy);
                            tmp[ptr] = val;
                            if (mask->data(ptr++))
                            {
                                v0 += (ix - xx) * val;
                                v1 += (iy - yy) * val;
                                v2 += val;
                            }
                        }
                }
                else {
                    for (int iy = yy - halfY; iy < yy + halfY; iy++)
                        for (int ix = xx - halfX; ix < xx + halfX; ix++) {
                            int ii = ix; if (ii < 0) ii = 0;
                            if (ii >= image->width) ii = image->width - 1;
                            int jj = iy; if (jj < 0) jj = 0;
                            if (jj >= image->height) jj = image->height - 1;
                            int val = image->data(ii, jj);
                            tmp[ptr] = val;
                            if (mask->data(ptr++))
                            {
                                v0 += (ix - xx) * val;
                                v1 += (iy - yy) * val;
                                v2 += val;
                            }
                        }
                }
#ifdef DEBUG
                if (xx - halfX == 12 && yy - halfY == 28 && ss == 8) {
                    for (int i = 0; i < 400; i++)
                        fprintf(stderr, "in[%d]=%d\n", i, tmp[i]);
                }
#endif
                // Actually compute the parameters of the affine fit.
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = LightingCorrectionMatrix * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);

                // Apply the affine correction, and build the histogram based
                // on pixels in the mask
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = -mask->height / 2; j < mask->height / 2; j++)
                    for (int i = -mask->width / 2; i < mask->width / 2; i++)
                    {
                        int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                        if (val < 0) val = 0;
                        if (val >= 512) val = 511;
                        if (mask->data(ptr)) hist[val]++;
                        tmp[ptr++] = val;
                    }

                // Build the cummulative histogram
                int* to = map;
                int* from = hist;
                int total = 0;
                for (int i = 0; i < 512; i++) {
                    int old = total;
                    total += *(from++);
                    *(to++) = old + total;
                }

#ifdef DEBUG
                fprintf(stderr, "total=%d!!!!\n", total);
#endif
                // Apply histogram equalization and copy the window into the
                // network inputs
                double scaleFactor = 1.0 / total;
                FastForwardUnit* unit = &(net->unitList[1]);
                int* p = tmp;
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[*(p++)] * scaleFactor - 1.0;
#ifdef DEBUG
                for (i = 0; i < mask->size; i++) tmp[i] = map[tmp[i]] - 320;
                float outputs[2];
                face_net(tmp, outputs);
#endif
                FastForwardPass(net);
#ifdef DEBUG
                fprintf(stderr, "F: %10.7f, %10.7f\n",
                    outputs[0] - net->unitList[net->firstOutput].activation,
                    outputs[1] - net->unitList[net->firstOutput + 1].activation);
                if (outputs[0] - net->unitList[net->firstOutput].activation > 1.0) {
                    face_net_debug(tmp, outputs, net);
                }
#endif
                // Check the two outputs (we applied one network, but really
                // it is two merged networks).  If both responded postively,
                // then add the detection on to the centroid of detections.
                if (net->unitList[net->firstOutput].activation > 0) {
                    FastForwardUnit* unit = &(net->unitList[1]);
                    FastForwardUnit* unit2 = &(net2->unitList[1]);
                    for (int i = 0; i < mask->size; i++)
                        (unit2++)->activation = (unit++)->activation;
                    FastForwardPass(net2);
                    if (net2->unitList[net2->firstOutput].activation > 0) {
#ifdef DEBUG
                        FastDebugNetwork(net);
                        fprintf(stderr, "at (%d,%d,%d)\n", xx - halfX, yy - halfY, ss);
                        exit(1);
#endif
                        totS += ss;
                        totX += xx * pow(1.2, ss);
                        totY += yy * pow(1.2, ss);
                        tot++;
                    }
                }
            }
    }
    delete[] tmp;
    if (tot == 0) return 0;

    // Compute the centroid, first in scale and then in position at that scale
    int newS = (int)floor(totS / tot + 0.5);
    if (newS < 0) newS = 0;
    if (newS >= numScales) newS = numScales - 1;
    *newX = (int)floor(totX / tot / pow(1.2, newS) + 0.5) - halfX;
    *newY = (int)floor(totY / tot / pow(1.2, newS) + 0.5) - halfY;
    *news = newS;
    return 1;
}

#ifdef CompiledNets
int FindNewLocationCompiled(int numScales, QImage* imagePyramid[],
    QImage* mask,
    int x, int y, int s, int dx, int dy, int ds,
    int step,
    int basenet,
    int* newX, int* newY, int* news)
{
    if (!LightingCorrectionMatrixInitialized)
        InitializeLightingCorrectionMatrix();
    double totX = 0;
    double totY = 0;
    double totS = 0;
    double tot = 0;
    int ptr;
    int hist[512];
    int map[512];
    Vec vec(3);
    int* tmp = new int[mask->size];
    int halfX = mask->width / 2;
    int halfY = mask->height / 2;
    x += halfX;
    y += halfY;
    FastForwardStruct* net = real_fast_list[basenet];
    for (int ss = s - ds; ss <= s + ds; ss++)
    {
        if (ss < 0 || ss >= numScales) continue;
        int cx = (int)floor(x * pow(1.2, s - ss) + 0.5);
        int cy = (int)floor(y * pow(1.2, s - ss) + 0.5);
        QImage* image = imagePyramid[ss];
        for (int yy = cy - dy; yy <= cy + dy; yy += step)
            for (int xx = cx - dx; xx <= cx + dx; xx += step)
            {
                ptr = 0;
                double v0 = 0, v1 = 0, v2 = 0;
                if (xx - halfX >= 0 && yy - halfY >= 0 &&
                    xx + halfX <= image->width && yy + halfY <= image->height) {
                    for (int iy = yy - halfY; iy < yy + halfY; iy++)
                        for (int ix = xx - halfX; ix < xx + halfX; ix++) {
                            int val = image->data(ix, iy);
                            tmp[ptr] = val;
                            if (mask->data(ptr++))
                            {
                                v0 += (ix - xx) * val;
                                v1 += (iy - yy) * val;
                                v2 += val;
                            }
                        }
                }
                else {
                    for (int iy = yy - halfY; iy < yy + halfY; iy++)
                        for (int ix = xx - halfX; ix < xx + halfX; ix++) {
                            int ii = ix; if (ii < 0) ii = 0;
                            if (ii >= image->width) ii = image->width - 1;
                            int jj = iy; if (jj < 0) jj = 0;
                            if (jj >= image->height) jj = image->height - 1;
                            int val = image->data(ii, jj);
                            tmp[ptr] = val;
                            if (mask->data(ptr++))
                            {
                                v0 += (ix - xx) * val;
                                v1 += (iy - yy) * val;
                                v2 += val;
                            }
                        }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = LightingCorrectionMatrix * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = -mask->height / 2; j < mask->height / 2; j++)
                    for (i = -mask->width / 2; i < mask->width / 2; i++)
                    {
                        int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2) + 256;
                        if (val < 0) val = 0;
                        if (val >= 512) val = 511;
                        if (mask->data(ptr)) hist[val]++;
                        tmp[ptr++] = val;
                    }
                int* to = map;
                int* from = hist;
                int total = -160;
                for (i = 0; i < 512; i++) {
                    int old = total;
                    total += *(from++);
                    *(to++) = old + total;
                }
                from = tmp;
                for (i = 0; i < mask->size; i++) {
                    *from = map[*from];
                    from++;
                }
                float output[2];
                face_net(tmp, output);
                if (output[0] > 0 && output[1] > 0)
                {
                    totS += ss;
                    totX += xx * pow(1.2, ss);
                    totY += yy * pow(1.2, ss);
                    tot++;
                }
            }
    }
    delete[] tmp;
    if (tot == 0) return 0;
    int newS = (int)floor(totS / tot + 0.5);
    if (newS < 0) newS = 0;
    if (newS >= numScales) newS = numScales - 1;
    *newX = (int)floor(totX / tot / pow(1.2, newS) + 0.5) - halfX;
    *newY = (int)floor(totY / tot / pow(1.2, newS) + 0.5) - halfY;
    *news = newS;
    return 1;
}

int FindNewLocationCompiled2(int numScales, QImage* imagePyramid[],
    QImage* mask,
    int x, int y, int s, int dx, int dy, int ds,
    int step,
    int basenet,
    int* newX, int* newY, int* news)
{
    if (!LightingCorrectionMatrixInitialized)
        InitializeLightingCorrectionMatrix();
    double totX = 0;
    double totY = 0;
    double totS = 0;
    double tot = 0;
    int ptr;
    int hist[512];
    int map[512];
    Vec vec(3);
    int* tmp = new int[mask->size];
    int halfX = mask->width / 2;
    int halfY = mask->height / 2;
    x += halfX;
    y += halfY;
    FastForwardStruct* net = real_fast_list[basenet];
    for (int ss = s - ds; ss <= s + ds; ss++)
    {
        if (ss < 0 || ss >= numScales) continue;
        int cx = (int)floor(x * pow(1.2, s - ss) + 0.5);
        int cy = (int)floor(y * pow(1.2, s - ss) + 0.5);
        QImage* image = imagePyramid[ss];
        for (int yy = cy - dy; yy <= cy + dy; yy += step)
            for (int xx = cx - dx; xx <= cx + dx; xx += step)
            {
                ptr = 0;
                double v0 = 0, v1 = 0, v2 = 0;
                if (xx - halfX >= 0 && yy - halfY >= 0 &&
                    xx + halfX <= image->width && yy + halfY <= image->height) {
                    for (int iy = yy - halfY; iy < yy + halfY; iy++)
                        for (int ix = xx - halfX; ix < xx + halfX; ix++) {
                            int val = image->data(ix, iy);
                            tmp[ptr] = val;
                            if (mask->data(ptr++))
                            {
                                v0 += (ix - xx) * val;
                                v1 += (iy - yy) * val;
                                v2 += val;
                            }
                        }
                }
                else {
                    for (int iy = yy - halfY; iy < yy + halfY; iy++)
                        for (int ix = xx - halfX; ix < xx + halfX; ix++) {
                            int ii = ix; if (ii < 0) ii = 0;
                            if (ii >= image->width) ii = image->width - 1;
                            int jj = iy; if (jj < 0) jj = 0;
                            if (jj >= image->height) jj = image->height - 1;
                            int val = image->data(ii, jj);
                            tmp[ptr] = val;
                            if (mask->data(ptr++))
                            {
                                v0 += (ix - xx) * val;
                                v1 += (iy - yy) * val;
                                v2 += val;
                            }
                        }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = LightingCorrectionMatrix * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = -mask->height / 2; j < mask->height / 2; j++)
                    for (i = -mask->width / 2; i < mask->width / 2; i++)
                    {
                        int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2) + 256;
                        if (val < 0) val = 0;
                        if (val >= 512) val = 511;
                        if (mask->data(ptr)) hist[val]++;
                        tmp[ptr++] = val;
                    }
                int* to = map;
                int* from = hist;
                int total = -160;
                for (i = 0; i < 512; i++) {
                    int old = total;
                    total += *(from++);
                    *(to++) = old + total;
                }
                from = tmp;
                for (i = 0; i < mask->size; i++) {
                    *from = map[*from];
                    from++;
                }
                float output[1];
                face12_net(tmp, output);
                if (output[0] > 0) {
                    face13_net(tmp, output);
                    if (output[0] > 0) {
                        totS += ss;
                        totX += xx * pow(1.2, ss);
                        totY += yy * pow(1.2, ss);
                        tot++;
                    }
                }
            }
    }
    delete[] tmp;
    if (tot == 0) return 0;
    int newS = (int)floor(totS / tot + 0.5);
    if (newS < 0) newS = 0;
    if (newS >= numScales) newS = numScales - 1;
    *newX = (int)floor(totX / tot / pow(1.2, newS) + 0.5) - halfX;
    *newY = (int)floor(totY / tot / pow(1.2, newS) + 0.5) - halfY;
    *news = newS;
    return 1;
}
#endif

int FindNewLocationRecordMax(int numScales, QImage* imagePyramid[],
    QImage* mask,
    int x, int y, int s,
    int dx, int dy, int ds, int step,
    int basenet,
    int* x1, int* y1, int* s1, double* o1,
    int* x2, int* y2, int* s2, double* o2)
{
    if (!LightingCorrectionMatrixInitialized)
        InitializeLightingCorrectionMatrix();
    int ptr;
    int hist[512];
    int map[512];
    Vec vec(3);
    int* tmp = new int[mask->size];
    int halfX = mask->width / 2;
    int halfY = mask->height / 2;
    x += halfX;
    y += halfY;
    FastForwardStruct* net = real_fast_list[basenet];
    FastForwardStruct* net2 = real_fast_list[basenet + 1];
    int first = 1;
    int verified = 0;
    for (int ss = s - ds; ss <= s + ds; ss++)
    {
        if (ss < 0 || ss >= numScales) continue;
        int cx = (int)floor(x * pow(1.2, s - ss) + 0.5);
        int cy = (int)floor(y * pow(1.2, s - ss) + 0.5);
        QImage* image = imagePyramid[ss];
        for (int yy = cy - dy; yy <= cy + dy; yy += step)
            for (int xx = cx - dx; xx <= cx + dx; xx += step)
            {
                ptr = 0;
                double v0 = 0, v1 = 0, v2 = 0;
                for (int iy = yy - halfY; iy < yy + halfY; iy++)
                    for (int ix = xx - halfX; ix < xx + halfX; ix++)
                    {
                        int ii = ix; if (ii < 0) ii = 0; if (ii >= image->width) ii = image->width;
                        int jj = iy; if (jj < 0) jj = 0; if (jj >= image->height) jj = image->height;
                        int val = image->data(ii, jj);
                        tmp[ptr] = val;
                        if (mask->data(ptr++))
                        {
                            v0 += (ix - xx) * val;
                            v1 += (iy - yy) * val;
                            v2 += val;
                        }
                    }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = LightingCorrectionMatrix * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = -mask->height / 2; j < mask->height / 2; j++)
                    for (int i = -mask->width / 2; i < mask->width / 2; i++)
                    {
                        int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2) + 256;
                        if (val < 0) val = 0;
                        if (val >= 512) val = 511;
                        if (mask->data(ptr)) hist[val]++;
                        tmp[ptr++] = val;
                    }
                int num = 0;
                for (int i = 0; i < 512; i++)
                {
                    map[i] = num;
                    num += hist[i];
                }
                int total = num;
                for (int i = 511; i >= 0; i--)
                {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / total;
                FastForwardUnit* unit = &(net->unitList[1]);
                FastForwardUnit* unit2 = &(net2->unitList[1]);
                int* p = tmp;
                for (int i = 0; i < mask->size; i++) {
                    unit->activation = map[*(p++)] * scaleFactor - 1.0;
                    (unit2++)->activation = (unit++)->activation;
                }
                FastForwardPass(net);
                FastForwardPass(net2);
                double out1 = net->unitList[net->firstOutput].activation;
                double out2 = net2->unitList[net2->firstOutput].activation;
                if ((out1 > 0.0 || out2 > 0.0) && verified < 1) verified = 1;
                if ((out1 > 0.0 && out2 > 0.0) && verified < 2) verified = 2;
                if (first || out1 > * o1) {
                    *o1 = out1;
                    *x1 = xx - halfX;
                    *y1 = yy - halfY;
                    *s1 = ss;
                }
                if (first || out2 > * o2) {
                    *o2 = out2;
                    *x2 = xx - halfX;
                    *y2 = yy - halfY;
                    *s2 = ss;
                }
                first = 0;
            }
    }
    delete[] tmp;
    return verified;
}

// USED
// Write detection information to a file.  ClientData argument indicates
// which file to save to.
void Informedia_OutputDetection(ClientData data, QImage* image,
    int x, int y, int width, int height,
    int level,
    double scale, double output, int orientation)
{
    FILE* outf = (FILE*)data;
    fprintf(outf, "%g %g %g %g %d %d %d %g %d\n",
        scale * x, scale * y,
        scale * (x + width), scale * (y + height),
        x, y, level, output, orientation);
}

// USED
// Save detection information in a list of detection.  The list is
// given in the ClientData argument
void Informedia_SaveDetections(ClientData data, QImage* image,
    int x, int y, int width, int height,
    int level,
    double scale, double output, int orientation)
{
    List<Detection>* detectionList = (List<Detection>*)data;
    detectionList->addLast(new Detection(x, y, level, output, orientation));
}
