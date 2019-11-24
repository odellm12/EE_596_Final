#include "search.h"

// Search an image for faces

#define LogFacesFound
#define MaskedHistogramEqualization
//#define OutputImage

#ifdef __GNUC__
#include <string.h>
#endif

#include <assert.h>
#ifndef __GNUC__
#include <string.h>
#endif
#include <stdio.h>
#include "tclHash.h"
#ifndef hpux
#include <math.h>
#endif
#include <stdlib.h>

#ifdef hpux
#include <math.h>
#endif
#include "search.h"
#include "system.h"
#include "mat.h"
#include "nn.h"
#include "fast2.h"
#include "img.h"
#include "myhebp.h"
#include "faces.h"

#include "vote.h"

#undef random
#define random rand

//#define DEBUG
//#define DebugX 0
//#define DebugY 0
//#define DebugS 1
//#define DebugX 46
//#define DebugY 23
//#define DebugS 3

#define OrientationMerging

Mat NNLayer1, NNLayer2;
char* NNDirectory;

#define FaceHeight 19
#define FaceWidth 19
#define FaceSize 283

static int FaceMask[FaceWidth * FaceHeight] = {
  0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
  0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
  0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
  0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
  0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
  0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
  0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
  0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };

extern long FileSize(char* fileName);

// USED
// Check that the scale is in range; if not make it be in range,
// and modify the x,y coords (which are associated with the scale)
// accordingly.
void LimitScale(int min, int max, int* s, int* x, int* y)
{
    int delta = 0;
    if (*s < min) delta = min - (*s);
    if (*s > max) delta = max - (*s);
    if (delta != 0) {
        *x = (int)floor(0.5 + *x * pow(1.2, -delta));
        *y = (int)floor(0.5 + *y * pow(1.2, -delta));
        *s += delta;
    }
}

#ifdef notdef
if (1)
{
    int* to = hist;
    for (int i = 0; i < 256; i++) *(to++) = 0;
    *to = tmp;
    for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
    {
        int val = 0;
        if (i >= 0 && j >= 0 & i < image->width && j < image->height)
            val = image->data(i, j);
        hist[val]++;
        *(to++) = val;
    }
}
else
{
    int* from = tmp;
    int* to = tmp;
    int ptr = 0;
    for (j = 0; j < 30; j++)
    {
        for (i = 0; i < 5; i++)
            hist[*(from++)]--;
        for (i = 0; i < 25; i++)
            *(to++) = *(from++);
        int yy = y + j - 10;
        for (i = x + 15; i < x + 20; i++)
        {
            int val = 0;
            if (i >= 0 && yy >= 0 & i < image->width && yy < image->height)
                val = image->data(i, yy);
            hist[val]++;
            *(to++) = val;
        }
    }
}
#endif

extern "C" {
    void ume_net(int* inputs, float* outputs);
}

// USED
// Search the given image using the 30x30 candidate detector.  The levelmask
// is used to indicate which portions of the image to not bother searching,
// because a face has already been found there.  When a face is found,
// the callback function is called with informatoin about the detection.
void SearchUmeEvenFaster(QImage * image, QImage * levelmask, int level,
    SearchCallback callback, ClientData data,
    double threshold)
{
    int x, y, i, j;

    if (levelmask != NULL) {
        // For each 10x10 pixel block where the center of a face could be,
        // see if more than 5 pixel locations still need to be scanned.  If
        // so, the block must be scanned.  This is indicated by a flag placed
        // at the upper-left corner of each block
        int xp = (image->width + 9) / 10;
        int yp = (image->height + 9) / 10;
        for (y = 0; y < yp; y++)
            for (x = 0; x < xp; x++)
            {
                int tot = 0, num = 0;
                for (j = y * 10; j < y * 10 + 10; j++)
                    for (i = x * 10; i < x * 10 + 10; i++)
                        if (i < levelmask->width && j < levelmask->height)
                        {
                            tot++;
                            if (!levelmask->data(i, j)) num++;
                        }
                if (num <= 5)
                    levelmask->data(x * 10, y * 10) = 255; else
                    levelmask->data(x * 10, y * 10) = 0;
            }
    }

    int tmp[900];              // Used to store the window
    int hist[256];             // Used to store histogram
    for (y = 0; y < image->height; y += 10)     // For each block
    {
        for (x = 0; x < image->width; x += 10)
        {
            if (levelmask != NULL && levelmask->data(x, y))
                continue;
            int i, j;

            int* to = hist;
            for (i = 0; i < 256; i++) *(to++) = 0;

            // Copy the window from the image into tmp.  The first loop is used
            // when the window is entirely inside the image, the second one is
            // used otherwise.  For pixels outside the image, the pixels at the
            // edge of the image are replicated.  The histogram is updated.
            to = tmp;
            if (x >= 10 && y >= 10 && x + 20 <= image->width && y + 20 <= image->height) {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++) {
                    int val = image->data(i, j);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            else {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++) {
                    int ii = i; if (ii < 0) ii = 0; if (ii >= image->width) ii = image->width - 1;
                    int jj = j; if (jj < 0) jj = 0; if (jj >= image->height) jj = image->height - 1;
                    int val = image->data(ii, jj);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }

            // Build a cummulative histogram
            int map[256];
            to = map;
            int* from = hist;
            int tot = 0;
            for (i = 0; i < 256; i++)
            {
                int old = tot;
                tot += *(from++);
                *(to++) = old + tot;
            }

            // Apply histogram equalization, write image into network input units
            double scaleFactor = 1.0 / tot;
            FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
            from = tmp;
            for (i = 0; i < 900; i++)
                (unit++)->activation = map[*(from++)] * scaleFactor - 1.0;

            // Apply the network
            double output = FastForwardPass(real_fast_list[0]);

            // If there is a detection, call the callback function to record
            // or otherwise deal with that detection
            if (output > threshold)
                callback(data, image, x - 5, y - 5, 20, 20, level, pow(1.2, level), output, 0);
        }
    }
}

#ifdef CompiledNets
void SearchUmeEvenFasterCompiled(QImage * image, QImage * levelmask,
    int level,
    SearchCallback callback, ClientData data)
{
    int x, y, i, j;
    int xp = (image->width + 9) / 10;
    int yp = (image->height + 9) / 10;
    for (y = 0; y < yp; y++)
        for (x = 0; x < xp; x++)
        {
            int tot = 0, num = 0;
            for (j = y * 10; j < y * 10 + 10; j++)
                for (i = x * 10; i < x * 10 + 10; i++)
                    if (i < levelmask->width && j < levelmask->height)
                    {
                        tot++;
                        if (!levelmask->data(i, j)) num++;
                    }
            if (num <= 5)
                levelmask->data(x * 10, y * 10) = 255; else
                levelmask->data(x * 10, y * 10) = 0;
        }

    int tmp[900];
    int hist[256];
    for (y = 0; y < image->height; y += 10)
    {
        for (x = 0; x < image->width; x += 10)
        {
            if (levelmask->data(x, y))
                continue;
            int i, j;

            int* to = hist;
            for (i = 0; i < 256; i++) *(to++) = 0;
            to = tmp;
            if (x >= 10 && y >= 10 && x + 20 <= image->width && y + 20 <= image->height) {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int val = image->data(i, j);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            else {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int ii = i; if (ii < 0) ii = 0; if (ii >= image->width) ii = image->width - 1;
                    int jj = j; if (jj < 0) jj = 0; if (jj >= image->height) jj = image->height - 1;
                    int val = image->data(ii, jj);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            int map[256];
            to = map;
            int* from = hist;
            int tot = -450;
            for (i = 0; i < 256; i++)
            {
                int old = tot;
                tot += *(from++);
                *(to++) = old + tot;
            }
            from = tmp;
            for (i = 0; i < 900; i++) {
                *from = map[*from];
                from++;
            }
            float output[1];
            ume_net(tmp, output);
            if (output[0] > 0)
                callback(data, image, x - 5, y - 5, 20, 20, level, 0, 0, 0);
        }
    }
}

#ifndef NoTcl
extern "C" {
    void ume_net(int* inputs, float* outputs);
}
#endif
#endif

#ifdef CompiledNets
int SearchUmeFasterStillCompiledCmd(ClientData data, Tcl_Interp * interp,
    int argc, char* argv[])
{
    if (argc != 3)
    {
        interp->result = "Usage: SearchUmeFasterStillCompiled <image> <levelmask>";
        return TCL_ERROR;
    }
    QImage* image = FindQImageWithName(argv[1]);
    QImage* levelmask = FindQImageWithName(argv[2]);

    Tcl_DString ds;
    Tcl_DStringInit(&ds);

    int x, y, i, j;
    int xp = (image->width + 9) / 10;
    int yp = (image->height + 9) / 10;
    for (y = 0; y < yp; y++)
        for (x = 0; x < xp; x++)
        {
            int tot = 0, num = 0;
            for (j = y * 10; j < y * 10 + 10; j++)
                for (i = x * 10; i < x * 10 + 10; i++)
                    if (i < levelmask->width && j < levelmask->height)
                    {
                        tot++;
                        if (!levelmask->data(i, j)) num++;
                    }
            if (num <= 5)
                levelmask->data(x * 10, y * 10) = 255; else
                levelmask->data(x * 10, y * 10) = 0;
        }

    int tmp[900];
    int hist[256];
    for (y = 0; y < image->height; y += 10)
    {
        for (x = 0; x < image->width; x += 10)
        {
            if (levelmask->data(x, y))
                continue;
            int i, j;

            int* to = hist;
            for (i = 0; i < 256; i++) *(to++) = 0;
            to = tmp;
            if (x >= 10 && y >= 10 && x + 20 <= image->width && y + 20 <= image->height) {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int val = image->data(i, j);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            else {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int ii = i; if (ii < 0) ii = 0; if (ii >= image->width) ii = image->width - 1;
                    int jj = j; if (jj < 0) jj = 0; if (jj >= image->height) jj = image->height - 1;
                    int val = image->data(ii, jj);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            int map[256];
            to = map;
            int* from = hist;
            int tot = -450;
            for (i = 0; i < 256; i++)
            {
                int old = tot;
                tot += *(from++);
                *(to++) = old + tot;
            }
            from = tmp;
            for (i = 0; i < 900; i++) {
                *from = map[*from];
                from++;
            }
            float output[1];
            ume_net(tmp, output);
            if (output[0] > 0)
            {
                char tem[256];
                sprintf(tem, "{ %d %d } ", x + 5, y + 5);
                Tcl_DStringAppend(&ds, tem, -1);
            }
        }
    }
    Tcl_DStringResult(interp, &ds);
    return TCL_OK;
}
#endif

void SearchUmeEvenFasterRegion(QImage * image, QImage * levelmask,
    int minX, int maxX, int minY, int maxY,
    int level,
    SearchCallback callback, ClientData data)
{
    int x, y, i, j;
    int tmp[900];
    int hist[256];
    minX = (int)floor(minX * pow(1.2, -level) + 0.5);
    maxX = (int)floor(maxX * pow(1.2, -level) + 0.5);
    minY = (int)floor(minY * pow(1.2, -level) + 0.5);
    maxY = (int)floor(maxY * pow(1.2, -level) + 0.5);
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (minX >= image->width) minX = image->width - 1;
    if (minY >= image->height) minY = image->height - 1;
    if (maxX < 0) maxX = 0;
    if (maxY < 0) maxY = 0;
    if (maxX > image->width) maxX = image->width;
    if (maxY > image->height) maxY = image->height;
    minX = (minX / 10) * 10;
    minY = (minY / 10) + 10;

    for (y = minY; y < maxY; y += 10)
    {
        for (x = minX; x < maxX; x += 10)
        {
            if (levelmask != NULL) {
                int tot = 0, num = 0;
                for (j = y; j < y + 10; j++)
                    for (i = x; i < x + 10; i++)
                        if (i < levelmask->width && j < levelmask->height)
                        {
                            tot++;
                            if (levelmask->data(i, j)) num++;
                        }
                if (num > tot - 5) continue;
            }

            int i, j;
            int* to = hist;
            for (i = 0; i < 256; i++) *(to++) = 0;
            to = tmp;
            if (x >= 10 && y >= 10 && x + 20 <= image->width && y + 20 <= image->height) {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int val = image->data(i, j);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            else {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int ii = i; if (ii < 0) ii = 0; if (ii >= image->width) ii = image->width - 1;
                    int jj = j; if (jj < 0) jj = 0; if (jj >= image->height) jj = image->height - 1;
                    int val = image->data(ii, jj);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            int map[256];
            to = map;
            int* from = hist;
            int num = 0;
            for (i = 0; i < 256; i++)
            {
                (*(to++)) = num;
                num += *(from++);
            }
            int tot = num;
            for (i = 0; i < 256; i++)
            {
                (*(--to)) += num;
                num -= *(--from);
            }
            from = tmp;
            double scaleFactor = 1.0 / tot;
            FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
            for (i = 0; i < 900; i++)
                (unit++)->activation = map[*(from++)] * scaleFactor - 1.0;

            if (FastForwardPass(real_fast_list[0]) > 0)
                callback(data, image, x - 5, y - 5, 20, 20, level, 0, 0, 0);
        }
    }
}

void SearchUmeEvenFasterRegionPrecise(QImage * image, QImage * levelmask,
    int minX, int maxX, int minY, int maxY,
    int level, int preciseNet,
    SearchCallback callback,
    ClientData data)
{
    int x, y, i, j;
    int tmp[900];
    int hist[256];
    minX = (int)floor(minX * pow(1.2, -level) + 0.5);
    maxX = (int)floor(maxX * pow(1.2, -level) + 0.5);
    minY = (int)floor(minY * pow(1.2, -level) + 0.5);
    maxY = (int)floor(maxY * pow(1.2, -level) + 0.5);
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (minX >= image->width) minX = image->width - 1;
    if (minY >= image->height) minY = image->height - 1;
    if (maxX < 0) maxX = 0;
    if (maxY < 0) maxY = 0;
    if (maxX > image->width) maxX = image->width;
    if (maxY > image->height) maxY = image->height;
    minX = (minX / 10) * 10;
    minY = (minY / 10) + 10;

    for (y = minY; y < maxY; y += 10)
    {
        for (x = minX; x < maxX; x += 10)
        {
            if (levelmask != NULL) {
                int tot = 0, num = 0;
                for (j = y; j < y + 10; j++)
                    for (i = x; i < x + 10; i++)
                        if (i < levelmask->width && j < levelmask->height)
                        {
                            tot++;
                            if (levelmask->data(i, j)) num++;
                        }
                if (num > tot - 5) continue;
            }

            int i, j;
            int* to = hist;
            for (i = 0; i < 256; i++) *(to++) = 0;
            to = tmp;
            if (x >= 10 && y >= 10 && x + 20 <= image->width && y + 20 <= image->height) {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int val = image->data(i, j);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            else {
                for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
                {
                    int ii = i; if (ii < 0) ii = 0; if (ii >= image->width) ii = image->width - 1;
                    int jj = j; if (jj < 0) jj = 0; if (jj >= image->height) jj = image->height - 1;
                    int val = image->data(ii, jj);
                    hist[val]++;
                    (*(to++)) = val;
                }
            }
            float map[256];
            float* mapto = map;
            int* from = hist;
            int tot = 0;
            for (i = 0; i < 256; i++)
            {
                int old = tot;
                tot += *(from++);
                (*(mapto++)) = (old + tot) / 900.0 - 1.0;
            }
            from = tmp;
            FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
            for (i = 0; i < 900; i++)
                (unit++)->activation = map[*(from++)];

            if (FastForwardPass(real_fast_list[0]) > 0)
            {
                FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                FastForwardUnit* unit2 = &(real_fast_list[preciseNet]->unitList[1]);
                for (i = 0; i < 900; i++)
                    (unit2++)->activation = (unit++)->activation;
                FastForwardPass(real_fast_list[preciseNet]);
                double xx = 0, tot = 0;
                unit = &(real_fast_list[preciseNet]->unitList
                    [real_fast_list[preciseNet]->firstOutput]);
                for (j = 0; j < 20; j++)
                {
                    double act = 1.0 + (unit++)->activation;
                    tot += act;
                    xx += act * (j - 10);
                }
                xx /= tot;
                double yy = 0;
                tot = 0;
                for (j = 0; j < 20; j++)
                {
                    double act = 1.0 + (unit++)->activation;
                    tot += act;
                    yy += act * (j - 10);
                }
                yy /= tot;
                callback(data, image, (int)floor(0.5 + x - xx - 5), (int)floor(0.5 + y - yy - 5),
                    20, 20, level, 0, 0, 0);
            }
        }
    }
}

void SearchUmeEvenFasterAll(QImage * image, int level,
    SearchCallback callback, ClientData data)
{
    int x, y, i, j;
    int tmp[900];
    int hist[256];
    for (y = 0; y < image->height; y += 10)
    {
        for (x = 0; x < image->width; x += 10)
        {
            int i, j;
            int* to = hist;
            for (i = 0; i < 256; i++) *(to++) = 0;
            to = tmp;
            for (j = y - 10; j < y + 20; j++) for (i = x - 10; i < x + 20; i++)
            {
                int val = 0;
                if (i >= 0 && j >= 0 && i < image->width && j < image->height)
                    val = image->data(i, j);
                hist[val]++;
                (*(to++)) = val;
            }
            int map[256];
            to = map;
            int* from = hist;
            int num = 0;
            for (i = 0; i < 256; i++)
            {
                (*(to++)) = num;
                num += *(from++);
            }
            int tot = num;
            for (i = 0; i < 256; i++)
            {
                (*(--to)) += num;
                num -= *(--from);
            }
            from = tmp;
            double scaleFactor = 1.0 / tot;
            FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
            for (i = 0; i < 900; i++)
                (unit++)->activation = map[*(from++)] * scaleFactor - 1.0;

            if (FastForwardPass(real_fast_list[0]) > 0)
                callback(data, image, x - 5, y - 5, 20, 20, level, 0, 0, 0);
        }
    }
}

void FindCentroidUme(QImage * image, int x, int y,
    double* totalX, double* totalY, double* total)
{
    if (x >= 0 && y >= 0 && x < image->width && y < image->height)
    {
        double val = image->data(x, y);
        if (val > 0)
        {
            (*totalX) += x * val;
            (*totalY) += y * val;
            (*total) += val;
            image->data(x, y) = 0;
            for (int i = -1; i <= 1; i++)
                for (int j = -1; j <= 1; j++)
                    if (i != 0 || j != 0)
                        FindCentroidUme(image, x + i, y + j, totalX, totalY, total);
        }
    }
}

void FindCentroidUmeFast(QImage * image, int x, int y,
    double* totalX, double* totalY, double* total,
    int threshold)
{
    if (x >= 0 && y >= 0 && x < image->width && y < image->height)
    {
        double val = image->data(x, y);
        if (val >= threshold)
        {
            val = val - threshold + 1;
            (*totalX) += x * val;
            (*totalY) += y * val;
            (*total) += val;
            image->data(x, y) = 0;
            for (int i = -1; i <= 1; i++)
                for (int j = -1; j <= 1; j++)
                    if (i != 0 || j != 0)
                        FindCentroidUmeFast(image, x + i, y + j, totalX, totalY, total, threshold);
        }
    }
}

void FindCentroidUmeFaster(QImage * image, int x, int y,
    double* totalX, double* totalY, double* total,
    int threshold)
{
    if (x >= 0 && y >= 0 && x < image->width && y < image->height)
    {
        double val = image->data(x, y);
        if (val >= threshold && val != 255)
        {
            val = val - threshold + 1;
            (*totalX) += x * val;
            (*totalY) += y * val;
            (*total) += val;
            image->data(x, y) = 0;
            for (int i = -1; i <= 1; i++)
                for (int j = -1; j <= 1; j++)
                    if (i != 0 || j != 0)
                        FindCentroidUmeFaster(image, x + i, y + j, totalX, totalY, total, threshold);
        }
    }
}

static int CompareIntPair(const void* p1, const void* p2)
{
    if (((int*)p1)[1] < ((int*)p2)[1]) return -1;
    if (((int*)p1)[1] > ((int*)p2)[1]) return 1;
    return 0;
}

int SearchFalsePositives(QImage * image,
    char* clusterFile, int numClusters,
    char* exampleFile, int maxExamples)
{
    double* cluster = new double[numClusters * FaceSize];
    int* positive = new int[numClusters];
    FILE* inf = fopen(clusterFile, "r");
    for (int i = 0; i < numClusters; i++)
    {
        fread((char*) & (positive[i]), sizeof(int), 1, inf);
        int tmp;
        fread((char*)(&tmp), sizeof(int), 1, inf);
        fread((char*) & (cluster[i * FaceSize]), sizeof(double), FaceSize, inf);
    }
    fclose(inf);
    for (int i = 0; i < numClusters * FaceSize; i++) cluster[i] = ntohd(cluster[i]);

    int ptr = 0;
    Mat mat = Zero(3, 3);
    for (int j = 0; j < FaceHeight; j++) for (int i = 0; i < FaceWidth; i++)
        if (FaceMask[ptr++])
        {
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
    mat = LUInverse(mat);

    int numExamples = 0;

    int* rowIndex = new int[(image->height - FaceHeight) * 2];
    for (int i = 0; i < image->height - FaceHeight; i++)
    {
        rowIndex[i * 2] = i;
        rowIndex[i * 2 + 1] = random();
    }
    qsort((char*)rowIndex, image->height - FaceHeight, sizeof(int) * 2, CompareIntPair);

    int* colIndex = new int[(image->width - FaceWidth) * 2];
    for (int i = 0; i < image->width - FaceWidth; i++)
    {
        colIndex[i * 2] = i;
        colIndex[i * 2 + 1] = random();
    }
    qsort((char*)colIndex, image->width - FaceWidth, sizeof(int) * 2, CompareIntPair);

    for (int xInd = 0; xInd < image->width - FaceWidth; xInd++)
    {
        int x = colIndex[xInd << 1];
        for (int yInd = 0; yInd < image->height - FaceHeight; yInd++)
        {
            int y = rowIndex[yInd << 1];
            Vec vec(3);
            int ptr = 0;
            int tmp[FaceWidth * FaceHeight];
            for (int j = 0; j < FaceHeight; j++) for (int i = 0; i < FaceWidth; i++)
            {
                Byte data = image->data(i + x, j + y);
                tmp[ptr] = data;
                if (FaceMask[ptr])
                {
                    vec(0) += i * data;
                    vec(1) += j * data;
                    vec(2) += data;
                }
                ptr++;
            }
            vec = mat * vec;
            int hist[512];
            for (int i = 0; i < 512; i++) hist[i] = 0;
            ptr = 0;
            for (int j = 0; j < FaceHeight; j++) for (int i = 0; i < FaceWidth; i++)
            {
                int val = tmp[ptr] - (int)(floor(0.5 + i * vec(0) + j * vec(1) + vec(2))) + 256;
                if (val < 0) val = 0;
                if (val >= 512) val = 511;
                hist[val]++;
                tmp[ptr++] = val;
            }
            int map[512];
            int num = 0;
            for (int i = 0; i < 512; i++)
            {
                map[i] = num;
                num += hist[i];
            }
            int tot = num;
            for (int i = 511; i >= 0; i--)
            {
                map[i] = (int)floor(0.5 + (127 * (num + map[i]) / tot - 127.0));
                num -= hist[i];
            }
            double vect[FaceSize];
            ptr = 0;
            for (int i = 0; i < FaceWidth * FaceHeight; i++)
                if (FaceMask[i])
                {
                    tmp[i] = map[tmp[i]];
                    vect[ptr++] = tmp[i] / 127.0;
                }
            int bestPos = -1;
            double bestDist = 0;
            ptr = 0;
            for (int i = 0; i < numClusters; i++)
            {
                double dist = 0;
                for (int j = 0; j < FaceSize; j++)
                {
                    double tmp = vect[j] - cluster[ptr++];
                    dist += tmp * tmp;
                }
                if (bestPos == -1 || dist < bestDist)
                {
                    bestPos = positive[i];
                    bestDist = dist;
                }
            }
            if (bestPos)
            {
                FILE* outf = fopen(exampleFile, "a");
                signed char data[FaceSize];
                ptr = 0;
                for (int i = 0; i < FaceWidth * FaceHeight; i++)
                    if (FaceMask[i]) data[ptr++] = tmp[i];
                fwrite((char*)data, sizeof(signed char), FaceSize, outf);
                fclose(outf);
                numExamples++;
                if (numExamples == maxExamples)
                {
                    fprintf(stderr, "Found %d false positives.\n", numExamples);
                    return numExamples;
                }
            }
        }
        fprintf(stderr, ".");
        fflush(stderr);
    }
    fprintf(stderr, "Found %d false positives.\n", numExamples);
    delete[] cluster;
    delete[] positive;
    delete[] rowIndex;
    delete[] colIndex;
    return numExamples;
}

void SearchFalsePositivesDist(QImage * image,
    char* clusterFile, int numClusters,
    char* exampleFile, int maxExamples)
{
    double* cluster = new double[numClusters * FaceSize];
    int* positive = new int[numClusters];
    FILE* inf = fopen(clusterFile, "r");
    for (int i = 0; i < numClusters; i++)
    {
        fread((char*) & (positive[i]), sizeof(int), 1, inf);
        int tmp;
        fread((char*)(&tmp), sizeof(int), 1, inf);
        fread((char*) & (cluster[i * FaceSize]), sizeof(double), FaceSize, inf);
    }
    fclose(inf);
    for (int i = 0; i < numClusters * FaceSize; i++) cluster[i] = ntohd(cluster[i]);

    int ptr = 0;
    Mat mat = Zero(3, 3);
    for (int j = 0; j < FaceHeight; j++) for (int i = 0; i < FaceWidth; i++)
        if (FaceMask[ptr++])
        {
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
    mat = LUInverse(mat);

    int numExamples = 0;

    int* rowIndex = new int[(image->height - FaceHeight) * 2];
    for (int i = 0; i < image->height - FaceHeight; i++)
    {
        rowIndex[i * 2] = i;
        rowIndex[i * 2 + 1] = random();
    }
    qsort((char*)rowIndex, image->height - FaceHeight, sizeof(int) * 2, CompareIntPair);

    int* colIndex = new int[(image->width - FaceWidth) * 2];
    for (int i = 0; i < image->width - FaceWidth; i++)
    {
        colIndex[i * 2] = i;
        colIndex[i * 2 + 1] = random();
    }
    qsort((char*)colIndex, image->width - FaceWidth, sizeof(int) * 2, CompareIntPair);

    Vec dists(numClusters + 1);
    dists(numClusters) = 0.5;
    Vec ans;

    for (int xInd = 0; xInd < image->width - FaceWidth; xInd++)
    {
        int x = colIndex[xInd << 1];
        for (int yInd = 0; yInd < image->height - FaceHeight; yInd++)
        {
            int y = rowIndex[yInd << 1];
            Vec vec(3);
            int ptr = 0;
            int tmp[FaceWidth * FaceHeight];
            for (int j = 0; j < FaceHeight; j++) for (int i = 0; i < FaceWidth; i++)
            {
                Byte data = image->data(i + x, j + y);
                tmp[ptr] = data;
                if (FaceMask[ptr])
                {
                    vec(0) += i * data;
                    vec(1) += j * data;
                    vec(2) += data;
                }
                ptr++;
            }
            vec = mat * vec;
            int hist[512];
            for (int i = 0; i < 512; i++) hist[i] = 0;
            ptr = 0;
            for (int j = 0; j < FaceHeight; j++) for (int i = 0; i < FaceWidth; i++)
            {
                int val = tmp[ptr] - (int)(floor(0.5 + i * vec(0) + j * vec(1) + vec(2))) + 256;
                if (val < 0) val = 0;
                if (val >= 512) val = 511;
                hist[val]++;
                tmp[ptr++] = val;
            }
            int map[512];
            int num = 0;
            for (int i = 0; i < 512; i++)
            {
                map[i] = num;
                num += hist[i];
            }
            int tot = num;
            for (int i = 511; i >= 0; i--)
            {
                map[i] = (int)floor(0.5 + (127 * (num + map[i]) / tot - 127.0));
                num -= hist[i];
            }
            double vect[FaceSize];
            ptr = 0;
            for (int i = 0; i < FaceWidth * FaceHeight; i++)
                if (FaceMask[i])
                {
                    tmp[i] = map[tmp[i]];
                    vect[ptr++] = tmp[i] / 127.0;
                }

            ptr = 0;
            for (int i = 0; i < numClusters; i++)
            {
                double d = 0;
                for (int j = 0; j < FaceSize; j++)
                {
                    double tmp = vect[j] - cluster[ptr++];
                    d += tmp * tmp;
                }
                dists(i) = sqrt(d);
            }

            ans = Sigmoid(NNLayer2 * Sigmoid(NNLayer1 * dists));
            if (ans(0) > 0.5)
            {
                FILE* outf = fopen(exampleFile, "a");
                SaveVec(outf, dists);
                fclose(outf);
                numExamples++;
                if (numExamples == maxExamples)
                {
                    fprintf(stderr, "Found %d false positives.\n", numExamples);
                    return;
                }
            }
        }
        fprintf(stderr, ".");
        fflush(stderr);
    }
    fprintf(stderr, "Found %d false positives.\n", numExamples);
    delete[] cluster;
    delete[] positive;
    delete[] rowIndex;
    delete[] colIndex;
}

#ifdef notdef

int EdgeDetectFaceCmd(ClientData data, Tcl_Interp * interp,
    int argc, char* argv[])
{
    if (argc != 2)
    {
        interp->result = "Usage: EdgeDetectFace <image>";
        return TCL_ERROR;
    }
    QImage* image = FindQImageWithName(argv[1]);
    Byte* newdata = (Byte*)malloc(image->size);
    for (int i = 0; i < image->width; i++)
        for (int j = 0; j < image->height; j++)
        {
            int dx = 0;
            if (i == 0) dx = image->data(1, j) - image->data(0, j); else
                if (i == image->width - 1) dx = image->data(i, j) - image->data(i - 1, j); else
                    dx = (image->data(i + 1, j) - image->data(i - 1, j)) / 2;
            int dy = 0;
            if (j == 0) dy = image->data(i, 1) - image->data(i, 0); else
                if (j == image->height - 1) dy = image->data(i, j) - image->data(i, j - 1); else
                    dy = (image->data(i, j + 1) - image->data(i, j - 1)) / 2;
            int val = (int)floor(0.5 + sqrt(dx * dx + dy * dy));
            if (val > 255) val = 255;
            newdata[i + j * image->width] = val;
        }
    image->NewBuffer(newdata, image->width, image->height);
    return TCL_OK;
}

#endif

struct Alignment;

void CorrectFaceDetection(Alignment * alignment, double* features,
    int* x, int* y, int* s, int* angle);

void CorrectFaceDetectionValid(Alignment * alignment, double* features,
    int* valid,
    int* x, int* y, int* s, int* angle);

// USED
// Create a hash table which maps file names to lists of desired detection
// locations, used for automatically checking detector outputs
Tcl_HashTable* ReadCorrectFaceDetectionsAlignment(Alignment * alignment,
    char* answerFile)
{
    Tcl_HashTable* table = new Tcl_HashTable;
    Tcl_InitHashTable(table, TCL_STRING_KEYS);

    // Read in the answer file
    FILE* inf = fopen(answerFile, "r");
    int done = 0;
    do {
        char line[1024];
        if (fgets(line, 1024, inf) == NULL) done = 1; else {
            double features[12];
            int valid[6];
            for (int i = 0; i < 6; i++) valid[i] = 0;
            char* filename = line;
            char* info = strchr(line, '{');
            *(strchr(filename, ' ')) = 0;
            //      fprintf(stderr,"filename:%s\n",filename);
            while (info != NULL) {
                //        fprintf(stderr,"info:%s\n",info);
                info++;
                int num = -1;
                if (strncmp(info, "leye ", strlen("leye ")) == 0) num = 0;
                if (strncmp(info, "reye ", strlen("reye ")) == 0) num = 1;
                if (strncmp(info, "nose ", strlen("nose ")) == 0) num = 2;
                if (strncmp(info, "lmouth ", strlen("lmouth ")) == 0) num = 3;
                if (strncmp(info, "cmouth ", strlen("cmouth ")) == 0) num = 4;
                if (strncmp(info, "rmouth ", strlen("rmouth ")) == 0) num = 5;
                if (num != -1) {
                    double x, y;
                    //          fprintf(stderr,"coord:%s\n",strchr(info,' ')+1);
                    sscanf(strchr(info, ' ') + 1, "%lf %lf}", &x, &y);
                    valid[num] = 1;
                    features[num * 2] = x;
                    features[num * 2 + 1] = y;
                }
                info = strchr(info, '{');
            }
            // Look up filename in hash table
            Tcl_HashEntry* entry = Tcl_FindHashEntry(table, filename);
            if (entry == NULL) {
                // If it is not there, add it
                int newEntry;
                entry = Tcl_CreateHashEntry(table, filename, &newEntry);
                Tcl_SetHashValue(entry, new List<Detection>);
            }
            // Figure out where the face should be detected, and add it to the list
            List<Detection>* detections = (List<Detection>*)(Tcl_GetHashValue(entry));
            int x, y, s, angle;
            CorrectFaceDetectionValid(alignment, features, valid, &x, &y, &s, &angle);
            detections->addLast(new Detection(x, y, s, 1, angle));
        }
    } while (!done);
    fclose(inf);
    return table;
}

List<Detection>* CorrectDetectionList(Tcl_HashTable * table,
    char* imagename)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(table, imagename);
    if (entry == NULL) {
        // If it is not there, add it
        int newEntry;
        entry = Tcl_CreateHashEntry(table, imagename, &newEntry);
        Tcl_SetHashValue(entry, new List<Detection>);
    }
    List<Detection>* detections = (List<Detection>*)(Tcl_GetHashValue(entry));
    return detections;
}

Tcl_HashTable* ReadCorrectFaceDetectionsFixed(char* answerFile)
{
    Tcl_HashTable* table = new Tcl_HashTable;
    Tcl_InitHashTable(table, TCL_STRING_KEYS);
    FILE* inf = fopen(answerFile, "r");
    int done = 0;
    do {
        char fileName[1024];
        double xe1, ye1, xe2, ye2, xn, yn, xl, yl, xc, yc, xr, yr;
        int num = fscanf(inf, "%s {leye %lf %lf} {reye %lf %lf} {nose %lf %lf} "
            "{lmouth %lf %lf} {cmouth %lf %lf} {rmouth %lf %lf}",
            fileName,
            &xe1, &ye1, &xe2, &ye2, &xn, &yn,
            &xl, &yl, &xc, &yc, &xr, &yr);
        if (num <= 0) done = 1; else {
            Tcl_HashEntry* entry = Tcl_FindHashEntry(table, fileName);
            if (entry == NULL) {
                int newEntry;
                entry = Tcl_CreateHashEntry(table, fileName, &newEntry);
                Tcl_SetHashValue(entry, new List<Detection>);
            }
            List<Detection>* detections = (List<Detection>*)(Tcl_GetHashValue(entry));
            double xe = (xe1 + xe2) / 2.0;
            double ye = (ye1 + ye2) / 2.0;
            double scale = 12.0 / (sqrt((xe - xc) * (xe - xc) + (ye - yc) * (ye - yc)));
            double xcenter = (xe + xc) / 2.0;
            double ycenter = (ye + yc) / 2.0 - scale / 12.0;
            int s = (int)floor(0.5 - log(scale) / log(1.2));
            if (scale < 0) scale = 0;
            int xx = (int)floor(0.5 + xcenter * exp(log(1.2) * -s));
            int yy = (int)floor(0.5 + ycenter * exp(log(1.2) * -s));
            detections->addLast(new Detection(xx, yy, s, 1, 0));
            //      if (strcmp(fileName,"/IUS/har1/har/usr0/har/faces/test/original1.gif")==0) {
            //        fprintf(stderr,"%g %g %g %g %g %g %g %g %g %g %g %g\n",
            //                xe1,ye1,xe2,ye2,xn,yn,xl,yl,xc,yc,xr,yr);
            //        fprintf(stderr,"%g %g %g %g %g\n",xe,ye,scale,xcenter,ycenter);
            //        fprintf(stderr,"%d %d %d\n\n",s,xx,yy);
            //      }
        }
    } while (!done);
    fclose(inf);
    return table;
}

void FreeCorrectFaceDetections(Tcl_HashTable * table)
{
    Tcl_HashSearch search;
    for (Tcl_HashEntry* entry = Tcl_FirstHashEntry(table, &search);
        entry != NULL;
        entry = Tcl_NextHashEntry(&search)) {
        List<Detection>* detections = (List<Detection>*)Tcl_GetHashValue(entry);
        delete detections;
    }
    Tcl_DeleteHashTable(table);
}

CheckResultsStruct* CheckResultsDirect(char* canvas,
    QImage * image, char* imageFile,
    List<Detection> * detections,
    Tcl_HashTable * answers, QImage * mask)
{
    int temWidth = image->width, temHeight = image->height;
    int numScales = 0;
    while (temWidth >= mask->width && temHeight >= mask->height)
    {
        char name[256];
        sprintf(name, "scale%d", numScales);
        QImage* image = new QImage(name);
        image->ZeroBuffer(temWidth, temHeight);
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }
    QImage** imageScale = new QImage * [numScales];
    for (int i = 0; i < numScales; i++)
    {
        char name[256];
        sprintf(name, "scale%d", i);
        imageScale[i] = FindQImageWithName(name);
    }
    int numFaces = 0;
    Tcl_HashEntry* entry = Tcl_FindHashEntry(answers, imageFile);
    if (entry != NULL) {
        List<Detection>* detections = (List<Detection>*)Tcl_GetHashValue(entry);
        for (Detection* d = detections->first; d != NULL; d = d->next) {
#ifdef OrientationMerging
            assert(d->orient >= 0 && d->orient < 360);
            if (d->orient > 30 && d->orient < 330) continue;
#endif
            int s = d->s;
            int xx = d->x;
            int yy = d->y;
            LimitScale(0, numScales - 1, &s, &xx, &yy);
            double xcenter = xx * pow(1.2, s);
            double ycenter = yy * pow(1.2, s);
            QImage* image = imageScale[s];
            if (xx < 0) xx = 0;
            if (yy < 0) yy = 0;
            if (xx >= image->width) xx = image->width - 1;
            if (yy >= image->height) yy = image->height - 1;
            image->data(xx, yy) = 1;
            numFaces++;
        }
    }

    int falsePos = 0, falseNeg = 0;
    for (Detection* d = detections->first; d != NULL; d = d->next) {
        int x = d->x, y = d->y, s = d->s;
        double confidence = d->output;
        if (confidence > 0.0) {
            int xc = x + (mask->width / 2);
            int yc = y + (mask->height / 2);
            int found = 0;
            for (int si = -4; si <= 4; si++)
            {
                if (si + s < 0 || si + s >= numScales) continue;
                int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                QImage* image = imageScale[si + s];
                //	int size=(int)floor(0.5+3*pow(1.2,si));
                int size = 4;
                for (int xx = xci - size; xx <= xci + size; xx++)
                    for (int yy = yci - size; yy <= yci + size; yy++)
                        if (xx >= 0 && xx < image->width && yy >= 0 && yy < image->height)
                            if (image->data(xx, yy)) {
                                found = 1;
                                image->data(xx, yy) = 2;
                            }
            }
        }
    }

    int examples = 0;
    for (int s = 0; s < numScales; s++)
    {
        QImage* image = imageScale[s];
        examples += (image->width - mask->width + 1) * (image->height - mask->height + 1);
        for (int x = 0; x < image->width; x++)
            for (int y = 0; y < image->height; y++)
                if (image->data(x, y) == 1)
                    falseNeg++;
    }

    for (int i = 0; i < numScales; i++)
        delete imageScale[i];
    delete[] imageScale;

    CheckResultsStruct* results = new CheckResultsStruct;
    results->falseNeg = falseNeg;
    results->numFaces = numFaces;
    results->falsePos = falsePos;
    results->examples = examples;

    return results;
}

void CountVotes2Direct(int width, int height,
    List<Detection> * detections, char* outFile,
    Tcl_HashTable * answers, char* imageFile)
{
    int temWidth = width, temHeight = height;
    int numScales = 0;
    while (temWidth >= 20 && temHeight >= 20)
    {
        char name[256];
        sprintf(name, "scale%d", numScales);
        QImage* image = new QImage(name);
        image->ZeroBuffer(temWidth, temHeight);
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }
    QImage** imageScale = new QImage * [numScales];
    for (int i = 0; i < numScales; i++)
    {
        char name[256];
        sprintf(name, "scale%d", i);
        imageScale[i] = FindQImageWithName(name);
    }
    int numFaces = 0;
    int done = 0;

    Tcl_HashEntry* entry = Tcl_FindHashEntry(answers, imageFile);
    if (entry != NULL) {
        List<Detection>* detections = (List<Detection>*)Tcl_GetHashValue(entry);
        for (Detection* d = detections->first; d != NULL; d = d->next) {
            int s = d->s;
            int xx = d->x;
            int yy = d->y;
            LimitScale(0, numScales - 1, &s, &xx, &yy);
            QImage* image = imageScale[s];
            if (xx < 0) xx = 0;
            if (yy < 0) yy = 0;
            if (xx >= image->width) xx = image->width - 1;
            if (yy >= image->height) yy = image->height - 1;
            image->data(xx, yy) = 0x80;
            numFaces++;
        }
    }
    fprintf(stderr, "Num correct faces=%d\n", numFaces);
    for (int num = 0; num < 3; num++)
    {
        int numFaces = 0;
        for (Detection* d = detections[num].first; d != NULL; d = d->next) {
            int x = d->x + 10;
            int y = d->y + 10;
            int s = d->s;
            LimitScale(0, numScales - 1, &s, &x, &y);
            QImage* image = imageScale[s];
            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (x >= image->width) x = image->width - 1;
            if (y >= image->height) y = image->height - 1;
            image->data(x, y) |= (1 << num);
            numFaces++;
        }
        fprintf(stderr, "numFaces=%d\n", numFaces);
    }

    FILE* outf = fopen(outFile, "a");
    for (int s = 0; s < numScales; s++)
    {
        QImage* image = imageScale[s];
        for (int x = 0; x < image->width; x++)
            for (int y = 0; y < image->height; y++)
                if (image->data(x, y) & 0x7F)
                {
                    int correct = 0;
                    for (int si = -4; si <= 4; si++)
                    {
                        if (si + s < 0 || si + s >= numScales) continue;
                        int xci = (int)floor(0.5 + x * exp(log(1.2) * -si));
                        int yci = (int)floor(0.5 + y * exp(log(1.2) * -si));
                        QImage* image = imageScale[si + s];
                        //            int sspread=(int)floor(0.5+6*pow(1.2,-si));
                        int sspread = 4;
                        for (int xx = xci - sspread; xx <= xci + sspread; xx++)
                            for (int yy = yci - sspread; yy <= yci + sspread; yy++)
                                if (xx >= 0 && xx < image->width && yy >= 0 && yy < image->height)
                                    if (image->data(xx, yy) & 0x80)
                                        correct = 1;
                    }
                    fprintf(outf, "%d", correct);
                    for (int net = 0; net < 3; net++)
                    {
                        for (int si = -1; si <= 1; si++)
                        {
                            int xci = (int)floor(0.5 + x * exp(log(1.2) * -si));
                            int yci = (int)floor(0.5 + y * exp(log(1.2) * -si));
                            for (int xx = xci - 1; xx <= xci + 1; xx++)
                                for (int yy = yci - 1; yy <= yci + 1; yy++)
                                {
                                    int val = 0;
                                    if (si + s >= 0 && si + s < numScales)
                                    {
                                        QImage* image = imageScale[si + s];
                                        if (xx >= 0 && yy >= 0 && xx < image->width && yy < image->height)
                                            val = (image->data(xx, yy) & (1 << net)) ? 1 : 0;
                                    }
                                    fprintf(outf, " %d", val);
                                }
                        }
                    }
                    fprintf(outf, "\n");
                }
    }
    fclose(outf);
    for (int i = 0; i < numScales; i++)
        delete imageScale[i];
    delete[] imageScale;
}

void CheckSearchInstrumentedDirect(int numScales, char* imageFile,
    Tcl_HashTable * answers, QImage * mask,
    char* tradeName)
{
    FILE* outf = fopen(tradeName, "w");

    QImage** outputScale = new QImage * [numScales];
    QImage** inputScale = new QImage * [numScales];
    for (int i = 0; i < numScales; i++)
    {
        char name[256];
        sprintf(name, "output%d", i);
        outputScale[i] = FindQImageWithName(name);
        sprintf(name, "input%d", i);
        inputScale[i] = new QImage(name);
        inputScale[i]->ZeroBuffer(outputScale[i]->width,
            outputScale[i]->height);
    }
    int numFaces = 0;

    Tcl_HashEntry* entry = Tcl_FindHashEntry(answers, imageFile);
    if (entry != NULL) {
        List<Detection>* detections = (List<Detection>*)Tcl_GetHashValue(entry);
        for (Detection* d = detections->first; d != NULL; d = d->next) {
            int s = d->s;
            int xx = d->x;
            int yy = d->y;
            LimitScale(0, numScales - 1, &s, &xx, &yy);
            QImage* image = inputScale[s];
            if (xx < 0) xx = 0;
            if (yy < 0) yy = 0;
            if (xx >= image->width) xx = image->width - 1;
            if (yy >= image->height) yy = image->height - 1;
            image->data(xx, yy) = 1;
            numFaces++;
        }
    }
    fprintf(stderr, "Num faces (correct) = %d\n", numFaces);

    int falsePos = 0, facesFound = 0;
    for (int val = 200; val >= 0; val--)
    {
        int numVal = 0;
        for (int s = 0; s < numScales; s++)
        {
            QImage* output = outputScale[s];
            for (int y = 0; y < output->height; y++)
                for (int x = 0; x < output->width; x++)
                    if (output->data(x, y) == val)
                    {
                        int xc = x;
                        int yc = y;
                        int found = 0;
                        for (int si = -4; si <= 4; si++)
                        {
                            if (si + s < 0 || si + s >= numScales) continue;
                            int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                            int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                            QImage* image = inputScale[si + s];
                            //              int size=(int)floor(0.5+6*pow(1.2,-si));
                            int size = 4;
                            for (int xx = xci - size; xx <= xci + size; xx++)
                                for (int yy = yci - size; yy <= yci + size; yy++)
                                    if (xx >= 0 && xx < image->width && yy >= 0 && yy < image->height)
                                        if (image->data(xx, yy))
                                        {
                                            found = 1;
                                            if (image->data(xx, yy) == 1) facesFound++;
                                            image->data(xx, yy) = 2;
                                        }
                        }
                        if (!found) falsePos++;
                        numVal++;
                    }
        }
        fprintf(outf, "%.2f %d %d %d\n",
            (val - 100) / 100.0, numVal, falsePos, numFaces - facesFound);
    }
    for (int i = 0; i < numScales; i++)
    {
        delete inputScale[i];
        delete outputScale[i];
    }
    delete[] inputScale;
    delete[] outputScale;
    fclose(outf);
}

void FindCentroid(QImage * images[], int numImages,
    int s, int x, int y, int val,
    double* totalS, double* totalX, double* totalY,
    double* total)
{
    if (s >= 0 && s < numImages &&
        x >= 0 && x < images[s]->width &&
        y >= 0 && y < images[s]->height &&
        images[s]->data(x, y) >= val)
    {
        (*total) += images[s]->data(x, y);
        (*totalS) += s * images[s]->data(x, y);
        (*totalX) += x * exp(log(1.2) * s) * images[s]->data(x, y);
        (*totalY) += y * exp(log(1.2) * s) * images[s]->data(x, y);
        images[s]->data(x, y) = 0;
        FindCentroid(images, numImages, s, x + 1, y, val,
            totalS, totalX, totalY, total);
        FindCentroid(images, numImages, s, x - 1, y, val,
            totalS, totalX, totalY, total);
        FindCentroid(images, numImages, s, x, y + 1, val,
            totalS, totalX, totalY, total);
        FindCentroid(images, numImages, s, x, y - 1, val,
            totalS, totalX, totalY, total);
        FindCentroid(images, numImages,
            s - 1, (int)floor(0.5 + x * 1.2), (int)floor(0.5 + y * 1.2), val,
            totalS, totalX, totalY, total);
        FindCentroid(images, numImages,
            s + 1, (int)floor(0.5 + x / 1.2), (int)floor(0.5 + y / 1.2), val,
            totalS, totalX, totalY, total);
    }
}

// USED
// Use NN arbitration to merge the outputs from three networks.
// For each network, there are three inputs to the arbitrator, which
// are the fraction of inputs in a 3x3 square in three levels of
// the detection pyramid around the current point of interest.  The
// output from the arbitrator net is the detection output.  The arbitation
// network is applied only when one of the individual networks responds
// at the point of interest.
void NetArbitrateDirect(int width, int height, int numLocFiles,
    List<Detection> * detections,
    List<Detection> * outputDetections,
    int arbnet)
{
    // Create the detection pyramid
    int temWidth = width, temHeight = height;
    int numScales = 0;
    while (temWidth >= 20 && temHeight >= 20) {
        char name[256];
        sprintf(name, "scale%d", numScales);
        QImage* image = new QImage(name);
        image->ZeroBuffer(temWidth, temHeight);
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }
    QImage** imageScale = new QImage * [numScales];
    for (int i = 0; i < numScales; i++) {
        char name[256];
        sprintf(name, "scale%d", i);
        imageScale[i] = FindQImageWithName(name);
    }
    int numFaces = 0;
    int done = 0;

    // For each of the input detection lists (networks)
    for (int num = 0; num < numLocFiles; num++) {
        int numFaces = 0;
        int done = 0;

        // Write the detections into the detection pyramid
        for (Detection* d = detections[num].first; d != NULL; d = d->next) {
            int x = d->x, y = d->y, s = d->s;
            QImage* image = imageScale[s];
            image->data(x + 10, y + 10) |= (1 << num);
            numFaces++;
        }
        //    fprintf(stderr,"numFaces=%d\n",numFaces);
    }
    //FILE *ans=fopen("/usr0/har/faces/our.votes","r");

      // Scan through the detection pyramid
    for (int s = 0; s < numScales; s++) {
        QImage* image = imageScale[s];
        for (int x = 0; x < image->width; x++)
            for (int y = 0; y < image->height; y++)
                // If any of the networks responded at this point
                if (image->data(x, y)) {
                    //          int ptr=1;
                    int ptr2 = 1;
                    // For each of the individual networks
                    for (int net = 0; net < numLocFiles; net++) {
                        // Go over the 3 scales
                        for (int si = -1; si <= 1; si++) {
                            int tot = 0;
                            int xci = (int)floor(0.5 + x * exp(log(1.2) * -si));
                            int yci = (int)floor(0.5 + y * exp(log(1.2) * -si));
                            // Total up the number of responses in a 3x3 region in this
                            // scale
                            for (int xx = xci - 1; xx <= xci + 1; xx++)
                                for (int yy = yci - 1; yy <= yci + 1; yy++) {
                                    int val = 0;
                                    if (si + s >= 0 && si + s < numScales) {
                                        QImage* image = imageScale[si + s];
                                        if (xx >= 0 && yy >= 0 && xx < image->width && yy < image->height)
                                            val = (image->data(xx, yy) & (1 << net)) ? 1 : 0;
                                    }
                                    //                  real_fast_list[0]->unitList[ptr++].activation=val;
                                    tot += val;
                                }
                            // Write fraction of detections into network
                            real_fast_list[arbnet]->unitList[ptr2++].activation = tot / 9.0;
                        }
                    }
                    // Apply arbitrator
                    double out = FastForwardPass(real_fast_list[arbnet]);
                    //char ansStr[1024];
                    //fgets(ansStr,1024,ans);
                    //int ansVal=0;
                    //sscanf(ansStr,"%d",&ansVal);
                    if (out > 0.0) {
#ifdef notdef
                        fprintf(stderr, "%s %g %d %d %d %d %d %d %d %g\n",
                            ansVal ? "POS" : "NEG", out,
                            (int)floor(0.5 + (x - 10) * pow(1.2, s)),
                            (int)floor(0.5 + (y - 10) * pow(1.2, s)),
                            (int)floor(0.5 + (x + 10) * pow(1.2, s)),
                            (int)floor(0.5 + (y + 10) * pow(1.2, s)),
                            x - 10, y - 10, s, out);
#endif
                        // Record positive outputs
                        outputDetections->addLast(new Detection(x - 10, y - 10, s, out, 0));
                    }
                }
    }
    // Remove detection pyramid
    for (int i = 0; i < numScales; i++)
        delete imageScale[i];
    delete[] imageScale;
}

#ifdef notdef
int FuzzyVote2Cmd(ClientData data, Tcl_Interp * interp,
    int argc, char* argv[])
{
    if (argc != 10)
    {
        interp->result = "Usage: FuzzyVote2 <width> <height> [location-files] <outputfile> <spread> <min> <collapse> <overlap> <filterodd>";
        return TCL_OK;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    char* locFiles = argv[3];
    char* outFile = argv[4];
    int spread = atoi(argv[5]);
    int search = atoi(argv[6]);
    int collapse = atoi(argv[7]);
    int overlap = atoi(argv[8]);
    int filterodd = atoi(argv[9]);
    int temWidth = width, temHeight = height;
    int numScales = 0;
    QImage* mask = GetFaceMask();
    while (temWidth >= mask->width && temHeight >= mask->height)
    {
        char name[256];
        sprintf(name, "scale%d", numScales);
        QImage* image = new QImage(name);
        image->ZeroBuffer(temWidth, temHeight);
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }
    QImage** imageScale = new QImage * [numScales];
    for (int i = 0; i < numScales; i++)
    {
        char name[256];
        sprintf(name, "scale%d", i);
        imageScale[i] = FindQImageWithName(name);
    }
    char** locFileList;
    int numLocFiles;
    Tcl_SplitList(interp, locFiles, &numLocFiles, &locFileList);
    for (int num = 0; num < numLocFiles; num++)
    {
        int numFaces = 0;
        int done = 0;
        FILE* inf = fopen(locFileList[num], "r");
        do {
            int x, y, s;
            double conf;
            char buf[1024];
            if (fgets(buf, 1024, inf) != NULL) {
                int numread = sscanf(buf, "%*f %*f %*f %*f %d %d %d %lf\n",
                    &x, &y, &s, &conf);
                if (numread < 0) done = 1; else {
                    if (conf < 0.0) continue;
                    if (filterodd && ((x & 1) || (y & 1))) continue;
                    int xc = x + mask->width / 2;
                    int yc = y + mask->height / 2;
                    for (int si = -spread; si <= spread; si++) {
                        if (si + s < 0 || si + s >= numScales) continue;
                        int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                        int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                        QImage* image = imageScale[si + s];
                        int sspread = (int)floor(0.5 + spread * pow(1.2, -si));
                        for (int xx = xci - sspread; xx <= xci + sspread; xx++)
                            for (int yy = yci - sspread; yy <= yci + sspread; yy++)
                                if (xx >= 0 && xx < image->width && yy >= 0 && yy < image->height) {
                                    if (image->data(xx, yy) == 255) {
                                        fprintf(stderr, "Overflow!\n");
                                        exit(1);
                                    }
                                    image->data(xx, yy)++;
                                }
                    }
                }
            else done = 1;
            numFaces++;
            }
        } while (!done);
        fclose(inf);
        //    fprintf(stderr,"numFaces=%d\n",numFaces);
    }
    free((char*)locFileList);
    int max = 0;
    for (int s = 0; s < numScales; s++)
    {
        QImage* image = imageScale[s];
        for (int i = 0; i < image->size; i++)
            if (image->data(i) > max) max = image->data(i);
    }
    FILE* outf = fopen(outFile, "w");

    for (int val = max; val >= search; val--)
    {
        fprintf(stderr, "Scanning for %d\n", val);
        for (s = 0; s < numScales; s++)
        {
            QImage* image = imageScale[s];
            for (int x = 0; x < image->width; x++)
                for (int y = 0; y < image->height; y++)
                    if (image->data(x, y) == val)
                    {
                        int cs, cx, cy;
                        if (collapse)
                        {
                            double total = 0.0;
                            double totalS = 0, totalX = 0, totalY = 0;
                            FindCentroid(imageScale, numScales, s, x, y, search,
                                &totalS, &totalX, &totalY, &total);
                            cs = (int)floor(0.5 + totalS / total);
                            cx = (int)floor(0.5 + totalX / total * exp(log(1.2) * -cs));
                            cy = (int)floor(0.5 + totalY / total * exp(log(1.2) * -cs));
                        }
                        else
                        {
                            cs = s;
                            cx = x;
                            cy = y;
                        }

                        // Now zap all the nearby faces
                        if (overlap)
                        {
                            for (int scale = 0; scale < numScales; scale++)
                            {
                                int xpos = (int)floor(0.5 + cx * pow(1.2, cs - scale));
                                int ypos = (int)floor(0.5 + cy * pow(1.2, cs - scale));
                                int size = 10 + (int)floor(0.5 + 10 * pow(1.2, cs - scale));
                                QImage* img = imageScale[scale];
                                for (int xx = xpos - size; xx <= xpos + size; xx++)
                                    for (int yy = ypos - size; yy <= ypos + size; yy++)
                                        if (xx >= 0 && xx < img->width && yy >= 0 && yy < img->height)
                                            img->data(xx, yy) = 0;
                            }
                        }

                        int xpos = (int)floor(0.5 + cx * (exp(log(1.2) * cs)));
                        int ypos = (int)floor(0.5 + cy * (exp(log(1.2) * cs)));
                        int sizex = (int)floor(0.5 + 0.5 * mask->width * (exp(log(1.2) * cs)));
                        int sizey = (int)floor(0.5 + 0.5 * mask->height * (exp(log(1.2) * cs)));
                        fprintf(stderr, "%d %d %d %d %d %d %d 1.0\n", xpos - sizex, ypos - sizey,
                            xpos + sizex, ypos + sizey,
                            cx - mask->width / 2, cy - mask->height / 2, cs);
                        fprintf(outf, "%d %d %d %d %d %d %d 1.0\n", xpos - sizex, ypos - sizey,
                            xpos + sizex, ypos + sizey,
                            cx - mask->width / 2, cy - mask->height / 2, cs);
                    }
        }
    }
    fclose(outf);
    for (i = 0; i < numScales; i++)
        delete imageScale[i];
    delete[] imageScale;
    return TCL_OK;
}
#endif

#ifdef __GNUC__
template class List<AccumElement>;
template class ListNode<AccumElement>;
template class List<Detection>;
template class ListNode<Detection>;
#endif

// USED
// Get the value of some location in the detection pyramid.  If that location
// has not already been allocated, create it and set it to zero.
static int FuzzyVoteAccumGet(Tcl_HashTable * accum, List<AccumElement> * bins,
    int x, int y, int s)
{
    int loc[3];
    loc[0] = x; loc[1] = y; loc[2] = s;
    Tcl_HashEntry* entry = Tcl_FindHashEntry(accum, (char*)loc);
    if (entry == NULL)
    {
        int newentry;
        entry = Tcl_CreateHashEntry(accum, (char*)loc, &newentry);
        AccumElement* elem = new AccumElement(x, y, s, 0, 0, 0);
        Tcl_SetHashValue(entry, elem);
        bins[0].addLast(elem);
    }
    return ((AccumElement*)(Tcl_GetHashValue(entry)))->value;
}

// USED
// Get the value of some location in the detection pyramid.  If that location
// does not exist, they return zero (but do not allocate that location).
static int FuzzyVoteAccumCheck(Tcl_HashTable * accum, List<AccumElement> * bins,
    int x, int y, int s)
{
    int loc[3];
    loc[0] = x; loc[1] = y; loc[2] = s;
    Tcl_HashEntry* entry = Tcl_FindHashEntry(accum, (char*)loc);
    if (entry == NULL) return 0;
    return ((AccumElement*)(Tcl_GetHashValue(entry)))->value;
}

// USED
// Set a value in the detection pyramid; if that location does not exist,
// then create it.  Also, add that location to the list of locations with
// that value.
static void FuzzyVoteAccumSet(Tcl_HashTable * accum, List<AccumElement> * bins,
    int x, int y, int s, int val)
{
    int loc[3];
    loc[0] = x; loc[1] = y; loc[2] = s;
    Tcl_HashEntry* entry = Tcl_FindHashEntry(accum, (char*)loc);
    AccumElement* elem = (AccumElement*)(Tcl_GetHashValue(entry));
    bins[elem->value].unchain(elem);
    if (val > 255) val = 255;
    bins[val].addLast(elem);
    elem->value = val;
}

// USED
// Set a value in the detection pyramid to zero.  If the location does not
// exist, then do not create it (because the default value is zero).
static void FuzzyVoteAccumZero(Tcl_HashTable * accum, List<AccumElement> * bins,
    int x, int y, int s)
{
    int loc[3];
    loc[0] = x; loc[1] = y; loc[2] = s;
    Tcl_HashEntry* entry = Tcl_FindHashEntry(accum, (char*)loc);
    if (entry == NULL) return;
    AccumElement* elem = (AccumElement*)(Tcl_GetHashValue(entry));
    bins[elem->value].unchain(elem);
    bins[0].addLast(elem);
    elem->value = 0;
}

// USED
// Given some point in the detection pyramid, locate all 6-connection
// locations with a value greater than or equal to the specified amount,
// and find their centroid.  The locations in the centroid are set to zero.
// Centroid in scale is computed by averaging the pyramid levels at which
// the faces are detected.
static void FindCentroidAccum(Tcl_HashTable * accum, List<AccumElement> * bins,
    int numImages,
    int s, int x, int y, int val,
    double* totalS, double* totalX, double* totalY,
    double* total)
{
    int value = FuzzyVoteAccumCheck(accum, bins, x, y, s);
    if (value >= val) {
        FuzzyVoteAccumZero(accum, bins, x, y, s);
        (*total) += value;
        (*totalS) += s * value;
        (*totalX) += x * pow(1.2, s) * value;
        (*totalY) += y * pow(1.2, s) * value;
        FindCentroidAccum(accum, bins, numImages, s, x + 1, y, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum(accum, bins, numImages, s, x - 1, y, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum(accum, bins, numImages, s, x, y + 1, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum(accum, bins, numImages, s, x, y - 1, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum(accum, bins, numImages,
            s - 1, (int)floor(0.5 + x * 1.2), (int)floor(0.5 + y * 1.2), val,
            totalS, totalX, totalY, total);
        FindCentroidAccum(accum, bins, numImages,
            s + 1, (int)floor(0.5 + x / 1.2), (int)floor(0.5 + y / 1.2), val,
            totalS, totalX, totalY, total);
    }
}

// Given some point in the detection pyramid, locate all 6-connection
// locations with a value greater than or equal to the specified amount,
// and find their centroid.  The locations in the centroid are set to zero.
// Centroid in scale is computed by averaging the sizes of the detected
// faces.
static void FindCentroidAccum3(Tcl_HashTable * accum, List<AccumElement> * bins,
    int numImages,
    int s, int x, int y, int val,
    double* totalS, double* totalX, double* totalY,
    double* total)
{
    int value = FuzzyVoteAccumCheck(accum, bins, x, y, s);
    if (value >= val)
    {
        FuzzyVoteAccumZero(accum, bins, x, y, s);
        (*total) += value;
        (*totalS) += pow(1.2, s) * value;
        (*totalX) += x * pow(1.2, s) * value;
        (*totalY) += y * pow(1.2, s) * value;
        FindCentroidAccum3(accum, bins, numImages, s, x + 1, y, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum3(accum, bins, numImages, s, x - 1, y, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum3(accum, bins, numImages, s, x, y + 1, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum3(accum, bins, numImages, s, x, y - 1, val,
            totalS, totalX, totalY, total);
        FindCentroidAccum3(accum, bins, numImages,
            s - 1, (int)floor(0.5 + x * 1.2), (int)floor(0.5 + y * 1.2), val,
            totalS, totalX, totalY, total);
        FindCentroidAccum3(accum, bins, numImages,
            s + 1, (int)floor(0.5 + x / 1.2), (int)floor(0.5 + y / 1.2), val,
            totalS, totalX, totalY, total);
    }
}

// USED
// Implementation of the arbitration mechanisms described in the paper.
// Inputs are lists of detections, the width and height of the image that
// is being processed, a callback function used to report the arbitration
// results, and a bunch of parameters for the arbitration itself.
void FuzzyVote2(int width, int height, int numLocFiles,
    List<Detection> detections[],
    SearchCallback callback, ClientData data,
    int spread, int search, int collapse, int overlap,
    int filterodd, QImage * mask)
{
    // Hash table represents which locations/scales in the detection are
    // filled in, and with what values.  Any missing entries are assumed
    // to be zero.
    Tcl_HashTable accum;
    Tcl_InitHashTable(&accum, 3);
    // An array of lists.  Each lists contains all the detection location/scales
    // with a particular value.  This allows the detections to be scanned in
    // order from highest to lowest with practically no overhead.
    List<AccumElement> bins[256];

    // Figure out the number of scales in the pyramid
    int temWidth = width, temHeight = height;
    int numScales = 0;
    while (temWidth >= mask->width && temHeight >= mask->height) {
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }

    // For each detection list given as input (all detections are treated
    // equally)
    for (int num = 0; num < numLocFiles; num++) {
        int numFaces = 0;
        int done = 0;
        // For each detection
        for (Detection* detect = detections[num].first; detect != NULL;
            detect = detect->next) {
            int x = detect->x, y = detect->y, s = detect->s;
            if (filterodd && ((x & 1) || (y & 1))) continue;
            int xc = x + mask->width / 2;
            int yc = y + mask->height / 2;
            // Spread out the detection in both scale and location by
            // "spread" levels or pixels, incrementing the value of each
            // location in the detection pyramid
            for (int si = -spread; si <= spread; si++) {
                if (si + s < 0 || si + s >= numScales) continue;
                int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                //        int sspread=(int)floor(0.5+spread*pow(1.2,-si));
                int sspread = spread;
                for (int xx = xci - sspread; xx <= xci + sspread; xx++)
                    for (int yy = yci - sspread; yy <= yci + sspread; yy++)
                        FuzzyVoteAccumSet(&accum, bins, xx, yy, s + si,
                            FuzzyVoteAccumGet(&accum, bins, xx, yy, s + si) + 1);
            }
            numFaces++;
        }
    }

    // Scan through the detection pyramid from highest to lowest value
    for (int val = 255; val >= search; val--) {
        while (!bins[val].empty()) {
            // Get the detection
            int x = bins[val].first->loc[0];
            int y = bins[val].first->loc[1];
            int s = bins[val].first->loc[2];
            int cs, cx, cy;

            // If we are to collapse nearby overlapping detections, find the
            // centroid of the detections around this location.  Otherwise, just
            // record this location.
            if (collapse) {
                double total = 0.0;
                double totalS = 0, totalX = 0, totalY = 0;
                FindCentroidAccum(&accum, bins, numScales, s, x, y, search,
                    &totalS, &totalX, &totalY, &total);
                cs = (int)floor(0.5 + totalS / total);
                cx = (int)floor(0.5 + totalX / total * exp(log(1.2) * -cs));
                cy = (int)floor(0.5 + totalY / total * exp(log(1.2) * -cs));
            }
            else {
                cs = s;
                cx = x;
                cy = y;
                FuzzyVoteAccumZero(&accum, bins, x, y, s);
            }

            // If we are to remove overlapping detections, scan through
            // the detection pyramid, removing all possible overlaps
            if (overlap) {
                for (int scale = 0; scale < numScales; scale++) {
                    int xpos = (int)floor(0.5 + cx * pow(1.2, cs - scale));
                    int ypos = (int)floor(0.5 + cy * pow(1.2, cs - scale));
                    int sizex = mask->width / 2 +
                        (int)floor(0.5 + mask->width / 2 * pow(1.2, cs - scale));
                    int sizey = mask->height / 2 +
                        (int)floor(0.5 + mask->height / 2 * pow(1.2, cs - scale));
                    for (int xx = xpos - sizex; xx <= xpos + sizex; xx++)
                        for (int yy = ypos - sizey; yy <= ypos + sizey; yy++)
                            FuzzyVoteAccumZero(&accum, bins, xx, yy, scale);
                }
            }

            // Record (or otherwise deal with) this detection
            callback(data, NULL, cx - mask->width / 2, cy - mask->height / 2,
                mask->width, mask->height,
                cs, pow(1.2, cs), 1.0, 0);
        }
    }
    // Clean up
    Tcl_DeleteHashTable(&accum);
}

// USED
// Implementation of the arbitration mechanisms described in the paper.
// Inputs are lists of detections, the width and height of the image that
// is being processed, a callback function used to report the arbitration
// results, and a bunch of parameters for the arbitration itself.
void FuzzyVote2Direct(int width, int height, int numLocFiles,
    List<Detection> * detections,
    List<Detection> * outputDetections,
    int spread, int search, int collapse, int overlap,
    int filterodd, QImage * mask)
{
    // Hash table represents which locations/scales in the detection are
    // filled in, and with what values.  Any missing entries are assumed
    // to be zero.
    Tcl_HashTable accum;
    Tcl_InitHashTable(&accum, 3);
    // An array of lists.  Each lists contains all the detection location/scales
    // with a particular value.  This allows the detections to be scanned in
    // order from highest to lowest with practically no overhead.
    List<AccumElement> bins[256];

    // Figure out the number of scales in the pyramid
    int temWidth = width, temHeight = height;
    int numScales = 0;
    while (temWidth >= mask->width && temHeight >= mask->height) {
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }

    // For each detection list given as input (all detections are treated
    // equally)
    for (int num = 0; num < numLocFiles; num++) {
        int numFaces = 0;
        int done = 0;
        for (Detection* detect = detections[num].first; detect != NULL;
            detect = detect->next) {
            // For each detection
            int x = detect->x, y = detect->y, s = detect->s;
            if (filterodd && ((x & 1) || (y & 1))) continue;
            if (detect->output <= 0.0) continue;
            int xc = x + mask->width / 2;
            int yc = y + mask->height / 2;
            // Spread out the detection in both scale and location by
            // "spread" levels or pixels, incrementing the value of each
            // location in the detection pyramid
            for (int si = -spread; si <= spread; si++) {
                if (si + s < 0 || si + s >= numScales) continue;
                int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                //        int sspread=(int)floor(0.5+spread*pow(1.2,-si));
                int sspread = spread;
                for (int xx = xci - sspread; xx <= xci + sspread; xx++)
                    for (int yy = yci - sspread; yy <= yci + sspread; yy++)
                        FuzzyVoteAccumSet(&accum, bins, xx, yy, s + si,
                            FuzzyVoteAccumGet(&accum, bins, xx, yy, s + si) + 1);
            }
            numFaces++;
        }
    }

    // Scan through the detection pyramid from highest to lowest value
    for (int val = 255; val >= search; val--) {
        while (!bins[val].empty()) {
            // Get the detection
            int x = bins[val].first->loc[0];
            int y = bins[val].first->loc[1];
            int s = bins[val].first->loc[2];
            int cs, cx, cy;

            // If we are to collapse nearby overlapping detections, find the
            // centroid of the detections around this location.  Otherwise, just
            // record this location.
            if (collapse) {
                double total = 0.0;
                double totalS = 0, totalX = 0, totalY = 0;
                FindCentroidAccum(&accum, bins, numScales, s, x, y, search,
                    &totalS, &totalX, &totalY, &total);
                cs = (int)floor(0.5 + totalS / total);
                cx = (int)floor(0.5 + totalX / total * exp(log(1.2) * -cs));
                cy = (int)floor(0.5 + totalY / total * exp(log(1.2) * -cs));
            }
            else {
                cs = s;
                cx = x;
                cy = y;
                FuzzyVoteAccumZero(&accum, bins, x, y, s);
            }

            // If we are to remove overlapping detections, scan through
            // the detection pyramid, removing all possible overlaps
            if (overlap) {
                for (int scale = 0; scale < numScales; scale++)
                {
                    int xpos = (int)floor(0.5 + cx * pow(1.2, cs - scale));
                    int ypos = (int)floor(0.5 + cy * pow(1.2, cs - scale));
                    int sizex = mask->width / 2 +
                        (int)floor(0.5 + mask->width / 2 * pow(1.2, cs - scale));
                    int sizey = mask->height / 2 +
                        (int)floor(0.5 + mask->height / 2 * pow(1.2, cs - scale));
                    for (int xx = xpos - sizex; xx <= xpos + sizex; xx++)
                        for (int yy = ypos - sizey; yy <= ypos + sizey; yy++)
                            FuzzyVoteAccumZero(&accum, bins, xx, yy, scale);
                }
            }
            // Record this detection
            outputDetections->addLast(new Detection(cx - mask->width / 2,
                cy - mask->height / 2,
                cs, 1.0, 0));
        }
    }
    Tcl_DeleteHashTable(&accum);
}

void FuzzyVote3(int width, int height, int numLocFiles,
    List<Detection> detections[],
    SearchCallback callback, ClientData data,
    int spread, int search, int collapse, int overlap,
    int filterodd, QImage * mask)
{
    Tcl_HashTable accum;
    Tcl_InitHashTable(&accum, 3);
    List<AccumElement> bins[256];
    int temWidth = width, temHeight = height;
    int numScales = 0;
    while (temWidth >= mask->width && temHeight >= mask->height)
    {
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }
    for (int num = 0; num < numLocFiles; num++)
    {
        int numFaces = 0;
        int done = 0;
        for (Detection* detect = detections[num].first; detect != NULL;
            detect = detect->next)
        {
            int xc = detect->x, yc = detect->y, s = detect->s;
            if (filterodd && ((xc & 1) || (yc & 1))) continue;
            for (int si = -spread; si <= spread; si++)
            {
                if (si + s < 0 || si + s >= numScales) continue;
                int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                int sspread = (int)floor(0.5 + spread * pow(1.2, -si));
                for (int xx = xci - sspread; xx <= xci + sspread; xx++)
                    for (int yy = yci - sspread; yy <= yci + sspread; yy++)
                        FuzzyVoteAccumSet(&accum, bins, xx, yy, s + si,
                            FuzzyVoteAccumGet(&accum, bins, xx, yy, s + si) + 1);
            }
            numFaces++;
        }
    }
    for (int val = 255; val >= search; val--)
    {
        while (!bins[val].empty()) {
            int x = bins[val].first->loc[0];
            int y = bins[val].first->loc[1];
            int s = bins[val].first->loc[2];
            int cs, cx, cy;
            if (collapse)
            {
                double total = 0.0;
                double totalS = 0, totalX = 0, totalY = 0;
                FindCentroidAccum3(&accum, bins, numScales, s, x, y, search,
                    &totalS, &totalX, &totalY, &total);
                cs = (int)floor(0.5 + log(totalS / total) / log(1.2));
                cx = (int)floor(0.5 + totalX / total * exp(log(1.2) * -cs));
                cy = (int)floor(0.5 + totalY / total * exp(log(1.2) * -cs));
            }
            else
            {
                cs = s;
                cx = x;
                cy = y;
                FuzzyVoteAccumZero(&accum, bins, x, y, s);
            }

            // Now zap all the nearby faces
            if (overlap)
            {
                for (int scale = 0; scale < numScales; scale++)
                {
                    int xpos = (int)floor(0.5 + cx * pow(1.2, cs - scale));
                    int ypos = (int)floor(0.5 + cy * pow(1.2, cs - scale));
                    int sizex = mask->width / 2 +
                        (int)floor(0.5 + mask->width / 2 * pow(1.2, cs - scale));
                    int sizey = mask->height / 2 +
                        (int)floor(0.5 + mask->height / 2 * pow(1.2, cs - scale));
                    for (int xx = xpos - sizex; xx <= xpos + sizex; xx++)
                        for (int yy = ypos - sizey; yy <= ypos + sizey; yy++)
                            FuzzyVoteAccumZero(&accum, bins, xx, yy, scale);
                }
            }
            callback(data, NULL, cx - mask->width / 2, cy - mask->height / 2,
                mask->width, mask->height,
                cs, pow(1.2, cs), 1.0, 0);
        }
    }
    Tcl_DeleteHashTable(&accum);
}

void FuzzyVote2Callback(ClientData data, QImage * image,
    int x, int y, int width, int height,
    int level,
    double scale, double output, int orient)
{
    FILE* outf = (FILE*)data;
    fprintf(outf, "%g %g %g %g %d %d %d %g %d\n",
        scale * x, scale * y,
        scale * (x + width), scale * (y + height),
        x, y, level, output, orient);
}

#ifdef notdef
void FuzzyVote2(int width, int height, int numLocFiles,
    List<Detection> detections[],
    SearchCallback callback, ClientData data,
    int spread, int search, int collapse, int overlap,
    int filterodd)
{
    int temWidth = width, temHeight = height;
    int numScales = 0;
    QImage* mask = GetFaceMask();
    while (temWidth >= mask->width && temHeight >= mask->height)
    {
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        numScales++;
    }
    temWidth = width;
    temHeight = height;
    QImage** imageScale = new QImage * [numScales];
    for (int i = 0; i < numScales; i++)
    {
        QImage* image = new QImage(NULL);
        image->ZeroBuffer(temWidth, temHeight);
        temWidth = (int)(floor(temWidth / 1.2));
        temHeight = (int)(floor(temHeight / 1.2));
        imageScale[i] = image;
    }
    for (int num = 0; num < numLocFiles; num++)
    {
        int numFaces = 0;
        int done = 0;
        for (Detection* detect = detections[num].first; detect != NULL;
            detect = detect->next)
        {
            int x = detect->x, y = detect->y, s = detect->s;
            if (filterodd && ((x & 1) || (y & 1))) continue;
            int xc = x + mask->width / 2;
            int yc = y + mask->height / 2;
            for (int si = -spread; si <= spread; si++)
            {
                if (si + s < 0 || si + s >= numScales) continue;
                int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
                int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
                QImage* image = imageScale[si + s];
                int sspread = (int)floor(0.5 + spread * pow(1.2, -si));
                for (int xx = xci - sspread; xx <= xci + sspread; xx++)
                    for (int yy = yci - sspread; yy <= yci + sspread; yy++)
                        if (xx >= 0 && xx < image->width && yy >= 0 && yy < image->height)
                        {
                            if (image->data(xx, yy) == 255)
                            {
                                fprintf(stderr, "Overflow!\n");
                                exit(1);
                            }
                            image->data(xx, yy)++;
                        }
            }
            numFaces++;
        }
        fprintf(stderr, "numFaces=%d\n", numFaces);
    }
    int max = 0;
    for (int s = 0; s < numScales; s++)
    {
        QImage* image = imageScale[s];
        for (int i = 0; i < image->size; i++)
            if (image->data(i) > max) max = image->data(i);
    }
    for (int val = max; val >= search; val--)
    {
        fprintf(stderr, "Scanning for %d\n", val);
        for (s = 0; s < numScales; s++)
        {
            QImage* image = imageScale[s];
            for (int x = 0; x < image->width; x++)
                for (int y = 0; y < image->height; y++)
                    if (image->data(x, y) == val)
                    {
                        int cs, cx, cy;
                        if (collapse)
                        {
                            double total = 0.0;
                            double totalS = 0, totalX = 0, totalY = 0;
                            FindCentroid(imageScale, numScales, s, x, y, search,
                                &totalS, &totalX, &totalY, &total);
                            cs = (int)floor(0.5 + totalS / total);
                            cx = (int)floor(0.5 + totalX / total * exp(log(1.2) * -cs));
                            cy = (int)floor(0.5 + totalY / total * exp(log(1.2) * -cs));
                        }
                        else
                        {
                            cs = s;
                            cx = x;
                            cy = y;
                        }

                        // Now zap all the nearby faces
                        if (overlap)
                        {
                            for (int scale = 0; scale < numScales; scale++)
                            {
                                int xpos = (int)floor(0.5 + cx * pow(1.2, cs - scale));
                                int ypos = (int)floor(0.5 + cy * pow(1.2, cs - scale));
                                int size = 10 + (int)floor(0.5 + 10 * pow(1.2, cs - scale));
                                QImage* img = imageScale[scale];
                                for (int xx = xpos - size; xx <= xpos + size; xx++)
                                    for (int yy = ypos - size; yy <= ypos + size; yy++)
                                        if (xx >= 0 && xx < img->width && yy >= 0 && yy < img->height)
                                            img->data(xx, yy) = 0;
                            }
                        }
                        callback(data, imageScale[cs], cx - mask->width / 2, cy - mask->height / 2,
                            mask->width, mask->height,
                            cs, pow(1.2, cs), 1.0, 0);
                    }
        }
    }
    for (i = 0; i < numScales; i++)
        delete imageScale[i];
    delete[] imageScale;
}
#endif

#ifdef notdef
// This version is faster, but applies the lighting correction calculation
// to the entire window rather than just to the masked portion.  Applying
// it to just the masked portion is slightly more complex, and the
// performance improvement is < 10%.
int SearchMultiCmd(ClientData data, Tcl_Interp * interp,
    int argc, char* argv[])
{
    if (argc != 3)
    {
        interp->result = "Usage: SearchMulti <image> {filelist}";
        return TCL_ERROR;
    }
    char** fileList;
    int numFiles;
    QImage* image = FindQImageWithName(argv[1]);
    Tcl_SplitList(interp, argv[2], &numFiles, &fileList);
    int i;
    FILE** openFileList = new FILE * [numFiles];
    for (i = 0; i < numFiles; i++) openFileList[i] = fopen(fileList[i], "w");
    free((char*)fileList);
    int ptr = 0;
    Mat mat = Zero(3, 3);
    QImage* mask = GetFaceMask();
    for (int j = 0; j < mask->height; j++) for (i = 0; i < mask->width; i++)
        if (mask->data(ptr++))
        {
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
    mat = LUInverse(mat);

    char cmd[1024];
    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    while (image->height >= mask->height && image->width >= mask->width)
    {
        for (int y = 0; y < image->height - mask->height + 1; y++)
        {
            v0 = 0; v1 = 0; v2 = 0;
            int ptr = 0;
            for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++)
            {
                Byte data = image->data(i, j + y);
                tmp[ptr] = data;
                if (mask->data(ptr++))
                {
                    v0 += i * data;
                    v1 += j * data;
                    v2 += data;
                }
            }
            for (int x = 0; x < image->width - mask->width + 1; x++)
            {
                int ptr = 0;
                if (x > 0)
                {
                    for (int i = 0; i < mask->size - 1; i++)
                        tmp[i] = tmp[i + 1];
                    ptr = mask->width - 1;
                    for (int j = 0; j < mask->height; j++)
                    {
                        Byte data = image->data(x - 1, j + y);
                        v1 -= j * data;
                        v2 -= data;
                        data = image->data(x + mask->width - 1, j + y);
                        v0 += mask->width * data;
                        v1 += j * data;
                        v2 += data;
                        tmp[ptr] = data;
                        ptr += mask->width;
                    }
                    v0 -= v2;
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (j = 0; j < mask->height; j++) for (i = 0; i < mask->width; i++)
                {
                    int val = tmp[ptr] - (int)(floor(0.5 + i * v0 + j * v1 + v2)) + 256;
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }
                int map[512];
                int num = 0;
                for (i = 0; i < 512; i++)
                {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (i = 511; i >= 0; i--)
                {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / tot;
                for (i = 0; i < mask->size; i++)
                    real_fast_list[0]->unitList[i + 1].activation =
                    map[tmp[i]] * scaleFactor - 1.0;
                for (int j = 0; j < numFiles; j++)
                    for (i = 1; i <= mask->size; i++)
                        real_fast_list[j]->unitList[i].activation =
                        real_fast_list[0]->unitList[i].activation;

                for (i = 0; i < numFiles; i++)
                {
                    real_outputs[0] = FastForwardPass(real_fast_list[i]);
                    if (real_outputs[0] > 0)
                    {
                        fprintf(openFileList[i], "%g %g %g %g %d %d %d %g\n",
                            scale * x + 2, scale * y + 2,
                            scale * (x + mask->width) + 2, scale * (y + mask->height) + 2,
                            x, y, (int)(floor(0.5 + log(scale) / log(1.2))),
                            real_outputs[0]);
                    }
                }
            }
        }
        SmoothQImage(image);
        ZoomQImage(image, 1.2, image);
        scale *= 1.2;
    }
    delete[] tmp;
    for (i = 0; i < numFiles; i++)
        fclose(openFileList[i]);
    delete[] openFileList;
    return TCL_OK;
}
#endif

void SearchMultiCallbackSave(ClientData data, QImage * image,
    int x, int y, int width, int height, int level,
    double scale, double output, int orient)
{
    FILE* outf = (FILE*)data;
    fprintf(outf, "%g %g %g %g %d %d %d %g %d\n",
        scale * x, scale * y,
        scale * (x + width), scale * (y + height),
        x, y, level, output, orient);
}

void SearchMulti(QImage * image, int numNetworks,
    SearchCallback callback, ClientData data[])
{
    int ptr = 0;
    Mat mat = Zero(3, 3);
    QImage* mask = FindQImageWithName("facemask");
    int halfW = mask->width / 2;
    int halfH = mask->height / 2;

    for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++)
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
    mat = LUInverse(mat);

    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    int level = 0;
    while (image->height >= mask->height && image->width >= mask->width) {
        for (int y = 0; y < image->height; y++) {
            for (int x = 0; x < image->width; x++) {
                int ptr = 0;
                v0 = 0; v1 = 0; v2 = 0;
                for (int j = -halfH; j < halfH; j++) {
                    for (int i = -halfW; i < halfW; i++) {
                        int ii = i + x;
                        int jj = j + y;
                        if (ii < 0) ii = 0;
                        if (ii >= image->width) ii = image->width - 1;
                        if (jj < 0) jj = 0;
                        if (jj >= image->height) jj = image->height - 1;
                        int data = image->data(ii, jj);

#ifdef DEBUG
                        if (x == DebugX && y == DebugY && level == DebugS) {
                            fprintf(stderr, "i,j=%d,%d data=%d\n", i + x, j + y, data);
                        }
#endif

                        tmp[ptr] = data;
                        if (mask->data(ptr++)) {
                            v0 += i * data;
                            v1 += j * data;
                            v2 += data;
                        }
                    }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++) {
                    int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }
                int map[512];
                int num = 0;
                for (int i = 0; i < 512; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 511; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;

                for (int i = 0; i < real_numNetworks; i++) {
                    if (i > 0) {
                        FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                        FastForwardUnit* unit1 = &(real_fast_list[i]->unitList[1]);
                        for (int j = 0; j < mask->size; j++)
                            (unit1++)->activation = (unit++)->activation;
                    }
                    double output = FastForwardPass(real_fast_list[i]);
#ifdef DEBUG
                    if (x == DebugX && y == DebugY && level == DebugS) {
                        fprintf(stderr, "output=%g\n", output);
                        exit(1);
                    }
#endif
                    if (output > 0) {
                        //          if (output>0.9996) {
                        callback(data[i], image, x - halfW, y - halfH, mask->width, mask->height,
                            level, scale, output, 0);
                        //            fprintf(stderr,"x=%d,y=%d:\n",x,y);
                        //            FastDebugNetwork(real_fast_list[i]);
                        //            exit(0);
                    }
                }
            }
        }
        ReduceSize(image, image);
        //    SmoothQImage(image);
        //    ZoomQImage(image,1.2,image);
        scale *= 1.2;
        level++;
    }
    delete[] tmp;
}

void SearchMultiAverage(QImage * image, int numNetworks,
    SearchCallback callback, ClientData data)
{
    int ptr = 0;
    Mat mat = Zero(3, 3);
    QImage* mask = FindQImageWithName("facemask");
    int halfW = mask->width / 2;
    int halfH = mask->height / 2;

    for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++)
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
    mat = LUInverse(mat);

    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    int level = 0;
    while (image->height >= mask->height && image->width >= mask->width) {
        for (int y = 0; y < image->height; y++) {
            for (int x = 0; x < image->width; x++) {
                int ptr = 0;
                v0 = 0; v1 = 0; v2 = 0;
                for (int j = -halfH; j < halfH; j++) {
                    for (int i = -halfW; i < halfW; i++) {
                        int ii = i + x;
                        int jj = j + y;
                        if (ii < 0) ii = 0;
                        if (ii >= image->width) ii = image->width - 1;
                        if (jj < 0) jj = 0;
                        if (jj >= image->height) jj = image->height - 1;
                        int data = image->data(ii, jj);

#ifdef DEBUG
                        if (x == DebugX && y == DebugY && level == DebugS) {
                            fprintf(stderr, "i,j=%d,%d data=%d\n", i + x, j + y, data);
                        }
#endif

                        tmp[ptr] = data;
                        if (mask->data(ptr++)) {
                            v0 += i * data;
                            v1 += j * data;
                            v2 += data;
                        }
                    }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++) {
                    int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }
                int map[512];
                int num = 0;
                for (int i = 0; i < 512; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 511; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;

                double averageOutput = 0;

                for (int i = 0; i < real_numNetworks; i++) {
                    if (i > 0) {
                        FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                        FastForwardUnit* unit1 = &(real_fast_list[i]->unitList[1]);
                        for (int j = 0; j < mask->size; j++)
                            (unit1++)->activation = (unit++)->activation;
                    }
                    double output = FastForwardPass(real_fast_list[i]);
                    averageOutput += output;
#ifdef DEBUG
                    if (x == DebugX && y == DebugY && level == DebugS) {
                        fprintf(stderr, "output=%g\n", output);
                        exit(1);
                    }
#endif
                }
                averageOutput /= real_numNetworks;
                if (averageOutput > 0.9996) {
                    callback(data, image, x - halfW, y - halfH, mask->width, mask->height,
                        level, scale, averageOutput, 0);
                }
            }
        }
        ReduceSize(image, image);
        //    SmoothQImage(image);
        //    ZoomQImage(image,1.2,image);
        scale *= 1.2;
        level++;
    }
    delete[] tmp;
}

// Version that doesn't search off the edges of the image
void SearchMulti2(QImage * image, int numNetworks,
    SearchCallback callback, ClientData data[])
{
    int ptr = 0;
    Mat mat = Zero(3, 3);
    QImage* mask = FindQImageWithName("facemask");
    int halfW = mask->width / 2;
    int halfH = mask->height / 2;

    for (int j = 0; j < mask->width; j++) for (int i = 0; i < mask->height; i++)
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
    mat = LUInverse(mat);

    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    int level = 0;
    while (image->height >= mask->height && image->width >= mask->width) {
        for (int y = 0; y < image->height - mask->height + 1; y++) {
            for (int x = 0; x < image->width - mask->width + 1; x++) {
                int ptr = 0;
                v0 = 0; v1 = 0; v2 = 0;
                for (int j = 0; j < mask->height; j++) {
                    for (int i = 0; i < mask->width; i++) {
                        int ii = i + x;
                        int jj = j + y;
                        //            if (ii<0) ii=0;
                        //            if (ii>=image->width) ii=image->width-1;
                        //            if (jj<0) jj=0;
                        //            if (jj>=image->height) jj=image->height-1;
                        int data = image->data(ii, jj);

#ifdef DEBUG
                        if (x == DebugX && y == DebugY && level == DebugS) {
                            fprintf(stderr, "i,j=%d,%d data=%d\n", i + x, j + y, data);
                        }
#endif

                        tmp[ptr] = data;
                        if (mask->data(ptr++)) {
                            v0 += i * data;
                            v1 += j * data;
                            v2 += data;
                        }
                    }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++) {
                    int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }
                int map[512];
                int num = 0;
                for (int i = 0; i < 512; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 511; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;

                for (int i = 0; i < real_numNetworks; i++) {
                    if (i > 0) {
                        FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                        FastForwardUnit* unit1 = &(real_fast_list[i]->unitList[1]);
                        for (int j = 0; j < mask->size; j++)
                            (unit1++)->activation = (unit++)->activation;
                    }
                    double output = FastForwardPass(real_fast_list[i]);
#ifdef DEBUG
                    if (x == DebugX && y == DebugY && level == DebugS) {
                        fprintf(stderr, "output=%g\n", output);
                        exit(1);
                    }
#endif
                    if (output > 0) {
                        callback(data[i], image, x, y, mask->width, mask->height,
                            level, scale, output, 0);
                        //            fprintf(stderr,"x=%d,y=%d:\n",x,y);
                        //            FastDebugNetwork(real_fast_list[i]);
                        //            exit(0);
                    }
                }
            }
        }
        ReduceSize(image, image);
        //    SmoothQImage(image);
        //    ZoomQImage(image,1.2,image);
        scale *= 1.2;
        level++;
    }
    delete[] tmp;
}

void SearchAstro(QImage * image, SearchCallback callback, ClientData data)
{
    int ptr = 0;
    Mat mat = Zero(3, 3);
    QImage* mask = FindQImageWithName("facemask");
    for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++)
        if (mask->data(ptr++))
        {
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
    mat = LUInverse(mat);

    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    int level = 0;
    while (image->height >= mask->height && image->width >= mask->width)
    {
        for (int y = 0; y < image->height - mask->height + 1; y++)
        {
            for (int x = 0; x < image->width - mask->width + 1; x++)
            {
                int ptr = 0;
                v0 = 0; v1 = 0; v2 = 0;
                for (int j = 0; j < mask->height; j++) {
                    for (int i = 0; i < mask->width; i++) {
                        Byte data = image->data(i + x, j + y);
                        tmp[ptr] = data;
                        if (mask->data(ptr++))
                        {
                            v0 += i * data;
                            v1 += j * data;
                            v2 += data;
                        }
                    }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++)
                {
                    int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }
                int map[512];
                int num = 0;
                for (int i = 0; i < 512; i++)
                {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 511; i >= 0; i--)
                {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / tot;
                for (int i = 0; i < mask->size; i++)
                    tmp[i] = (int)floor(0.5 + 127.0 * (map[tmp[i]] * scaleFactor - 1.0));
                double output = 0 /* Call your program here using tmp[] */;
                if (output > 0)
                    callback(data, image, x, y, mask->width, mask->height,
                        level, scale, output, 0);
            }
        }
        SmoothQImage(image);
        ZoomQImage(image, 1.2, image);
        scale *= 1.2;
        level++;
    }
    delete[] tmp;
}

// USED
// Apply multiple NNs to an image, using both lighting correction and
// histogram equalization on each window.
void SearchMultiMerge(QImage * image, int numNetworks,
    SearchCallback callback, ClientData data[])
{
    int ptr = 0;
    QImage* mask = FindQImageWithName("facemask");

    int halfW = mask->width / 2;
    int halfH = mask->height / 2;

    // Set up the matrix for lighting correction
    Mat mat = Zero(3, 3);
    for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++)
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
    mat = LUInverse(mat);

    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    int level = 0;

    // Loop over levels in image pyramid
    while (image->height >= mask->height && image->width >= mask->width) {
        for (int y = 0; y < image->height; y++) {
            for (int x = 0; x < image->width; x++) {
                int ptr = 0;
                v0 = 0; v1 = 0; v2 = 0;

                // Extract the window and begin the affine lighting fit
                for (int j = -halfH; j < halfH; j++) {
                    for (int i = -halfW; i < halfW; i++) {
                        int ii = i + x;
                        int jj = j + y;
                        if (ii < 0) ii = 0;
                        if (ii >= image->width) ii = image->width - 1;
                        if (jj < 0) jj = 0;
                        if (jj >= image->height) jj = image->height - 1;
                        int data = image->data(ii, jj);
                        tmp[ptr] = data;
                        if (mask->data(ptr++)) {
                            v0 += i * data;
                            v1 += j * data;
                            v2 += data;
                        }
                    }
                }
                // Compute affine lighting
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;

                // Apply lighting correction, and compute histogram
                for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++) {
                    int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }

                // Compute cumulative histogram
                int map[512];
                int num = 0;
                for (int i = 0; i < 512; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 511; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }

                // Copy window into network, applying histogram equalization
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;
                FastForwardPass(real_fast_list[0]);
                // Check the output from each of the networks
                for (int i = 0; i < numNetworks; i++) {
                    double output = real_fast_list[0]->
                        unitList[real_fast_list[0]->firstOutput + i].activation;
                    if (output > 0)
                        callback(data[i], image, x, y, mask->width, mask->height,
                            level, scale, output, 0);
                }
            }
        }
        //    SmoothQImage(image);
        //    ZoomQImage(image,1.2,image);
        ReduceSize(image, image);
        scale *= 1.2;
        level++;
    }
    // Clean up
    delete[] tmp;
}

void SearchMultiMergeFFT(QImage * image, int numNetworks,
    SearchCallback callback, ClientData data[])
{
    int ptr = 0;
    Mat mat = Zero(3, 3);
    QImage* mask = FindQImageWithName("facemask");
    for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++)
        if (mask->data(ptr++))
        {
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
    mat = LUInverse(mat);

    int* tmp = new int[mask->size];

    Vec vec(3);
    double v0 = 0, v1 = 0, v2 = 0;
    double scale = 1.0;
    int level = 0;
    while (image->height >= mask->height && image->width >= mask->width)
    {
        for (int y = 0; y < image->height - mask->height + 1; y++)
        {
            for (int x = 0; x < image->width - mask->width + 1; x++)
            {
                int ptr = 0;
                v0 = 0; v1 = 0; v2 = 0;
                for (int j = 0; j < mask->height; j++) {
                    for (int i = 0; i < mask->width; i++) {
                        Byte data = image->data(i + x, j + y);
                        tmp[ptr] = data;
                        if (mask->data(ptr++))
                        {
                            v0 += i * data;
                            v1 += j * data;
                            v2 += data;
                        }
                    }
                }
                vec(0) = v0; vec(1) = v1; vec(2) = v2;
                vec = mat * vec;
                v0 = vec(0); v1 = vec(1); v2 = vec(2);
                int hist[512];
                for (int i = 0; i < 512; i++) hist[i] = 0;
                ptr = 0;
                for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++)
                {
                    int val = tmp[ptr] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (mask->data(ptr)) hist[val]++;
                    tmp[ptr++] = val;
                }
                int map[512];
                int num = 0;
                for (int i = 0; i < 512; i++)
                {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 511; i >= 0; i--)
                {
                    map[i] += num;
                    num -= hist[i];
                }
                double scaleFactor = 1.0 / tot;
                ImageToFFTFast(mask->width, mask->height, tmp, map, scaleFactor,
                    real_fast_list[0]);
                FastForwardPass(real_fast_list[0]);
                for (int i = 0; i < numNetworks; i++)
                {
                    double output = real_fast_list[0]->
                        unitList[real_fast_list[0]->firstOutput + i].activation;
                    if (output > 0)
                        callback(data[i], image, x, y, mask->width, mask->height,
                            level, scale, output, 0);
                }
            }
        }
        SmoothQImage(image);
        ZoomQImage(image, 1.2, image);
        scale *= 1.2;
        level++;
    }
    delete[] tmp;
}

// USED
// Apply combination of router and individual view networks to an image.
// The router indicates which of the view networks to apply.
void SearchRouter(QImage * image, QImage * mask,
    SearchCallback callback, ClientData data[])
{
    int i;
    int ptr = 0;
    int* tmp = new int[mask->size];

    int halfW = mask->width / 2;
    int halfH = mask->height / 2;
    int level = 0;
    double scale = 1.0;
    while (image->height >= mask->height && image->width >= mask->width) {
        for (int y = 0; y < image->height; y++) {
            for (int x = 0; x < image->width; x++) {
                // Extract window from image, and compute histogram
                int ptr = 0;
                int hist[256];
                for (i = 0; i < 256; i++) hist[i] = 0;
                for (int j = -halfH; j < halfH; j++) for (int i = -halfW; i < halfW; i++) {
                    int ii = i + x;
                    int jj = j + y;
                    if (ii < 0) ii = 0;
                    if (ii >= image->width) ii = image->width - 1;
                    if (jj < 0) jj = 0;
                    if (jj >= image->height) jj = image->height - 1;
                    int data = ((int)image->data(ii, jj));
                    tmp[ptr++] = data;
                    hist[data]++;
                }
                // Compute cummulative histogram
                int map[256];
                int num = 0;
                for (i = 0; i < 256; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (i = 255; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }
                // Feed image into network, applying histogram equalization
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                for (i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;
                FastForwardPass(real_fast_list[0]);
                // Check the router outputs, invoking the view network for each
                // output that is turned on.
                for (i = 0; i < 5; i++) {
                    int outlist = 0;      // Which detection list to append to
                    if (real_fast_list[0]->unitList[real_fast_list[0]->firstOutput + i]
                        .activation > 0.0) {
                        // If router was positive, figure out which network to use
                        int net = i - 1, flip = 0;
                        if (i == 0) { net = 3; flip = 1; }
                        if (i == 1) { net = 2; flip = 1; }
                        double output = 0.0;
                        while (net < real_numNetworks) {
                            // Copy image into network, mirroring if necessary
                            FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                            FastForwardUnit* unit1 = &(real_fast_list[net]->unitList[1]);
                            if (!flip) {
                                for (int j = 0; j < mask->size; j++)
                                    (unit1++)->activation = (unit++)->activation;
                            }
                            else {
                                for (int y = 0; y < mask->height; y++) {
                                    for (int x = 0; x < mask->width; x++)
                                        (unit1++)->activation = (unit + mask->width - 1 - x)->activation;
                                    unit += mask->width;
                                }
                            }
                            output = FastForwardPass(real_fast_list[net]);
                            // If network detect face, record it
                            if (output > 0) {
                                callback(data[outlist], image, x - halfW, y - halfH,
                                    mask->width, mask->height,
                                    level, scale, output, i);
                            }
                            net += 3;     // Go to next network for this view
                            outlist++;  // Go to next output list for this view
                        }
                    }
                }
            }
        }
        ZoomQImage(image, 1.2, image);
        scale *= 1.2;
        level++;
    }
    // Clean up
    delete[] tmp;
}
